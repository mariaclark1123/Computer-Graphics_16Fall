#include <algorithm>
#include <GL/freeglut.h>
#include "texture.h"
#include <windows.h>
#include "viewport.h"

#define M_PI 3.14159265358979323846

const int OOCP = 3;	//order of control points
const int LOD = 128;	//level of detail, the number of lines each curve is divided into
const int DIM = 3;

//ADD
static int EXP = 500;
int Rotate_axis = 0;
bool elastic = false;
bool stop_rotate = true;
float rotate_angle[2] = { 0.0,0.0 };

int combinations[(OOCP+1)/2+1];	//nCr
double bezierConstants[LOD+1][OOCP+1];	//nCr * t^r * (1-t)^(n-r)

double controlPoints[OOCP+1][OOCP+1][6][DIM];
double intermediatePoints[OOCP+1][LOD+1][6][DIM];
double surfacePoints[LOD+1][LOD+1][6][DIM];
double surfaceNormals[LOD+1][LOD+1][6][DIM];
double texCoords[LOD+1][LOD+1][6][2];

int selectedView, selectedPoint[2];
const int INIT_SIZE = 800;
int width = INIT_SIZE;
int height = INIT_SIZE;
//add surface size
static int size = 400;

int mouseButton = -1;
int lastX, lastY;
double radius;
Vector3d eye, center, upVector;

void CalculateCombinations()
{
	combinations[0] = 1;
	for (int i = 1; i <= OOCP; i++)
	{
		for (int j = i / 2; j >= 0; j--)
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

double Length(double x, double y, double z)
{
	return std::sqrt(x*x + y*y + z*z);
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
				intermediatePoints[i][j][index][axis] = 0;
				for (int l = 0; l <= OOCP; l++)
					intermediatePoints[i][j][index][axis] += bezierConstants[j][l] * controlPoints[l][i][index][axis];
			}
	
	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
			for (int axis = 0; axis < DIM; axis++)
			{
				surfacePoints[i][j][index][axis] = 0;
				for (int l = 0; l <= OOCP; l++)
					surfacePoints[i][j][index][axis] += bezierConstants[j][l] * intermediatePoints[l][i][index][axis];
			}

	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
		{
			double tangents[4][DIM], normals[4][DIM];
			
			for (int axis = 0; axis < DIM; axis++)
			{
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
				surfaceNormals[i][j][index][axis] = 0;
				for (int d = 0; d < 4; d++)
					surfaceNormals[i][j][index][axis] += normals[d][axis];
			}

			Normalize(surfaceNormals[i][j][index]);
		}
}

void InitPoints()
{
	controlPoints[0][0][0][0] = -size;
	controlPoints[0][0][0][1] = size;
	controlPoints[0][0][0][2] = size;
	controlPoints[0][3][0][0] = -size;
	controlPoints[0][3][0][1] = -size;
	controlPoints[0][3][0][2] = size;
	controlPoints[3][0][0][0] = size;
	controlPoints[3][0][0][1] = size;
	controlPoints[3][0][0][2] = size;
	controlPoints[3][3][0][0] = size;
	controlPoints[3][3][0][1] = -size;
	controlPoints[3][3][0][2] = size;

	//initial bezier surface
	for (int i = 1; i < OOCP; i++)
	{
		//diagonal
		controlPoints[i][i][0][0] = size * (2 * i / (double)OOCP - 1);
		controlPoints[i][i][0][1] = size * (1 - 2 * i / (double)OOCP);
		controlPoints[i][i][0][2] = 1.5 * size + size * (0.5 - std::abs((double)i / OOCP - 0.5) - std::abs((double)i / OOCP - 0.5)) + EXP ;

		controlPoints[i][3 - i][0][0] = size * (2 * i / (double)OOCP - 1);
		controlPoints[i][3 - i][0][1] = size * (1 - 2 * (3 - i) / (double)OOCP);
		controlPoints[i][3 - i][0][2] = 1.5 * size + size * (0.5 - std::abs((double)i / OOCP - 0.5) - std::abs((double)(3 - i) / OOCP - 0.5)) + EXP;
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
			controlPoints[i][j][1][2] = -controlPoints[i][j][0][2];
		}
	//left bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][2][0] = -controlPoints[i][j][0][2];
			controlPoints[i][j][2][1] = controlPoints[i][j][0][1];
			controlPoints[i][j][2][2] = controlPoints[i][j][0][0];
		}
	//right bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][3][0] = controlPoints[i][j][0][2];
			controlPoints[i][j][3][1] = controlPoints[i][j][0][1];
			controlPoints[i][j][3][2] = -controlPoints[i][j][0][0];
		}
	//up bezier surface
	for (int i = 0; i <= OOCP; i++)
		for (int j = 0; j <= OOCP; j++)
		{
			controlPoints[i][j][4][0] = controlPoints[i][j][0][0];
			controlPoints[i][j][4][1] = -controlPoints[i][j][0][2];
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

	for (int i = 0; i < 6;i++)
		EvaluateSurface(i);
}

