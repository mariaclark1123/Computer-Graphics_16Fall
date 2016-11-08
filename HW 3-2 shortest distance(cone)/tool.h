#include "curve.h"

typedef struct color
{
	double red;
	double green;
	double blue;
}Color;

typedef struct Colorpoint
{
	Point pt;
	double red;
	double green;
	double blue;
};

void draw_point(Point pt);
void draw_line(Point pt1, Point pt2);
void draw_plane(double z);
//Set word's color
void SetColor(unsigned short ForeColor, unsigned short BackGroundColor);
void drawString(const char* str);