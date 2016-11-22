/*
1,2,3,4,5 카메라 제어
ESC - quit
*/
#include <iostream>
#include <GL/glut.h>
#include <cmath>

#include "MeshFactory.h"
#include "ObjMesh.h"

using namespace std;

void Display();
void Reshape (int w, int h);
void Keyboard(unsigned char key, int x, int y);
void Timer(int value);
void Idle();
void draw_grid (void);
void mouse (int button, int state, int x, int y);
void motion(int x, int y);

void Grid();

ObjMesh objMesh;
int DRAG = 0;//+

//camera
float camPos[3] = {0, 1.5, 2.5};
float cntPos[3] = {0, 0.6, 0};
#define GRIDSIZE 2.0
int GRID;
double prevy, prevx;
/* Global variables for eye position and up vector */
GLfloat eye[3];
GLfloat upv[3];


void Init()
{
	CMeshFacotry::LoadObjModel("bunny.obj", &objMesh);

	glDepthFunc(GL_LESS); // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing

    glClearColor (0.0,0.0,0.0,1.0); //clear the screen to black

	/* Initialize eye position and up vector  */
    eye[0]=0.0; eye[1]=0.0; eye[2]=10.0;
    upv[0]=0.0; upv[1]=1.0; upv[2]=0.0;
	GLfloat light_ambient[] =
    {0.0, 0.0, 0.0, 1.0};
    GLfloat light_diffuse[] =
    {1.0, 1.0, 1.0, 1.0};
    GLfloat light_specular[] =
    {1.0, 1.0, 1.0, 1.0};
/* light_position is NOT default value */
    GLfloat light_position[] =
    {1.0, 1.0, 0.0, 0.0};
    GLfloat global_ambient[] =
    {0.75, 0.75, 0.75, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

   /* glFrontFace(GL_CW);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);*/

	glEnable(GL_SMOOTH);
}


int main (int argc, char **argv) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 640);
    glutCreateWindow("Hit SPACEBAR to toggle FPS mode");

	Init();

   // glutIgnoreKeyRepeat(1);

    glutDisplayFunc(Display);
	
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

  //  glutIdleFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
   // glutIdleFunc(Idle);

    glutMainLoop();

    return 0;
}


void Display (void) 
{
	GLfloat ambient[] = {0.4, 0.4, 0.4, 1.0};//+
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer

	  /*  material has moderate ambient reflection *///+
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camPos[0], 2.5, 5.0, cntPos[0], cntPos[1], cntPos[2], 0.0,1.0,0.0);
	//-gluLookAt(camPos[0], 25, 50, 0, 0, 0, 0.0,1.0,0.0);
	//+
	gluLookAt(eye[0], eye[1], eye[2],
	      0.0, 0.0, 0.0,
              upv[0], upv[1], upv[2]);
	//-Grid();	
	glPushMatrix();
    if (GRID) draw_grid();	
    
    glPopMatrix();

	//-glPushMatrix();
	//-static float a = 0;
	//-glTranslatef(0 , 6, 0);
//	glRotatef(a+=0.05, 0,1,0);
	objMesh.Render();
	//-glPopMatrix();

    glutSwapBuffers(); //swap the buffers
}

void Reshape (int w, int h) 
{
    glViewport (0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode (GL_PROJECTION); 

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 0.1 , 100000.0);
    glMatrixMode (GL_MODELVIEW); 
}

void Keyboard(unsigned char key, int x, int y)
{
    if(key == 27) {
        exit(1);
	}
	if(key == '1'){
		camPos[0] = -5; camPos[1] = 10; camPos[2] = 15;
		cntPos[0] = -5; cntPos[1] = 5; cntPos[2] = 0;
	}
	else if(key == '2'){
		camPos[0] = 7; camPos[1] = 10; camPos[2] = 20;
		cntPos[0] = 5; cntPos[1] = 7; cntPos[2] = 0;
	}
	else if(key == '3'){
		camPos[0] = 0; camPos[1] = 15; camPos[2] = 25;
		cntPos[0] = 0; cntPos[1] = 8; cntPos[2] = 0;
	}
	else if(key == '4'){
		camPos[0] = 20; camPos[1] = 20; camPos[2] = 25;
		cntPos[0] = 0; cntPos[1] = 7; cntPos[2] = 0;
	}
	else if(key == '5'){
		camPos[0] = -20; camPos[1] = 30; camPos[2] = 5;
		cntPos[0] = 10; cntPos[1] = 5; cntPos[2] = 0;
	}
   
}

