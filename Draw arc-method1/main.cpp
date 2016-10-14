/* -*- mode: c -*- */
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <gl/freeglut.h>
#include <time.h>
#include "curve.h"

/* global */
CubicBezierCurve curve;
GLsizei width = 640, height = 480;
int edit_ctrlpts_idx = -1, pnum,cur_num;
REAL point_t[10];
REAL curvature_ex[5];
REAL *all_time;
REAL coefficient1[2], coefficient2[2], coefficient3[2];

void SetColor(unsigned short ForeColor, unsigned short BackGroundColor)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, (ForeColor % 16) | (BackGroundColor % 16 * 16));
}

//Draw circle
void draw_dot(REAL x, REAL y)
{
	GLfloat Pi = 3.1415926536f;
	REAL R = 5.0;
	glBegin(GL_POLYGON);
	for (int i = 0; i<20; ++i)
		glVertex2f(x+R*cos(2 * Pi / 20*i), y+R*sin(2 * Pi / 20*i));
	glEnd();
}

//When mouse touch the point
int hit_index(CubicBezierCurve *curve, int x, int y)
{
	int i;
	for (i = 0; i<4; ++i)
	{
		REAL tx = curve->control_pts[i][0] - x;
		REAL ty = curve->control_pts[i][1] - y;
		//In the point area
		if ((tx * tx + ty * ty) < 30)
		{
			printf("mouse location:(%d %d)\n", x, y);
			return i;
		}
	}
	return -1;
}

//Caculate curvature extreme points
REAL cal_CurEx(CubicBezierCurve curve, REAL t)
{
	REAL result_x, result_xx, result_xxx, result_y, result_yy, result_yyy;
	REAL part1, part2, result;

	/*coefficient1 is b1-b0 (c)
	coefficient2 is b2-2b1+b0 (b)
	coefficient3 is b3-3b2+3b1-b0 (a)*/
	//C'(t)/3 = (b1-b0) + t(2b2-4b1+2b0) + t^2(b3-3b2+3b1-b0)
	//C''(t)/6 = (b2-2b1+b0) + t(b3-3b2+3b1-b0)
	//C'''(t)/6 = (b3-3b2+3b1-b0)

	//x'(t)
	result_x = 3 * (coefficient1[0] + 2 * t*coefficient2[0] + t*t*coefficient3[0]);
	//x''(t)
	result_xx = 6 * (coefficient2[0] + t*coefficient3[0]);
	//x'''(t)
	result_xxx = 6 * coefficient3[0];
	//y'(t)
	result_y = 3 * (coefficient1[1] + 2 * t*coefficient2[1] + t*t*coefficient3[1]);
	//y''(t)
	result_yy = 6 * (coefficient2[1] + t*coefficient3[1]);
	//y'''(t)
	result_yyy = 6 * coefficient3[1];

	//(x'(t)*x'(t)+y'(t)*y'(t)*(x'(t)*y'''(t)-x'''(t)*y'(t)) - 3(x'(t)*x''(t)+y'(t)*y''(t))*(x'(t)*y''(t)-x''(t)*y'(t)))
	part1 = (result_x*result_x + result_y*result_y) * (result_x*result_yyy - result_y*result_xxx);
	part2 = 3 * (result_x*result_xx + result_y*result_yy) * (result_x*result_yy - result_y*result_xx);
	result = part1 - part2;
	return result;
}

