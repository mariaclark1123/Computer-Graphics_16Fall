#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glut.h>
#include "curve.h"

//Set word's color
void SetColor(unsigned short ForeColor, unsigned short BackGroundColor)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (ForeColor % 16) | (BackGroundColor % 16 * 16));
}

//Draw annotation
void drawString(const char* str)
{
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall)
	{
		isFirstCall = 0;
		lists = glGenLists(MAX_CHAR);

		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

void draw_point(Point pt)
{
	glColor3f(0.0, 0.1, 1.0);
	glPointSize(8.0);
	glBegin(GL_POINTS);
	glVertex3f(pt[0], pt[1], 0.0);
	glEnd();	
}

void draw_line(Point pt1, Point pt2)
{
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(pt1[0], pt1[1], 0.0);
	glVertex3f(pt2[0], pt2[1], 0.0);
	glEnd();
}

void draw_plane(double z)
{
    //Set plane color to white
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glVertex3f(-1000.0, -1000.0, z);
	glVertex3f(2400.0, 0.0, z);
	glVertex3f(2400.0, 1000.0, z);
	glVertex3f(0.0, 1000.0, z);
	glEnd();
}