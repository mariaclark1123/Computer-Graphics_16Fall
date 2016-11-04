/* -*- mode: c -*- */
#ifndef _CURVE_H_
#define _CURVE_H_
#define MAX_CHAR 128
#define PI 3.1415926
#define PRECISION   1e-5
#define EPS         1e-6        /* data type is float */

typedef float REAL;
typedef REAL  Point[2];

typedef struct CubicBezierCurve
{
	Point control_pts[4];
} CubicBezierCurve;

#ifdef DEBUG
void PRINT_CTRLPTS(CubicBezierCurve* crv);
#else
#   define PRINT_CTRLPTS(X)
#endif

#define SET_PT2(V, V1, V2) do { (V)[0] = (V1); (V)[1] = (V2); } while (0)
#define SET_VALUE(V,V1) {V=V1;}
void evaluate(const CubicBezierCurve *curve, const REAL t, Point value);

#endif /* _CURVE_H_ */

//void SetColor(unsigned short ForeColor, unsigned short BackGroundColor);
//int hit_index(CubicBezierCurve *curve, int x, int y);
//void draw_dot(REAL x, REAL y, int n);
//REAL cal_CurEx(CubicBezierCurve curve, REAL t);
//void find_CurEx(CubicBezierCurve curve, REAL t_start, REAL t_end);
//int quadrant(float x, float y);
//void draw_circle(Point center, Point startpt, Point endpt);
//void Draw_boundarc(CubicBezierCurve curve, REAL start, REAL end);
//void draw_connectline(CubicBezierCurve curve, REAL start, REAL end);
//void find_inflection();
//void drawString(const char* str);
//void init();
//void reshape_callback(GLint nw, GLint nh);
//void display_callback(void);
//void mouse_callback(GLint button, GLint action, GLint x, GLint y);
//void mouse_move_callback(GLint x, GLint y);
