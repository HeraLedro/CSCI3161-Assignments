/*  RollerCoaster.cpp   */

// #include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <GL/glut.h>


#define MAX_LINE_SIZE 100

#define DBL_EPSILON 2.2204460492503131E-16 /*	For SquareRoot function, from studio.segger.com	*/

typedef struct
{
	double x, y, z;	//	Specify axes
}Vector;

/*  global variables    */
static int maxPoints, ID, xMax, yMax, trackLocation, cameraMode;
static double eyeX, eyeY, eyeZ, atX, atY, atZ, upX, upY, upZ, cameraPhi = 0;
static Vector **coords, **spline, *trackEye, *trackAt, *trackUp;

/*	OpenGL Function Declarations	*/
static void	MyDisplay(void);
static void	MyTimer(int value);
static void	MyKey(unsigned char key, int x, int y);
static void MyReshape(int w, int h);
static void	Init(void);

/*  Drawing and Helper Functions    */
int InitCoords();
double Nu(double u, int i);
void DrawSky();
void DrawSurface();
void DrawSpline(int uMax);

/*  Display List Functions  */
void GetDisplayList();

/*	Spline Operations	*/
void Q(double u, Vector* ret);
double R3(double t);
double R2(double t);
double R1(double t);
double R0(double t);

/*	Vector Functions	*/
Vector* ZeroVector();
Vector* InitVector(double x, double y, double z);
void SetCoords(Vector* v, double x, double y, double z);
double VectorMagnitude(Vector* v);
Vector* Normalize(Vector* v);
Vector* Scale(Vector* v, double factor);
double DotProduct(Vector* v1, Vector* v2);
Vector* CrossProduct(Vector* v1, Vector* v2);
void Assign(Vector* v1, Vector* v2);
void Sum(Vector* v1, Vector* v2);
void Diff(Vector* v1, Vector* v2);
Vector* Prod(Vector* v, double factor);
Vector* Quot(Vector* v, double factor);

using namespace std;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(750, 750);
	glutCreateWindow("Coaster");
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(MyKey);
	glutReshapeFunc(MyReshape);
	glutTimerFunc(33, MyTimer, 0);
	glClearColor(0.0, 0.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glPointSize(5);
	Init();
	GetDisplayList();
	glutMainLoop();

	return 0;
}

static void Init()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	int succ = InitCoords();
    /*  If the file read is unsuccessful, exit the program. */
    if (!succ)
	{
		exit(0);
	}
	eyeX = 6;
	eyeY = 4;
	eyeZ = 0;
	atX = atY = atZ = 0;
	upX = upZ = 0;
	upY = 1;
	trackEye = ZeroVector();
	trackAt = ZeroVector();
	trackUp = ZeroVector();
	trackUp->y = 1;
	trackLocation = 0;
	cameraMode = 0;

	ID = glGenLists(1);
}

int InitCoords()
{
	ifstream file("SplineCoords.txt");
	string line;
	if (file.is_open())
	{
		int size = 0, i = 0;
		double x[3];
        /*  Parse each line of the file.    */
		while (!file.eof())
		{
			printf("Vector %d:", (i + 1));
            /*  Capture the first line and store it as size.    */
            if (size == 0)
			{
				file >> size;
				maxPoints = size;
				coords = (Vector**)malloc(maxPoints*sizeof(Vector*));
				spline = (Vector**)malloc((maxPoints - 2) * 100 * sizeof(Vector*));
			}
            /*  For each line following, capture the decimal value and store it
                in the appropriate location in the x-array. Once captured,
                initialize the location of the coordinates as a vector.
                Enable (uncomment) print for debugging. */
			else
			{
				file >> x[0];
				file >> x[1];
				file >> x[2];
				coords[i] = (Vector*)malloc(sizeof(Vector));
				coords[i]->x = x[0];
				coords[i]->y = x[1];
				coords[i]->z = x[2];
				// printf("\t%lf, %lf, %lf\n", coords[i]->x, coords[i]->y, coords[i]->z);
				i++;
			}
		}
		return 1;
	}

	else
	{
		return 0;
	}
}