//Search curvature extreme points in every segement
void find_CurEx(CubicBezierCurve curve, REAL t_start, REAL t_end,int num)
{
#define RES 100
	REAL interval, t, temp;
	REAL cur_ex[RES];
	Point start, end;
	int i;

	//initialization
	for (i = 0; i < RES; i++)
		cur_ex[i] = 0;

	//Test start and end point
	if (cal_CurEx(curve, t_start) == 0)
	{
		SetColor(4, 0);
		evaluate(&curve, t_start, start);
		draw_dot(start[0], start[1]);
		printf("Curvature extreme point---t is %f, point is (%.2f,%.2f)\n", t_start, start[0], start[1]);
		SetColor(7, 0);
	}
	if (num % 2 == 1)
	{
		if (cal_CurEx(curve, t_end) == 0)
		{
			evaluate(&curve, t_end, end);
			draw_dot(end[0], end[1]);
			printf("Curvature extreme point---t is %f, point is (%.2f,%.2f)\n", t_end, end[0], end[1]);
		}
	}

	interval = (t_end - t_start) / (RES-1);

	glColor3f(1.0, 0.0, 0.0);

	for (i = 0; i < RES; i++)
	{
	    t = t_start + interval*i;
		cur_ex[i] = cal_CurEx(curve, t);
	}

	for (i = 0; i < RES-1; i++)
	{
		if (cur_ex[i] * cur_ex[i + 1] < 0)
		{
			Point pt;
			REAL middle_point;
			middle_point = t_start + interval*(2 * i + 1) / 2.0;
			SetColor(4, 0);
			evaluate(&curve, middle_point, pt);
			draw_dot(pt[0], pt[1]);
			printf("curvature extreme point---t is %f, point is (%.4f,%.4f)\n", middle_point, pt[0], pt[1]);

			curvature_ex[cur_num] = middle_point;
			cur_num++;
			
			//Set word color to white and background to black
			SetColor(7, 0);
		}
	}
}

//Judge the point in which quadrant
int quadrant(float x,float y)
{
	if (x >= 0 && y >= 0)
		return 1;
	else if (x < 0 && y > 0)
		return 2;
	else if (x <= 0 && y <= 0)
		return 3;
	else if (x > 0 && y < 0)
		return 4;
}

//Draw arc with circle center and start,end point
void draw_circle(Point center, Point startpt, Point endpt)
{
	float vector_x[2], vector_y[2];
	float radius1,radius2,radius,theta1,theta2,theta_divide;
	float cos1, cos2, sin1, sin2,sin_d,cos_d,temp,temp_x,temp_y;
	Point circle_pt[9];
	int i;

	//vector for startpt
	vector_x[0] = startpt[0] - center[0];
	vector_y[0] = startpt[1] - center[1];
	//vector for endpt
	vector_x[1] = endpt[0] - center[0];
	vector_y[1] = endpt[1] - center[1];

	//Get radius
	radius1 = sqrt(vector_x[0] * vector_x[0] + vector_y[0] * vector_y[0]);
	radius2 = sqrt(vector_x[1] * vector_x[1] + vector_y[1] * vector_y[1]);
	radius = max(radius1, radius2);
	
	//When first point in the first or second quadrant
	if (quadrant(vector_x[0], vector_y[0]) <3)
	{
		//First point
		cos1 = vector_x[0] / radius;
		cos2 = vector_x[1] / radius;
		theta1 = acos(cos1);
		theta2 = acos(cos2);

		//1.If the second point is in the first or second quadrant
		//	theta2 = theta2;
		//2.If the second point is in the third or fourth quadrant
		if (quadrant(vector_x[1], vector_y[1]) > 2)
		{
			temp = 2 * PI - theta2;
			if (fabs(theta1 - temp) < PI)
				theta2 = 2 * PI - theta2;
			else
				theta2 = -theta2;
		}
	}
	
	//When first point in the third or fourth quadrant
	if (quadrant(vector_x[0], vector_y[0]) > 2)
	{
		cos1 = vector_x[0] / radius;
		cos2 = vector_x[1] / radius;
		theta1 = acos(cos1);
		theta2 = acos(cos2);

		//1.If the second point is in the first or second quadrant
		if (quadrant(vector_x[1], vector_y[1]) < 3)
		{
			//Judge the theta1
			temp = 2 * PI - theta1;
			if (fabs(temp - theta2) < PI)
				theta1 = 2 * PI - theta1;
			else
				theta1 = -theta1;
		}
		//2.If the second point is in the third or fourth quadrant
		if (quadrant(vector_x[1], vector_y[1]) > 2)
		{
			theta1 = 2 * PI - theta1;
            theta2= 2 * PI - theta2;
		}
	}
	//divide the segement to 16 pieces
	theta_divide = (theta1 - theta2) / 8;
	sin_d = sin(theta_divide);
	cos_d = cos(theta_divide);
	for (i = 0; i < 9; i++)
	{
		//Circle1
		circle_pt[i][0] = center[0] + radius*cos(theta1 - i * theta_divide);
		circle_pt[i][1] = center[1] + radius*sin(theta1 - i * theta_divide);
		/*temp_x = vector_x[i];
		temp_y = vector_y[i];
		circle_pt[i+1][0] = center[0] + temp_x * cos_d - vector_y[0] * sin_d;
		circle_pt[i+1][1] = center[1] + temp_y * sin_d + vector_y[0] * cos_d;*/
	}
	//Draw circle
	for (i = 0; i < 8; i++)
	{
		glBegin(GL_LINES);
		{
			glVertex2f(circle_pt[i][0], circle_pt[i][1]);
			glVertex2f(circle_pt[i + 1][0], circle_pt[i + 1][1]);
		}
		glEnd();
	}
}

