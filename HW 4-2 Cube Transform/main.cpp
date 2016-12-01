#include <algorithm>
#include <GL/freeglut.h>
#include "texture.h"
#include <windows.h>
#include "viewport.h"
#include "tool.h"

#define M_PI 3.14159265358979323846

int EXP = 500;
const int OOCP = 3;	//order of control points
const int LOD = 128;	//level of detail, the number of lines each curve is divided into
const int DIM = 3;
static GLfloat theta[] = { 0.0,0.0,0.0 };
static GLint axis = 2;
int combinations[(OOCP+1)/2+1];	//nCr
double bezierConstants[LOD+1][OOCP+1];	//nCr * t^r * (1-t)^(n-r)
double controlPoints[OOCP+1][OOCP+1][6][DIM]; //surface control points add[6]
double intermediatePoints[OOCP+1][LOD+1][6][DIM]; //intermediatePoints add[6]
double surfacePoints[LOD+1][LOD+1][6][DIM]; //surface add[6]
double surfaceNormals[LOD+1][LOD+1][6][DIM]; //surfacenormal add[6]
double texCoords[LOD+1][LOD+1][6][2]; //surface texturecoord add[6]

const int INIT_SIZE = 800;
int width = INIT_SIZE;
int height = INIT_SIZE;
int size = INIT_SIZE / 2;
bool elastic = false; //false means going down

int mouseButton = -1;
int lastX, lastY;
double radius;
Vector3d eye, center, upVector;

GLfloat vertices[][3] = { { -size,-size,-size },{ size,-size,-size },
{ size,size,-size },{ -size,size,-size },{ -size,-size,size },
{ size,-size,size },{ size,size,size },{ -size,size,size } };

void DrawPolygon(int a, int b, int c, int d)
{
	glBegin(GL_POLYGON);
	glVertex3fv(vertices[a]);
	glVertex3fv(vertices[b]);
	glVertex3fv(vertices[c]);
	glVertex3fv(vertices[d]);
	glEnd();
}

void DrawCube()
{
	DrawPolygon(0, 3, 2, 1);
	DrawPolygon(2, 3, 7, 6);
	DrawPolygon(0, 4, 7, 3);
	DrawPolygon(1, 2, 6, 5);
	DrawPolygon(4, 5, 6, 7);
	DrawPolygon(0, 1, 5, 4);
}

void CalculateCombinations()
{
	combinations[0] = 1;
	for (int i = 1; i <= OOCP; i++)
	{
		for (int j = i / 2; j > 0; j--)//
			combinations[j] += combinations[j-1];
		if (i % 2 == 1)
			combinations[i/2+1] = combinations[i/2];
	}
}

void CalculateBezierConstants()
{
	for (int i = 0; i <= LOD; i++)
	{
		double t = (double) i / LOD;
		double it = 1-t;

		for (int j = 0; j <= OOCP / 2; j++)
		{
			bezierConstants[i][j] = combinations[j] * pow(t, j) * pow(it, OOCP - j);
			bezierConstants[i][OOCP-j] = combinations[j] * pow(t, OOCP - j) * pow(it, j);
		}
	}
}

void Normalize(double* v)
{
	double l = Length(v[0], v[1], v[2]);
	if (l > 0)
		for (int i = 0; i < DIM; i++)
			v[i] /= l;
}

