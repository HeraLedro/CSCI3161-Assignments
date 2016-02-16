// #include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define myTranslate2D(x,y) glTranslated(x, y, 0.0)
#define myScale2D(x,y) glScalef(x, y, 1.0)
#define myRotate2D(angle) glRotatef(RAD2DEG*angle, 0.0, 0.0, 1.0)


#define MAX_PHOTONS		8
#define MAX_ASTEROIDS	16
#define MAX_VERTICES	16


#define drawCircle() glCallList(circle)


/* -- display list for drawing a circle ------------------------------------- */

static GLuint	circle;

void buildCircle() {
	GLint   i;

	circle = glGenLists(1);
	glNewList(circle, GL_COMPILE);
	glBegin(GL_POLYGON);
	for (i = 0; i<40; i++)
		glVertex2d(cos(i*M_PI / 20.0), sin(i*M_PI / 20.0));
	glEnd();
	glEndList();
}


/* -- type definitions ------------------------------------------------------ */

typedef struct Coords {
	double		x, y;
} Coords;

typedef struct {
	double	x, y, phi, dx, dy;
	Coords vertices[4];
} Ship;

typedef struct {
	int	active;
	double	x, y, dx, dy;
} Photon;

typedef struct {
	int	active, nVertices;
	double	x, y, rad, phi, dx, dy, dphi;
	Coords	coords[MAX_VERTICES];
} Asteroid;


/* -- function prototypes --------------------------------------------------- */

static void	myDisplay(void);
static void	myTimer(int value);
static void	myKey(unsigned char key, int x, int y);
static void	keyPress(int key, int x, int y);
static void	keyRelease(int key, int x, int y);
static void	myReshape(int w, int h);

static void	init(void);
static void initShip(Ship* s);
static void	initAsteroid(Asteroid *a, double x, double y, double size);
static void initPhotons(Photon* p);
static void	drawShip(Ship *s);
static void	drawPhoton(Photon *p);
static void	drawAsteroid(Asteroid *a);
static void DrawCircle(Asteroid* a);
static void DrawAsteroidDust(Asteroid* a);

static void myIdle();
static void FirePhoton(Photon* p);
static void MoveShip(Ship* s);
static void MovePhoton(Photon* p);
static void MoveAsteroid(Asteroid* a);
static int InAsteroidBounds(Asteroid* a, Photon* p);
static int CheckPointCollision(Asteroid* a, float x, float y);
static int CheckShipCollision(Ship* s, Asteroid* a);
static void DestroyAsteroid(Asteroid* a);
static void DestroyShip(Ship* s);
static void DrawShipDust(Ship* s);
static void SplitAsteroid(Asteroid* a);

static double	myRandom(double min, double max);


/* -- global variables ------------------------------------------------------ */

static int		up = 0, down = 0, left = 0, right = 0, fire = 0;	/* state of cursor keys */
static double	xMax, yMax;
static int		activeAsteroids = 0, roundAsteroids = 0;	//	Number of alive asteroids and state of asteroids: circular or polygonal
static Ship		ship;
static Photon	photons[MAX_PHOTONS];
static Asteroid	asteroids[MAX_ASTEROIDS*MAX_ASTEROIDS];		//	Changed to allow for splitting asteroids, which is
															//		upper bound by MaxAsteroids^2



/* -- main ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(750, 750);
	glutCreateWindow("Asteroids");
	buildCircle();
	glutDisplayFunc(myDisplay);
	glutIgnoreKeyRepeat(1);
	glutIdleFunc(myIdle);
	glutKeyboardFunc(myKey);
	glutSpecialFunc(keyPress);
	glutSpecialUpFunc(keyRelease);
	glutReshapeFunc(myReshape);
	glutTimerFunc(33, myTimer, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	init();

	glutMainLoop();

	return 0;
}


/* -- callback functions ---------------------------------------------------- */

void myDisplay()
{
	/*
	*	display callback function
	*/

	int	i;

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	drawShip(&ship);

	for (i = 0; i < MAX_PHOTONS; i++)
	{
		if (photons[i].active)
		{
			drawPhoton(&photons[i]);
		}
	}
	for (i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (asteroids[i].active)
		{
			drawAsteroid(&asteroids[i]);
		}
	}

	glutSwapBuffers();
}

