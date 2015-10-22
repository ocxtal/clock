
/**
 * @file clock.c
 */
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include <ncurses.h>

/* drawing algorithms */
typedef struct _draw {
	void (*setpixel)(int, int, int);
} draw_t;

/**
 * @fn draw_init
 * @brief initialize drawing context
 */
draw_t *draw_init(void (*setpixel)(int x, int y, int color))
{
	draw_t *d = malloc(sizeof(draw_t));
	if(d == NULL) { return(NULL); }
	d->setpixel = setpixel;
	return(d);
}

/**
 * @fn draw_line
 * @brief draw line with the Bresenham algorithm
 */
inline int abs(int a) { return(a > 0 ? a : -a); }
#define swap(a, b) { int _t = (a); (a) = (b); (b) = _t; }
void draw_line_bresenham(draw_t const *d, int color, int sx, int sy, int ex, int ey)
{
	int dx = ex - sx, dy = ey - sy, xs, ys;
	int steep = abs(dy) >= abs(dx);

	if(steep) { swap(sx, sy); swap(ex, ey); swap(dx, dy); }
	if(dx >= 0) { xs = 1; } else { xs = -1; dx = -dx; }
	if(dy >= 0) { ys = 1; } else { ys = -1; dy = -dy; }
	int e = 2*dy - dx, x, y = sy, xd, yd, xx, yy;
	for(x = sx; x != ex; x += xs) {
		if(steep) { xx = y; yy = x; } else { xx = x; yy = y; }
		d->setpixel(color, xx, yy);
		if(e > 0) { e += 2 * (dy - dx); y += ys; } else { e += 2 * dy; }
	}
}
void draw_line(draw_t const *d, int color, int sx, int sy, int ex, int ey)
{
	int i;
	if(sx == ex) {
		if(sy > ey) { swap(sy, ey); }
		for(i = sy; i <= ey; i++) { d->setpixel(color, sx, i); }
	} else if(sy == ey) {
		if(sx > ex) { swap(sx, ex); }
		for(i = sx; i <= ex; i++) { d->setpixel(color, i, sy); }
	} else {
		draw_line_bresenham(d, color, sx, sy, ex, ey);
	}
	return;
}

/**
 * @fn draw_circle
 * @brief midpoint circle algorithm
 */
void draw_circle(draw_t const *d, int color, int cx, int cy, int r)
{
	int f = 1 - r;
	int x = 0, y = r;
	int dx = 1, dy = -2 * r;

	/* yosumi ni ten wo utsu */
	d->setpixel(color, cx + r, cy);
	d->setpixel(color, cx - r, cy);
	d->setpixel(color, cx, cy + r);
	d->setpixel(color, cx, cy - r);

	while(x < y) {
		if(f >= 0) { y--; dy += 2; f += dy; }
		x++; dx += 2; f += dx;
		d->setpixel(color, cx + x, cy + y);
		d->setpixel(color, cx - x, cy + y);
		d->setpixel(color, cx + x, cy - y);
		d->setpixel(color, cx - x, cy - y);
		d->setpixel(color, cx + y, cy + x);
		d->setpixel(color, cx - y, cy + x);
		d->setpixel(color, cx + y, cy - x);
		d->setpixel(color, cx - y, cy - x);
	}
	return;
}

/**
 * @fn draw_destroy
 */
void draw_destroy(draw_t *d)
{
	free(d);
}

/* setpixel function */
/**
 * @fn setpixel
 * @brief set a point with ncurses
 */
void setpixel(int color, int x, int y)
{
	move(y, 2*x); addstr(color == 0 ? " " : "●");
	return;
}
void draw_polar_line(draw_t const *d, int color, int cx, int cy, int sd, int ed, double rad)
{
	double ex = sin(rad);
	double ey = cos(rad);
	draw_line(d, color,
		cx + sd * ex, cy - sd * ey,
		cx + ed * ex, cy - ed * ey);
	return;
}

int main(int argc, char *argv[])
{
	double const pi = 355.0/113.0;
	int i, r, cx, cy;
	time_t t;
	struct tm *lt;

	draw_t *d = NULL;
	setlocale(LC_ALL, "");
	initscr();

	d = draw_init(setpixel);
	cx = COLS / 4;
	cy = LINES / 2;
	r = 9 * (cx < cy ? cx : cy) / 10;
	erase();
	draw_circle(d, 1, COLS/4, LINES/2, r);
	refresh();
	while(1) {
		time(&t);
		lt = localtime(&t);
		double rh = 2.0 * pi * lt->tm_hour / 12.0;
		double rm = 2.0 * pi * lt->tm_min / 60.0;
		double rs = 2.0 * pi * lt->tm_sec / 60.0;
		/* redraw */
		for(i = 0; i < 12; i++) {
			draw_polar_line(d, 1, cx, cy, 0.8 * r, 0.95 * r, 2.0 * pi * i / 12.0);
		}
		draw_polar_line(d, 1, cx, cy, -0.1 * r, 0.95 * r, rs);
		draw_polar_line(d, 1, cx, cy, -0.05 * r, 0.8 * r, rm);
		draw_polar_line(d, 1, cx, cy, -0.05 * r, 0.7 * r, rh);
		refresh();

		sleep(1);
		/* erase */
		draw_polar_line(d, 0, cx, cy, -0.1 * r, 0.95 * r, rs);
		draw_polar_line(d, 0, cx, cy, -0.05 * r, 0.8 * r, rm);
		draw_polar_line(d, 0, cx, cy, -0.05 * r, 0.7 * r, rh);
	}
	draw_destroy(d);
	return 0;
}

/**
 * end of clock.c
 */
