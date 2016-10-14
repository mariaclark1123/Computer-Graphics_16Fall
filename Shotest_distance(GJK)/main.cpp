#include "curve.h"
#include "viewport.h"
#include "GJK.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <GL/glut.h>

using namespace std;

BicubicBezierSurface surface;
GLsizei width = 800, height = 600;
double viewportwidth = 400, viewportheight = 300;

/*global parameters*/
int selectedscene = 0;
int selected = -1;
int movept_selected = -1;

bool isDrawControlMesh = true;
bool isDottedLine = false;

const int INSIDE = 0; // 000000
const int LEFT = 1;   // 000001
const int RIGHT = 2;  // 000010
const int BOTTOM = 4; // 000100
const int TOP = 8;    // 001000
const int BACK = 16;  // 010000
const int FRONT = 32; // 100000

Vector3d eye;
Vector3d center;
Vector3d upVector;

bool isDragging = false;
double radius;

#define RES 32
#define MAX_CHAR 128

REAL points[RES + 1][RES + 1][3];         //3 dimentional points' group of surface

int mouseButton = -1;
int lastX = -1;
int lastY = -1;

Point target;
REAL movept[3];

//square1 array to store all data 
square1 array1[1365];
priority_queue<square1, vector<square1>, NodeCmp> pri_tetra;

//9:49
//Search the shortest distance's point and draw the line
void draw_shortdistance(square1* tetra, Point movept)
{
	Object_structure obj1, obj2;

	REAL wpt1[3] = { 0, 0, 0 }, wpt2[3] = { 0, 0, 0 };
	REAL sq_dist;

	//Tetra
	REAL array1[4][3] = { { points[tetra->index1[0]][tetra->index1[1]][0], points[tetra->index1[0]][tetra->index1[1]][1], points[tetra->index1[0]][tetra->index1[1]][2] }
		,{ points[tetra->index2[0]][tetra->index2[1]][0], points[tetra->index2[0]][tetra->index2[1]][1], points[tetra->index2[0]][tetra->index2[1]][2] }
		,{ points[tetra->index3[0]][tetra->index3[1]][0], points[tetra->index3[0]][tetra->index3[1]][1], points[tetra->index3[0]][tetra->index3[1]][2] }
	    ,{ points[tetra->index4[0]][tetra->index4[1]][0], points[tetra->index4[0]][tetra->index4[1]][1], points[tetra->index4[0]][tetra->index4[1]][2] } };
	//Point
	REAL array2[1][3] = { movept[0], movept[1], movept[2] };

	int tetrahedron1[20] = { 4, 8, 12, 16, 1, 2, 3, -1, 0, 2, 3, -1, 0, 1, 3, -1, 0, 1, 2, -1 };
	int point2[3] = { 1, 0, -1 };

	obj1.numpoints = 4;
	obj1.vertices = array1;
	obj1.rings = tetrahedron1;

	obj2.numpoints = 1;
	obj2.vertices = array2;
	obj2.rings = point2;

	REAL tr1[3][4] = { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 } };
	REAL tr2[3][4] = { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 } };

	sq_dist = gjk_distance(&obj1, tr1, &obj2, tr2, wpt1, wpt2, NULL, 0);
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex3f(wpt1[0], wpt1[1], wpt1[2]);
	glEnd();
	//Draw shortest distance line
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(wpt1[0], wpt1[1], wpt1[2]);
	glVertex3f(wpt2[0], wpt2[1], wpt2[2]);
	glEnd();

}
//Caculate the shortest distance between points and tetra
void set_tetra(square1* tetra, Point movept, int index)
{
	Object_structure obj1, obj2;

	REAL wpt1[3] = { 0, 0, 0 }, wpt2[3] = { 0, 0, 0 };
	REAL sq_dist;

	//Tetra
	REAL array1[4][3] = { { points[tetra->index1[0]][tetra->index1[1]][0], points[tetra->index1[0]][tetra->index1[1]][1], points[tetra->index1[0]][tetra->index1[1]][2] }
		,{ points[tetra->index2[0]][tetra->index2[1]][0], points[tetra->index2[0]][tetra->index2[1]][1], points[tetra->index2[0]][tetra->index2[1]][2] }
	,{ points[tetra->index3[0]][tetra->index3[1]][0], points[tetra->index3[0]][tetra->index3[1]][1], points[tetra->index3[0]][tetra->index3[1]][2] }
	,{ points[tetra->index4[0]][tetra->index4[1]][0], points[tetra->index4[0]][tetra->index4[1]][1], points[tetra->index4[0]][tetra->index4[1]][2] } };
	//Point
	REAL array2[1][3] = { movept[0], movept[1], movept[2] };

	int tetrahedron1[20] = { 4, 8, 12, 16, 1, 2, 3, -1, 0, 2, 3, -1, 0, 1, 3, -1, 0, 1, 2, -1 };
	int point2[3] = { 1, 0, -1 };

	obj1.numpoints = 4;
	obj1.vertices = array1;
	obj1.rings = tetrahedron1;

	obj2.numpoints = 1;
	obj2.vertices = array2;
	obj2.rings = point2;

	REAL tr1[3][4] = { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 } };
	REAL tr2[3][4] = { { 1, 0, 0, 0 },{ 0, 1, 0, 0 },{ 0, 0, 1, 0 } };
	
	sq_dist = gjk_distance(&obj1, tr1, &obj2, tr2, NULL, NULL, NULL, 0);
	//printf("sq_dist is %f\n", sq_dist);
	//Set shortest distance
	tetra->short_length = sq_dist;
	tetra->array_index = index;
}