//Draw boundary arc
void Draw_boundarc(CubicBezierCurve curve, REAL start, REAL end)
{
	Point pt1, pt0, ptt1[8],ptt2[8], cir1, cir2;
	float vector1[2], vector2[2],theta1,theta2,theta_divide;
	float coefficient, dividend1, dividend0, derivative_x0, derivative_y0, derivative_x1, derivative_y1, radius1,radius2;
	float cos1, sin1, cos2, sin2;
	int i;

	evaluate(&curve, start, pt0);
	evaluate(&curve, end, pt1);

	derivative_x0 = 3 * (coefficient1[0] + 2*start*coefficient2[0] + start*start*coefficient3[0]);
	derivative_x1 = 3 * (coefficient1[0] + 2*end*coefficient2[0] + end*end*coefficient3[0]);
	derivative_y0 = 3 * (coefficient1[1] + 2*start*coefficient2[1] + start*start*coefficient3[1]);
	derivative_y1 = 3 * (coefficient1[1] + 2*end*coefficient2[1] + end*end*coefficient3[1]);
	dividend0 = 2 * ((pt1[0] - pt0[0]) * derivative_y0 - (pt1[1] - pt0[1])*derivative_x0);
	dividend1 = 2 * ((pt1[0] - pt0[0]) * derivative_y1 - (pt1[1] - pt0[1])*derivative_x1);

	//x1^2+y1^2-x0^2-y0^2
	coefficient = pt1[0] * pt1[0] + pt1[1] * pt1[1] - pt0[0] * pt0[0] - pt0[1] * pt0[1];

	cir1[0] = (coefficient * derivative_y0 - 2 * (pt1[1] - pt0[1])*(pt0[0] * derivative_x0 + pt0[1] * derivative_y0)) / dividend0;
	cir1[1] = (2 * (pt1[0] - pt0[0])*(pt0[0] * derivative_x0 + pt0[1] * derivative_y0) - coefficient * derivative_x0) / dividend0;
	//printf("circle11 is (%f,%f)\n", cir1[0],cir1[1]);

	cir2[0] = (coefficient * derivative_y1 - 2 * (pt1[1] - pt0[1])*(pt1[0] * derivative_x1 + pt1[1] * derivative_y1)) / dividend1;
	cir2[1] = (2 * (pt1[0] - pt0[0])*(pt1[0] * derivative_x1 + pt1[1] * derivative_y1) - coefficient * derivative_x1) / dividend1;
	//printf("circle12 is (%f,%f)\n", cir1[0], cir1[1]);

	//Draw line between circle center and the midpoint
	//glColor3f(0.0, 0.0, 1.0);
	//glBegin(GL_LINES);
	//{
	//	glVertex2f((pt0[0] + pt1[0]) / 2.0, (pt0[1] + pt1[1]) / 2.0);
	//	glVertex2f(cir1[0], cir1[1]);
	//	glVertex2f((pt0[0] + pt1[0]) / 2.0, (pt0[1] + pt1[1]) / 2.0);
	//	glVertex2f(cir2[0], cir2[1]);
	//	/*glVertex2f(cir1[0], cir1[1]);
	//	glVertex2f(pt0[0], pt0[1]);
	//	glVertex2f(cir1[0], cir1[1]);
	//	glVertex2f(pt1[0], pt1[1]);
	//	glVertex2f(cir2[0], cir2[1]);
	//	glVertex2f(pt0[0], pt0[1]);
	//	glVertex2f(cir2[0], cir2[1]);
	//	glVertex2f(pt1[0], pt1[1]);*/
	//}
	//glEnd();
	//
	//glColor3f(1.0, 0.0, 0.0);
	//draw_dot(cir1[0], cir1[1], 3);
	//glColor3f(0.0, 1.0, 0.0);
	//draw_dot(cir2[0], cir2[1], 3);
	//glColor3f(1.0, 0.0, 0.0);

	glColor3f(1.0, 0.0, 0.0);
	draw_circle(cir1, pt0, pt1);
	draw_circle(cir2, pt0, pt1);
}

