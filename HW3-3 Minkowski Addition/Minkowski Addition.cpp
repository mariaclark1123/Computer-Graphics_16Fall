#include <GL/freeglut.h>
#include "curve.h"

void minkowski(Point *obj1, int obj1_num, Point *obj2, int obj2_num)
{
#define RES 100
	int i, j;
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	{
		for (i = 0; i < RES * obj1_num; i++)
			for (j = 0; j < RES * obj2_num; j++)
			{
				Point pt;
				pt[0] = obj1[i][0] + obj2[j][0];
				pt[1] = obj1[i][1] + obj2[j][1];
				glVertex2f(pt[0], pt[1]);
			}
	}
	glEnd();
}