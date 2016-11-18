#pragma once
#ifndef _TOOL_H_
#define _TOOL_H_

#include <vector>
#include "curve.h"
using namespace std;

typedef struct Obj_Pt
{
	int x;
	int y;
	/*bool operator < (const Obj_Pt &a)const
	{
		if (a.x != x)
			return a.x < x;
		else
			return a.y < y;
	}*/
}Obj_Pt;

void draw_boundbox(int xmin1, int xmax1, int ymin1, int ymax1);
void minkowski(Point *obj1, int obj1_num, Point *obj2, int obj2_num);
void innerminkowski(vector<Obj_Pt> &a, vector<Obj_Pt> &b);
void SETcurve_ctrlpt(CubicBezierCurve* curve, int num, Point* ptarray);

//Draw annotation
void drawString(const char* str);
REAL get_Xmax(Point* array, int num);
REAL get_Xmin(Point* array, int num);
REAL get_Ymax(Point* array, int num);
REAL get_Ymin(Point* array, int num);
REAL sq_dis(Point p1, Point p2);
int hit_index(Point* ctrlpts, int num, int x, int y);
#endif