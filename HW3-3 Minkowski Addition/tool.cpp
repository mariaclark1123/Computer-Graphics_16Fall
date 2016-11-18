/* Used for printing or drawing*/
#include "tool.h"
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

int hit_index(Point* ctrlpts, int num, int x, int y)
{
	for (int i = 0; i < num; i++)
	{
		REAL tx = ctrlpts[i][0] - x;
		REAL ty = ctrlpts[i][1] - y;
		if ((tx * tx + ty * ty) < 30)
			return i;
	}
	return -1;
}

void SETcurve_ctrlpt(CubicBezierCurve* curve, int num, Point* ptarray)
{
	int i, j, count = 0;
	curve[num - 1].control_pts[3][0] = ptarray[0][0];
	curve[num - 1].control_pts[3][1] = ptarray[0][1];
	for (i = 0; i < num; i++)
	{
		count--;
		for (j = 0; j < 4; j++)
		{
			if (i == (num - 1) && j == 3)
				break;
			else
			{
				count++;
				curve[i].control_pts[j][0] = ptarray[count][0];
				curve[i].control_pts[j][1] = ptarray[count][1];
			}
		}
	}
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

//Draw dot
void draw_dot(REAL x, REAL y)
{
	GLdouble Pi = 3.1415926536f;
	float R = 5.0;
	glBegin(GL_POLYGON);
	for (int i = 0; i<20; ++i)
		glVertex2f(x + R*cos(2 * Pi / 20 * i), y + R*sin(2 * Pi / 20 * i));
	glEnd();
}

void draw_plane(REAL x,REAL y,REAL z)
{
	glColor3f(0.0, 0.5, 1.0);
	//Draw rows
	glBegin(GL_LINES);
	for (int i = 0; i < 9; i++)
	{
		glVertex3f(x, y - 25 * i, z);
		glVertex3f(x + 200, y - 25 * i, z);
	}
	glEnd();

	//Draw columns
	glBegin(GL_LINES);
	for (int i = 0; i < 9; i++)
	{
		glVertex3f(x + 25 * i, y, z);
		glVertex3f(x + 25 * i, y - 200, z);
	}
	glEnd();
}

//Caculate the distance between two points
REAL sq_dis(Point p1, Point p2)
{
	REAL distance;
	distance = (p2[1] - p1[1])*(p2[1] - p1[1]) + (p2[0] - p1[0])*(p2[0] - p1[0]);
	return distance;
}

REAL get_Xmax(Point* array, int num)
{
	int i;
	REAL max;
	max = array[0][0];
	for (i = 1; i < num; i++)
	{
		if (array[i][0] > max)
			max = array[i][0];
	}
	return max;
}

REAL get_Xmin(Point* array, int num)
{
	int i;
	REAL min;
	min = array[0][0];
	for (i = 1; i < num; i++)
	{
		if (array[i][0] < min)
			min = array[i][0];
	}
	return min;
}

REAL get_Ymax(Point* array, int num)
{
	int i;
	REAL max;
	max = array[0][1];
	for (i = 1; i < num; i++)
	{
		if (array[i][1] > max)
			max = array[i][1];
	}
	return max;
}

REAL get_Ymin(Point* array, int num)
{
	int i;
	REAL min;
	min = array[0][1];
	for (i = 1; i < num; i++)
	{
		if (array[i][1] < min)
			min = array[i][1];
	}
	return min;
}

void draw_boundbox(int xmin1, int xmax1, int ymin1, int ymax1)
{
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	{
		glVertex2f(xmin1, ymax1);
		glVertex2f(xmax1, ymax1);
		glVertex2f(xmax1, ymax1);
		glVertex2f(xmax1, ymin1);
		glVertex2f(xmax1, ymin1);
		glVertex2f(xmin1, ymin1);
		glVertex2f(xmin1, ymin1);
		glVertex2f(xmin1, ymax1);
	}
	glEnd();
}