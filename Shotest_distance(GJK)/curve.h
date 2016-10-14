#pragma once
#ifndef _CURVE_H_
#define _CURVE_H_

#define PRECISION   1e-5
#define EPS         1e-6        /* data type is REAL */

typedef double REAL;
typedef REAL Point[3];

typedef struct BicubicBezierSurface
{
	Point control_pts[4][4];
} BicubicBezierSurface;

struct square1
{
	int array_index;
	int index1[2];
	int index2[2];
	int index3[2];
	int index4[2];
	int index_center[2];
	int flat = -1;
	REAL short_length;
};

//Used for priority queue comparision
struct NodeCmp
{
	bool operator()(square1 &na, square1 &nb)
	{
		if (na.short_length != nb.short_length)
			return na.short_length >= nb.short_length;
		else
			return na.array_index <= nb.array_index;
	}
};

#ifdef DEBUG
void PRINT_CTRLPTS(CubicBezierCurve* crv);
#else
#   define PRINT_CTRLPTS(X)
#endif

#define SET_PT3(V, V1, V2, V3) do { (V)[0] = (V1); (V)[1] = (V2); (V)[2] = (V3); } while (0)

void evaluate(const BicubicBezierSurface *curve, const REAL t1, const REAL t2, Point value); 
void draw_dot(REAL x, REAL y);
REAL sq_dis(Point p1, Point p2);

#endif /* _CURVE_H_ */
