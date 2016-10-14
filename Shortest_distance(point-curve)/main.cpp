#include <stdio.h>
#include <math.h>
#include <gl/freeglut.h>
#include <queue>
#include <time.h>
#include "curve.h"
#include <windows.h>

using namespace std;

/* global */
CubicBezierCurve curve;
GLsizei width = 640, height = 480;
REAL point_t[10];
REAL curvature_ex[5];
REAL coefficient1[2], coefficient2[2], coefficient3[2];
REAL *all_time;
//Static array for store arc
Cur_Seg array_arc[12][2048];
int edit_ctrlpts_idx = -1, edit_movept = -1, exin_num, curex_num, situation = 0;

Point movept;

priority_queue<Cur_Seg, vector<Cur_Seg>, NodeCmp> pri_arc;

//Search curvature extreme points in every segement
void find_CurEx(CubicBezierCurve curve, REAL t_start, REAL t_end, REAL coefficient1[2], REAL coefficient2[2], REAL coefficient3[2], int num)
{
	REAL interval, t, temp;
	REAL cur_ex[RES];
	Point start, end;
	int i;

	glColor3f(1.0, 0.0, 0.0);
	//Initialization
	for (i = 0; i < RES; i++)
		cur_ex[i] = 0;

	//Test start and end point
	if (cal_CurEx(t_start, coefficient1, coefficient2, coefficient3) == 0)
	{
		SetColor(4, 0);
		evaluate(&curve, t_start, start);
		draw_dot(start[0], start[1]);
		//printf("Curvature extreme point---t is %f, point is (%.2f,%.2f)\n", t_start, start[0], start[1]);
		SetColor(7, 0);
	}
	if (num % 2 == 1)
	{
		if (cal_CurEx(t_end, coefficient1, coefficient2, coefficient3) == 0)
		{
			evaluate(&curve, t_end, end);
			draw_dot(end[0], end[1]);
			//printf("Curvature extreme point---t is %f, point is (%.2f,%.2f)\n", t_end, end[0], end[1]);
		}
	}

	interval = (t_end - t_start) / (RES - 1);

	for (i = 0; i < RES; i++)
	{
		t = t_start + interval*i;
		cur_ex[i] = cal_CurEx(t, coefficient1, coefficient2, coefficient3);
	}

	for (i = 0; i < RES - 1; i++)
	{
		if (cur_ex[i] * cur_ex[i + 1] < 0)
		{
			Point pt;
			REAL middle_point;
			middle_point = t_start + interval*(2 * i + 1) / 2.0;
			SetColor(4, 0);
			evaluate(&curve, middle_point, pt);
			draw_dot(pt[0], pt[1]);
			//printf("curvature extreme point---t is %f, point is (%.4f,%.4f)\n", middle_point, pt[0], pt[1]);

			curvature_ex[curex_num] = middle_point;
			curex_num++;

			//Set word color to white and background to black
			SetColor(7, 0);
		}
	}
}

//Set thickness to seg
void set_thickness(Cur_Seg *seg)
{
	REAL dis_io, thickness;
	//If start point and end point is very close, set thickness to 0
	if (sq_dis(seg->start, seg->end) < 0.1)
		seg->thickness = 0;
	else
	{
		dis_io = sqrt(sq_dis(seg->cir_in, seg->cir_out));
		thickness = seg->r_out + dis_io - seg->r_in;
		seg->thickness = thickness;
	}
}


