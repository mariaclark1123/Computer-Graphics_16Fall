#pragma warning(disable:4146)
#pragma warning(disable:4996)
#include <algorithm>
#include <GL/freeglut.h>
#include "texture.h"
#include "viewport.h"
#include <windows.h>
#include <cmath>

#define M_PI 3.14159265358979323846

int Rotate_axis = 0;
float rotate_angle[2] = { 0.0,0.0 };
const int INIT_SIZE = 600;
int width = INIT_SIZE;
int height = INIT_SIZE;

int mouseButton = -1;
int lastX, lastY;
double radius;
Vector3d eye, center, upVector;

/*load model*/
//parameters
int numVertex = 0;
int numNormals = 0;
int numTexcoords = 0;
int numFaces = 0;/*texture face's number is the same*/

float(*vertices)[3] = NULL;
float(*normals)[3] = NULL;

int(*faces)[3] = NULL;
int(*facenormal)[3] = NULL;
int(*textureFace)[3] = NULL;
float(*texCoords)[2] = NULL;

void Init()
{
	char line[256];//get line

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Spherical Environment Mapping");

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);

	unsigned int tex;
	//load image;
	initPNG(&tex, "grace_probe.png", width, height);

	//load model
	FILE* fp = fopen("bunny.txt", "r");

	if (fp == NULL)
	{
		printf("%s file can not open", "bunny.txt");
		exit(1);
	}
	while (!feof(fp))
	{
		fgets(line, 256, fp);
		if (line[0] == 'v')
		{
			if (line[1] == 't') {	//texture:vt ***
				numTexcoords++;
			}
			else if (line[1] == 'n') {	//normal:vn ***
				numNormals++;
			}
			else if (line[1] == ' ')   //vertex:v ***
				numVertex++;
		}
		else if (line[0] == 'f') {	//face:f ***
			numFaces++;
		}
	}
	//back to start point of file
	rewind(fp);

	printf("number of v : %d\n", numVertex);
	printf("number of vn : %d\n", numNormals);
	//	printf("number of vt : %d\n", numTexcoords);
	printf("number of f : %d\n", numFaces);

	vertices = (float(*)[3])malloc(sizeof(float) * 3 * numVertex);		//정점 
	normals = (float(*)[3])malloc(sizeof(float) * 3 * numNormals);
	texCoords = (float(*)[2])malloc(sizeof(float) * 2 * numTexcoords);	//텍스쳐 
	faces = (int(*)[3])malloc(sizeof(int) * 3 * numFaces);				//면 (정점)
																		//textureFace = (int(*)[3])malloc(sizeof(int) * 3 * numFaces);	    //면 (텍스쳐)
	facenormal = (int(*)[3])malloc(sizeof(int) * 3 * numFaces);         //면 (normal)

	int j = 0, t = 0, n = 0, k = 0;
	int IdxFace = 0;
	int IdxTexCoord = 0;

	while (!feof(fp))
	{
		fgets(line, 256, fp);
		if (line[0] == 'v')
		{
			/* vt */
			if (line[1] == 't')
			{
				fseek(fp, -(strlen(line) + 1), SEEK_CUR);		//한줄 앞으로

				float x, y, z;
				fscanf(fp, "%s %f %f %f", line, &x, &y, &z);
				texCoords[t][0] = x;	texCoords[t][1] = y;	texCoords[t][2] = z;
				t++;
			}
			else if (line[1] == 'n')
			{
				/* vn */
				fseek(fp, -(strlen(line) + 1), SEEK_CUR);		//한줄 앞으로

				float x, y, z;
				fscanf(fp, "%s %f %f %f", line, &x, &y, &z);
				normals[n][0] = x;	normals[n][1] = y;	normals[n][2] = z;
				n++;
			}
			else
			{
				/* v */
				fseek(fp, -(strlen(line) + 1), SEEK_CUR);		//한줄 앞으로

				float x, y, z;
				fscanf(fp, "%s %f %f %f", line, &x, &y, &z);
				vertices[j][0] = x;	vertices[j][1] = y;	vertices[j][2] = z;
				j++;
			}
		}
		else if (line[0] == 'f')
		{
			fseek(fp, -(strlen(line) + 1), SEEK_CUR);		    //한줄 앞으로

			int x1, x2, y1, y2, z1, z2;
			fscanf(fp, "%s %d//%d %d//%d %d//%d", line, &x1, &x2, &y1, &y2, &z1, &z2);
			//면의 정점                                                        
			faces[IdxFace][0] = x1 - 1;
			faces[IdxFace][1] = y1 - 1;
			faces[IdxFace][2] = z1 - 1;
			facenormal[IdxFace][0] = x2 - 1;
			facenormal[IdxFace][1] = y2 - 1;
			facenormal[IdxFace][2] = z2 - 1;

			IdxFace++;
		}
	}
	eye = Vector3d(0, 0, 1000);
	center = Vector3d(0, 0, 0);
	upVector = Vector3d(0, 1, 0);
}

void spinDisplay()
{
	rotate_angle[Rotate_axis] += 2.5;
	if (rotate_angle[Rotate_axis] >= 360.0)
		rotate_angle[Rotate_axis] -= 360.0;
	glutPostRedisplay();
}

void DrawModel()
{
	//define texture
	float texture[5000][2];
	float Sin[2], Cos[2];
	for (int i = 0; i < 2; i++)
	{
		Sin[i] = sin(rotate_angle[i] * M_PI / 180);
		Cos[i] = cos(rotate_angle[i] * M_PI / 180);
	}
	float rx, ry, rz;
	for (int i = 0; i < numVertex; i++)
	{

		Vector3d mapped;
		mapped.sub(center, eye);

		/* rotate normal */
		rx = (Cos[1]) * normals[i][0] + (Sin[1]) * normals[i][2];
		ry = (Sin[0] * Sin[1]) * normals[i][0] + (Cos[0]) * normals[i][1] + (-Sin[0] * Cos[1]) * normals[i][2];
		rz = (-Cos[0] * Sin[1]) * normals[i][0] + (Sin[0]) * normals[i][1] + (Cos[0] * Cos[1]) * normals[i][2];
		Vector3d n = Vector3d(rx, ry, rz);
		n.scale(-2 * n.dot(mapped));
		mapped.add(n);
		mapped.normalize();

		double sum = sqrt(mapped.x*mapped.x + mapped.y*mapped.y + pow(mapped.z + 1, 2));
		texture[i][0] = (mapped.x / sum + 1) / 2;
		texture[i][1] = (-mapped.y / sum + 1) / 2;
	}
	//Draw
	glPushMatrix();
	glScalef(5000.0, 5000.0, 5000.0);
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < numFaces - 1; i++)
	{
		glBegin(GL_TRIANGLES);
		for (int j = 0; j < 3; j++)
		{
			float p[3] = { vertices[faces[i][j]][0], vertices[faces[i][j]][1], vertices[faces[i][j]][2] };
			float t[2] = { texture[faces[i][j]][0],texture[faces[i][j]][1] };
			glTexCoord2fv(t);
			glVertex3fv(p);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void displayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width, width, -height, height, -100000, 100000);
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(rotate_angle[0], 1.0, 0.0, 0.0);
	glRotatef(rotate_angle[1], 0.0, 1.0, 0.0);
	DrawModel();
	glutSwapBuffers();
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
	else if (key == 32) //space
	{
		Rotate_axis++;
		Rotate_axis = (Rotate_axis) % 2;
		printf("%d axis\n", Rotate_axis);
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

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	Init();
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);

	glutIdleFunc(spinDisplay);
	glutMainLoop();
	return 0;
}