void myTimer(int value)
{
	/*
	*	timer callback function
	*/

	/* advance the ship */
	if (left == 1)
	{
		ship.phi += 0.3;
	}
	if (right == 1)
	{
		ship.phi -= 0.3;
	}
	if (up == 1)
	{
		ship.dx += -0.1*sin(ship.phi) / 2;
		ship.dy += 0.1*cos(ship.phi) / 2;
	}
	if (down == 1)
	{
		ship.dx += 0.1*sin(ship.phi) / 2;
		ship.dy += -0.1*cos(ship.phi) / 2;
	}

	if (ship.dx > 3)
	{
		ship.dx = 3;
	}
	if (ship.dy > 3)
	{
		ship.dy = 3;
	}
	MoveShip(&ship);
	drawShip(&ship);
	for (int i = 0; i < MAX_PHOTONS; i++)
	{
		if (photons[i].active == 1)
		{
			MovePhoton(&photons[i]);
		}
	}


	/* advance photon laser shots, eliminating those that have gone past
	the window boundaries */
	if (fire == 1)
	{
		for (int i = 0; i < MAX_PHOTONS; i++)
		{
			if (photons[i].active == 0)
			{
				FirePhoton(&photons[i]);
				break;
			}
		}
	}
	fire = 0;
	/* advance asteroids */
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (asteroids[i].active)
		{
			MoveAsteroid(&asteroids[i]);
			/*	Check if the asteroid has been hit by a photon.	*/
			for (int j = 0; j < MAX_PHOTONS; j++)
			{
				InAsteroidBounds(&asteroids[i], &photons[j]);
			}
		}
	}

	/* test for and handle collisions */

	/*	Check for ship collision*/
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (asteroids[i].active)
		{
			if (CheckShipCollision(&ship, &asteroids[i]))
			{
				DestroyShip(&ship);
			}
		}
	}

	if (activeAsteroids <= 0)
	{
		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (i % 2 == 0) {
				double x = 0, y = 0;
				while (fabs(myRandom(0, 100) - ship.x) < 10)
				{
					x = myRandom(0, 100);
				}
				while (fabs(myRandom(0, 100) - ship.y) < 10)
				{
					y = myRandom(0, 100);
				}
				initAsteroid(&asteroids[i], x, y, 20);
				activeAsteroids++;
			}
			else
			{
				asteroids[i].active = 0;
			}
		}
	}

	glutPostRedisplay();

	glutTimerFunc(33, myTimer, value);		/* 30 frames per second */
}

void myKey(unsigned char key, int x, int y)
{
	/*
	*	keyboard callback function; add code here for firing the laser,
	*	starting and/or pausing the game, etc.
	*/

	if (key == 32)
	{
		fire = 1;
	}
	if (key == 's')
	{
		if (roundAsteroids)
		{
			roundAsteroids = 0;
		}
		else
		{
			roundAsteroids = 1;
		}
	}
	if (key == 'q')
	{
		exit(0);
	}
}

void keyPress(int key, int x, int y)
{
	/*
	*	this function is called when a special key is pressed; we are
	*	interested in the cursor keys only
	*/

	switch (key)
	{
	case 100:
		left = 1; break;
	case 101:
		up = 1; break;
	case 102:
		right = 1; break;
	case 103:
		down = 1; break;
	}
}

void keyRelease(int key, int x, int y)
{
	/*
	*	this function is called when a special key is released; we are
	*	interested in the cursor keys only
	*/

	switch (key)
	{
	case 100:
		left = 0; break;
	case 101:
		up = 0; break;
	case 102:
		right = 0; break;
	case 103:
		down = 0; break;
	}
}

void myReshape(int w, int h)
{
	/*
	*	reshape callback function; the upper and lower boundaries of the
	*	window are at 100.0 and 0.0, respectively; the aspect ratio is
	*  determined by the aspect ratio of the viewport
	*/

	xMax = 100.0*w / h;
	yMax = 100.0;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, xMax, 0.0, yMax, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
}


/* -- other functions ------------------------------------------------------- */

void init()
{
	/*
	* set parameters including the numbers of asteroids and photons present,
	* the maximum velocity of the ship, the velocity of the laser shots, the
	* ship's coordinates and velocity, etc.
	*/
	initShip(&ship);
	glPointSize(3);
	int i = 0;
	for (i = 0; i < MAX_PHOTONS; i++)
	{
		initPhotons(&photons[i]);
	}
	initPhotons(photons);
	for (i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (i % 3 == 0) {
			double x = 0, y = 0;
			while (fabs(myRandom(0, 100) - ship.x) < 10)
			{
				x = myRandom(0, 100);
			}
			while (fabs(myRandom(0, 100) - ship.y) < 10)
			{
				y = myRandom(0, 100);
			}
			initAsteroid(&asteroids[i], x, y, 15);
			activeAsteroids++;
		}
		else
		{
			asteroids[i].active = 0;
		}
	}
}