void set_arc(REAL time1, REAL time2, Cur_Seg* arc)
{
	Point pt1, pt0, cir1, cir2;
	REAL coefficient, dividend1, dividend0, derivative_x0, derivative_y0, derivative_x1, derivative_y1, radius1, radius2;
	//	printf("address of arc is %p\n", arc);
	evaluate(&curve, time1, pt0);
	evaluate(&curve, time2, pt1);
	arc->start[0] = pt0[0];
	arc->start[1] = pt0[1];
	arc->end[0] = pt1[0];
	arc->end[1] = pt1[1];
	//	printf("1 time1 is %f time2 is %f\n", arc->start_t, arc->end_t);
	arc->start_t = time1;
	arc->end_t = time2;
	//printf("2 time1 is %f time2 is %f\n", arc->start_t,arc->end_t);

	derivative_x0 = 3 * (coefficient1[0] + 2 * time1 * coefficient2[0] + time1 * time1 * coefficient3[0]);
	derivative_x1 = 3 * (coefficient1[0] + 2 * time2 * coefficient2[0] + time2 * time2 * coefficient3[0]);
	derivative_y0 = 3 * (coefficient1[1] + 2 * time1 * coefficient2[1] + time1 * time1 * coefficient3[1]);
	derivative_y1 = 3 * (coefficient1[1] + 2 * time2 * coefficient2[1] + time2 * time2 * coefficient3[1]);
	dividend0 = 2 * ((pt1[0] - pt0[0]) * derivative_y0 - (pt1[1] - pt0[1])*derivative_x0);
	dividend1 = 2 * ((pt1[0] - pt0[0]) * derivative_y1 - (pt1[1] - pt0[1])*derivative_x1);

	//x1^2+y1^2-x0^2-y0^2
	coefficient = pt1[0] * pt1[0] + pt1[1] * pt1[1] - pt0[0] * pt0[0] - pt0[1] * pt0[1];

	cir1[0] = (coefficient * derivative_y0 - 2 * (pt1[1] - pt0[1])*(pt0[0] * derivative_x0 + pt0[1] * derivative_y0)) / dividend0;
	cir1[1] = (2 * (pt1[0] - pt0[0])*(pt0[0] * derivative_x0 + pt0[1] * derivative_y0) - coefficient * derivative_x0) / dividend0;

	cir2[0] = (coefficient * derivative_y1 - 2 * (pt1[1] - pt0[1])*(pt1[0] * derivative_x1 + pt1[1] * derivative_y1)) / dividend1;
	cir2[1] = (2 * (pt1[0] - pt0[0])*(pt1[0] * derivative_x1 + pt1[1] * derivative_y1) - coefficient * derivative_x1) / dividend1;

	radius1 = sq_dis(arc->start, cir1);
	radius2 = sq_dis(arc->start, cir2);

	if (radius1 < radius2)
	{
		arc->cir_out[0] = cir1[0];
		arc->cir_out[1] = cir1[1];
		arc->r_out = sqrt(radius1);
		arc->cir_in[0] = cir2[0];
		arc->cir_in[1] = cir2[1];
		arc->r_in = sqrt(radius2);
	}
	else
	{
		arc->cir_out[0] = cir2[0];
		arc->cir_out[1] = cir2[1];
		arc->r_out = sqrt(radius2);
		arc->cir_in[0] = cir1[0];
		arc->cir_in[1] = cir1[1];
		arc->r_in = sqrt(radius1);
	}
}

void draw_arc(Point center, Point start, Point end, REAL radius)
{
	REAL theta1, theta2, theta_divide;
	REAL cos1, cos2, sin_d, cos_d, angle_temp;
	Vector vs, ve;
	Point circle_pt[17];                  /*divide one arc to 16 pieces*/
	int i;

	vs[0] = start[0] - center[0];
	vs[1] = start[1] - center[1];
	ve[0] = end[0] - center[0];
	ve[1] = end[1] - center[1];


	//When first point in the first or second quadrant
	if (quadrant(vs) < 3)
	{
		//First point
		cos1 = vs[0] / radius;
		theta1 = acos(cos1);

		cos2 = ve[0] / radius;
		theta2 = acos(cos2);

		//1.If the second point is in the first or second quadrant
		//	theta2 = theta2;
		//2.If the second point is in the third or fourth quadrant
		if (quadrant(ve) > 2)
		{
			angle_temp = 2 * PI - theta2;
			if (fabs(theta1 - angle_temp) < PI)
				theta2 = 2 * PI - theta2;
			else
				theta2 = -theta2;
		}
	}

	//When first point in the third or fourth quadrant
	if (quadrant(vs) > 2)
	{
		cos1 = vs[0] / radius;
		theta1 = acos(cos1);

		cos2 = ve[0] / radius;
		theta2 = acos(cos2);

		//1.If the second point is in the first or second quadrant
		if (quadrant(ve) < 3)
		{
			//Judge the theta1
			angle_temp = 2 * PI - theta1;
			if (fabs(angle_temp - theta2) <= PI)
				theta1 = 2 * PI - theta1;
			else
				theta1 = -theta1;
		}

		//2.If the second point is in the third or fourth quadrant
		if (quadrant(ve) > 2)
		{
			theta1 = 2 * PI - theta1;
			theta2 = 2 * PI - theta2;
		}
	}
	//printf("theta1 is %f, theta2 is %f\n", theta1, theta2);
	theta_divide = (theta1 - theta2) / 16;
	sin_d = sin(-theta_divide);
	cos_d = cos(-theta_divide);

	circle_pt[0][0] = center[0] + radius*cos(theta1);
	circle_pt[0][1] = center[1] + radius*sin(theta1);

	circle_pt[16][0] = center[0] + radius*cos(theta2);
	circle_pt[16][1] = center[1] + radius*sin(theta2);
	for (i = 1; i < 16; i++)
	{
		circle_pt[i][0] = center[0] + cos_d*(circle_pt[i - 1][0] - center[0]) - sin_d*(circle_pt[i - 1][1] - center[1]);
		circle_pt[i][1] = center[1] + sin_d*(circle_pt[i - 1][0] - center[0]) + cos_d*(circle_pt[i - 1][1] - center[1]);
	}
	glColor3f(1.0, 0.0, 0.0);
	//Draw circle
	for (i = 0; i < 16; i++)
	{
		glBegin(GL_LINES);
		{
			glVertex2f(circle_pt[i][0], circle_pt[i][1]);
			glVertex2f(circle_pt[i + 1][0], circle_pt[i + 1][1]);
		}
		glEnd();
	}
}