void draw_connectline(CubicBezierCurve curve, REAL start, REAL end)
//Draw connecting line
{
	Point pt1, pt0, midpt;

	evaluate(&curve, start, pt0);
	evaluate(&curve, end, pt1);

	//Midpoint between pt1 and pt2
	midpt[0] = (pt0[0] + pt1[0]) / 2.0;
	midpt[1] = (pt0[1] + pt1[1]) / 2.0;

	//Draw the connect line between pt1 and pt2
	glColor3f(0.0, 0.0, 1.0);
	
	glBegin(GL_LINES);
	glVertex2f(pt0[0], pt0[1]);
	glVertex2f(pt1[0], pt1[1]);
	glEnd();
}

//find_inflection code
void find_inflection()
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
	//printf("b1x - b0x = %f\nb2x - 2b1x + b0x = %f\nb3x - 3b2x + 3b1x - b0x = %f\n", coefficient1[0], coefficient2[0], coefficient3[0]);
	//printf("For x, B^2 - 4AC = %f\n\n", judge[0]);
	//printf("b1y - b0y = %f\nb2y - 2b1y + b0y = %f\nb3y - 3b2y + 3b1y - b0y = %f\n", coefficient1[1], coefficient2[1], coefficient3[1]);
	//printf("For y, B^2 - 4AC = %f\n\n", judge[1]);
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
					printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t,pt[0],pt[1]);
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
					printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t1,pt[0],pt[1]);
					pointnum++;
				}
				if (t2 > 0 && t2 < 1)
				{
					Point pt;
					evaluate(&curve, t2, pt);
					draw_dot(pt[0], pt[1]);
					point_t[pointnum] = t2;
					printf("Extreme point-------------t is %f, point is (%.2f,%.2f)\n", t2, pt[0], pt[1]);
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
					printf("Extreme point--------------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
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
				printf("inflection point-----------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
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
				printf("inflection point----------t is %f, point is (%.2f,%.2f)\n", t1, pt[0], pt[1]);
				pointnum++;
			}
			if (t2 > 0 && t2 < 1)
			{
				Point pt;
				evaluate(&curve, t2, pt);
				draw_dot(pt[0], pt[1]);
				point_t[pointnum] = t2;
				printf("inflection point----------t is %f, point is (%.2f,%.2f)\n", t2,pt[0],pt[1]);
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
				printf("Inflection point----------t is %f, point is (%.2f,%.2f)\n", t, pt[0], pt[1]);
				pointnum++;
			}
		}

	}
	//The number of all used points' timing
	pnum = pointnum+1;
	point_t[pnum-1] = 1.0;
	
	REAL temp;
	//Sorting the points' timing
	for (i = 0; i < pnum; i++)
	{
		for (int j = 0; j < pnum -1- i; j++)
		{
			if (point_t[j] > point_t[j + 1])
			{
				temp = point_t[j];
				point_t[j] = point_t[j + 1];
				point_t[j + 1] = temp;
			}
		}
	}
	
	//for (i = 0; i < pnum; i++)
		//printf("time is %f\n", point_t[i]);
	//Set the word color to white
	SetColor(7, 0);
}