void EvaluateSurface(int index)
{
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= LOD; j++)
			for (int axis = 0; axis < DIM; axis++)
			{
				//add index
				intermediatePoints[i][j][index][axis] = 0;
				for (int l = 0; l <= OOCP; l++)
					intermediatePoints[i][j][index][axis] += bezierConstants[j][l] * controlPoints[l][i][index][axis];
			}
	
	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
			for (int axis = 0; axis < DIM; axis++)
			{
				//surface add[6]
				surfacePoints[i][j][index][axis] = 0;
				for (int l = 0; l <= OOCP; l++)
					surfacePoints[i][j][index][axis] += bezierConstants[j][l] * intermediatePoints[l][i][index][axis];//surface add[6]
			}

	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
		{
			double tangents[4][DIM], normals[4][DIM];
			
			for (int axis = 0; axis < DIM; axis++)
			{
				//surface add [6]
				tangents[0][axis] = j > 0? surfacePoints[i][j-1][index][axis] - surfacePoints[i][j][index][axis] : 0;
				tangents[1][axis] = i < LOD? surfacePoints[i+1][j][index][axis] - surfacePoints[i][j][index][axis] : 0;
				tangents[2][axis] = j < LOD? surfacePoints[i][j+1][index][axis] - surfacePoints[i][j][index][axis] : 0;
				tangents[3][axis] = i > 0? surfacePoints[i-1][j][index][axis] - surfacePoints[i][j][index][axis] : 0;
			}

			for (int d = 0; d < 4; d++)
			{
				int c = (d + 1) % 4;
				normals[d][0] = tangents[c][1]*tangents[d][2] - tangents[c][2]*tangents[d][1];
				normals[d][1] = tangents[c][2]*tangents[d][0] - tangents[c][0]*tangents[d][2];
				normals[d][2] = tangents[c][0]*tangents[d][1] - tangents[c][1]*tangents[d][0];
				Normalize(normals[d]);
			}

			for (int axis = 0; axis < DIM; axis++)
			{
				//surface normal add[6]
				surfaceNormals[i][j][index][axis] = 0;
				for (int d = 0; d < 4; d++)
					surfaceNormals[i][j][index][axis] += normals[d][axis]; //surface normal add[6]
			}
			Normalize(surfaceNormals[i][j][index]);
		}
}

void InitPoints()
{
	controlPoints[0][0][0][0] = - size;
	controlPoints[0][0][0][1] = size;
	controlPoints[0][0][0][2] = size;
	controlPoints[0][3][0][0] = - size;
	controlPoints[0][3][0][1] = - size;
	controlPoints[0][3][0][2] = size;
	controlPoints[3][0][0][0] = size;
	controlPoints[3][0][0][1] = size;
	controlPoints[3][0][0][2] = size;
	controlPoints[3][3][0][0] = size;
	controlPoints[3][3][0][1] = - size;
	controlPoints[3][3][0][2] = size;

	//initial bezier surface
	for (int i = 1; i < OOCP; i++)
		{
			//diagonal
			controlPoints[i][i][0][0] = size * (2 * i / (double)OOCP - 1);
			controlPoints[i][i][0][1] = size * (1 - 2 * i / (double)OOCP);
			controlPoints[i][i][0][2] = 1.5 * size + size * (0.5 - std::abs((double)i / OOCP - 0.5) - std::abs((double)i / OOCP - 0.5)) + EXP;

			controlPoints[i][3-i][0][0] = size * (2 * i / (double)OOCP - 1);
			controlPoints[i][3-i][0][1] = size * (1 - 2 * (3 - i) / (double)OOCP);
			controlPoints[i][3-i][0][2] = 1.5 * size + size * (0.5 - std::abs((double)i / OOCP - 0.5) - std::abs((double)(3 - i) / OOCP - 0.5)) + EXP;
	}
	//vertical
	for (int j = 1; j < OOCP; j++)
	{
		controlPoints[0][j][0][0] = controlPoints[0][0][0][0];
		controlPoints[0][j][0][1] = controlPoints[0][0][0][1] - 266.666 * j;
		controlPoints[0][j][0][2] = controlPoints[0][0][0][2];

		controlPoints[OOCP][j][0][0] = controlPoints[OOCP][0][0][0];
		controlPoints[OOCP][j][0][1] = controlPoints[OOCP][0][0][1] - 266.666 * j;
		controlPoints[OOCP][j][0][2] = controlPoints[OOCP][0][0][2];
	}
	//horizontal
	for (int i = 1; i < OOCP; i++)
	{
		controlPoints[i][0][0][0] = controlPoints[0][0][0][0] + 266.666 * i;
		controlPoints[i][0][0][1] = controlPoints[0][0][0][1];
		controlPoints[i][0][0][2] = controlPoints[0][0][0][2];

		controlPoints[i][OOCP][0][0] = controlPoints[0][OOCP][0][0] + 266.666 * i;
		controlPoints[i][OOCP][0][1] = controlPoints[0][OOCP][0][1];
		controlPoints[i][OOCP][0][2] = controlPoints[0][OOCP][0][2];
	}
	
	//back bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][1][0] = controlPoints[i][j][0][0];
			controlPoints[i][j][1][1] = controlPoints[i][j][0][1];
			controlPoints[i][j][1][2] = - controlPoints[i][j][0][2];
		}
	//left bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][2][0] = - controlPoints[i][j][0][2];
			controlPoints[i][j][2][1] = controlPoints[i][j][0][1];
			controlPoints[i][j][2][2] = controlPoints[i][j][0][0];
		}
	//right bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][3][0] = controlPoints[i][j][0][2];
			controlPoints[i][j][3][1] = controlPoints[i][j][0][1];
			controlPoints[i][j][3][2] = - controlPoints[i][j][0][0];
		}
	//up bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][4][0] = controlPoints[i][j][0][0];
			controlPoints[i][j][4][1] = - controlPoints[i][j][0][2];
			controlPoints[i][j][4][2] = controlPoints[i][j][0][1];
		}
	//down bezier curve
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][5][0] = controlPoints[i][j][0][0];
			controlPoints[i][j][5][1] = controlPoints[i][j][0][2];
			controlPoints[i][j][5][2] = controlPoints[i][j][0][1];
		}
}

