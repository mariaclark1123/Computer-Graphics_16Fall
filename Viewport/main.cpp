#include "viewport.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <GL/glut.h>

using namespace std;

GLsizei width = 800, height = 600; //DC
REAL viewportwidth = 400, viewportheight = 300;

/*global parameters*/
int selectedscene = 0;
int selected = -1;

bool isDrawControlMesh = true;
bool isDottedLine = false;

Vector3d eye;
Vector3d center;
Vector3d upVector;

bool isDragging = false; //DM
REAL radius;
#define RES 256
#define MAX_CHAR 128

int mouseButton = -1;
int lastX = -1;
int lastY = -1;

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

void init()
{

	eye = Vector3d(750, 750, 750);
	center = Vector3d(0, 0, 0);
	upVector = Vector3d(0, 1, 0);
}

void reshape_callback(GLint nw, GLint nh)
{
	width = nw;
	height = nh;
	viewportwidth = width / 2.0f;
	viewportheight = height / 2.0f;
	radius = sqrt(viewportwidth * viewportwidth + viewportheight * viewportheight) / 2;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void display_callback()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0, 0, 0);
	//viewport
	glBegin(GL_LINES);
	glVertex3f(-1, 0, 0);
	glVertex3f(1, 0, 0);
	glEnd();
	//viewport ordinate
	glBegin(GL_LINES);
	glVertex3f(0, -1, 0);
	glVertex3f(0, 1, 0);
	glEnd();
	//Setting the points' size
	glPointSize(9.0f);

	// XY
	glViewport(0, viewportheight, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (REAL)viewportwidth, 0, (REAL)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--Object1 in XY--");

	/* --------XZ--------- */
	glViewport(0, 0, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (REAL)viewportwidth, 0, (REAL)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--Object2 in XY--");                          //Draw control points in XZ
 
  
 
	/* ---------YZ--------- */
	glViewport(viewportwidth, 0, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (REAL)viewportwidth, 0, (REAL)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--YZ--");

	/* --------3D-------- */
	glViewport(viewportwidth, viewportheight, viewportwidth, viewportheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("BicubicBezierSurface");
	glRasterPos2f(10.0f, 260.0f);
	drawString("by Fangda Chen");
	gluPerspective(25, width / (REAL)height, 0.1, 25000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);

	glEnable(GL_DEPTH_TEST);
	//Draw x,y,z coordinates
	glBegin(GL_LINES);
	glColor3f(1.0f, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(500.0f, 0, 0);
	glColor3f(0, 1.0f, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 500.0f, 0);
	glColor3f(0, 0, 1.0f);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 500.0f);
	glEnd();
	
	glDisable(GL_DEPTH_TEST);
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
			scene = 3;
			y -= (int)viewportheight;
		}
	}
	else
	{
		x -= (int)viewportwidth;
		if (y < viewportheight)
			scene = 2;
		else
		{
			scene = 4;
			y -= (int)viewportheight;
		}
	}

	if (action == GLUT_UP)
	{
		isDragging = false;
		mouseButton = -1;
	}
	//if Scene2 3D Surface has been chosen
	if (scene == 2)
	{
		if (action == GLUT_DOWN)
		{
			mouseButton = button;
			isDragging = true;
			lastX = x;
			lastY = y;
		}
	}
	//If 2D Scene has been chosen
	else
	{
		//when action button is mouse's left button
		if (button == GLUT_LEFT_BUTTON)
		{
			switch (action)
			{
			case GLUT_DOWN:
				selectedscene = scene;
				break;
			case GLUT_UP:
				selected = -1;
				break;
			default: break;
			}
		}
	}
	glutPostRedisplay();
}

// void glutMotionFunc(void (*func)(int x, int y));
void mouse_move_callback(GLint x, GLint y)
{
	//Last position
	Vector3d lastP = getMousePoint(lastX, lastY, viewportwidth, viewportheight, radius);
	//current position
	Vector3d currentP = getMousePoint(x - viewportwidth, y, viewportwidth, viewportheight, radius);

	//When action happened in left button
	//Rotate
	if (mouseButton == GLUT_LEFT_BUTTON)
	{
		Vector3d rotateVector;
		rotateVector.cross(currentP, lastP);
		REAL angle = -currentP.angle(lastP) * 2;
		rotateVector = unProjectToEye(rotateVector, eye, center, upVector);

		Vector3d dEye;
		dEye.sub(center, eye);
		dEye = rotate(dEye, rotateVector, -angle);
		upVector = rotate(upVector, rotateVector, -angle);
		eye.sub(center, dEye);
	}
	//When action happened in right button
	//Enlarge or shrink
	else if (mouseButton == GLUT_RIGHT_BUTTON)
	{
		Vector3d dEye;
		dEye.sub(center, eye);
		REAL offset = 0.025;
		if ((y - lastY) < 0)
		{
			dEye.scale(1 - offset);
		}
		else
		{
			dEye.scale(1 + offset);
		}
		eye.sub(center, dEye);
	}
	//When action happened in middle button
	//Move
	else if (mouseButton == GLUT_MIDDLE_BUTTON)
	{
		REAL dx = x - viewportwidth - lastX;
		REAL dy = y - lastY;
		if (dx != 0 || dy != 0)
		{
			Vector3d moveVector(dx, dy, 0);
			moveVector = unProjectToEye(moveVector, eye, center, upVector);
			moveVector.normalize();
			REAL eyeDistance = Vector3d(eye).distance(Vector3d(center));
			moveVector.scale(std::sqrt(dx*dx + dy*dy) / 1000 * eyeDistance);
			center.add(moveVector);
			eye.add(moveVector);
		}
	}
	//Find the lastX position and lastY position
	lastX = x - viewportwidth;
	lastY = y;
	//printf("lastX is %d lastY is %d\n", lastX, lastY);
	if (selected != -1)
	{
		int xx = 0;
		int yy = 0;
		//Judge which area is the movept
		switch (selectedscene)
		{
		case 1:
			xx = 0, yy = 1;
			break;
		case 3:
			xx = 0, yy = 2;
			y -= (int)viewportheight;
			break;
		case 4:
			xx = 1, yy = 2;
			x -= (int)viewportwidth;
			y -= (int)viewportheight;
			break;
		}
		x = max(x, 0);
		x = min(x, (int)viewportwidth);
		y = max((int)viewportheight - y, 0);
		y = min(y, (int)viewportheight);
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
	glutInitWindowPosition(500, 300);
	glutInitWindowSize(width, height);
	glutCreateWindow("Beizer Surface Editor");

	init();
	glutReshapeFunc(reshape_callback);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(mouse_move_callback);
	glutDisplayFunc(display_callback);
	glutKeyboardFunc(keyboard_callback);
	glutMainLoop();
	return 0;
}
