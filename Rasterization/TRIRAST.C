/***
	AUTHOR: LANDEN BARKER
	DATE: 03/16/2025

	DESCRIPTION: A SIMPLE TRIANGLE RASTERIZATION ALGORITHM THAT
	INCORPORATES THE BRESENHAM LINE ALGORITHM. TRIANGLES ARE GIVEN
	THREE COORDINATES AND ARE DRAWN AND COLOURED ACCORDINGLY.

	USAGE: THIS PROJECT IS LICENSED UNDER THE GNU GENERAL PUBLIC
	LICENSE AND IS OPEN FOR ANYONE TO USE WITHOUT REQUIRING THE
	ATTRIBUTION OF THE ORIGINAL AUTHOR.

	HOW TO SETUP TURBOC ON MS-DOS:
	    1. DOWNLOAD + SETUP DOSBOX
	    2. DOWNLOAD TURBOC FROM:
		'archive.org/details/msdos_borland_turbo_c_2.01'
	    3. UNZIP REPOSITORY INTO C:\TC
	    4. RUN DOSBox AND ENTER THE COMMAND:
		'MOUNT C C:\TC'
	    5. ENTER THE COMMANDS:
		'
			c:\> C:
			c:\> cd TC
			c:\> TC
		'
	HOW TO SETUP PROJECT IN TURBOC:
	    1. DOWNLOAD GITHUB PROJECT
	    2. EXTRACT FILE CONTENTS TO A DIRECTORY OF YOUR CHOOSING
	    3. COPY THE FOLDER CONTENTS INTO C:\TC

	RUNNING THE PROJECT IN TURBOC:
	    1. OPEN TURBOC ON MS-DOS
	    2. PRESS F3
	    3. IN THE MESSAGE BOX, TYPE OUT:
		'C:\TC\TRIRAST.C'
	    4. PRESS F10, HOVER OVER RUN AND PRESS ENTER
	    5. SELECT OPTION 1 IN THE RUN MENU (RUN)

	THIS SHOULD BUILD AND RUN THE PROJECT! TO ADD MORE TRIANGLES,
	JUST WRITE OUT ANOTHER FUNCTION CALL FOR ADD_TRIANGLE IN MAIN().

	ADD_TRIANGLE HAS THE PARAMETERS:
	add_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
		     unsigned char color) { ... }

	Where (x0,y0),(x1,y1),(x2,y2) are the points on the triangle and
	unsigned char color is the VGA COLOUR PALLETE.
	------------------------------------------------------
	COLOR:					INDEX:			RGB:
	BLACK					0				(0,0,0)
	GREEN					1				(0,0,170)
	GREEN					2				(0,170,0)
	CYAN					3				(0,170,170)
	RED 					4				(170,0,0)
	MAGENTA                 5               (170,0,170)
	BROWN                   6               (170,85,0)
	LIGHT GREY              7               (170,170,170)
	DARK GREY               8               (85,85,85)
	BLUE (BRIGHT)           9               (85,85,255)
	GREEN (BRIGHT)          10              (85,255,85)
	CYAN (BRIGHT)           11              (85,255,255)
	RED (BRIGHT)            12              (255,85,85)
	MAGENTA (BRIGHT)        13              (255,85,255)
	YELLOW                  14              (255,255,85)
	WHITE                   15              (255,255,255)
	------------------------------------------------------
	THESE ARE THE STANDARD 16 COLOURS SUPPORTED BY VGA IN TURBOC. YOU
	CAN USE ANY OF THESE NUMBERS (0-15) TO SPECIFY THE COLOUR IN YOUR
	CODE. FOR EXAMPLE, 10 IS GREEN (BRIGHT), 15 IS WHITE, AND 1 IS
	BLUE.

	IF YOU ARE LOOKING FOR MORE ADVANCED COLOR MANIPULATION (256-COLOR
	MODE OR CUSTOM COLOURS), YOU WOULD NEED TO WORK WITH TEH VGA
	PALETTE REGISTERS OR CHANGE THE MODE TO SOMETHING LIKE VGA 256-COLOR
	MODE (SUPPORTED BY MODE13H).
***/

#include <dos.h>
#include <conio.h>
#include <time.h>

#define VGA_MODE 0x13
#define TEXT_MODE 0x03
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define MAX_TRIANGLES 10

unsigned char far *VGA = (unsigned char far*)0xA0000000L;