//Draw nonation in the window
void drawString(const char* str) {
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

//Curve initialization
void init()
{
	SET_PT2(curve.control_pts[0], 50, 100);
	SET_PT2(curve.control_pts[1], 200, 300);
	SET_PT2(curve.control_pts[2], 400, 300);
	SET_PT2(curve.control_pts[3], 550, 100);

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

void display_callback(void)
{
#define RES 100
	int i,pointnum=0;
	float start, end,temp;
	/* curve */
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ub(0, 0, 0);
	//notation
	/*glRasterPos2f(10.0f, 460.0f);
	drawString("Cubic Bezier Curve");
	glRasterPos2f(10.0f, 440.0f);
	drawString("---by Fangda Chen");*/

	start = clock();    //Running time start
						
	//curvature extreme array initialization
	//PART1 CACULATE THE COEFFCIENTS
	for (int i = 0; i < 2; i++)
	{
		/* b1-b0 */
		coefficient1[i] = curve.control_pts[1][i] - curve.control_pts[0][i];
		/* b2-2b1+b0 */
		coefficient2[i] = curve.control_pts[2][i] - 2 * curve.control_pts[1][i] + curve.control_pts[0][i];
		/* b3-3b2+3b1-b0 */
		coefficient3[i] = curve.control_pts[3][i] - 3 * curve.control_pts[2][i] + 3 * curve.control_pts[1][i] - curve.control_pts[0][i];
	}

	cur_num = 0;
	pnum = 0;
	for (i = 0; i < 5; i++)
		curvature_ex[i] = 0;
	
	//Draw curve
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= RES; ++i)
	{
		Point pt;
		//t=i/100
		const REAL t = (REAL)i / (REAL)RES;
		//pt is point value
		evaluate(&curve, t, pt);
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();
	glLineWidth(1.0);

	/* control mesh */
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i<4; ++i)
	{
		REAL *pt = curve.control_pts[i];
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();

	/* Draw control pts */
	glColor3f(0.0, 0.0, 1.0);
	glPointSize(10.0);
	glBegin(GL_POINTS);
	for (i = 0; i<4; ++i)
	{
		REAL *pt = curve.control_pts[i];
		glVertex2f(pt[0], pt[1]);
	}
	glEnd();
	glColor3ub(255, 0, 0);

	glPointSize(10.0);
	//FIND INFLECTION
	find_inflection();
	end = clock();   //Running time end
	
	//fine curvature extreme points
	for (i = 0; i < pnum - 1; i++)
		find_CurEx(curve, point_t[i], point_t[i + 1],i);
	//Get the all needed points for drawing arc
	pointnum = cur_num + pnum;
	all_time = (REAL*)malloc(sizeof(REAL)*pointnum);
	for (i = 0; i < pnum; i++)
		all_time[i] = point_t[i];
	for (i = 0; i < cur_num; i++)
		all_time[i + pnum] = curvature_ex[i];

	//Sorting the points' timing
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

	//Draw arc
	for (i = 0; i < pointnum-1; i++)
	{
		Draw_boundarc(curve, all_time[i], (all_time[i]+all_time[i + 1])/2.0);
		Draw_boundarc(curve, (all_time[i] + all_time[i + 1]) / 2.0, all_time[i + 1]);
		Draw_boundarc(curve, all_time[i], all_time[i + 1]);
		draw_connectline(curve, all_time[i], all_time[i + 1]);
		draw_connectline(curve, all_time[i], (all_time[i] + all_time[i + 1]) / 2.0);
		draw_connectline(curve, (all_time[i] + all_time[i + 1]) / 2.0, all_time[i + 1]);
	}
	//printf("cur_num is %d,pointnum is %d\n", cur_num,pointnum);
	printf("\n\n");
	glutSwapBuffers();
}

void mouse_callback(GLint button, GLint action, GLint x, GLint y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		switch (action)
		{
		case GLUT_DOWN:
			edit_ctrlpts_idx = hit_index(&curve, x, height - y);
			break;

		case GLUT_UP:
			edit_ctrlpts_idx = -1;
			break;
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
	glutMotionFunc(mouse_move_callback);

	glutMainLoop();
	return 0;
}