void Init()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Spherical Environment Mapping");

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	CalculateCombinations();
	CalculateBezierConstants();
	InitPoints();

	unsigned int tex;
	int width, height;
	initPNG(&tex, "grace_probe.png", width, height);


	eye = Vector3d(0, 0, 1000);
	center = Vector3d(0, 0, 0);
	upVector = Vector3d(0, 1, 0);
}

void SelectViewport(bool clear)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width, width, -height, height, -100000, 100000);

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);
	glViewport(0, 0, width, height);

	if (clear)
	{
		glScissor(0, 0, width, height);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void DrawSurface(int index)
{
	EvaluateSurface(index);
	/*****normal translate*****/
	float Sin[3], Cos[3];
	for (int i = 0; i < 3; i++)
	{
		Sin[i] = sin(theta[i] * M_PI / 180);
		Cos[i] = cos(theta[i] * M_PI / 180);
	}
	float rx, ry, rz;

	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
		{
			// calculating reflecting vector
			Vector3d mapped;
			mapped.sub(center, eye);
			//surface normal add [6]
			rx = Cos[1] * Cos[2] * surfaceNormals[i][j][index][0] + Cos[1] * Sin[2] * surfaceNormals[i][j][index][1] + Sin[1] * surfaceNormals[i][j][index][2];
			ry = (Sin[0] * Sin[1] * Cos[2] - Cos[0] * Sin[2])* surfaceNormals[i][j][index][0] + (Sin[0] * Sin[1] * Sin[2] + Cos[0] * Cos[2]) * surfaceNormals[i][j][index][1] - Sin[0] * Cos[1] * surfaceNormals[i][j][index][2];
			rz = (-Cos[0] * Sin[1] * Cos[2] - Sin[0] * Cos[2])* surfaceNormals[i][j][index][0] + (-Cos[0] * Sin[1] * Sin[2] + Sin[0] * Cos[2]) * surfaceNormals[i][j][index][1] + Cos[0] * Cos[1] * surfaceNormals[i][j][index][2];

			Vector3d n = Vector3d(rx, ry, rz);
			n.scale(-2 * n.dot(mapped));
			mapped.add(n);
			mapped.normalize();

			// for sphere mapping
			double sum = sqrt(mapped.x*mapped.x + mapped.y*mapped.y + pow(mapped.z + 1, 2));
			texCoords[i][j][index][0] = (mapped.x / sum + 1) / 2;
			texCoords[i][j][index][1] = (-mapped.y / sum + 1) / 2;
		}
	/************/
	
	//print surface control points
	/*for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
				int j = 0;
			printf("control point[%d][%d][0] is (%f,%f,%f)\n",i,j, controlPoints[i][j][0][0], controlPoints[i][j][0][1], controlPoints[i][j][0][2]);
		}
	glEnd();*/

	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < LOD; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= LOD; j++)
		{
			glTexCoord2dv(texCoords[i][j][index]);
			//surface add[6]
			glVertex3dv(surfacePoints[i][j][index]);
			glTexCoord2dv(texCoords[i+1][j][index]);
			//surface add[6]
			glVertex3dv(surfacePoints[i+1][j][index]);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);}