void initShip(Ship* s)
{
	s->x = s->y = 50;
	s->phi = 0;
	s->dx = s->dy = 0;
	s->vertices[0].x = 0; s->vertices[0].y = 3;
	s->vertices[1].x = -2; s->vertices[1].y = -3;
	s->vertices[2].x = 0; s->vertices[2].y = -1.5;
	s->vertices[3].x = 2; s->vertices[3].y = -3;
}

void initPhotons(Photon* p)
{
	p->x = ship.x;
	p->y = ship.y;
	p->active = 0;

}

void initAsteroid(Asteroid *a, double x, double y, double size)
{
	/*
	*	generate an asteroid at the given position; velocity, rotational
	*	velocity, and shape are generated randomly; size serves as a scale
	*	parameter that allows generating asteroids of different sizes; feel
	*	free to adjust the parameters according to your needs
	*/

	double	theta, r;
	int		i;

	a->x = x;
	a->y = y;
	a->phi = 0.0;
	a->dx = myRandom(-0.8, 0.8);
	a->dy = myRandom(-0.8, 0.8);
	a->dphi = myRandom(-0.1, 0.1);

	a->nVertices = 6 + rand() % (MAX_VERTICES - 6);
	for (i = 0; i<a->nVertices; i++)
	{
		theta = 2.0*M_PI*i / a->nVertices;
		r = size*myRandom(2.0, 3.0) / 5;
		a->coords[i].x = -r*sin(theta);
		a->coords[i].y = r*cos(theta);
	}

	//	Find the max radius of the asteroid. Used for bounding circles.
	float dist = 0;
	a->rad = 0;
	for (i = 0; i < a->nVertices; i++)
	{
		dist = pow(a->coords[i].x, 2) + pow(a->coords[i].y, 2);
		/*	If the squared distance */
		if (a->rad*a->rad < dist)
		{
			a->rad = sqrt(dist);
		}
	}
	a->active = 1;
}

void drawShip(Ship *s)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//printf("Ship Coordinates - (%f, %f)\n", newX, newY);
	myTranslate2D(s->x, s->y);
	myRotate2D(s->phi);
	glBegin(GL_LINE_LOOP);
	glVertex2f(s->vertices[0].x, s->vertices[0].y);
	glVertex2f(s->vertices[1].x, s->vertices[1].y);
	glVertex2f(s->vertices[2].x, s->vertices[2].y);
	glVertex2f(s->vertices[3].x, s->vertices[3].y);
	glEnd();
	glPopMatrix();
}

void DrawShipDust(Ship* s)
{
	s->vertices[0].x = s->vertices[0].y += myRandom(0, 0.005);
	s->vertices[1].x = s->vertices[1].y += myRandom(0, 0.005);
	s->vertices[2].x = s->vertices[2].y += myRandom(0, 0.005);
	s->vertices[3].x = s->vertices[3].y += myRandom(0, 0.005);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glBegin(GL_LINES);
	for (int i = 0; i < 3; i++)
	{
		glVertex2f(s->vertices[i].x, s->vertices[i].y);
		glVertex2f(s->vertices[i + 1].x, s->vertices[i + 1].y);
	}
	glEnd();
	glPopMatrix();
}

void MoveShip(Ship* s)
{
	s->x += s->dx;
	s->y += s->dy;

	if (s->x > xMax)
	{
		s->x -= xMax;
	}
	if (s->x < 0)
	{
		s->x += xMax;
	}
	if (s->y > yMax)
	{
		s->y -= yMax;
	}
	if (s->y < 0)
	{
		s->y += yMax;
	}

}

void FirePhoton(Photon* p)
{
	int i = 0;
	p->active = 1;
	p->x = ship.x;
	p->y = ship.y;
	p->dx = -1.5*sin(ship.phi);
	p->dy = 1.5*cos(ship.phi);
	printf("Active Asteroids: %d\n", activeAsteroids);
}

void drawPhoton(Photon *p)
{

	//glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	//printf("(%f, %f)\n", p->x, p->y);
	//myTranslate2D(p->x, p->y);
	glBegin(GL_POINTS);
	glVertex2f(p->x, p->y);
	glEnd();
	glPopMatrix();
}

void MovePhoton(Photon* p)
{
	p->x += p->dx;
	p->y += p->dy;
	/*	If the photon extends past the limits of the screen,
	terminate it.*/
	if (p->x < 0 || p->y < 0 || p->x > xMax || p->y > yMax)
	{
		p->x = ship.x;
		p->y = ship.y;
		p->active = 0;
	}
}