//max/70
int flat1(square1 s)
{
	double restriction = 0, distance = 0, distance1 = 0, distance2 = 0, temp = 0, x = 0, y = 0, z = 0;

	//temp=(pt1.x-pt3.x)^2+(pt1.y-pt3.y)^2+(pt1.z-pt3.z)^2
	temp = (points[s.index1[0]][s.index1[1]][0] - points[s.index3[0]][s.index3[1]][0])*(points[s.index1[0]][s.index1[1]][0] - points[s.index3[0]][s.index3[1]][0])
		+ (points[s.index1[0]][s.index1[1]][1] - points[s.index3[0]][s.index3[1]][1])*(points[s.index1[0]][s.index1[1]][1] - points[s.index3[0]][s.index3[1]][1])
		+ (points[s.index1[0]][s.index1[1]][2] - points[s.index3[0]][s.index3[1]][2])*(points[s.index1[0]][s.index1[1]][2] - points[s.index3[0]][s.index3[1]][2]);

	//The distance between pt1 and pt3
	distance1 = pow(temp, 1.0 / 3);

	//temp=(pt2.x-pt4.x)^2+(pt2.y-pt4.y)^2+(pt2.z-pt4.z)^2
	temp = (points[s.index2[0]][s.index2[1]][0] - points[s.index4[0]][s.index4[1]][0])*(points[s.index2[0]][s.index2[1]][0] - points[s.index4[0]][s.index4[1]][0])
		+ (points[s.index2[0]][s.index2[1]][1] - points[s.index4[0]][s.index4[1]][1])*(points[s.index2[0]][s.index2[1]][1] - points[s.index4[0]][s.index4[1]][1])
		+ (points[s.index2[0]][s.index2[1]][2] - points[s.index4[0]][s.index4[1]][2])*(points[s.index2[0]][s.index2[1]][2] - points[s.index4[0]][s.index4[1]][2]);
	//The distance between pt2 and pt4
	distance2 = pow(temp, 1.0 / 3);
	//The restriction
	restriction = max(distance1, distance2);
	restriction = restriction / 60.0;

	//The center of points1,2,3,4
	x = (points[s.index1[0]][s.index1[1]][0] + points[s.index2[0]][s.index2[1]][0] + points[s.index3[0]][s.index3[1]][0] + points[s.index4[0]][s.index4[1]][0]) / 4.0;
	y = (points[s.index1[0]][s.index1[1]][1] + points[s.index2[0]][s.index2[1]][1] + points[s.index3[0]][s.index3[1]][1] + points[s.index4[0]][s.index4[1]][1]) / 4.0;
	z = (points[s.index1[0]][s.index1[1]][2] + points[s.index2[0]][s.index2[1]][2] + points[s.index3[0]][s.index3[1]][2] + points[s.index4[0]][s.index4[1]][2]) / 4.0;
	temp = (points[s.index_center[0]][s.index_center[1]][0] - x)*(points[s.index_center[0]][s.index_center[1]][0] - x)
		+ (points[s.index_center[0]][s.index_center[1]][1] - y)*(points[s.index_center[0]][s.index_center[1]][1] - y)
		+ (points[s.index_center[0]][s.index_center[1]][2] - z)*(points[s.index_center[0]][s.index_center[1]][2] - z);
	//The distance between s.center and the square's center
	distance = pow(temp, 1.0 / 3);

	if (distance > restriction)
		return 0;
	else
		return 1;
}