typedef struct {
    int x0, y0, x1, y1, x2, y2;
    unsigned char color;
} Triangle;

Triangle triangles[MAX_TRIANGLES];
int triangle_count = 0;

void put_pixel(int x, int y, unsigned char color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        VGA[y * SCREEN_WIDTH + x] = color;
}

void set_video_mode(unsigned char mode) {
    union REGS regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    int86(0x10, &regs, &regs);
}

void clear_screen(unsigned char color) {
    int i;
    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        VGA[i] = color;
}

void draw_line(int x0, int y0, int x1, int y1, unsigned char color) {
    int dx, sx, dy, sy, err, e2;
    
    dx = abs(x1 - x0);
    sx = x0 < x1 ? 1 : -1;
    dy = -abs(y1 - y0);
    sy = y0 < y1 ? 1 : -1;
    err = dx + dy;

    while (1) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void fill_triangle(Triangle t) {
    int temp, sx, ex, i, j, totalHeight, segmentHeight, secondHalf, alpha, beta;
    int x0, y0, x1, y1, x2, y2, color;

    x0 = t.x0; y0 = t.y0;
    x1 = t.x1; y1 = t.y1;
    x2 = t.x2; y2 = t.y2;
    color = t.color;

    if (y0 > y1) { temp = y0; y0 = y1; y1 = temp; temp = x0; x0 = x1; x1 = temp; }
    if (y0 > y2) { temp = y0; y0 = y2; y2 = temp; temp = x0; x0 = x2; x2 = temp; }
    if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; temp = x1; x1 = x2; x2 = temp; }

    totalHeight = y2 - y0;

    for (i = 0; i <= totalHeight; i++) {
        segmentHeight = y1 - y0;
        secondHalf = i > segmentHeight || segmentHeight == 0;
        alpha = secondHalf ? (i - segmentHeight) * 256 / (y2 - y1) : i * 256 / segmentHeight;
        beta = i * 256 / totalHeight;

        sx = x0 + ((x2 - x0) * beta >> 8);
        ex = secondHalf ? x1 + ((x2 - x1) * alpha >> 8) : x0 + ((x1 - x0) * alpha >> 8);

        if (sx > ex) { temp = sx; sx = ex; ex = temp; }

        for (j = sx; j <= ex; j++)
            put_pixel(j, y0 + i, color);
    }
}

void add_triangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned char color) {
    if (triangle_count < MAX_TRIANGLES) {
        triangles[triangle_count].x0 = x0;
        triangles[triangle_count].y0 = y0;
        triangles[triangle_count].x1 = x1;
        triangles[triangle_count].y1 = y1;
        triangles[triangle_count].x2 = x2;
        triangles[triangle_count].y2 = y2;
        triangles[triangle_count].color = color;
        triangle_count++;
    }
}

void draw_all_triangles() {
    int i;
    for (i = 0; i < triangle_count; i++) {
        fill_triangle(triangles[i]);
        draw_line(triangles[i].x0, triangles[i].y0, triangles[i].x1, triangles[i].y1, 15);
        draw_line(triangles[i].x1, triangles[i].y1, triangles[i].x2, triangles[i].y2, 15);
        draw_line(triangles[i].x2, triangles[i].y2, triangles[i].x0, triangles[i].y0, 15);
    }
}

void draw_fps(int fps) {
    char buffer[10];
    int i, j;
    
    itoa(fps, buffer, 10);

    for (i = 0; buffer[i] != '\0'; i++) {
        for (j = 0; j < 8; j++) {
            put_pixel(5 + i * 8, 5 + j, 15);
        }
    }
}

void game_loop() {
    clock_t start, end;
    int frames = 0, fps = 0;
    
    while (!kbhit()) {
        start = clock();
        clear_screen(0);
        draw_all_triangles();
        draw_fps(fps);
        end = clock();
        frames++;
        if (end - start > CLK_TCK) {
            fps = frames;
            frames = 0;
        }
    }
}

int main() {
    set_video_mode(VGA_MODE);
    clear_screen(0);

    add_triangle(50, 50, 150, 100, 100, 150, 10);
    add_triangle(120, 30, 200, 80, 170, 160, 12);
    add_triangle(60, 120, 140, 180, 90, 190, 9);

    game_loop();

    set_video_mode(TEXT_MODE);
    return 0;
}