int judge_part(Point start, Point end, Point cir_in, Point cir_out, Point p)
{
	//Judge the point's position
	Vector v, vv, vpin, vpout;
	Vector v1, v1v;
	Vector v2, v2v;
	Vector v3, v3v;
	Vector v4, v4v;
	//Line vertical
	v[0] = cir_out[0] - cir_in[0];
	v[1] = cir_out[1] - cir_in[1];
	vv[0] = -v[1];
	vv[1] = v[0];
	vpin[0] = p[0] - cir_in[0];
	vpin[1] = p[1] - cir_in[1];
	vpout[0] = p[0] - cir_out[0];
	vpout[1] = p[1] - cir_out[1];

	//Left part
	v1[0] = start[0] - cir_in[0];
	v1[1] = start[1] - cir_in[1];
	v1v[0] = -v1[1];
	v1v[1] = v1[0];

	v2[0] = start[0] - cir_out[0];
	v2[1] = start[1] - cir_out[1];
	v2v[0] = -v2[1];
	v2v[1] = v2[0];
	//Right part
	v3[0] = end[0] - cir_in[0];
	v3[1] = end[1] - cir_in[1];
	v3v[0] = v3[1];
	v3v[1] = -v3[0];

	v4[0] = end[0] - cir_out[0];
	v4[1] = end[1] - cir_out[1];
	v4v[0] = v4[1];
	v4v[1] = -v4[0];

	if (vv[0] * v1[0] + vv[1] * v1[1] >= 0)
	{
		//printf("start point is left of the center line\n");

		//left to line_vertical
		if (vpin[0] * vv[0] + vpin[1] * vv[1] >= 0)
		{
			//left to line 1
			if (vpin[0] * v1v[0] + vpin[1] * v1v[1] >= 0)
			{
				//left to line 2
				if (vpout[0] * v2v[0] + vpout[1] * v2v[1] >= 0)
					return 2;
				//right to line2
				else
					return 4;
			}
			//right to line1
			else
			{
				//below circle inner
				if (sq_dis(p, cir_in) <= sq_dis(start, cir_in))
					return 1;
				else
				{
					//between circle inner and circle outer
					if (sq_dis(p, cir_out) <= sq_dis(start, cir_out))
						return 0;
					//upon circle outer
					else
						return 4;
				}
			}
		}
		//right to line_vertical
		else
		{
			//right to line 3
			if (vpin[0] * v3v[0] + vpin[1] * v3v[1] >= 0)
			{
				//right to line4
				if (vpout[0] * v4v[0] + vpout[1] * v4v[1] >= 0)
					return 3;
				//left to line4
				else
					return 4;
			}
			//left to line3
			else
			{
				//below circle inner
				if (sq_dis(p, cir_in) <= sq_dis(start, cir_in))
					return 1;
				else
				{
					//between circle inner and circle outer
					if (sq_dis(p, cir_out) <= sq_dis(start, cir_out))
						return 0;
					//upon circle outer
					else
						return 4;
				}
			}
		}
	}
	else
	{
		//Line vertical
		v[0] = cir_out[0] - cir_in[0];
		v[1] = cir_out[1] - cir_in[1];
		vv[0] = v[1];
		vv[1] = -v[0];

		//Left part
		v1[0] = start[0] - cir_in[0];
		v1[1] = start[1] - cir_in[1];
		v1v[0] = v1[1];
		v1v[1] = -v1[0];

		v2[0] = start[0] - cir_out[0];
		v2[1] = start[1] - cir_out[1];
		v2v[0] = v2[1];
		v2v[1] = -v2[0];
		//Right part
		v3[0] = end[0] - cir_in[0];
		v3[1] = end[1] - cir_in[1];
		v3v[0] = -v3[1];
		v3v[1] = v3[0];

		v4[0] = end[0] - cir_out[0];
		v4[1] = end[1] - cir_out[1];
		v4v[0] = -v4[1];
		v4v[1] = v4[0];

		//left to line_vertical
		if (vpin[0] * vv[0] + vpin[1] * vv[1] >= 0)
		{
			//left to line 1
			if (vpin[0] * v1v[0] + vpin[1] * v1v[1] >= 0)
			{
				//left to line 2
				if (vpout[0] * v2v[0] + vpout[1] * v2v[1] >= 0)
					return 2;
				//right to line2
				else
					return 4;
			}
			//right to line1
			else
			{
				//below circle inner
				if (sq_dis(p, cir_in) <= sq_dis(start, cir_in))
					return 1;
				else
				{
					//between circle inner and circle outer
					if (sq_dis(p, cir_out) <= sq_dis(start, cir_out))
						return 0;
					//upon circle outer
					else
						return 4;
				}
			}
		}
		//right to line_vertical
		else
		{
			//right to line 3
			if (vpin[0] * v3v[0] + vpin[1] * v3v[1] >= 0)
			{
				//right to line4
				if (vpout[0] * v4v[0] + vpout[1] * v4v[1] >= 0)
					return 3;
				//left to line4
				else
					return 4;
			}
			//left to line3
			else
			{
				//below circle inner
				if (sq_dis(p, cir_in) <= sq_dis(start, cir_in))
					return 1;
				else
				{
					//between circle inner and circle outer
					if (sq_dis(p, cir_out) <= sq_dis(start, cir_out))
						return 0;
					//upon circle outer
					else
						return 4;
				}
			}
		}
	}
}

