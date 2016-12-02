#pragma warning(disable:4146)
#pragma warning(disable:4996)
#include <algorithm>
#include <GL/freeglut.h>
#include "texture.h"
#include "viewport.h"
#include <windows.h>
#include <cmath>

typedef struct Texture
{
	float x;
	float y;
}Texture;

#define M_PI 3.14159265358979323846

int selectedView, selectedPoint[2];
const int INIT_SIZE = 800;
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

double Length(double x, double y, double z)
{
	return std::sqrt(x*x + y*y + z*z);
}

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
	//	textureFace = (int(*)[3])malloc(sizeof(int) * 3 * numFaces);			//면 (텍스쳐)
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
			//면의 정점                                                          순서 정보
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

enum {FRONT, UP, LEFT, ROTATE};

void SelectViewport(int view, bool clear)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width, width, -height, height, -100000, 100000);

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);

	glViewport(0, 0, width, height);

	if (clear)
	{
		glScissor(0, 0, width, height);
		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void DrawModel()
{
	//define texture
	float texture[5000][2];

	for (int i = 0; i < numVertex; i++)
	{
			Vector3d mapped;
			mapped.sub(center, eye);

			Vector3d n = Vector3d(normals[i][0], normals[i][1], normals[i][2]);
			//change
			if (n.dot(eye) < 0)
				n.scale(-1);
			//change
			Vector3d lx, ly = upVector, lz = eye;
			lz.normalize();
			lx.cross(ly, lz);
			
			//change
			//N is the relative position of n from z.
			Vector3d N;
			N.sub(n, lz);

			texture[i][0] = (N.dot(lx) + 1) / 2;
			texture[i][1] = (-N.dot(ly) + 1) / 2;
		//	printf("texture is (%f，%f)\n", texture[i][0], texture[i][1]);
	}
	//Draw
	glPushMatrix();
//	glColor3f(1.0, 1.0, 0.0);
	glScalef(5000.0, 5000.0, 5000.0);
	glEnable(GL_TEXTURE_2D);
	for (int i = 0; i < numFaces - 1; i++)
	{
		//printf("faces %d is vertice %d\n", i, faces[i + 0] * 3 + 0);
		
		float p1[3] = { vertices[faces[i][0]][0], vertices[faces[i][0]][1], vertices[faces[i][0]][2] };
		float p2[3] = { vertices[faces[i][1]][0], vertices[faces[i][1]][1], vertices[faces[i][1]][2] };
		float p3[3] = { vertices[faces[i][2]][0], vertices[faces[i][2]][1], vertices[faces[i][2]][2] };
		float t1[2] = { texture[faces[i][0]][0],texture[faces[i][0]][1]};
		float t2[2] = { texture[faces[i][1]][0],texture[faces[i][1]][1]};
		float t3[2] = { texture[faces[i][2]][0],texture[faces[i][2]][1]};
	
		glBegin(GL_TRIANGLES);
		glTexCoord2fv(t1);
		glVertex3fv(p1);
		glTexCoord2fv(t2);
		glVertex3fv(p2);
		glTexCoord2fv(t3);
		glVertex3fv(p3);

		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

int n = 0;

void displayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
	glEnable(GL_SCISSOR_TEST);
	//SelectViewport(view, true);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width, width, -height, height, -100000, 100000);
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);
	glViewport(0, 0, width, height);


	DrawModel();
	glDisable(GL_SCISSOR_TEST);

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
	glutPostRedisplay();
}

void mouseCallback(int button, int action, int x, int y)
{
	if (action == GLUT_DOWN)
	{

		selectedView = ROTATE;
		lastX = x;
		lastY = y;

		mouseButton = button;
	}
	else if (action == GLUT_UP)
	{
		selectedView = -1;
		mouseButton = -1;
	}
}

void motionCallback(int x, int y)
{
	if (selectedView == ROTATE)
	{
		Vector3d lastP = getMousePoint(lastX, lastY, width, height, radius);
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
			double dx = x - width / 2.0f - lastX;
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
	}
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
	glutMainLoop();
	return 0;
}