int n = 0;

void spinDisplay()
{
	theta[axis] += 5.0;
	if (theta[axis] > 360.0) theta[axis] -= 360.0;
	/* display(); */
	
	if (EXP == 500)
		elastic = false;
	else if (EXP == -500)
		elastic = true;
	if (elastic)
		EXP += 20;
	else
		EXP -= 20;
	glutPostRedisplay();
}

void reshapeCallback(int nw, int nh)
{
	width = nw;
	height = nh;

	radius = std::sqrt(width * width + height * height) / 4;
}

void displayCallback()
{
	InitPoints();
	glEnable(GL_SCISSOR_TEST);
	SelectViewport(true);
	glRotatef(theta[0], 1.0, 0.0, 0.0);
	glRotatef(theta[1], 0.0, 1.0, 0.0);
	glRotatef(theta[2], 0.0, 0.0, 1.0);

	glPushMatrix();

	glColor3f(1.0, 0.0, 0.0);
	glPointSize(10.0);
	glBegin(GL_POINTS);
	for (int index = 0; index < 6; index++)
		for (int i = 0; i <= OOCP; i++)
			for (int j = 0; j <= OOCP; j++)
				glVertex3f(controlPoints[i][j][index][0], controlPoints[i][j][index][1], controlPoints[i][j][index][2]);
	glEnd();

	/*glColor3f(1.0, 0.9, 0.9);
	DrawCube();*/
	glPopMatrix();

	//up and down
	glPushMatrix();
	DrawSurface(5);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(4);
	glPopMatrix();

	//left and right
	glPushMatrix();
	DrawSurface(3);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(2);
	glPopMatrix();

	//Front and back
	glPushMatrix();
	DrawSurface(1);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(0);
	glPopMatrix();

	glDisable(GL_SCISSOR_TEST);

	glutSwapBuffers();
}

void keyboardCallback(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	else if (key == 32) //space
	{
		axis++;
		axis = (axis) % 3;
		printf("%d axis\n", axis);
	}
	glutPostRedisplay();
}

void mouseCallback(int button, int action, int x, int y)
{
	if (action == GLUT_DOWN)
	{
		lastX = x;
		lastY = y;

		mouseButton = button;
	}
	else if (action == GLUT_UP)
		mouseButton = -1;
}

void motionCallback(int x, int y)
{
		Vector3d lastP = getMousePoint(lastX, lastY, width , height , radius);
		Vector3d currentP = getMousePoint(x, y, width, height, radius);

		if (mouseButton == GLUT_LEFT_BUTTON)
		{
			Vector3d rotateVector;
			rotateVector.cross(currentP, lastP);
			double angle = -currentP.angle(lastP) * 2;
			rotateVector = unProjectToEye(rotateVector, eye, center, upVector);

			Vector3d dEye;
			dEye.sub(center, eye);
			dEye = rotate(dEye, rotateVector, -angle);
			upVector = rotate(upVector, rotateVector, -angle);
			eye.sub(center, dEye);
		}
		else if (mouseButton == GLUT_RIGHT_BUTTON) {
			Vector3d dEye;
			dEye.sub(center, eye);
			double offset = 0.025;
			if ((y - lastY) < 0) {
				dEye.scale(1 - offset);
			}
			else {
				dEye.scale(1 + offset);
			}
			eye.sub(center, dEye);
		}
		else if (mouseButton == GLUT_MIDDLE_BUTTON) {
			double dx = x - lastX;
			double dy = y - lastY;
			if (dx != 0 || dy != 0)
			{
				Vector3d moveVector(dx, dy, 0);
				moveVector = unProjectToEye(moveVector, eye, center, upVector);
				moveVector.normalize();
				double eyeDistance = Vector3d(eye).distance(Vector3d(center));
				moveVector.scale(std::sqrt(dx*dx + dy*dy) / 1000 * eyeDistance);
				center.add(moveVector);
				eye.add(moveVector);
			}
		}
		lastX = x;
		lastY = y;
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	Init();
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	glutIdleFunc(spinDisplay);
	glutMainLoop();
	return 0;
}