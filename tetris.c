#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "tetris.h"

const int status_w=9;
#define status_invisible_top	4
/* display height + 4 (max. stone height above the display, as the stones as generated above the display and then drop into the area) */
const int status_h=10+status_invisible_top;
const int brick_w=4;

/*
 * All Tetris bricks fit into a 2x4 matrix!
 * Thus, all strings need to be of 8=4x2 length.
 * https://en.wikipedia.org/w/index.php?title=Tetris&oldid=757072162#Colors_of_Tetriminos
 *
 * Lit pixels are encoded by '*' and dark pixels by ' ', all
 * matrices have size 8 chars (4x2).
 */
const char __stone1[] =
	"    "
	"****";
const char __stone2[] =
	"*** "
	"  * ";
const char __stone3[] =
	"*** "
	"*   ";
const char __stone4[] =
	"**  "
	"**  ";
const char __stone5[] =
	" ** "
	"**  ";
const char __stone6[] =
	"*** "
	" *  ";
const char __stone7[] =
	"**  "
	" ** ";
const char *__stones[] =
	{__stone1, __stone2, __stone3, __stone4, __stone5, __stone6, __stone7, NULL};
const t_color __stones_c[] =
	{1, 2, 3, 4, 5, 6, 7};

struct t_status {
	/* status matrix */
	t_color *matrix;
	uint8_t m_width, m_height;

	/* information about the currently moving brick */
	int8_t ms_x0, ms_y0;
	int8_t ms_type;
	uint8_t ms_color;
};


/*
* It is more efficient to use a 1-D matrix instead of the 2-D matrix.
* This function converts the (x,y) tuple into a linear index.
*/
int xy2ind(int x, int y)
{
	return(y*status_w+x);
}
void tetris_m_set(struct t_status *s, int x, int y, int c)
{
	/* sanity checking */
	if((x>=0) && (x<=s->m_width-1) && (y>=0) && (y<=s->m_height-1))
	{
		s->matrix[xy2ind(x,y)]=c;
	}

	/* do nothing if outside of array */
}
uint8_t tetris_m_get(struct t_status *s, int x, int y)
{
	/* sanity checking */
	if((x>=0) && (x<=s->m_width-1) && (y>=0) && (y<=s->m_height-1))
	{
		return(s->matrix[xy2ind(x,y)]);
	}

	return(255);
}

/*******************************************************************/

/* internal worker function (don't use to interface from outside) */
t_color tetris_query_pixel_w(struct t_status *s, uint8_t *is_moving,
	int row, int col)
{
	int b_row, b_col;
	int moving_brick_color;

	*is_moving=0;

	/*
	 * The current brick is *not* part of the status
	 * matrix => we have to add the pixels of the
	 * current brick here while displaying.
	 * Map coordinates into the moving brick pixmap.
	 */
	b_row = row - s->ms_y0;
	b_row = -b_row;
	b_col = col - s->ms_x0;

	/* check if we're just sampling within current brick pixmap. */
	moving_brick_color=0;
	if((b_row>=0) && (b_row<2) && (b_col>=0) && (b_col<4))
	{
		moving_brick_color = __stones[s->ms_type][b_col+4*b_row];
		if(moving_brick_color=='*')
			moving_brick_color=s->ms_color;
		else
			moving_brick_color=0;
	}

	/* if we just hit a lit pixel of the moving brick ... */
	if(moving_brick_color)
	{
		*is_moving=1;
		return(moving_brick_color);
	}

	return(s->matrix[xy2ind(col,row)]);
}

/* #define __STANDALONE__ is set via command line arguments to gcc */
#ifdef __STANDALONE__
void tetris_display_matrix(struct t_status *s)
{
	int row,col;
	int b_row, b_col;
	int brick_color=0;
	uint8_t is_moving;
	t_color pixel_color;

	for(row=0; row<s->m_height; row++)
	{
		printf("%02d|", row);
		for(col=0; col<s->m_width; col++)
		{
			is_moving=0;
			pixel_color=tetris_query_pixel_w(s, &is_moving, row, col);

/* old display code */
#if 0
			/*
			 * The current brick is *not* part of the status
			 * matrix => we have to add the pixels of the
			 * current brick here while displaying.
			 * Calculate coordinates into the moving brick
			 * pixmap.
			 */
			b_row = row - s->ms_y0;
			b_row = -b_row;
			b_col = col - s->ms_x0;

			/* check if we're just sampling the current brick pixmap. */
			brick_color=0;
			if((b_row>=0) && (b_row<2) && (b_col>=0) && (b_col<4))
			{
				brick_color = __stones[s->ms_type][b_col+4*b_row];
				if(brick_color=='*')
					brick_color=1;
				else
					brick_color=0;
			}

			if(brick_color)
				putchar('X');
			else if(s->matrix[xy2ind(col,row)])
				putchar('*');
			else
				putchar(' ');
#endif
			if(is_moving)
				putchar('X');
			else if(pixel_color)
				putchar('*');
			else
				putchar(' ');
		}
		puts("|");
	}
}
#endif

void tetris_inject_new(struct t_status *s, int brick_type)
{
	int x0;

	/* the new brick should be approx horizontally centered */
	x0 = (status_w-brick_w)/2;
	s->ms_x0=x0;
	s->ms_y0=1;
	s->ms_type=brick_type;
	s->ms_color=__stones_c[brick_type];
}