void drawAsteroid(Asteroid *a)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	myTranslate2D(a->x, a->y);
	myRotate2D(a->phi);

	glBegin(GL_POLYGON);
	if (roundAsteroids)
	{
		for (int i = 0; i < 40; i++)
		{
			glVertex2f(a->rad*cos(i*M_PI/20), a->rad*sin(i*M_PI/20));
		}
	}
	else
	{
		for (int i = 0; i < a->nVertices; i++)
		{
			glVertex2f(a->coords[i].x, a->coords[i].y);
		}
	}
	glEnd();

	glPopMatrix();
}

void MoveAsteroid(Asteroid* a)
{
	a->x += a->dx;
	a->y += a->dy;
	if (a->x > xMax)
	{
		a->x -= xMax;
	}
	if (a->y > yMax)
	{
		a->y -= yMax;
	}
	if (a->x < 0)
	{
		a->x += xMax;
	}
	if (a->y < 0)
	{
		a->y += yMax;
	}
	a->phi += a->dphi;
}

void SplitAsteroid(Asteroid* a)
{
	/*	If the asteroid ha a radius larger than 1, we consider it
	as a candidate for splitting.*/
	if (a->rad > 3)
	{
		float dx = a->dx, dy = a->dy;
		int i = 0;
		while (asteroids[i].active != 0)
		{
			i++;
		}
		Asteroid* b = &asteroids[i];
		if (i < MAX_ASTEROIDS)
		{
			initAsteroid(b, a->x + a->dx *0.5, a->y + a->dy, 0.8*a->rad);
			activeAsteroids++;
		}
		initAsteroid(a, a->x + a->dx, a->x + a->dy*0.5, 0.8*a->rad);
		a->dx = 1.5 * dx;
		a->dy = 0.75*dy;
		b->dx = 0.75*dx;
		b->dy = 1.5 * dy;

	}
	else
	{
		DestroyAsteroid(a);
	}
}

void myIdle()
{
	glutPostRedisplay();
}

/*	Detects photon presence in asteroid bounding circle.
Returns 1 if true, */
int InAsteroidBounds(Asteroid* a, Photon* p)
{
	if (a->active == 0 || p->active == 0)
	{
		return 0;
	}
	/*	Given a circle with a center cX, cY and a point with coordinates pX, pY,
	we define a circle as follows:

	diffX = cX - pX
	diffY = cY - pY
	powX = diffX^2
	powY = diffY^2
	dist = sqrt(powX + powY)

	*/
	float pDist = pow(a->x - p->x, 2) + pow(a->y - p->y, 2);

	/*	If the squared distance between the point and the center of the circle is
	smaller than or equal to the squared radius, then we are within the
	bounding circle of the asteroid.*/
	if (pDist < a->rad*a->rad)
	{
		SplitAsteroid(a);
		p->active = 0;
		p->x = ship.x;
		p->y = ship.y;
		return 1;
	}
	else
	{
		return 0;
	}
}

int CheckPointCollision(Asteroid* a, float x, float y)
{
	if (a->active == 0)
	{
		return 0;
	}
	float pDist = pow(a->x - x, 2) + pow(a->y - y, 2);
	/*	If the point is not in the asteroid, return false.*/
	if (a->rad*a->rad*0.6 < pDist)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int CheckShipCollision(Ship* s, Asteroid* a)
{
	float pDist = 0;
	int collision = 0;
	/*	For each vertex of the ship, check if it is in the bounding circle of
	the asteroid*/
	for (int i = 0; i < 4; i++)
	{
		collision += CheckPointCollision(a, s->vertices[i].x + s->x, s->vertices[i].y + s->y);
	}
	printf("Collision: %d\n", collision);

	/*	For each vertex of the asteroid, do polygon-polygon check to see if it
	is in the polygonal bounds of the ship*/

	return collision;

}

void DestroyAsteroid(Asteroid* a)
{
	/*	*/
	a->active = 0;

	activeAsteroids--;
	DrawAsteroidDust(a);
}

/*	Pause all animations and show the ship being blown up.*/
void DestroyShip(Ship* s)
{
	for (int i = 0; i < MAX_PHOTONS; i++)
	{
		if (photons[i].active)
		{
			photons[i].dx = 0;
			photons[i].dy = 0;
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (asteroids[i].active == 1)
		{
			asteroids[i].dx = 0;
			asteroids[i].dy = 0;
			asteroids[i].dphi = 0;
		}
	}
	for (int i = 0; i < 10000; i++)
	{
		DrawShipDust(s);
	}
	Sleep(5000);
	init();
}

void DrawAsteroidDust(Asteroid* a)
{

}



/* -- helper function ------------------------------------------------------- */

double myRandom(double min, double max)
{
	double	d;

	/* return a random number uniformly draw from [min,max] */
	d = min + (max - min)*(rand() % 0x7fff) / 32767.0;

	return d;
}