void draw_shortpath(Cur_Seg seg, Point p)
{
	int situation;
	Vector unit;
	REAL dist, length;
	//Judge which part is the move point located
	situation = judge_part(seg.start, seg.end, seg.cir_in, seg.cir_out, p);

	//printf("Area %d\n", situation);
	glColor3f(0.0, 0.0, 1.0);
	glLineWidth(2.0);
	if (sq_dis(seg.start, seg.end) <= 1)
	{
		dist = sqrt(sq_dis(movept, seg.start));
		draw_line(p, seg.start);
	}
	else
	{
		switch (situation)
		{
			REAL b1, b2, k;
		case 0:
			dist = 0;
			break;

		case 1:
			dist = sqrt(sq_dis(seg.start, seg.cir_in)) - sqrt(sq_dis(p, seg.cir_in));
			unit[0] = (p[0] - seg.cir_in[0]) / sqrt(sq_dis(p, seg.cir_in));
			unit[1] = (p[1] - seg.cir_in[1]) / sqrt(sq_dis(p, seg.cir_in));

			glBegin(GL_LINES);
			glVertex2f(p[0], p[1]);
			glVertex2f(p[0] + unit[0] * dist, p[1] + unit[1] * dist);
			glEnd();
			break;

		case 2:
			draw_line(p, seg.start);
			dist = sqrt(sq_dis(p, seg.start));
			break;

		case 3:
			draw_line(p, seg.end);
			dist = sqrt(sq_dis(p, seg.end));
			break;

		case 4:
			dist = sqrt(sq_dis(p, seg.cir_out)) - sqrt(sq_dis(seg.start, seg.cir_out));
			unit[0] = (p[0] - seg.cir_out[0]) / sqrt(sq_dis(p, seg.cir_out));
			unit[1] = (p[1] - seg.cir_out[1]) / sqrt(sq_dis(p, seg.cir_out));

			glBegin(GL_LINES);
			glVertex2f(p[0], p[1]);
			glVertex2f(p[0] - unit[0] * dist, p[1] - unit[1] * dist);
			glEnd();
			break;

		default:
			break;
		}
	}
}

