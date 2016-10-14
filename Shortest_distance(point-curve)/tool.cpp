/* Used for printing or drawing*/
#include "curve.h"
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

//Draw nonation in the window
void drawString(const char* str) {
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
	GLfloat Pi = 3.1415926536f;
	REAL R = 5.0;
	glBegin(GL_POLYGON);
	for (int i = 0; i<20; ++i)
		glVertex2f(x + R*cos(2 * Pi / 20 * i), y + R*sin(2 * Pi / 20 * i));
	glEnd();
}

//When mouse touch the point
int hit_index(CubicBezierCurve *curve, int x, int y)
{
	int i;
	for (i = 0; i<4; ++i)
	{
		REAL tx = curve->control_pts[i][0] - x;
		REAL ty = curve->control_pts[i][1] - y;
		//In the point area
		if ((tx * tx + ty * ty) < 30)
		{
			//	printf("mouse location:(%d %d)\n", x, y);
			return i;
		}
	}
	return -1;
}

//When touch the move point
int hit_movept(Point movept, int x, int y)
{
	if (((x - movept[0])* (x - movept[0]) + (y - movept[1]) * (y - movept[1])) < 30)
		return 1;
	else
		return -1;
}

//Caculate the distance between two points
REAL sq_dis(Point p1, Point p2)
{
	float distance;
	distance = (p2[1] - p1[1])*(p2[1] - p1[1]) + (p2[0] - p1[0])*(p2[0] - p1[0]);
	return distance;
}

//Judge the point in which quadrant
int quadrant(Vector v)
{
	if (v[0] >= 0 && v[1] >= 0)
		return 1;
	else if (v[0] < 0 && v[1] > 0)
		return 2;
	else if (v[0] <= 0 && v[1] <= 0)
		return 3;
	else if (v[0] > 0 && v[1] < 0)
		return 4;
}

//Draw connecting line
void draw_connectline(CubicBezierCurve curve, REAL start, REAL end)
{
	Point pt1, pt0, midpt;

	evaluate(&curve, start, pt0);
	evaluate(&curve, end, pt1);

	//Midpoint between pt1 and pt2
	midpt[0] = (pt0[0] + pt1[0]) / 2.0;
	midpt[1] = (pt0[1] + pt1[1]) / 2.0;

	//Draw the connect line between pt1 and pt2
	glColor3f(0.0, 0.0, 1.0);

	glBegin(GL_LINES);
	glVertex2f(pt0[0], pt0[1]);
	glVertex2f(pt1[0], pt1[1]);
	glEnd();
}

void draw_line(Point pt1, Point pt2)
{
	glBegin(GL_LINES);
	glVertex2f(pt1[0], pt1[1]);
	glVertex2f(pt2[0], pt2[1]);
	glEnd();
}

//Caculate curvature extreme points
REAL cal_CurEx(REAL t, REAL coefficient1[2], REAL coefficient2[2], REAL coefficient3[2])
{
	REAL result_x, result_xx, result_xxx, result_y, result_yy, result_yyy;
	REAL part1, part2, result;

	//C'(t)/3 = (b1-b0) + t(2b2-4b1+2b0) + t^2(b3-3b2+3b1-b0)
	//C''(t)/6 = (b2-2b1+b0) + t(b3-3b2+3b1-b0)
	//C'''(t)/6 = (b3-3b2+3b1-b0)

	//x'(t)
	result_x = 3 * (coefficient1[0] + 2 * t*coefficient2[0] + t*t*coefficient3[0]);
	//x''(t)
	result_xx = 6 * (coefficient2[0] + t*coefficient3[0]);
	//x'''(t)
	result_xxx = 6 * coefficient3[0];
	//y'(t)
	result_y = 3 * (coefficient1[1] + 2 * t*coefficient2[1] + t*t*coefficient3[1]);
	//y''(t)
	result_yy = 6 * (coefficient2[1] + t*coefficient3[1]);
	//y'''(t)
	result_yyy = 6 * coefficient3[1];

	//(x'(t)*x'(t)+y'(t)*y'(t)*(x'(t)*y'''(t)-x'''(t)*y'(t)) - 3(x'(t)*x''(t)+y'(t)*y''(t))*(x'(t)*y''(t)-x''(t)*y'(t)))
	part1 = (result_x*result_x + result_y*result_y) * (result_x*result_yyy - result_y*result_xxx);
	part2 = 3 * (result_x*result_xx + result_y*result_yy) * (result_x*result_yy - result_y*result_xx);
	result = part1 - part2;
	return result;
}