/*  Create a cylinder for the sky   */
void DrawSky()
{
	double radius = 30, height = 20;
	int sections = 30;
	for (int i = 0; i < sections; i++)
	{
		double theta = (double)i*(2*M_PI);
		double next = (double)(i + 1)*(2 * M_PI);
		glBegin(GL_TRIANGLE_STRIP);
			glColor3f(0, 0, 0);	//	Black
			glVertex3f(0, height, 0);
			glVertex3f(radius*cos(theta), height, radius*sin(theta));
			glVertex3f(radius*cos(next), height, radius*sin(next));
			glVertex3f(radius*cos(next), -height, radius*sin(next));
			glVertex3f(radius*cos(theta), -height, radius*sin(theta));
			glVertex3f(0, -height, 0);
		glEnd();
	}
}

/*  Create a large circle for the ground    */
void DrawSurface()
{
	glBegin(GL_QUADS);
		glColor3f(0, 1, 0);   //  Green
		glVertex3f(30, -3, 30);
		glVertex3f(-30, -3, 30);
		glVertex3f(-30, -3, -30);
		glVertex3f(30, -3, -30);
	glEnd();
}

/*  Draw the track  */
void DrawSpline(int uMax)
{
	glLineWidth(5);
	glBegin(GL_LINE_STRIP);
	glColor3f(1, 0, 0);

	/*	Print control points for debugging  */
	// for (int i = 0; i < maxPoints; i++)
	// {
	// 	printf("Vector %d:\n", i);
	// 	printf("\t%lf, %lf, %lf\n", coords[i]->x, coords[i]->y, coords[i]->z);
	// }
	int j = 0;
	for (double u = 3; u <= uMax; u += 0.005)
	{
		/*	Find the products associated with each point	*/
		int i = (int)u;
		double rem = u - i;
		Vector* v0 = Prod(coords[i - 3], R3(rem));
		Vector* v1 = Prod(coords[i - 2], R2(rem));
		Vector* v2 = Prod(coords[i - 1], R1(rem));
		Vector* v3 = Prod(coords[i], R0(rem));

		/*	Get q(u)	*/
		Vector* vSum = ZeroVector();
		Sum(v0, vSum);
		Sum(v1, vSum);
		Sum(v2, vSum);
		Sum(v3, vSum);
        /*  Draw the spline point and store it in the splines array */
		glVertex3f(vSum->x, vSum->y, vSum->z);
		spline[j] = InitVector(vSum->x, vSum->y, vSum->z);
		j++;
        /*  Memory management: free all variables.  */
		free(v0);
		free(v1);
		free(v2);
		free(v3);
		free(vSum);
	}
	glEnd();
}

static void MyDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	//printf("I work\n");
	if (cameraMode == 0)
	{
		gluLookAt(/*eye*/   eyeX, eyeY, eyeZ,
				  /*at*/    atX, atY, atZ,
				  /*up*/    upX, upY, upZ);
	}
	if (cameraMode == 1)
	{
		gluLookAt(/*eye*/   trackEye->x, trackEye->y, trackEye->z,
				  /*at*/    trackAt->x, trackAt->y, trackAt->z,
				  /*up*/    trackUp->x, trackUp->y, trackUp->z);
	}


	glCallList(ID);

	glutSwapBuffers();
}

static void MyTimer(int value)
{
	/*  Move Camera around track	*/
	cameraPhi += 0.01;
	eyeX = 10*sin(cameraPhi);
	eyeZ = 10*cos(cameraPhi);

	/*  Progress Camera along track */
	trackLocation = ((trackLocation+1)%(4800-5))+7;
	if (trackLocation + 1 >= 4800)
	{
		trackLocation = 0;
	}
	if (cameraMode == 1)
	{
		trackEye->x = spline[trackLocation]->x;
		trackEye->y = spline[trackLocation]->y + 0.25;
		trackEye->z = spline[trackLocation]->z;
		trackAt->x = spline[trackLocation + 1]->x;
		trackAt->y = spline[trackLocation + 1]->y + 0.25;
		trackAt->z = spline[trackLocation + 1]->z;
		trackUp->x = 0;
		trackUp->y = 1;
		trackUp->z = 0;
	}

	/*	Draw Control Points*/


	/*  Draw the scene */
	glutPostRedisplay();
	glutTimerFunc(33, MyTimer, 1);
}