//caculate the shortest pathvoid
void setshortest_path(Cur_Seg *seg, Point p)
{
	int situation;
	Vector unit;
	REAL dist, length;

	situation = judge_part(seg->start, seg->end, seg->cir_in, seg->cir_out, p);
	glColor3f(0.0, 1.0, 0.0);
	//When start point is closed to end point
	if (sq_dis(seg->start, seg->end) <= 1)
	{
		dist = sqrt(sq_dis(movept, seg->start));
		seg->short_length = dist;
	}
	else
	{
		switch (situation)
		{
		case 0:
			dist = 0;
			seg->short_length = dist;
			break;

		case 1:
			dist = sqrt(sq_dis(seg->start, seg->cir_in)) - sqrt(sq_dis(p, seg->cir_in));
			seg->short_length = dist;
			break;

		case 2:
			dist = sqrt(sq_dis(p, seg->start));
			seg->short_length = dist;
			break;

		case 3:
			dist = sqrt(sq_dis(p, seg->end));
			seg->short_length = dist;
			break;

		case 4:
			dist = sqrt(sq_dis(p, seg->cir_out)) - sqrt(sq_dis(seg->start, seg->cir_out));
			seg->short_length = dist;
			unit[0] = (p[0] - seg->cir_out[0]) / sqrt(sq_dis(p, seg->cir_out));
			unit[1] = (p[1] - seg->cir_out[1]) / sqrt(sq_dis(p, seg->cir_out));
			break;

		default:
			break;
		}
	}
}

//Mode0 - Draw boundary arc for whole curve
void Draw_boundarc(struct Cur_Seg* arc)
{
	draw_arc(arc->cir_in, arc->start, arc->end, arc->r_in);
	draw_arc(arc->cir_out, arc->start, arc->end, arc->r_out);
}

