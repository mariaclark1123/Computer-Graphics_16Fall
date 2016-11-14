#include "curve.h"
#include "viewport.h"
#include "tool.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <GL/glut.h>

using namespace std;

CubicBezierCurve obj1[2];
CubicBezierCurve obj2[5];

GLsizei width = 1200, height = 600; //DC
REAL viewportwidth = 400, viewportheight = 300;

/*global parameters*/
int selectedscene = 0;
int obj1_edit_ctrlpts_idx = -1;
int obj2_edit_ctrlpts_idx = -1;

bool isDrawControlMesh = true;
bool isDottedLine = false;

Point obj1_pts[200];
Point obj2_pts[500];
Point obj1ctrl[6];
Point obj2ctrl[15];

#define RES 256
#define MAX_CHAR 128

int mouseButton = -1;
int lastX = -1;
int lastY = -1;

Point target;

void set_pts(Point* pt_array1, CubicBezierCurve* curve_array1, int curve_num1, Point* pt_array2, CubicBezierCurve* curve_array2, int curve_num2)
{
	int index,i;
	for (index = 0; index < curve_num1; index++)
	{
		for (i = 0; i < 100; ++i)
		{
			Point pt;
			const REAL t = (REAL)i / (REAL)99;
			evaluate(&obj1[index], t, pt);
			pt_array1[index * 100 + i][0] = pt[0];
			pt_array1[index * 100 + i][1] = pt[1];
			//printf("pt is (%f %f)\n", pt[0], pt[1]);
		}
	}
	for (index = 0; index < curve_num2; index++)
	{
		for (i = 0; i < 100; ++i)
		{
			Point pt;
			const REAL t = (REAL)i / (REAL)99;
			evaluate(&obj2[index], t, pt);
			pt_array2[index * 100 + i][0] = pt[0];
			pt_array2[index * 100 + i][1] = pt[1];
			//printf("pt is (%f %f)\n", pt[0], pt[1]);
		}
	}
}

void init()
{
	obj1ctrl[0][0] = 180;
	obj1ctrl[0][1] = 190;

	obj1ctrl[1][0] = 70;
	obj1ctrl[1][1] = 255;

	obj1ctrl[2][0] = 40;
	obj1ctrl[2][1] = 100;

	obj1ctrl[3][0] = 180;
	obj1ctrl[3][1] = 25;
	
	obj1ctrl[4][0] = 305;
	obj1ctrl[4][1] = 95;

	obj1ctrl[5][0] = 310;
	obj1ctrl[5][1] = 255;
	//OBJ2
	obj2ctrl[0][0] = 200;
	obj2ctrl[0][1] = 190;

	obj2ctrl[1][0] = 196;
	obj2ctrl[1][1] = 254;

	obj2ctrl[2][0] = 154;
	obj2ctrl[2][1] = 252;

	obj2ctrl[3][0] = 145;
	obj2ctrl[3][1] = 188;

	obj2ctrl[4][0] = 74;
	obj2ctrl[4][1] = 190;

	obj2ctrl[5][0] = 86;
	obj2ctrl[5][1] = 152;

	obj2ctrl[6][0] = 132;
	obj2ctrl[6][1] = 137;
	
	obj2ctrl[7][0] = 95;
	obj2ctrl[7][1] = 70;

	obj2ctrl[8][0] = 122;
	obj2ctrl[8][1] = 37;

	obj2ctrl[9][0] = 167;
	obj2ctrl[9][1] = 110;
	
	obj2ctrl[10][0] = 204;
	obj2ctrl[10][1] = 45;

	obj2ctrl[11][0] = 245;
	obj2ctrl[11][1] = 78;

	obj2ctrl[12][0] = 207;
	obj2ctrl[12][1] = 140;
	
	obj2ctrl[13][0] = 250;
	obj2ctrl[13][1] = 148;

	obj2ctrl[14][0] = 270;
	obj2ctrl[14][1] = 191;
}