static void MyReshape(int w, int h)
{
	xMax = 100 * w/h;
	yMax = 100;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75, w/h, 0.2, 500);
	glMatrixMode(GL_MODELVIEW);
}

static void MyKey(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		cameraPhi -= 0.5;
		break;
	case 'd':
		cameraPhi += 0.5;
		break;
	case 'q':
		exit(0);
	case ' ':
		if (cameraMode == 0)
		{
			cameraMode = 1;
		}
		else
		{
			cameraMode = 0;
		}
		break;
	default:
		break;

	}
}


void GetDisplayList()
{
	glNewList(ID, GL_COMPILE);
	DrawSky();
	DrawSurface();
	DrawSpline(maxPoints - 3);
	glEndList();
}


/*	Vector Functions	*/

Vector* ZeroVector()
{
	Vector* v = (Vector*)calloc(3, sizeof(Vector));
	return v;
}

Vector* InitVector(double x, double y, double z)
{
	Vector* v = (Vector*)malloc(sizeof(Vector));
	v->x = x;
	v->y = y;
	v->z = z;
	return v;
}

void SetCoords(Vector* v, double x, double y, double z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

double VectorMagnitude(Vector* v)
{
	return sqrt(pow(v->x, 2) + pow(v->y, 2) + pow(v->z, 2));
}

Vector* Normalize(Vector* v)
{
	double m = VectorMagnitude(v);
	double x = v->x / m;
	double y = v->y / m;
	double z = v->z / m;
	return InitVector(x, y, z);
}

Vector* Scale(Vector* v, double factor)
{
	return InitVector(v->x * factor, v->y * factor, v->z * factor);
}

double DotProduct(Vector* v1, Vector* v2)
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

Vector* CrossProduct(Vector* v1, Vector* v2)
{
	/*	(unit)	|	i	|	j	|	k	|
		(v1)	|	x1	|	y1	|	z1	|
		(v2)	|	x2	|	y2	|	z2	|	*/

	double xN = (v1->y * v2->z) - (v1->z * v2->y);
	double yN = (v1->z * v2->x) - (v1->x * v2->z);
	double zN = (v1->x * v2->y) - (v1->y * v2->x);
	return InitVector(xN, yN, zN);
}

void Assign(Vector* v1, Vector* v2)
{
	v2->x = v1->x;
	v2->y = v1->y;
	v2->z = v1->z;
}

void Sum(Vector* v1, Vector* v2)
{
	v2->x += v1->x;
	v2->y += v1->y;
	v2->z += v1->z;
}

void Diff(Vector* v1, Vector* v2)
{
	v2->x -= v1->x;
	v2->y -= v1->y;
	v2->z -= v1->z;

}

Vector* Prod(Vector* v, double factor)
{
	Vector* n = ZeroVector();
	n->x = v->x * factor;
	n->y = v->y * factor;
	n->z = v->z * factor;
	return n;
}

Vector* Quot(Vector* v, double factor)
{
	v->x /= factor;
	v->y /= factor;
	v->z /= factor;
	return v;
}


/*	Spline Operations	*/

double Nu(double u, int i)
{
	/*  Error check */
	if (u < 0 || u > 4)
	{
		return 0;
	}

	else
	{
		/*  R0  */
		if (u < i)
		{
			return R0(u);
		}
		/*  R1  */
		else if (u < i + 1)
		{
			return R1(u);
		}
		/*  R2  */
		else if (u < i + 2)
		{
			return R2(u);
		}
		/*  R3  */
		else
		{
			return R3(1);
		}
	}
}

double R3(double t)
{
	return pow((1 - t), 3) / 6;
	/*	Derivative:	-0.5(1-t^2)*/
	/*	Second Derivative: t	*/
}
double R2(double t)
{
	return (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
	/*	Derivative:	1.5t^2 - 2t	*/
	/*	Second Derivative: 3t - 2	*/
}
double R1(double t)
{
	return(-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
	/*	Derivative:	-1.5t^2 + t + 0.5	*/
	/*	Second Derivative: 3t + 1	*/
}
double R0(double t)
{
	return pow(t, 3) / 6;
	/*	Derivative:	1.5t^2	*/
	/*	Second Derivative: 3t*/
}