//find_exinf code
void find_exinf()
{
	int i, pointnum = 1;
	REAL t, t_sq, t1, t2, a, b, c, judgement;
	REAL derivative1[2] = { 0 }, derivative2[2] = { 0 }, result2 = 0, judge[2] = { 0 };

	//array point_t Initialization
	for (i = 0; i < 10; i++)
		point_t[i] = 0;

	for (i = 0; i < 2; i++)
	{
		/* judge = b^2 - 4ac*/
		judge[i] = 4.0 * coefficient2[i] * coefficient2[i] - 4 * coefficient3[i] * coefficient1[i];
	}

	//Set the [extreme point] word color and point color to red
	SetColor(8, 0);
	glColor3f(0.0, 0.0, 0.0);
	//PART2 CACULATE THE EXTREME POINTS' T
	for (i = 0; i < 2; i++)
	{
		if (coefficient3[i] == 0)
		{
			if (coefficient2[i] != 0)
			{
				t = -coefficient1[i] / (2.0 * coefficient2[i]);
				if (t > 0 && t < 1)
				{
					Point pt;
					evaluate(&curve, t, pt);
					draw_dot(pt[0], pt[1]);

					point_t[pointnum] = t;
					//printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
					pointnum++;
				}
			}
		}
		else
		{
			if (judge[i] > 0)
			{
				t1 = (-2.0*coefficient2[i] + sqrt(judge[i])) / (2.0*coefficient3[i]);
				t2 = (-2.0*coefficient2[i] - sqrt(judge[i])) / (2.0*coefficient3[i]);
				if (t1 > 0 && t1 < 1)
				{
					Point pt;
					evaluate(&curve, t1, pt);
					draw_dot(pt[0], pt[1]);
					point_t[pointnum] = t1;
					//printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t1, pt[0], pt[1]);
					pointnum++;
				}
				if (t2 > 0 && t2 < 1)
				{
					Point pt;
					evaluate(&curve, t2, pt);
					draw_dot(pt[0], pt[1]);
					point_t[pointnum] = t2;
					//printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t2, pt[0], pt[1]);
					pointnum++;
				}
			}

			if (judge[i] == 0)
			{
				t = -coefficient2[i] / coefficient3[i];
				if (t > 0 && t < 1)
				{
					Point pt;
					evaluate(&curve, t, pt);
					draw_dot(pt[0], pt[1]);
					point_t[pointnum] = t;
					//printf("Extreme point--------------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
					pointnum++;
				}
			}
		}
	}
	//Set the [inflection point] word color and point color to blue
	SetColor(2, 0);
	glColor3f(0.0, 1.0, 0.0);
	//Inflection points' requirement: (x''(t)y'(t) ? x'(t)y''(t))/18 = 0.
	/*
	x''(t)/6 = (b3-3b2+3b1-b0)t + (b2-2b1+b0)
	= coefficient_a[0]*t + coefficient_b[0]/2
	x'(t)/3  = (b3-3b2+3b1-b0)t^2 + (2b2-4b1+2b0)t+(b1-b0)
	= coefficient_a[0]*t^2 + coefficient_b[0]*t + coefficient_c[0]

	y''(t)/6 = (b3-3b2+3b1-b0)t+(b2-2b1+b0)
	= coefficient_a[1]*t + coefficient_b[1]/2
	y'(t)/3  = (b3-3b2+3b1-b0)t^2+(2b2-4b1+2b0)t+(b1-b0)
	= coefficient_a[1]*t^2 + coefficient_b[1]*t + coefficient_c[1]

	(x''(t)y'(t) ? x'(t)y''(t))/18 = 0
	co_a[0]*co_a[1]*t^3 + co_a[0]*co_b[1]*t^2 + co_a[0]*co_c[1]*t + co_b[0]/2*co_a[1]t^2 + co_b[0]/2*co_b[1]*t + co_b[0]/2*co_c[1]
	-
	co_a[1]*co_a[0]*t^3 + co_a[1]*co_b[0]*t^2 + co_a[1]*co_c[0]*t + co_b[1]/2*co_a[0]t^2 + co_b[1]/2*co_b[0]*t + co_b[1]/2*co_c[0]
	=
	(co_a[0]*co_b[1]/2-co_a[1]*co_b[0]/2)t^2 + (co_a[0]*co_c[1] - co_a[1]*co_c[0])*t + (co_b[0]/2*co_c[1]-co_b[1]/2*co_c[0])
	*/
	c = coefficient2[0] * coefficient1[1] - coefficient2[1] * coefficient1[0];
	b = coefficient3[0] * coefficient1[1] - coefficient3[1] * coefficient1[0];
	a = coefficient3[0] * coefficient2[1] - coefficient3[1] * coefficient2[0];
	judgement = b*b - 4 * a * c;
	if (a == 0)
	{
		if (b != 0)
		{
			t = -c / b;
			if (t > 0 && t < 1)
			{
				Point pt;
				evaluate(&curve, t, pt);
				draw_dot(pt[0], pt[1]);
				//draw_points(t);
				point_t[pointnum] = t;
				//printf("inflection point-----------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
				pointnum++;
			}
		}
	}
	else
	{
		if (judgement>0)
		{
			t1 = (-b + sqrt(judgement)) / (2.0*a);
			t2 = (-b - sqrt(judgement)) / (2.0*a);
			if (t1 > 0 && t1 < 1)
			{
				Point pt;
				evaluate(&curve, t1, pt);
				draw_dot(pt[0], pt[1]);
				point_t[pointnum] = t1;
				//printf("inflection point----------t is %f, point is (%.2f,%.2f)\n", t1, pt[0], pt[1]);
				pointnum++;
			}
			if (t2 > 0 && t2 < 1)
			{
				Point pt;
				evaluate(&curve, t2, pt);
				draw_dot(pt[0], pt[1]);
				point_t[pointnum] = t2;
				//printf("inflection point----------t is %f, point is (%.2f,%.2f)\n", t2, pt[0], pt[1]);
				pointnum++;
			}
		}
		if (judgement == 0)
		{
			t = -b / (2.0*a);
			if (t > 0 && t < 1)
			{
				Point pt;
				evaluate(&curve, t, pt);
				draw_dot(pt[0], pt[1]);
				point_t[pointnum] = t;
				//printf("Inflection point----------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
				pointnum++;
			}
		}

	}
	//The number of all used points' timing
	exin_num = pointnum + 1;
	point_t[exin_num - 1] = 1.0;

	REAL temp;
	//Sorting the points' timing
	for (i = 0; i < exin_num; i++)
	{
		for (int j = 0; j < exin_num - 1 - i; j++)
		{
			if (point_t[j] > point_t[j + 1])
			{
				temp = point_t[j];
				point_t[j] = point_t[j + 1];
				point_t[j + 1] = temp;
			}
		}
	}
	SetColor(7, 0);
}

//Curve initialization
void init()
{
	SET_PT2(curve.control_pts[0], 50, 100);
	SET_PT2(curve.control_pts[1], 200, 300);
	SET_PT2(curve.control_pts[2], 400, 300);
	SET_PT2(curve.control_pts[3], 550, 100);
	SET_PT2(movept, 100, 300);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0, width, 0, height);
}

void reshape_callback(GLint nw, GLint nh)
{
	width = nw;
	height = nh;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
}

//Draw curve
void initial_curve(CubicBezierCurve curve)
{
	int i;
	/* Draw control pts */
	glPointSize(10.0);
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_POINTS);
	for (i = 0; i<4; ++i)
	{
		REAL *pt = curve.control_pts[i];
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();

	/* Draw curve */
	glLineWidth(2.0);
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= RES; ++i)
	{
		Point pt;
		const REAL t = (REAL)i / (REAL)RES;
		//pt is point value
		evaluate(&curve, t, pt);
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();

	/* Draw control mesh */
	glLineWidth(1.0);
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i<4; ++i)
	{
		REAL *pt = curve.control_pts[i];
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();
}