void reshape_callback(GLint nw, GLint nh)
{
	width = nw;
	height = nh;
	viewportwidth = width / 2.0f;
	viewportheight = height / 2.0f;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void display_callback()
{
	//initial
	SETcurve_ctrlpt(obj1, 2, obj1ctrl);
	SETcurve_ctrlpt(obj2, 5, obj2ctrl); 

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(0, 0, 0);
	//Draw viewport line
	//viewport
	glBegin(GL_LINES);
	glVertex3f(-1, 0, 0);
	glVertex3f(0, 0, 0);
	glEnd();
	//viewport ordinate
	glBegin(GL_LINES);
	glVertex3f(0, -1, 0);
	glVertex3f(0, 1, 0);
	glEnd();
	
	//Set the curve pts to array
	set_pts(obj1_pts, obj1, 2, obj2_pts, obj2, 5);
	int xmax1, xmin1, ymax1, ymin1;
	int xmax2, xmin2, ymax2, ymin2;

	xmax1 = get_Xmax(obj1_pts, 200);
	xmin1 = get_Xmin(obj1_pts, 200);
	ymax1 = get_Ymax(obj1_pts, 200);
	ymin1 = get_Ymin(obj1_pts, 200);
	//printf("xmax1 is %d, xmin1 is %d, ymax is %d, ymin is %d\n", xmax1, xmin1, ymax1, ymin1);

	xmax2 = get_Xmax(obj2_pts, 500);
	xmin2 = get_Xmin(obj2_pts, 500);
	ymax2 = get_Ymax(obj2_pts, 500);
	ymin2 = get_Ymin(obj2_pts, 500);
	//printf("xmax1 is %d, xmin1 is %d, ymax is %d, ymin is %d\n", xmax1, xmin1, ymax1, ymin1);

	/******************OBJ1****************/
	glViewport(0, viewportheight, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (REAL)viewportwidth, 0, (REAL)viewportheight);
	
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--Object1 in XY--");
	for (int i = 0; i < 2; i++)
		draw_curve(obj1[i]);
	draw_boundbox(xmin1, xmax1, ymin1, ymax1);
	/*****************OBJ2****************/
	glViewport(0, 0, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (REAL)viewportwidth, 0, (REAL)viewportheight);
	
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--Object2 in XY--");                          //Draw control points in XZ
	for (int i = 0; i < 5;i++)
		draw_curve(obj2[i]);
	draw_boundbox(xmin2, xmax2, ymin2, ymax2);
 
	/* ----Minkowski Addition---- */
	glViewport(viewportwidth, 0, width, 2 * viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, 2 * (REAL)viewportwidth, 0, 2 * (REAL)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 580.0f);
	drawString("--Minkowski Addition--");
	minkowski(obj1_pts, 2, obj2_pts, 5);

	glutSwapBuffers();
}

// void glutMouseFunc(void (*func)(int button, int state, int x, int y));
void mouse_callback(GLint button, GLint action, GLint x, GLint y)
{
	int scene = 0;
	//Choose scene
	if (x < viewportwidth)
	{
		if (y < viewportheight)
			scene = 1;
		else
		{
			//has already changed the coordinate proper to scene3
			scene = 3;
			y -= (int)viewportheight;
		}
	}
	else
	{
		x -= (int)viewportwidth;
		scene = 2;
	}
	//dui
	//when action button is mouse's left button
	if (button == GLUT_LEFT_BUTTON)
	{
		switch (action)
		{
		case GLUT_DOWN:
			if (scene == 1)
			{
				int temp = viewportheight - y;
				int index; 
				index = hit_index(obj1ctrl, 6, x, temp);
				if (index != -1)
				{
					obj1_edit_ctrlpts_idx = index;
					//printf("index is %d\n", index);
				}
			}
			else if (scene == 3)
			{
				int temp = viewportheight - y;
				int index;
				index = hit_index(obj2ctrl, 15, x, temp);
				if (index != -1)
				{
					obj2_edit_ctrlpts_idx = index;
					//printf("index is %d\n", index);
				}
			}
			selectedscene = scene;
			break;
		case GLUT_UP:
			//Initialize the select signature
			obj1_edit_ctrlpts_idx = -1;
			obj2_edit_ctrlpts_idx = -1;
			break;
		default: break;
		}
	}
	glutPostRedisplay();
}

// void glutMotionFunc(void (*func)(int x, int y));
void mouse_move_callback(GLint x, GLint y)
{
	int curve_index, point_index;
	if (obj1_edit_ctrlpts_idx != -1)
	{
		if (x < viewportwidth && x > 0 && y < viewportheight && y > 0)
		{
			obj1ctrl[obj1_edit_ctrlpts_idx][0] = (REAL)x;
			obj1ctrl[obj1_edit_ctrlpts_idx][1] = (REAL)(viewportheight - y);
		}
	}
	if (obj2_edit_ctrlpts_idx != -1)
	{
		if (x < viewportwidth && x > 0 && y < height && y > viewportheight)
		{
			obj2ctrl[obj2_edit_ctrlpts_idx][0] = (REAL)x;
			obj2ctrl[obj2_edit_ctrlpts_idx][1] = (REAL)(height - y);
		}
	}
	glutPostRedisplay();
}

// void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
void keyboard_callback(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'i': case 'I':
		init();
		break;
	case 'l': case 'L':
		isDottedLine ^= true;
		break;
	case 'c': case 'C':
		isDrawControlMesh ^= true;
		break;
	case (27) : exit(0); break;
	default: break;
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(400, 300);
	glutInitWindowSize(width, height);
	glutCreateWindow("Minkoski Addition");

	init();
	glutReshapeFunc(reshape_callback);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(mouse_move_callback);
	glutDisplayFunc(display_callback);
	glutKeyboardFunc(keyboard_callback);
	glutMainLoop();

	return 0;
}
