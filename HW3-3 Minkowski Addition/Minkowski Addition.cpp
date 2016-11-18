#include <iostream>
#include <GL/freeglut.h>
#include "curve.h"
#include <vector>
#include <set>

using namespace std;

typedef struct Obj_Pt
{
	int x;
	int y;
	//bool operator < (const Obj_Pt &a)const
	//{
	//	if (a.x != x)
	//		return a.x < x;
	//	else
	//		return a.y < y;
	//}
}Obj_Pt;

//void minkowski(Point *obj1, int obj1_num, Point *obj2, int obj2_num, vector<Obj_Pt> &a,int num1, vector<Obj_Pt> &b, int num2)
void minkowski(Point *obj1, int obj1_num, Point *obj2, int obj2_num)
{
#define RES 100
	int i, j;
	glColor3f(1.0, 0.7, 0.7);
	glPointSize(5.0);
	glBegin(GL_POINTS);
	{
		for (i = 0; i < RES * obj1_num; i++)
			for (j = 0; j < RES * obj2_num; j++)
			{
				Point pt;
				pt[0] = obj1[i][0] + obj2[j][0];
				pt[1] = obj1[i][1] + obj2[j][1];
				glVertex2f(pt[0], pt[1]);
			}
	}
	glEnd();
}

void innerminkowski(vector<Obj_Pt> &a, vector<Obj_Pt> &b)
{
	int i, j, num1,num2;
	REAL x, y, temp=0;
	//11:18
	//set<Obj_Pt>myset;
	Obj_Pt obj;

	num1 = a.size();
	num2 = b.size();
	//printf("obj1's size is %d\n", num1);
	//printf("obj2's size is %d\n", num2);

	for (i = 0; i < num1; i++)
	{
		obj.x = a[i].x;
		obj.y = a[i].y;
		for (j = 0; j < num2; j++)
		{ 
			//two points' addition
			obj.x += b[j].x;
			obj.y += b[j].y;
			//no same obj
		//	if (myset.count(obj) == 0)
			{
				//insert the point's addition to myset
			//	myset.insert(obj);
				//draw the point
				/*******************/
				glColor3f(1.0, 0.7, 0.7);
				glPointSize(6.0);
				glBegin(GL_POINTS); 
				glVertex2d(obj.x, obj.y);
				glEnd();
				/*******************/
			}
			obj.x = a[i].x;
			obj.y = a[i].y;
		}
	}
	//printf("set's size is %d\n", myset.size());
	//11:19
	//int count = 0;
	//glColor3f(1.0, 0.5, 0.6);
	//glPointSize(3.0);
	//glBegin(GL_POINTS);
	//for (vector<Obj_Pt>::iterator it1 = a.begin(); it1 < a.end(); it1++) {
	//	for (vector<Obj_Pt>::iterator it2 = b.begin(); it2 < b.end(); it2++) {
	//		x = it1->x + it2->x;
	//		y = it1->y + it2->y;

	//		//std::cout << "x is " << x << " y is " << y << endl;
	//		glVertex2d(x, y);
	//	}
	//}
	//glEnd();
	//myset.erase(myset.begin(), myset.end());
}