//Mode0 - draw shortest path
void PrintfCur(Cur_Seg* na)
{
	draw_shortpath(*na, movept);
	SetColor(2, 0);
	printf("shortest distance is %f\n", na->short_length);
	SetColor(7, 0);
	printf("t is between %f - %f\n\n", na->start_t, na->end_t);
}

//If situation is 1, subdivide arc
void sub_arc()
{
	int i;
	Cur_Seg arc1;
	int index1, index2;
	REAL middle_time;

	for (i = 0; i < 10; i++)
	{
		Cur_Seg temp = pri_arc.top();
		//printf("temp is (%f,%f)\n", temp.start_t, temp.end_t);
		if (temp.thickness > 0.0001 && temp.col<2047)
		{
			index1 = temp.row, index2 = temp.col;
			middle_time = (temp.start_t + temp.end_t) / 2.0;
			//divide the shortest path temp and pop the parent temp
			array_arc[index1][2 * index2 + 1].row = index1;
			array_arc[index1][2 * index2 + 1].col = 2 * index2 + 1;
			set_arc(temp.start_t, middle_time, &array_arc[index1][2 * index2 + 1]);
			setshortest_path(&array_arc[index1][2 * index2 + 1], movept);
			set_thickness(&array_arc[index1][2 * index2 + 1]);

			array_arc[index1][2 * index2 + 2].row = index1;
			array_arc[index1][2 * index2 + 2].col = index2 + 2;
			set_arc(middle_time, temp.end_t, &array_arc[index1][2 * index2 + 2]);
			setshortest_path(&array_arc[index1][2 * index2 + 2], movept);
			set_thickness(&array_arc[index1][2 * index2 + 2]);

			//	push left child
			pri_arc.push(array_arc[index1][2 * index2 + 1]);
			//push right child
			pri_arc.push(array_arc[index1][2 * index2 + 2]);
			printf("Pushed array_arc[%d][%d], thickness is %f\n", index1, 2 * index2 + 1, array_arc[index1][2 * index2 + 1].thickness);
			printf("Pushed array_arc[%d][%d], thickness is %f\n", index1, 2 * index2 + 2, array_arc[index1][2 * index2 + 2].thickness);
			//printf("left thickness is %f right thickness is %f\n", array_arc[index1][2 * index2].thickness, array_arc[index1][2 * index2+1].thickness);
			//pop parent node
			printf("Poped array_arc[%d][%d], thickness is %f\n", pri_arc.top().row, pri_arc.top().col, pri_arc.top().thickness);
			Draw_boundarc(&temp);
			pri_arc.pop();
		}
		else
			break;
	}

	arc1 = pri_arc.top();
	printf("After subdivision:thickness is %f, shortest path is %f\n\n", arc1.thickness, arc1.short_length);
	draw_shortpath(arc1, movept);
}

