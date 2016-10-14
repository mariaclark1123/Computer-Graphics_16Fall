/* -*- mode: c -*- */
#ifndef _CURVE_H_
#define _CURVE_H_
#define MAX_CHAR 128
#define PI 3.1415926
#define PRECISION   1e-5
#define EPS         1e-6        /* data type is float */
#define RES 100

typedef double REAL;
typedef REAL Point[2];
typedef REAL Vector[2];

typedef struct CubicBezierCurve
{
	Point control_pts[4];
} CubicBezierCurve;


//Structure of curve segment
typedef struct Cur_Seg
{
	int row, col;
	Point start;
	Point end;
	Point cir_in;
	Point cir_out;

	REAL r_in;
	REAL r_out;
	REAL start_t;
	REAL end_t;
	REAL short_length;
	REAL thickness;
}Cur_Seg;


struct NodeCmp
{
	bool operator()(Cur_Seg &na, Cur_Seg &nb)
	{
		//?õÌÓ­ÖØ?Üôìé??£¬priorityqueueäÎğÎõÌÓ­ÖØ?á³ÓğÓŞ?ã·ÛÉÖª
		if (na.short_length != nb.short_length)
			return na.short_length >= nb.short_length;
		//?õÌÓ­ÖØ?ßÓÔõ?£¬priorityqueueäÎğÎthickness?ÓŞÓğá³
		else
			return na.thickness <= nb.thickness;
	}
};

typedef struct Circle
{
	Point center;
	REAL radius;
	Vector vs, ve;
};
#ifdef DEBUG
void PRINT_CTRLPTS(CubicBezierCurve* crv);
#else
#   define PRINT_CTRLPTS(X)
#endif

#define SET_PT2(V, V1, V2) do { (V)[0] = (V1); (V)[1] = (V2); } while (0)
#define SET_VALUE(V,V1) {V=V1;}
void evaluate(const CubicBezierCurve *curve, const REAL t, Point value);
void SetColor(unsigned short ForeColor, unsigned short BackGroundColor);
void draw_dot(REAL x, REAL y);
void draw_line(Point pt1, Point pt2);
void drawString(const char* str);
int hit_index(CubicBezierCurve *curve, int x, int y);
int hit_movept(Point movept, int x, int y);
REAL sq_dis(Point p1, Point p2);
int quadrant(Vector v);
void draw_connectline(CubicBezierCurve curve, REAL start, REAL end);
void PrintfCur(Cur_Seg *na);
void set_arc(REAL time1, REAL time2, Cur_Seg* arc);
void setshortest_path(Cur_Seg* seg, Point p);
void draw_shortpath(Cur_Seg, Point p);
void draw_arc(Point center, Point start, Point end, REAL radius);
int divideshortpath(REAL st, REAL et, Point start, Point end, Point cir_in, Point cir_out, Point p);
REAL cal_CurEx(REAL t, REAL coefficient1[2], REAL coefficient2[2], REAL coefficient3[2]);
void find_CurEx(CubicBezierCurve curve, REAL t_start, REAL t_end, REAL coefficient1[2], REAL coefficient2[2], REAL coefficient3[2], int num);
#endif /* _CURVE_H_ */
