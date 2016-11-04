/* -*- mode: c -*- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>
#include "curve.h"

typedef struct color 
{
	int red;
	int green;
	int blue;
}Color;

/* global */
CubicBezierCurve curve[7];
GLsizei width = 640, height = 480;
Color curve_color[7];
int edit_ctrlpts_idx[7] = { -1 ,-1, -1, -1, -1, -1, -1};

int hit_index(CubicBezierCurve *curve, int x, int y)
{
	int i;
	for (i = 0; i<4; ++i) 
	{
		REAL tx = curve->control_pts[i][0] - x;
		REAL ty = curve->control_pts[i][1] - y;
		if ((tx * tx + ty * ty) < 30)
			return i;
	}
	return -1;
}

void init()
{
	//set color
	srand(time(NULL));
	for (int i = 0; i < 7; i++)
	{
		curve_color[i].red = rand() % 256;
		curve_color[i].green = rand() % 256;
		curve_color[i].blue = rand() % 256;

		if (i > 0)
		{
			for (int j = 0; j < i; j++)
				if (fabs(curve_color[i].red - curve_color[j].red) < 30 && fabs(curve_color[i].green - curve_color[j].green) < 30
					&& fabs(curve_color[i].blue - curve_color[j].blue) < 30)
				{
					curve_color[i].red = rand() % 256;
					curve_color[i].green = rand() % 256;
					curve_color[i].blue = rand() % 256;
				}
		}
	}

	//set initial curve control points
	SET_PT2(curve[0].control_pts[0], 25, 30);
	SET_PT2(curve[0].control_pts[1], 50, 100);
	SET_PT2(curve[0].control_pts[2], 200, 100);
	SET_PT2(curve[0].control_pts[3], 150, 40);

	SET_PT2(curve[1].control_pts[0], 25, 90);
	SET_PT2(curve[1].control_pts[1], 50, 160);
	SET_PT2(curve[1].control_pts[2], 200, 170);
	SET_PT2(curve[1].control_pts[3], 190, 118);

	SET_PT2(curve[2].control_pts[0], 330, 190);
	SET_PT2(curve[2].control_pts[1], 360, 230);
	SET_PT2(curve[2].control_pts[2], 480, 270);
	SET_PT2(curve[2].control_pts[3], 455, 210);

	SET_PT2(curve[3].control_pts[0], 35, 190);
	SET_PT2(curve[3].control_pts[1], 60, 300);
	SET_PT2(curve[3].control_pts[2], 20, 300);
	SET_PT2(curve[3].control_pts[3], 200, 210);

	SET_PT2(curve[4].control_pts[0], 350, 380);
	SET_PT2(curve[4].control_pts[1], 370, 330);
	SET_PT2(curve[4].control_pts[2], 580, 370);
	SET_PT2(curve[4].control_pts[3], 520, 200);

	SET_PT2(curve[5].control_pts[0], 260, 40);
	SET_PT2(curve[5].control_pts[1], 420, 60);
	SET_PT2(curve[5].control_pts[2], 480, 100);
	SET_PT2(curve[5].control_pts[3], 490, 50);

	SET_PT2(curve[6].control_pts[0], 150, 300);
	SET_PT2(curve[6].control_pts[1], 180, 400);
	SET_PT2(curve[6].control_pts[2], 290, 200);
	SET_PT2(curve[6].control_pts[3], 500, 400);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, width, 0, height,-1000, 1000);
}

void reshape_callback(GLint nw, GLint nh)
{
	width = nw;
	height = nh;
	glViewport(0, 0, width, height);                                                                                                                 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
    glOrtho(0, width, 0, height, -1000, 1000);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 100, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void draw_curvecone(CubicBezierCurve curve)
{
#define Div 16
	for (int i = 0; i < 16; i++)
	{
		    Point pt;
			const REAL t = (REAL)i / (REAL)Div;
			evaluate(&curve, t, pt);
			glVertex3f(pt[0], pt[1], 0.0);
	}
}

void show_curve(CubicBezierCurve curve)
{

#define Div 256
	int i;
	REAL tran_x[Div], tran_y[Div];
	
	//glColor3ub(red, green, blue);

	glPushMatrix();
	for (i = 0; i < Div; i++)
	{
		Point pt;
		const REAL t = (REAL)i / (REAL)Div;
		evaluate(&curve, t, pt);
		
		tran_x[i] = pt[0];
		tran_y[i] = pt[1];

		if (i > 0)
		{
			glTranslatef(tran_x[i] - tran_x[i - 1], tran_y[i] - tran_y[i - 1], 0.0);
		}
		else
			glTranslatef(tran_x[i], tran_y[i], 0.0);
		glutSolidCone(1900, 200, 100, 1);
	}
	glPopMatrix();
}

void draw_curvemesh(CubicBezierCurve curve)
{
#define RES 100
	int i;
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= RES; ++i) {
		Point pt;
		const REAL t = (REAL)i / (REAL)RES;
		evaluate(&curve, t, pt);
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();

	/* control mesh */
	glColor3ub(255, 255, 255);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i<4; ++i) 
	{
		REAL *pt = curve.control_pts[i];
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();

	/* control pts */
	glColor3ub(0, 0, 255);
	glPointSize(7.0);
	glBegin(GL_POINTS);
	for (i = 0; i<4; ++i) {
		REAL *pt = curve.control_pts[i];
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();
}

void display_callback(void)
{
	REAL red, green, blue;

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//clear the color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	for (int i = 0; i < 7; i++)
	{
		glColor3ub(curve_color[i].red, curve_color[i].green, curve_color[i].blue);
		show_curve(curve[i]);
	}
	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i < 7; i++)
		draw_curvemesh(curve[i]);
	glutSwapBuffers();
}

void mouse_callback(GLint button, GLint action, GLint x, GLint y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		for (int i = 0; i < 7; i++)
		switch (action) {
			{
		case GLUT_DOWN:
			edit_ctrlpts_idx[i] = hit_index(&curve[i], x, height - y);
			break;
		case GLUT_UP:
			edit_ctrlpts_idx[i] = -1;
			break;
			}
		}
	}
}

void mouse_move_callback(GLint x, GLint y)
{
	for (int i = 0; i < 7; i++)
	{
		if (edit_ctrlpts_idx[i] != -1) {
			curve[i].control_pts[edit_ctrlpts_idx[i]][0] = (REAL)x;
			curve[i].control_pts[edit_ctrlpts_idx[i]][1] = (REAL)(height - y);
		}
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	//Open depth buffer
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Bezier Editor");

	init();
	glutReshapeFunc(reshape_callback);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(mouse_move_callback);
	glutDisplayFunc(display_callback);
	glutMainLoop();
	return 0;
}