//Draw square
void draw_square1(square1 sq)
{
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	{
		glVertex3f(points[sq.index1[0]][sq.index1[1]][0], points[sq.index1[0]][sq.index1[1]][1], points[sq.index1[0]][sq.index1[1]][2]);
		glVertex3f(points[sq.index2[0]][sq.index2[1]][0], points[sq.index2[0]][sq.index2[1]][1], points[sq.index2[0]][sq.index2[1]][2]);
		glVertex3f(points[sq.index3[0]][sq.index3[1]][0], points[sq.index3[0]][sq.index3[1]][1], points[sq.index3[0]][sq.index3[1]][2]);
		glVertex3f(points[sq.index4[0]][sq.index4[1]][0], points[sq.index4[0]][sq.index4[1]][1], points[sq.index4[0]][sq.index4[1]][2]);
		glVertex3f(points[sq.index1[0]][sq.index1[1]][0], points[sq.index1[0]][sq.index1[1]][1], points[sq.index1[0]][sq.index1[1]][2]);
	}
	glEnd();
}

//Show 1D array
void show1()
{
	int i, j, k, start = 0, temp;
	//Caculate the first square's flat
	array1[0].flat = flat1(array1[0]);
	for (j = 0; j < 5; j++)
	{
		temp = pow(4, j);
		//printf("start is %d end is %d\n", start, start + temp);
		for (i = start; i < start + temp; i++)
		{
			if (array1[0].flat == 1)
				draw_square1(array1[0]);
			else
			{
				k = 4 * i;
				if (array1[i].flat == 0)
				{
					if (array1[k + 1].flat == 1)
						draw_square1(array1[k + 1]);
					if (array1[k + 2].flat == 1)
						draw_square1(array1[k + 2]);
					if (array1[k + 3].flat == 1)
						draw_square1(array1[k + 3]);
					if (array1[k + 4].flat == 1)
						draw_square1(array1[k + 4]);

				}
			}
		}
		start += temp;
	}
}

