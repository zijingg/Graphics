#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#ifdef __APPLE__  // include Mac OS X verions of headers
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glut.h>
#endif

#define XOFF          50
#define YOFF          50
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 600

struct Circle {
    int x, y, r;
};

void display(void);
void myinit(void);
float scale();
void draw_circle(int x, int y, int r);
void display_clrcles(void);
void idle(void);

/* Function to handle file input; modification may be needed */
void file_in(void);

/* Global variables */
vector<Circle> circles;
int circleCount;
int x_p, y_p, radius;
float f = 1.0f;
const int K = 100; 
bool animation;
/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{

    // get user input to choose from the option
    char input;
    char animate;
    cout << "Draw a circle from integers or file? (i/f): ";
    cin>>input;
    while (input != 'i' && input != 'f') {
        cout << "Invalid Input" << endl;
        cout << "Draw a circle from integers or file? (i/f): ";
        cin >> input;
    }

    cout << "Animate circles? (y/n): ";
    cin >> animate;
    while (animate != 'y' && animate != 'n') {
        cout << "Invalid Input" << endl;
        cout << "Animate circles? (y/n): " << endl;
        cin >> animate;
    }

    // if integers are need to draw a circle
    if (input == 'i') {
        cout << "Enter x-coordinate: ";
        cin >> x_p;
        cout << "Enter y-coordinate: ";
        cin >> y_p;
        cout << "Enter radius: ";
        cin >> radius;
        
        if (animate == 'y') {
            animation = true;
        } else {
            animation = false;
        }

        // draw a circle based on the given information
        glutInit(&argc, argv);

        /* Use both double buffering and Z buffer */
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

        glutInitWindowPosition(XOFF, YOFF);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("Assignment 1");
        glutDisplayFunc(display);
        glutIdleFunc(idle);

    } else {
        /* Function call to handle file input here */
        file_in();

        if (animate == 'y') {
            animation = true;
        } else {
            animation = false;
        }

        // draw circles based on the given information
        glutInit(&argc, argv);

        /* Use both double buffering and Z buffer */
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

        glutInitWindowPosition(XOFF, YOFF);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("Assignment 1");
        glutDisplayFunc(display_clrcles);
        glutIdleFunc(idle);
    }

    myinit();
    glutMainLoop();

    return 0;
}

/*----------
file_in(): file input function. Modify here.
------------*/
void file_in(void)
{
    // get the file name
    string fileName;
    cout << "what filename? ";
    cin >> fileName;

    // open file
    ifstream inFile;
    inFile.open(fileName);
    while (!inFile) {
        cout << "FILE FAILED TO OPEN! " << endl;
		cout << "What filename? ";
		cin >> fileName;
		inFile.clear();
		inFile.open(fileName);
    }

    // read the file
    Circle circle;
    inFile >> circleCount;
    while (inFile >> circle.x >> circle.y >> circle.r) {
        circles.push_back(circle);
    }
    inFile.close();
}

/*---------------------------------------------------------------------
display(): This function is called once for _every_ frame. 
---------------------------------------------------------------------*/
void display(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0f, 0.84f, 0.0f);         /* draw in golden yellow */
    glPointSize(1.0);                     /* size of each point */

    glBegin(GL_POINTS);
    //glVertex2i(300, 300);               /* draw a vertex here */

    int curr_radius = radius;
    if (animation) {
        curr_radius = radius * f / K;
    }

    draw_circle(x_p, y_p, curr_radius);
    glEnd();

    glFlush();                            /* render graphics */

    glutSwapBuffers();                    /* swap buffers */
}

/*---------------------------------------------------------------------
myinit(): Set up attributes and viewing
---------------------------------------------------------------------*/
void myinit()
{
    glClearColor(0.0f, 0.0f, 0.92f, 0.0f);    /* blue background*/

    /* set up viewing */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
}

// draw points across the circle's octants
void circlePoint(int x, int y, int x_p, int y_p) {
    glVertex2i(x + x_p, y + y_p);
    glVertex2i(x - x_p, y + y_p);
    glVertex2i(x + x_p, y - y_p);
    glVertex2i(x - x_p, y - y_p);
    glVertex2i(x + y_p, y + x_p);
    glVertex2i(x - y_p, y + x_p);
    glVertex2i(x + y_p, y - x_p);
    glVertex2i(x - y_p, y - x_p);
}

/*---------------------------------------------------------------------
draw_circle(): Bresenham's algorithm
---------------------------------------------------------------------*/
void draw_circle(int x, int y, int r) {
    // set the start point
    int x0 = 0;
    int y0 = r;
    // set D_start
    int d = 5 - 4 * r;

    // loop til x0 coordinate reachees y0
    while (x0 <= y0) {
        circlePoint(x, y, x0, y0);
        // increase x by 1
        x0++;

        // update d based on the decision parameter
        if (d < 0) { // choose E
            d += 8 * x0 + 4;
        } else { // choose SE
            y0--;
            d += 8 * x0 - 8 * y0 + 4;
        }

    }
}

/*---------------------------------------------------------------------
scale(): To fit all circles in the window
---------------------------------------------------------------------*/
float scale() {
    int minX = 0;
    int maxX = WINDOW_WIDTH;
    int minY = 0;
    int maxY = WINDOW_HEIGHT;

    // iterate through all circles to find the min and max
    for (const Circle& circle: circles) {
        minX = min(minX, circle.x - circle.r);
        maxX = max(maxX, circle.x + circle.r);
        minY = min(minY, circle.y - circle.r);
        maxY = max(maxY, circle.y + circle.r);
    }
    // consider the distance of the furthest circle from the origin
    int distanceX = max(abs(maxX), abs(minX));
    int distanceY = max(abs(maxY), abs(minY));

    // find the scale factor for x and y axes
    float scaleX = static_cast<float>(WINDOW_WIDTH) / (distanceX * 2);
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / (distanceY * 2);
    
    // adjust scale factors to maintain aspect ratie; pick the smaller scale factor
    float scale_factor = min(scaleX, scaleY);
    if (scale_factor < 1) {
        return scale_factor;
    } else {
        return  1;
    }
    
}

/*---------------------------------------------------------------------
display_clrcles(): Display circles after transformation
---------------------------------------------------------------------*/
void display_clrcles(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(1.0f, 0.84f, 0.0f);         /* draw in golden yellow */
    glPointSize(1.0);                     /* size of each point */
    glBegin(GL_POINTS);

    // apply scale factors to the circle's coordinates
    float scale_factor = scale();
    for (Circle& circle: circles) {
        x_p = round(circle.x * scale_factor) + WINDOW_WIDTH / 2;
        y_p = round(circle.y * scale_factor) + WINDOW_HEIGHT / 2; 
        // add animation or not
        if (animation) {
            radius = (circle.r * f / K) * scale_factor;
        } else {
            radius = circle.r * scale_factor;
        }
        
        draw_circle(x_p, y_p, radius);
    }            
    
    glEnd();

    glFlush();                            /* render graphics */

    glutSwapBuffers();                    /* swap buffers */

}

void idle(void)
{
	// increase animation factor by 1
	f += 1.0f;  

    if (f > K) f = 1.0f; // repeat again when it reaches K

    glutPostRedisplay(); 
    //glutTimerFunc(33, timer, 0);
}
