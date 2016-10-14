/* Used for printing or drawing*/
#include "curve.h"
#include "GJK.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl/freeglut.h>
#include <math.h>
#include <float.h>
#include <windows.h>

//Set word's color
void SetColor(unsigned short ForeColor, unsigned short BackGroundColor)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (ForeColor % 16) | (BackGroundColor % 16 * 16));
}

//Draw dot
void draw_dot(REAL x, REAL y)
{
	GLfloat Pi = 3.1415926536f;
	float R = 5.0;
	glBegin(GL_POLYGON);
	for (int i = 0; i<20; ++i)
		glVertex2f(x + R*cos(2 * Pi / 20 * i), y + R*sin(2 * Pi / 20 * i));
	glEnd();
}

//Caculate the distance between two points
REAL sq_dis(Point p1, Point p2)
{
	REAL distance;
	distance = (p2[1] - p1[1])*(p2[1] - p1[1]) + (p2[0] - p1[0])*(p2[0] - p1[0]);
	return distance;
}