void mouse (int button, int state, int x, int y) 
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    prevx = x;
    prevy = y;
    DRAG = 1;
  }

  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    DRAG=0;
}

void Idle()
{
    Display();
}

void draw_grid (void)
{

	float ii;

	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		for (ii = -(GRIDSIZE); ii <= GRIDSIZE; ii += (GRIDSIZE / 11)) { 
			glVertex3f(-GRIDSIZE, 0.0, ii);
			glVertex3f(GRIDSIZE, 0.0, ii);
			glVertex3f(ii, 0.0, -GRIDSIZE);
			glVertex3f(ii, 0.0, GRIDSIZE);
		}

		glColor3f(0.0, 1.0, 0.0);
		for (ii = -(GRIDSIZE); ii <= GRIDSIZE; ii += (GRIDSIZE / 11)) { 
			glVertex3f(ii, GRIDSIZE, 0.0);
			glVertex3f(ii, -(GRIDSIZE), 0.0);
			glVertex3f(GRIDSIZE,ii, 0.0);
			glVertex3f(-GRIDSIZE, ii, 0.0);
		}

		glColor3f(1.0, 0.0, 1.0);
		for (ii = -(GRIDSIZE); ii <= GRIDSIZE; ii += (GRIDSIZE / 11)) { 
			glVertex3f(0.0, ii, -(GRIDSIZE));
			glVertex3f(0.0, ii, GRIDSIZE);
			glVertex3f(0.0,  -(GRIDSIZE) ,ii);
			glVertex3f(0.0, GRIDSIZE, ii);
		}

	glEnd();
}

double rot_swing[3][3] = { 1, 0, 0, 0, 1, 0, 0, 0, 1};
double rot_elev[3][3] = { 1, 0, 0, 0, 1, 0, 0, 0, 1};
double swing, elevation;

void motion(int x, int y)
{
if(DRAG==1)
  {
    double swingr, elevationr, temp[3];
    int i, j;
    double orig_eye[3] = {0, 0, 10};
    double orig_up[3] = {0, 1, 0};

    swing += (double)(x - prevx);
    elevation += (double)(y - prevy);

    /* convert swing and elevation to radian */
    swingr = swing * 3.141592 / 180.0;
    elevationr = elevation * 3.141592 / 180.0;
    
    /* building rotation matrices */
    rot_elev[1][1] = cos(elevationr);    rot_elev[1][2] = -sin(elevationr);
    rot_elev[2][1] = sin(elevationr);    rot_elev[2][2] = cos(elevationr);

    rot_swing[0][0] = cos(swingr);    rot_swing[0][2] = sin(swingr);
    rot_swing[2][0] = -sin(swingr);   rot_swing[2][2] = cos(swingr);

    
    /* Applying rotations to eye_position */
    for(i=0;i<3;i++)
    {
		temp[i] = 0;
		for(j=0;j<3;j++)
			temp[i] += rot_elev[i][j] * orig_eye[j];	
		}
    
		for(i=0;i<3;i++)
		{
			eye[i] = 0;
				for(j=0;j<3;j++)
					eye[i] += rot_swing[i][j] * temp[j];	
		}


    /* Applying rotations to up_vector */
    for(i=0;i<3;i++)
    {
		temp[i] = 0;
		for(j=0;j<3;j++)
			temp[i] += rot_elev[i][j] * orig_up[j];	
	}    

	for(i=0;i<3;i++)
	{
		upv[i] = 0;
			for(j=0;j<3;j++)
					upv[i] += rot_swing[i][j] * temp[j];	
	}
    
    glutPostRedisplay();
    prevx = x;
    prevy = y;

  }
}

void Grid()
{
	int i;
    glPushMatrix();
    glColor3f(1,1,1);
    for(i=-50; i < 50; i++) 
	{
        glBegin(GL_LINES);
		glNormal3f(0,1,0);
        glVertex3f(i, 0, -50);
        glVertex3f(i, 0, 50);
        glEnd();
    }
	
    for( i=-50; i < 50; i++) 
	{
        glBegin(GL_LINES);
		glNormal3f(0,1,0);
        glVertex3f(-50, 0, i);
        glVertex3f(50, 0, i);
        glEnd();
    }
    glPopMatrix();
}