//Draw all bounding box
void draw_1darray()
{
	int i, j, start = 0;

	for (int le = 0; le < 5; le++)
	{
		//Start position
		int result = pow(4, le);
		//One level to next level
		for (i = start; i < start + result; i++)
		{
			//if(array1[i].flat==0)
			//12:36
			//18:39
			//array1[i].array_index = i;
			{
				j = 4 * i;
				//First divided part
				array1[j + 1].index1[0] = array1[i].index1[0];                                //Divided square's index1 is big square index1
				array1[j + 1].index1[1] = array1[i].index1[1];

				array1[j + 1].index2[0] = array1[i].index1[0];
				array1[j + 1].index2[1] = array1[i].index_center[1];

				array1[j + 1].index3[0] = array1[i].index_center[0];
				array1[j + 1].index3[1] = array1[i].index_center[1];

				array1[j + 1].index4[0] = array1[i].index_center[0];
				array1[j + 1].index4[1] = array1[i].index1[1];

				if (le < 4)
				{
					array1[j + 1].index_center[0] = (array1[j + 1].index1[0] + array1[j + 1].index3[0]) / 2;
					array1[j + 1].index_center[1] = (array1[j + 1].index1[1] + array1[j + 1].index3[1]) / 2;
					array1[j + 1].flat = flat1(array1[j + 1]);
				}
				//added
				if (le == 4)
				{
					array1[j + 1].flat = 1;
					array1[j + 1].index_center[0] = 0;
				}

				//Second divided part
				array1[j + 2].index1[0] = array1[i].index2[0];                                //Divided square's index2 is big square index2
				array1[j + 2].index1[1] = array1[i].index2[1];

				array1[j + 2].index2[0] = array1[i].index2[0];
				array1[j + 2].index2[1] = array1[i].index_center[1];

				array1[j + 2].index3[0] = array1[i].index_center[0];
				array1[j + 2].index3[1] = array1[i].index_center[1];

				array1[j + 2].index4[0] = array1[i].index_center[0];
				array1[j + 2].index4[1] = array1[i].index2[1];

				if (le < 4)
				{
					array1[j + 2].index_center[0] = (array1[j + 2].index1[0] + array1[j + 2].index3[0]) / 2;
					array1[j + 2].index_center[1] = (array1[j + 2].index1[1] + array1[j + 2].index3[1]) / 2;
					array1[j + 2].flat = flat1(array1[j + 2]);
				}

				//added
				if (le == 4)
				{
					array1[j + 2].flat = 1;
					array1[j + 1].index_center[0] = 0;

				}

				//Third divided part
				array1[j + 3].index1[0] = array1[i].index3[0];                                //Divided square's index3 is big square index3
				array1[j + 3].index1[1] = array1[i].index3[1];

				array1[j + 3].index2[0] = array1[i].index3[0];
				array1[j + 3].index2[1] = array1[i].index_center[1];

				array1[j + 3].index3[0] = array1[i].index_center[0];
				array1[j + 3].index3[1] = array1[i].index_center[1];

				array1[j + 3].index4[0] = array1[i].index_center[0];
				array1[j + 3].index4[1] = array1[i].index3[1];

				if (le < 4)
				{
					array1[j + 3].index_center[0] = (array1[j + 3].index1[0] + array1[j + 3].index3[0]) / 2;
					array1[j + 3].index_center[1] = (array1[j + 3].index1[1] + array1[j + 3].index3[1]) / 2;
					array1[j + 3].flat = flat1(array1[j + 3]);
				}

				//added
				if (le == 4)
				{
					array1[j + 3].flat = 1;
					array1[j + 1].index_center[0] = 0;
				}

				//Fourth divided part
				array1[j + 4].index1[0] = array1[i].index4[0];                                //Divided square's index4 is big square index4
				array1[j + 4].index1[1] = array1[i].index4[1];

				array1[j + 4].index2[0] = array1[i].index4[0];
				array1[j + 4].index2[1] = array1[i].index_center[1];

				array1[j + 4].index3[0] = array1[i].index_center[0];
				array1[j + 4].index3[1] = array1[i].index_center[1];

				array1[j + 4].index4[0] = array1[i].index_center[0];
				array1[j + 4].index4[1] = array1[i].index4[1];

				if (le < 4)
				{
					array1[j + 4].index_center[0] = (array1[j + 4].index1[0] + array1[j + 4].index3[0]) / 2;
					array1[j + 4].index_center[1] = (array1[j + 4].index1[1] + array1[j + 4].index3[1]) / 2;
					array1[j + 4].flat = flat1(array1[j + 4]);
				}

				//added
				if (le == 4)
				{
					array1[j + 4].flat = 1;
					array1[j + 1].index_center[0] = 0;
				}
			}
		}
		start += result;
	}
	//show1();
}

