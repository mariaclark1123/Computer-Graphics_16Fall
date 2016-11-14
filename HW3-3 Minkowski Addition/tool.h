#pragma once
#ifndef _TOOL_H_
#define _TOOL_H_

#include "curve.h"

void draw_dot(REAL x, REAL y);
void draw_plane(REAL x,REAL y,REAL z);
void draw_points(Point* pt, int num);
void draw_boundbox(int xmin1, int xmax1, int ymin1, int ymax1);
void minkowski(Point *obj1, int obj1_num, Point *obj2, int obj2_num);
void SETcurve_ctrlpt(CubicBezierCurve* curve, int num, Point* ptarray);

//Draw annotation
void drawString(const char* str);
int get_Xmax(Point* array, int num);
int get_Xmin(Point* array, int num);
int get_Ymax(Point* array, int num);
int get_Ymin(Point* array, int num);
int hit_index(Point* ctrlpts, int num, int x, int y);
REAL sq_dis(Point p1, Point p2);

#endif