void display_callback(void)
{
	int i, j, pointnum = 0, arcnum = 0;
	REAL temp;
	Cur_Seg arc;
	//initialize the arc array
	for (i = 0; i < 12; i++)
		for (j = 0; j < 2048; j++)
		{
			array_arc[i][j].col = 0;
			array_arc[i][j].row = 0;
		}

	//Initialize the number of curvature extreme points, extreme and inflection points
	curex_num = 0;
	exin_num = 0;
	//curvature extreme array initialization
	for (i = 0; i < 5; i++)
		curvature_ex[i] = 0;

	/* curve */
	glClear(GL_COLOR_BUFFER_BIT);
	initial_curve(curve);
	//notation
	/*glRasterPos2f(10.0f, 460.0f);
	drawString("Cubic Bezier Curve");
	glRasterPos2f(10.0f, 440.0f);
	drawString("---by Fangda Chen");*/

	//PART1 CACULATE THE COEFFCIENTS
	/*coefficient1 is b1-b0 (c)
	coefficient2 is b2-2b1+b0 (b)
	coefficient3 is b3-3b2+3b1-b0 (a)*/

	for (int i = 0; i < 2; i++)
	{
		/* b1-b0 */
		coefficient1[i] = curve.control_pts[1][i] - curve.control_pts[0][i];
		/* b2-2b1+b0 */
		coefficient2[i] = curve.control_pts[2][i] - 2 * curve.control_pts[1][i] + curve.control_pts[0][i];
		/* b3-3b2+3b1-b0 */
		coefficient3[i] = curve.control_pts[3][i] - 3 * curve.control_pts[2][i] + 3 * curve.control_pts[1][i] - curve.control_pts[0][i];
	}

	/*move point*/
	draw_dot(movept[0], movept[1]);

	/*Find extreme and inflection points,curvature extreme points*/
	find_exinf();
	for (i = 0; i < exin_num - 1; i++)
		find_CurEx(curve, point_t[i], point_t[i + 1], coefficient1, coefficient2, coefficient3, i);

	//Combine the curvature extreme points and extreme points and inflection points
	pointnum = curex_num + exin_num;
	/*Get the all needed points for drawing arc*/
	arcnum = pointnum - 1;

	/*conformity time array*/
	all_time = (REAL*)malloc(sizeof(REAL)*pointnum);
	for (i = 0; i < exin_num; i++)
		all_time[i] = point_t[i];
	for (i = 0; i < curex_num; i++)
		all_time[i + exin_num] = curvature_ex[i];

	//Sorting the points' timing-bubble sort
	for (i = 0; i < pointnum; i++)
	{
		for (int j = 0; j < pointnum - 1 - i; j++)
		{
			if (all_time[j] > all_time[j + 1])
			{
				temp = all_time[j];
				all_time[j] = all_time[j + 1];
				all_time[j + 1] = temp;
			}
		}
	}
	//Initialize the first level curve segment
	for (i = 0; i < arcnum; i++)
	{
		//souvenir
		//   printf("all time[i] - all time[i+1] is (%f - %f)\n", all_time[i], all_time[i + 1]);
		//printf("1 address of array_arc[%d][0] is %p start_t is %f, end_t is %f\n", i, &array_arc[i][0],array_arc[i][0].start_t,array_arc[i][0].end_t);
		array_arc[i][0].row = i;
		array_arc[i][0].col = 0;

		set_arc(all_time[i], all_time[i + 1], &array_arc[i][0]);
		setshortest_path(&array_arc[i][0], movept);
		set_thickness(&array_arc[i][0]);
		pri_arc.push(array_arc[i][0]);
		//printf("2 address of array_arc[%d][0] is %p start_t is %f, end_t is %f\n", i, &array_arc[i][0], array_arc[i][0].start_t, array_arc[i][0].end_t);
	}
	arc = pri_arc.top();

	//RIGHT 1:34
	//Get the shortest path arc
	if (situation == 0)
	{
		//	arc = pri_arc.top();
		for (i = 0; i < arcnum; i++)
		{
			Draw_boundarc(&array_arc[i][0]);
		}
		//Draw the shortest path in mode0 (bounding arc)
		PrintfCur(&arc);
	}
	else
	{
		SetColor(2, 0);
		printf("Before subdivision:thickness is %f, shortest path is %f\n", arc.thickness, arc.short_length);
		SetColor(7, 0);
		//subdivide curve with push and pop
		sub_arc();
	}
	//RIGHT 2£º02
	//Realse priority queue
	while (!pri_arc.empty())
		pri_arc.pop();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case'c':
	{
		situation += 1;
		situation = situation % 2;
	}
	case'C':
	{
		situation += 1;
		situation = situation % 2;
	}
	}
	glutPostRedisplay();
}

void mouse_callback(GLint button, GLint action, GLint x, GLint y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		switch (action)
		{
		case GLUT_DOWN:
		{
			edit_ctrlpts_idx = hit_index(&curve, x, height - y);
			edit_movept = hit_movept(movept, x, height - y);
			break;
		}
		case GLUT_UP:
		{
			edit_ctrlpts_idx = -1;
			edit_movept = -1;
			break;
		}
		}
	}
}

void mouse_move_callback(GLint x, GLint y)
{
	if (edit_ctrlpts_idx != -1)
	{
		curve.control_pts[edit_ctrlpts_idx][0] = (REAL)x;
		curve.control_pts[edit_ctrlpts_idx][1] = (REAL)(height - y);
	}
	if (edit_movept != -1)
	{
		movept[0] = (REAL)x;
		movept[1] = (REAL)(height - y);
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(800, 200);
	glutInitWindowSize(width, height);
	glutCreateWindow("Bezier Editor");

	init();
	glutReshapeFunc(reshape_callback);
	glutDisplayFunc(display_callback);
	glutMouseFunc(mouse_callback);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(mouse_move_callback);

	glutMainLoop();
	return 0;
}