void tetris_materialize_moving_stone(struct t_status *s)
{
	int b_row,b_col;
	int row, col;
	int brick_color;

	/* code stolen from tetris_collision_worker */
	for(b_row=0; b_row<2; b_row++)
	{
		for(b_col=0; b_col<4; b_col++)
		{
			row = (-b_row)+s->ms_y0;
			col = b_col + s->ms_x0;

			brick_color = __stones[s->ms_type][b_col+4*b_row];
			if(brick_color=='*')
				brick_color=1;
			else
				brick_color=0;

			if(brick_color)
				s->matrix[xy2ind(col,row)] = s->ms_color;
		}
	}

}


int tetris_collision_worker(struct t_status *s)
{
	int b_row,b_col;
	int row, col;
	int brick_color;
	int got_collision;

	got_collision=0;
	for(b_row=0; b_row<2; b_row++)
	{
		for(b_col=0; b_col<4; b_col++)
		{
			/* see this pixel of the brick is actually occupied */
			brick_color = __stones[s->ms_type][b_col+4*b_row];
			if(brick_color=='*')
				brick_color=1;
			else
				brick_color=0;

			/*
			 * The current brick is *not* part of the status
			 * matrix => need to calculate coordinates of
			 * the occupied brick pixels in the status matrix.
			 */
			row = (-b_row)+s->ms_y0;
			col = b_col + s->ms_x0;
			if(s->matrix[xy2ind(col,row)] && brick_color)
				got_collision=1;
		}
	}

	if(got_collision)
	{
		return(1);
	}
	return(0);

}
int tetris_can_drop(struct t_status *s)
{
	struct t_status s_tmp;
	int r;

	memcpy(&s_tmp, s, sizeof(struct t_status));

	/* try to move the brick one lower */
	if(s_tmp.ms_y0>=13)
		return(1);

	s_tmp.ms_y0++;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_collision_worker(&s_tmp);

	return(r);
}
int tetris_can_left(struct t_status *s)
{
	struct t_status s_tmp;
	int r;

	memcpy(&s_tmp, s, sizeof(struct t_status));

	/* try to move the brick one to the left */
	if(s_tmp.ms_x0<=0)
		return(1);	/* cannot move to left */

	s_tmp.ms_x0--;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_collision_worker(&s_tmp);

	return(r);
}
int tetris_can_right(struct t_status *s)
{
	struct t_status s_tmp;
	int r;

	memcpy(&s_tmp, s, sizeof(struct t_status));

	/* try to move the brick one to the right */
	if(s_tmp.ms_x0>=status_w-4)
		return(1);	/* cannot move to the right */

	s_tmp.ms_x0++;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_collision_worker(&s_tmp);

	return(r);
}

/*
 * These functions return 1 when the current active ("moving")
 * brick hits some obstacle and needs to be "materialized"
 * into the status matrix.
 */
int tetris_stone_drop(struct t_status *s)
{
	int r;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_can_drop(s);
	if(r==0)
		s->ms_y0++;

	return(r);
}
int tetris_stone_l(struct t_status *s)
{
	int r;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_can_left(s);
	if(r==0)
		s->ms_x0--;

	return(r);
}
int tetris_stone_r(struct t_status *s)
{
	int r;

	/* see if there is a collision when moving one step down,
	   update coordinates when no collision. */
	r=tetris_can_right(s);
	if(r==0)
		s->ms_x0++;

	return(r);
}




/* #define __STANDALONE__ is set via command line arguments to gcc */
#ifdef __STANDALONE__
int main(void)
{
	uint8_t status_matrix[status_w*status_h];
	struct t_status s;
	int j;

	memset(&s,0,sizeof(struct t_status));
	memset(status_matrix,0,status_w*status_h);
	s.matrix=status_matrix;
	s.m_width=status_w;
	s.m_height=status_h;

	puts("====");
	tetris_inject_new(&s,1);
	puts("====");
	tetris_display_matrix(&s);
	puts("====");
	for(j=0; j<15; j++)
	{
		int r;

		r = tetris_stone_r(&s); // tetris_stone_drop(&s);
		printf("\n\n===== %d (status: %d)\n", j, r);
		tetris_display_matrix(&s);
		if(r)
		{
			tetris_materialize_moving_stone(&s);
			break;
		}
	}

	puts("XXXX");
	s.ms_type=4;
	s.ms_y0=7;
	s.ms_x0--;
	tetris_display_matrix(&s);

	puts("XXXXXXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXXXXXX");
	for(j=0; j<15; j++)
	{
		int r;

		r = tetris_stone_drop(&s);
		printf("\n\n===== %d (status: %d)\n", j, r);
		tetris_display_matrix(&s);
		if(r)
		{
			tetris_materialize_moving_stone(&s);
			break;
		}
	}

	puts("XXXX");
	s.ms_y0=7;
	s.ms_x0--;
	tetris_display_matrix(&s);

	puts("XXXXXXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXXXXXX");
	for(j=0; j<15; j++)
	{
		int r;

		r = tetris_stone_drop(&s);
		printf("\n\n===== %d (status: %d)\n", j, r);
		tetris_display_matrix(&s);
		if(r)
		{
			tetris_materialize_moving_stone(&s);
			break;
		}
	}


	return(0);
}
#endif