void RayTest(int mouse_x, int mouse_y)
{
	double x = mouse_x;
	double y = height - mouse_y;

	double model[16], proj[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	int viewport[4] = { 0.0f, 0.0f, width, height };
	double ax, ay, az, bx, by, bz;
	gluUnProject(mouse_x, mouse_y, 0.0, model, proj, viewport, &ax, &ay, &az);
	gluUnProject(mouse_x, mouse_y, 1.0, model, proj, viewport, &bx, &by, &bz);

	double cx, cy, cz;
	cx = ax - bx;
	cy = ay - by;
	cz = az - bz;

	target[0] = ax - cx * ay / cy;
	target[1] = az - cz * ay / cy;
}

//Surface hit_index code
int hit_index(int x, int y, int scene)
{
	int xx, yy;
	switch (scene)
	{
	case 1:
		xx = 0, yy = 1;                                  //xy
		break;
	case 3:
		xx = 0, yy = 2;                                  //yz
		break;
	case 4:
		xx = 1, yy = 2;                                  //xz
		break;
	}
	int min = 30;
	int minp = -1;
	for (int i = 0; i < 4; i++)                           //Control the hit_index points' column
	{
		for (int j = 0; j < 4; j++)                       //Control the hit_index points' row
		{
			double tx = surface.control_pts[i][j][xx] - x;
			double ty = surface.control_pts[i][j][yy] - y;
			//A circle's domain:30
			if ((tx * tx + ty * ty) < min)
			{
				min = (tx * tx + ty * ty);
				minp = i * 10 + j;
			}
		}
	}
	//printf("minp is %d\n", minp);
	return minp;
}
//Line hit_index code, return the hit index
int hit_index_line(int x, int y, int scene)
{
	int xx, yy, sign = -1;
	switch (scene)
	{
	case 1:
		xx = 0, yy = 1;                                  //xy
		break;
	case 3:
		xx = 0, yy = 2;                                  //yz
		break;
	case 4:
		xx = 1, yy = 2;                                  //xz
		break;
	}
	int min = 30;
	int minp = -1;
	//Caculate the line's points is in the domain or not
	for (int i = 0; i < 2; i++)
	{
		double tx = movept[xx] - x;
		double ty = movept[yy] - y;
		if ((tx * tx + ty * ty) < min)
			sign = i;
	}
	return sign;
}

//Caculate the surface points
void calc_surface()
{
	for (int i = 0; i <= RES; i++)
		for (int j = 0; j <= RES; j++)
		{
			evaluate(&surface, i / (float)RES, j / (float)RES, points[i][j]);
		}
}

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
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			SET_PT3(surface.control_pts[i][j], 50 * i + 50, 20 * i - 75 * (i == 2) + 200 - j * 50, j * 50 + 50);

	calc_surface();                                          //Draw surface

	eye = Vector3d(750, 750, 750);
	center = Vector3d(0, 0, 0);
	upVector = Vector3d(0, 1, 0);

	//Points of the start point and end point of line_1 initialization
	movept[0] = 200;
	movept[1] = 100;
	movept[2] = 120;

	//Square array[0][0] initialization
	array1[0].index1[0] = 0;
	array1[0].index1[1] = 0;

	array1[0].index2[0] = 0;
	array1[0].index2[1] = 32;

	array1[0].index3[0] = 32;
	array1[0].index3[1] = 32;

	array1[0].index4[0] = 32;
	array1[0].index4[1] = 0;

	array1[0].index_center[0] = 16;
	array1[0].index_center[1] = 16;
}

