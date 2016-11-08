/* -*- mode: c -*- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>
#include "viewport.h"
#include "curve.h"
#include "tool.h"

/* global */
CubicBezierCurve curve;
Colorpoint movept;
GLsizei width = 800, height = 480;
Color curve_color1, curve_color2;
Colorpoint cur_pt[1025];
int edit_ctrlpts_idx = -1;
int movept_select = -1;
int selectedscene = 0;
int selected = -1;
bool isDragging = false;


Vector3d eye;
Vector3d center;
Vector3d upVector;

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

int hit_point(Point p1, int x, int y)
{
	REAL tx = p1[0] - x;
	REAL ty = p1[1] - y;
	if ((tx * tx + ty * ty) < 30)
		return 1;
	else
		return -1;
}

void init()
{
	//set color
	srand(time(0));

	curve_color1.red = 1;
	curve_color1.green = 0;
	curve_color1.blue = 0;

	curve_color2.red = 0;
	curve_color2.green = 1;
	curve_color2.blue = 0;
	
	movept.pt[0] = 200;
	movept.pt[1] = 350;
	
	//set initial curve control points
	SET_PT2(curve.control_pts[0], 150, 190);
	SET_PT2(curve.control_pts[1], 360, 300);
	SET_PT2(curve.control_pts[2], 500, 300);
	SET_PT2(curve.control_pts[3], 600, 210);
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
	//gluLookAt(0.0, 0.0, 100, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	gluLookAt(0.0, 0.0, 10, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

//Show cones on the curve
void show_curve(CubicBezierCurve curve)
{
#define Div 1025
	int i;
	REAL tran_x[Div], tran_y[Div];
	Color unit;

	unit.red = (curve_color2.red - curve_color1.red) / Div;
	unit.green = (curve_color2.green - curve_color1.green) / Div;
	unit.blue = (curve_color2.blue - curve_color1.blue) / Div;
	
	glPushMatrix();
	for (i = 0; i < Div; i++)
	{
		Point pt;
		const REAL t = (REAL)i / (REAL)(Div-1);
		evaluate(&curve, t, pt);
		
		tran_x[i] = pt[0];
		tran_y[i] = pt[1];
		glColor3f(curve_color1.red + unit.red * i, curve_color1.green + unit.green * i, curve_color1.blue + unit.blue * i);

		cur_pt[i].red = curve_color1.red + unit.red * i;
		cur_pt[i].green = curve_color1.green + unit.green * i;
		cur_pt[i].blue = curve_color1.blue + unit.blue * i;

		if (i > 0)
		{
			glTranslatef(tran_x[i] - tran_x[i - 1], tran_y[i] - tran_y[i - 1], 0.0);
		}
		else
			glTranslatef(tran_x[i], tran_y[i], 0.0);
		glutSolidCone(500, 200, 100, 1);
	}
	glPopMatrix();
}

void draw_curvemesh(CubicBezierCurve curve)
{
#define RES 100
	int i;
	glColor3f(1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= RES; ++i) {
		Point pt;
		const REAL t = (REAL)i / (REAL)RES;
		evaluate(&curve, t, pt);
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();

	/* control mesh */
	glColor3ub(255, 0, 255);
	glLineWidth(1.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i<4; ++i) 
	{
		REAL *pt = curve.control_pts[i];
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();

	/* control pts */
	glColor3ub(255, 255, 255);
	glPointSize(7.0);
	glBegin(GL_POINTS);
	for (i = 0; i<4; ++i) 
	{
		REAL *pt = curve.control_pts[i];
		glVertex3f(pt[0], pt[1], 0.0);
	}
	glEnd();
}

void display_callback(void)
{
#define YSL 0.0009765
	REAL red, green, blue;
	Point target;
	//initialization
	for (int i = 0; i < 1025; i++)
	{
		Point pt;
		const REAL t = (REAL)i / (REAL)Div;
		evaluate(&curve, t, pt);
		cur_pt[i].red = 0;
		cur_pt[i].green = 0;
		cur_pt[i].blue = 0;
		cur_pt[i].pt[0] = pt[0];
		cur_pt[i].pt[1] = pt[1];
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	//clear the color buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	show_curve(curve);
	
	GLfloat rgb[3];
	glReadPixels(movept.pt[0], movept.pt[1], 1, 1, GL_RGB, GL_FLOAT, rgb);
	printf("movept's color is (%f, %f, %f)\n", rgb[0], rgb[1], rgb[2]);
	
	for (int i = 0; i < 1025; i++)
	{
		if (((rgb[0] - cur_pt[i].red) < YSL) && ((rgb[1] - cur_pt[i].green) < YSL) && ((rgb[2] - cur_pt[i].blue) < YSL))
		{
			target[0] = cur_pt[i].pt[0];
			target[1] = cur_pt[i].pt[1];
			break;
		}
	}
	draw_plane(140);//??z?0-200îÜ????ï¸ØüêÆ?êÆá³£¬200îÜ?ý¦èÇîïö¦?ÖõßÓ?éÍï·Óð??coneîÜî¼Ý»Öõ£¬Ó£ä²èÇîïÜôò±Ô³200ãÀ??Öõ
	glDisable(GL_DEPTH_TEST);
	
	draw_point(target);
	draw_line(target, movept.pt);
	draw_point(movept.pt);
	draw_curvemesh(curve);

	printf("target is (%f,%f)\n", target[0], target[1]);
	glutSwapBuffers();
}

void mouse_callback(GLint button, GLint action, GLint x, GLint y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		switch (action) 
		{
		case GLUT_DOWN:
			edit_ctrlpts_idx = hit_index(&curve, x, height - y);
			movept_select = hit_point(movept.pt, x, height - y);
			break;
		case GLUT_UP:
			edit_ctrlpts_idx = -1;
			movept_select = -1;
			break;
		}
	}
}

void mouse_move_callback(GLint x, GLint y)
{
	if (edit_ctrlpts_idx != -1)
	{
		if (x < 800 && x > 0 && y < 480 && y > 0)
		{
			curve.control_pts[edit_ctrlpts_idx][0] = (REAL)x;
			curve.control_pts[edit_ctrlpts_idx][1] = (REAL)(height - y);
		}
	}
	if (movept_select != -1)
	{
		if (x < 800 && x > 0 && y < 480 && y > 0)
		{
			movept.pt[0] = (REAL)x;
			movept.pt[1] = (REAL)(height - y);
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
