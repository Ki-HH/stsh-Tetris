#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

const int status_w=9;
#define status_invisible_top	4
/* display height + 4 (max. stone height above the display, as the stones as generated above the display and then drop into the area) */
const int status_h=10+status_invisible_top;
const int brick_w=4;

/*
 * All Tetris bricks fit into a 2x4 matrix!
 * Thus, all strings need to be of 8=4x2 length.
 * https://en.wikipedia.org/w/index.php?title=Tetris&oldid=757072162#Colors_of_Tetriminos
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

struct t_status {
	uint8_t *matrix;
	uint8_t m_width, m_height;

	int8_t ms_x0, ms_y0;
	int8_t ms_type;
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

void tetris_display_matrix(struct t_status *s)
{
	int row,col;
	int b_row, b_col;
	int brick_color=0;

	for(row=0; row<s->m_height; row++)
	{
		printf("%02d|", row);
		for(col=0; col<s->m_width; col++)
		{
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
		}
		puts("|");
	}
}

void tetris_inject_new(struct t_status *s, int brick_type)
{
	int x0;

	/* the new brick should be approx horizontally centered */
	x0 = (status_w-brick_w)/2;
	s->ms_x0=x0;
	s->ms_y0=1;
	s->ms_type=brick_type;
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
				s->matrix[xy2ind(col,row)] = 1;
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
/*
 * Returns 1 when the current active ("moving") brick hits some
 * obstacle and needs to be "materialized" into the status matrix.
 */
int tetris_drop_stone(struct t_status *s)
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
	if(r==0)
		s->ms_y0++;

	return(r);
}

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

		r = tetris_drop_stone(&s);
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

		r = tetris_drop_stone(&s);
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

		r = tetris_drop_stone(&s);
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