void Init()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Spherical Environment Mapping");
	glEnable(GL_DEPTH_TEST);
	CalculateCombinations();
	CalculateBezierConstants();
//	InitPoints();

	unsigned int tex;
	int width, height;
	initPNG(&tex, "grace_probe.png", width, height);

	eye = Vector3d(0, 0, 1000);
	center = Vector3d(0, 0, 0);
	upVector = Vector3d(0, 1, 0);
}

void DrawSurface(int index)
{
	//rotated normal
	float rx, ry, rz;
	float Sin[2], Cos[2];
	for (int i = 0; i < 2; i++)
	{
		Sin[i] = sin(rotate_angle[i] * M_PI / 180);
		Cos[i] = cos(rotate_angle[i] * M_PI / 180);
	}

	for (int i = 0; i <= LOD; i++)
		for (int j = 0; j <= LOD; j++)
		{
			// calculating reflecting vector
			Vector3d mapped;
			mapped.sub(center, eye);

			/* rotate normal */
			rx = (Cos[1]) * surfaceNormals[i][j][index][0] + (Sin[1]) * surfaceNormals[i][j][index][2];
			ry = (Sin[0] * Sin[1]) * surfaceNormals[i][j][index][0] + (Cos[0]) * surfaceNormals[i][j][index][1] + (-Sin[0] * Cos[1]) * surfaceNormals[i][j][index][2];
			rz = (-Cos[0] * Sin[1]) * surfaceNormals[i][j][index][0] + (Sin[0]) * surfaceNormals[i][j][index][1] + (Cos[0] * Cos[1]) * surfaceNormals[i][j][index][2];
		
			Vector3d n = Vector3d(rx, ry, rz);
			n.scale(-2 * n.dot(mapped));
			mapped.add(n);
			mapped.normalize();

			// for sphere mapping
			double sum = sqrt(mapped.x*mapped.x + mapped.y*mapped.y + pow(mapped.z+1,2));
			texCoords[i][j][index][0] = (mapped.x / sum + 1) / 2;
			texCoords[i][j][index][1] = (-mapped.y / sum + 1) / 2;
		}

	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < LOD; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j <= LOD; j++)
		{
			glTexCoord2dv(texCoords[i][j][index]);
			glVertex3dv(surfacePoints[i][j][index]);
			glTexCoord2dv(texCoords[i+1][j][index]);
			glVertex3dv(surfacePoints[i+1][j][index]);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);}

void displayCallback()
{
	InitPoints();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width, width, -height, height, -100000, 100000);

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(0.8, 0.8, 0.8);
	glRotatef(rotate_angle[0], 1.0, 0.0, 0.0);
	glRotatef(rotate_angle[1], 0.0, 1.0, 0.0);

    glPushMatrix();
	DrawSurface(0);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(1);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(2);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(3);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(4);
	glPopMatrix();

	glPushMatrix();
	DrawSurface(5);
	glPopMatrix();

	glutSwapBuffers();
}

void spinDisplay()
{
	if (stop_rotate == false)
	{
		rotate_angle[Rotate_axis] += 5;
		if (rotate_angle[Rotate_axis] >= 360.0)
			rotate_angle[Rotate_axis] -= 360.0;
	}

	if (EXP == 500)
		elastic = false;
	else if (EXP == -500)
		elastic = true;
	if (elastic)
		EXP += 50;
	else
		EXP -= 50;
	glutPostRedisplay();
}

void reshapeCallback(int nw, int nh)
{
	width = nw;
	height = nh;

	radius = std::sqrt(width * width + height * height) / 4;
}

void keyboardCallback(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	if (key == 32) //space
	{
		stop_rotate = false;
		Rotate_axis++;
		Rotate_axis = (Rotate_axis) % 2;
	}
	if (key == 13)
		stop_rotate = true;
	glutPostRedisplay();
}


double ToLocalX(int x)
{
	return 4*x - width;
}

double ToLocalY(int y)
{
	return height - 4*y;
}
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	Init();
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);
	glutKeyboardFunc(keyboardCallback);
	glutIdleFunc(spinDisplay);
	glutMainLoop();
	return 0;
}