void reshape_callback(GLint nw, GLint nh)
{
	width = nw;
	height = nh;
	viewportwidth = width / 2.0f;
	viewportheight = height / 2.0f;
	radius = std::sqrt(viewportwidth * viewportwidth + viewportheight * viewportheight) / 2;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

////12:34
void search_sd()
{
	REAL distance;
	square1 top;
	set_tetra(&array1[0], movept,0);
	
	pri_tetra.push(array1[0]);
	top = pri_tetra.top();
	while (top.array_index < 341)
	{
		printf("while\n");
		pri_tetra.pop();

		set_tetra(&array1[4 * top.array_index + 1], movept, 4 * top.array_index + 1);
		pri_tetra.push(array1[4 * top.array_index + 1]);
		if (array1[4 * top.array_index + 1].short_length == top.short_length)
			break;

		set_tetra(&array1[4 * top.array_index + 2], movept, 4 * top.array_index + 2);
		pri_tetra.push(array1[4 * top.array_index + 2]);
		if (array1[4 * top.array_index + 2].short_length == top.short_length)
			break;

		set_tetra(&array1[4 * top.array_index + 3], movept, 4 * top.array_index + 3);
		pri_tetra.push(array1[4 * top.array_index + 3]);
		if (array1[4 * top.array_index + 3].short_length == top.short_length)
			break;

		set_tetra(&array1[4 * top.array_index + 4], movept, 4 * top.array_index + 4);
		pri_tetra.push(array1[4 * top.array_index + 4]);
		if (array1[4 * top.array_index + 4].short_length == top.short_length)
			break;

    	top = pri_tetra.top();
		printf("top is %d\n", top.array_index);
	}
	printf("The shortest distance tetra's index is %d\n", top.array_index);
	draw_shortdistance(&top, movept);
}

//12:34
//void search_sd()
//{
//	REAL distance;
//	square1 top;
//	set_tetra(&array1[0], movept, 0);
//
//	pri_tetra.push(array1[0]);
//	top = pri_tetra.top();
//	while (top.array_index + 4 < 85)
//	{
//		printf("while\n");
//		top = pri_tetra.top();
//		printf("you poped %d out\n", top.array_index);
//		//Pop the top
//		pri_tetra.pop();
//
//		set_tetra(&array1[4 * top.array_index + 1], movept, 4 * top.array_index + 1);
//		if (array1[4 * top.array_index + 1].short_length == top.short_length)
//			break;
//		pri_tetra.push(array1[4 * top.array_index + 1]);
//		printf("you pushed %d in\n", array1[4 * top.array_index + 1].array_index);
//
//		set_tetra(&array1[4 * top.array_index + 2], movept, 4 * top.array_index + 2);
//		if (array1[4 * top.array_index + 2].short_length == top.short_length)
//			break;
//		pri_tetra.push(array1[4 * top.array_index + 2]);
//		printf("you pushed %d in\n", array1[4 * top.array_index + 2].array_index);
//
//		set_tetra(&array1[4 * top.array_index + 3], movept, 4 * top.array_index + 3);
//		if (array1[4 * top.array_index + 3].short_length == top.short_length)
//			break;
//		pri_tetra.push(array1[4 * top.array_index + 3]);
//		printf("you pushed %d in\n", array1[4 * top.array_index + 3].array_index);
//
//		set_tetra(&array1[4 * top.array_index + 4], movept, 4 * top.array_index + 4);
//		if (array1[4 * top.array_index + 4].short_length == top.short_length)
//			break;
//		pri_tetra.push(array1[4 * top.array_index + 4]);
//		printf("you pushed %d in\n", array1[4 * top.array_index + 4].array_index);
//	}
//	top = pri_tetra.top();
//	printf("The shortest distance tetra's index is %d\n", top.array_index);
//	draw_shortdistance(&top, movept);
//}

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
	gluOrtho2D(0, (double)viewportwidth, 0, (double)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--XY--");

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);                            //Draw control points in XY
	for (int i = 0; i < 4; i++)                    //Change the row
		for (int j = 0; j < 4; j++)                //Change the column
		{
			glVertex2f(surface.control_pts[i][j][0], surface.control_pts[i][j][1]);  //Point BicubicBeziersFAS
		}
	glEnd();

	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);                             //Draw lines in XY
	for (int i = 0; i < 4; i++)
	{
		//printf("i is %d ----------------------------\n", i);
		for (int j = 0; j < 3; j++)
		{
			glVertex2f(surface.control_pts[i][j][0], surface.control_pts[i][j][1]);
			glVertex2f(surface.control_pts[i][j + 1][0], surface.control_pts[i][j + 1][1]);
			glVertex2f(surface.control_pts[j][i][0], surface.control_pts[j][i][1]);
			glVertex2f(surface.control_pts[j + 1][i][0], surface.control_pts[j + 1][i][1]);
		}
	}
	glEnd();

	//Draw the line_1 points in XY
	glColor3f(0.0f, 0.0f, 1.0f);
	draw_dot(movept[0],movept[1]);

	// XZ
	glViewport(0, 0, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (double)viewportwidth, 0, (double)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--XZ--");                          //Draw control points in XZ
	glColor3f(1.0f, 0.0f, 0.0);
	glBegin(GL_POINTS);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			glVertex2f(surface.control_pts[i][j][0], surface.control_pts[i][j][2]);
	glEnd();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);                             //Draw lines in XZ
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
		{
			glVertex2f(surface.control_pts[i][j][0], surface.control_pts[i][j][2]);
			glVertex2f(surface.control_pts[i][j + 1][0], surface.control_pts[i][j + 1][2]);
			glVertex2f(surface.control_pts[j][i][0], surface.control_pts[j][i][2]);
			glVertex2f(surface.control_pts[j + 1][i][0], surface.control_pts[j + 1][i][2]);
		}
	glEnd();
	//Draw the line_1 points in XZ
	glColor3f(0.0f, 0.0f, 1.0f);
	draw_dot(movept[0], movept[2]);
 
	// YZ
	glViewport(viewportwidth, 0, viewportwidth, viewportheight);
	glLoadIdentity();
	gluOrtho2D(0, (double)viewportwidth, 0, (double)viewportheight);
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("--YZ--");
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);                             //Draw control points in YZ
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			glVertex2f(surface.control_pts[i][j][1], surface.control_pts[i][j][2]);
	glEnd();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);                              //Draw lines in YZ
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
		{
			glVertex2f(surface.control_pts[i][j][1], surface.control_pts[i][j][2]);
			glVertex2f(surface.control_pts[i][j + 1][1], surface.control_pts[i][j + 1][2]);
			glVertex2f(surface.control_pts[j][i][1], surface.control_pts[j][i][2]);
			glVertex2f(surface.control_pts[j + 1][i][1], surface.control_pts[j + 1][i][2]);
		}
	glEnd();

	//Draw the line_1 points in YZ
	glColor3f(0.0f, 0.0f, 1.0f);
	draw_dot(movept[1], movept[2]);

	// 3D
	glViewport(viewportwidth, viewportheight, viewportwidth, viewportheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//Designer annotation
	glColor3ub(0, 1.0, 1.0);
	glRasterPos2f(10.0f, 280.0f);
	drawString("BicubicBezierSurface");
	glRasterPos2f(10.0f, 260.0f);
	drawString("by Fangda Chen");
	gluPerspective(25, width / (double)height, 0.1, 25000);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, upVector.x, upVector.y, upVector.z);

	glEnable(GL_DEPTH_TEST);
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

	glColor3f(0, 0, 0);
	
	//One dimentional array success
	draw_1darray();

	glColor3f(0.0f, 1.0f, 0.7f);
	glBegin(GL_QUADS);                                    //Draw quads to make a surface
	for (int i = 0; i < RES; i++)
	{
		for (int j = 0; j < RES; j++)
		{
			glVertex3f(points[i][j][0], points[i][j][1], points[i][j][2]);
			glVertex3f(points[i + 1][j][0], points[i + 1][j][1], points[i + 1][j][2]);
			glVertex3f(points[i + 1][j + 1][0], points[i + 1][j + 1][1], points[i + 1][j + 1][2]);
			glVertex3f(points[i][j + 1][0], points[i][j + 1][1], points[i][j + 1][2]);
		}
	}
	glEnd();

	//Draw the move points in 3D
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_POINTS);
	glVertex3f(movept[0], movept[1], movept[2]);
	glEnd();

	//12:08
	search_sd();
	
	//Realse priority queue
	while (!pri_tetra.empty())
		pri_tetra.pop();

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
				//Find the minp of the surface, which point has been chosen
				selected = hit_index(x, (int)viewportheight - y, scene);
				//Find which point in the line has been chosen
				movept_selected = hit_index_line(x, (int)viewportheight - y, scene);
				//printf("surface selected is %d,line selected is %d\n",selected, movept_selected);
				break;
			case GLUT_UP:
				//Initialize the select signature
				selected = -1;
				movept_selected = -1;
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
		double angle = -currentP.angle(lastP) * 2;
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
		double offset = 0.025;
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
		double dx = x - viewportwidth - lastX;
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
	//Find the lastX position and lastY position
	lastX = x - viewportwidth;
	lastY = y;
	//printf("lastX is %d lastY is %d\n", lastX, lastY);
	if (selected != -1 || movept_selected != -1)
	{
		int xx = 0;
		int yy = 0;
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
		if (selected != -1)
		{
			surface.control_pts[selected / 10][selected % 10][xx] = static_cast<double>(x);
			surface.control_pts[selected / 10][selected % 10][yy] = static_cast<double>(y);
			//printf("selected is %d, control_pts[%d][%d],x is %d,y is %d\n", selected, selected / 10, selected % 10,x,y);
			calc_surface();
		}
		else
		{
			movept[xx] = x;
			movept[yy] = y;
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
