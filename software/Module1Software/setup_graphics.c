/*
 * 	main.c - contains the following functions:
 * 	show_angle		 	: displays angle line and number entered by user
 * 	clear_last_line		: colours the last drawn shape back to the background colour
 * 	draw_power_bar		: draws empty power bar for specified player
 * 	update_power		: displays velocity level in power bar for specified player
 * 	draw_box			: draws a box using the pixel_drawer with specified coordinates and colour
 * 	draw_ball			: draws a diamond (low-res circle) at specified position and colour
 *
 *  Created on: 2014-10-01
 *  Author: Adam
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Math.h>
#include "io.h"
#include "system.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#define drawer_base (volatile int *) 0x6000 //must be base address of pixel_drawer avalon_slave
#define BACKGROUND 0x00000

unsigned int background_colour = 0x00000;

//struct to store coordinates of drawn last line
typedef struct Shape_coord{
	int x_1, x_2, y_1, y_2;
}Shape_coord;

//draws a line at the input angle from the horizontal
void show_angle(int player, double angle, alt_up_pixel_buffer_dma_dev* pixel_buffer, Shape_coord *last_line, alt_up_char_buffer_dev * char_buffer);
//clears the last line drawn
void clear_last_line(Shape_coord *last_line, alt_up_pixel_buffer_dma_dev* pixel_buffer);
//draws empty power bar for specified player on the screen
void draw_power_bar(int player);
//update power bar based on input velocity
void update_power(int player, double velocity);
//draws a box of input colour between x_1, y_2 and x_2, y_2 using the pixel_drawer
void draw_box(int x_1, int y_1, int x_2, int y_2, unsigned int colour);
//draws a ball [diamond] of input colour with radius 2px about centre point (x,y)
void draw_ball(int x, int y, unsigned int colour);
int main_show(void);
int main_show(void){
	//setup pixel buffer
	alt_up_pixel_buffer_dma_dev* pixel_buffer;
	pixel_buffer = alt_up_pixel_buffer_dma_open_dev("/dev/video_pixel_buffer_dma_0");
	if (pixel_buffer == 0) {
		printf("error initializing pixel buffer (check name in alt_up_pixel_buffer_dma_open_dev)\n");
	}
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer, SRAM_0_BASE);
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
	srand(time(NULL));

	//setup char buffer
	alt_up_char_buffer_dev * char_buffer;
	char_buffer = alt_up_char_buffer_open_dev("/dev/video_character_buffer_with_dma_0");
	alt_up_char_buffer_init(char_buffer); //initialize char buffer
	alt_up_char_buffer_clear(char_buffer); //clear char buffer to erase previous angle text

	//boundaries
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 0, 0, 320, 1,rand() % 0x10000, 0); //x1, y1, x2, y2
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 0, 0, 1, 240,rand() % 0x10000, 0); //x1, y1, x2, y2
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 0, 240, 320, 239,rand() % 0x10000, 0); //x1, y1, x2, y2
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 320, 240, 319, 0,rand() % 0x10000, 0); //x1, y1, x2, y2

	//player1 rectangle
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 2, 238, 20, 210,rand() % 0x10000, 0); //25x50 rectangle
	//player2 rectangle
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 318, 238, 300, 210,rand() % 0x10000, 0); //25x50 rectangle

	Shape_coord last_line;
	last_line.x_1 = 0;
	last_line.x_2 = 0;
	last_line.y_1 = 0;
	last_line.y_2 = 0;

	//draw empty power bars
	draw_power_bar(1);
	draw_power_bar(2);

	draw_ball(100, 100, 0xFF000);

	//some code to test functionality
	while(1){
		int player = 1;
		double angle;
		printf("Enter an angle:");
		scanf("%lf", &angle);
		printf("Angle is %lf \n", angle);
		clear_last_line(&last_line, pixel_buffer);
		alt_up_char_buffer_clear(char_buffer); //clear char buffer to erase previous angle text
		alt_up_char_buffer_init(char_buffer); //initialize again
		show_angle(player, angle, pixel_buffer, &last_line, char_buffer);
		double velocity;
		while(1){
			printf("Enter an velocity 0-100:");
			scanf("%lf", &velocity);
			if(velocity == -1) break;
			printf("Velocity is %lf \n", velocity);
			update_power(player, velocity);
		}
	}

	return 0;
}

void show_angle(int player, double angle, alt_up_pixel_buffer_dma_dev* pixel_buffer, Shape_coord *last_line, alt_up_char_buffer_dev * char_buffer){
	double theta = angle*M_PI/180;
	//TODO: convert input angle number into string...
	int x_1, x_2;
	int y_1 = 195;
	int y_2 = y_1-15*sin(theta);
	int line_length = 50;

	if(player == 1){
		x_1 = 40;
		x_2 = x_1 + line_length*cos(theta);
		//alt_up_char_buffer_string(char_buffer, "ANGLE 1", 10, 53);	// Show degrees
	}
	else{
		x_1 = 320 - 40;
		x_2 = x_1 - line_length*cos(theta);
		//alt_up_char_buffer_string(char_buffer, "ANGLE 2", 60, 53);	// Show degrees
	}

	alt_up_pixel_buffer_dma_draw_line(pixel_buffer, x_1, y_1, x_2, y_2, 0xFFFFF, 0); //draw line
	//store endpoints of line for later
	last_line->x_1 = x_1;
	last_line->x_2 = x_2;
	last_line->y_1 = y_1;
	last_line->y_2 = y_2;

	return;
}

void clear_last_line(Shape_coord *last_line, alt_up_pixel_buffer_dma_dev* pixel_buffer){
	alt_up_pixel_buffer_dma_draw_line(pixel_buffer, last_line->x_1, last_line->y_1, last_line->x_2, last_line->y_2, background_colour, 0);
}

void draw_power_bar(int player){
	int x_1, y_1, bar_width, bar_height;
	x_1 = 30; y_1 = 230; bar_width = 52; bar_height=5;
	if(player == 1){
		draw_box(x_1, y_1, x_1+bar_width, y_1+bar_height, 0xFFFFF);
		draw_box(x_1+1, y_1+1, x_1+bar_width-1, y_1+bar_height-1, 0x00000);
	}
	else{//player2 power bar
		draw_box(320-x_1, y_1, 320-(x_1+bar_width), y_1+bar_height, 0xFFFFF);
		draw_box(320-(x_1+1), y_1+1, 320-(x_1+bar_width-1), y_1+bar_height-1, 0x00000);
	}

}

//assumes velocity range is from 0-100
void update_power(int player, double velocity){
	draw_power_bar(player); //clear previous level
	if(player==1){
		draw_box(31, 231, 31+(velocity/2), 234, 0xFF000);
	}
	else{
		draw_box(320-31, 231, 320-(31+(velocity/2)), 234, 0xFF000);
	}
	return;
}

void draw_box(int x_1, int y_1, int x_2, int y_2, unsigned int colour){
	IOWR_32DIRECT(drawer_base,0,x_1);
	IOWR_32DIRECT(drawer_base,4,y_1);
	IOWR_32DIRECT(drawer_base,8,x_2);
	IOWR_32DIRECT(drawer_base,12,y_2);
	IOWR_32DIRECT(drawer_base,16,colour);
	IOWR_32DIRECT(drawer_base,20,1);
	while(IORD_32DIRECT(drawer_base,20)==0);
}

void draw_ball(int x, int y, unsigned int colour){
	draw_box(x-2, y, x+2, y, colour);
	draw_box(x, y-2, x, y+2, colour);
	draw_box(x-1, y-1, x+1, y+1, colour);
}