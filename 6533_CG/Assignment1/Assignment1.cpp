// CS-GY-6533 Assignment1
// Ahhyun Moon (am12180@nyu.edu)
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

int pX, pY, r;
int circles;
float circle_scale;
const int K = 60; // K factor for Animation. Higher value for slower animation. 
bool animation_on;
vector<int> circles_x;
vector<int> circles_y;
vector<int> circles_r;
int circle_size;
void draw_circle(void);
void display(void);
void display_circles(void);
void myinit(void);
void idle(void);
int s_radius();

/* Function to handle file input; modification may be needed */
void file_in(void);
void animation(void);
/*-----------------
The main function
------------------*/
int main(int argc, char **argv)
{
    // Get user input to decide whether to 
    // draw from three integers or a file.
    char input;
    cout << "Draw a circle from integers or a file? (i/f):  ";
    cin >> input;
    while (input != 'i' && input != 'f'){
        cout << "Please enter 'i' or 'f':   ";
        cin >> input;
    }
    // Get pX and pY coordinates for circle origin
    // and r for radius. 
    if (input == 'i' || input == 'I'){
        cout << "X-coordinate: ";
        cin >> pX;
        cout << "Y-coordinate: ";
        cin >> pY;
        cout << "Radius: ";
        cin >> r;
        glutInit(&argc, argv);
        /* Use both double buffering and Z buffer */
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowPosition(XOFF, YOFF);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("Assignment 1");
        // Circle display function.
        glutDisplayFunc(display);

    } else if (input == 'f' || input == 'f'){
        // Handle file input.
        file_in();
        // Get user input to add/skip animation.
        animation();
        glutInit(&argc, argv);
        /* Use both double buffering and Z buffer */
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowPosition(XOFF, YOFF);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("Assignment 1");
        // Set display_circles as display function. 
        glutDisplayFunc(display_circles);
        // Set idle function for animation.
        glutIdleFunc(idle); 
    }
    myinit();
    glutMainLoop();
    return 0;
}
// File input function
void file_in(void)
{
    // Get user input file name
    string fileName;
    cout << "Enter your file name:  ";
    cin >> fileName;
    // Open file
    ifstream file;
    file.open(fileName);
    // Get correct file name when cannot find the file
    while (!file.is_open()){
        cout << "File does not exist. Re-enter your file name:  ";
        cin >> fileName;
        file.open(fileName);
    }
    // Read first value for number of circles
    file >> circles;
    // Read x,y,r of each row and store them in vectors
    int x_cord, y_cord, radius;
    for (int i = 0; i < circles; i++){
        file >> x_cord >> y_cord >> radius;
        circles_x.push_back(x_cord);
        circles_y.push_back(y_cord);
        circles_r.push_back(radius);
    }
}
// User input function for animation
void animation(void) {
    // Get user input
    char a;
    cout << "Add animation (y/n):   ";
    cin >> a;
    while (a != 'y' && a != 'n'){
        cout << "Please enter 'y' or 'n':   ";
        cin >> a;
    }
    // Set global variable animation
    if (a == 'y' || a == 'Y'){
        animation_on = true;
    } else if (a == 'n' || a == 'N') {
        animation_on = false;
    }
}
// Draw pixel at x, y using glVertex2i
void draw(int x, int y) {
    glVertex2i(x + pX, y + pY);
}
// Draw points on circle in octets (8 sections)
void circlePoint(int x, int y) {
    draw(x, y);
    draw(-x, y);
    draw(x, -y);
    draw(-x, -y);
    draw(y, x);
    draw(-y, x);
    draw(y, -x);
    draw(-y, -x);
}
// Draw_circle function derived from Bresenham’s scan-conversion algorithm. 
void draw_circle() {
    // Set first point as (r, 0)
    int x = r;
    int y = 0;
    // Set D_start 
    int d = 1 - r;
    // Loop until y coordinate reaches x
    while (y <= x){
        // Draw point x,y
        circlePoint(x, y);
        // Increase y by 1
        y++;
        // If mid-point(decision variable) is inside the circle,
        // Choose N and update decision variable.
        if (d < 0) d += 2*y + 1;
        // If mid-point(decision variable) is outside the circle,
        // Choose NW by decreasing x by 1 and update decision variable.
        else {
            x--;
            d += 2*y - 2*x + 1;
        }
    }
}
// scale() function to find scale factor for circles
float scale(){
    // Find the left-most, right-most, top, and bottom pixel in given input file
    int left = 0;
    int right = WINDOW_WIDTH;
    int top = WINDOW_HEIGHT;
    int bottom = 0;
    for (int i = 0; i < circles; i++){
        left = min(left, (circles_x[i] - circles_r[i]));
        right = max(right, (circles_x[i] + circles_r[i]));
        top = max(top, (circles_y[i] + circles_r[i]));
        bottom = min(bottom, (circles_y[i] - circles_r[i]));
    }
    // Pick the farthest point from origin on x-axis 
    int x_max = max(abs(left), abs(right));
    // Pick the farthest point from origin on y-axis 
    int y_max = max(abs(top), abs(bottom));
    // Calculate x/y scales and pick smaller value to fit all the pixel 
    float x_scale = WINDOW_WIDTH / (x_max * 2.0);
    float y_scale = WINDOW_HEIGHT / (y_max * 2.0);
    float scale = min(x_scale, y_scale);
    // Return scale value only when circles cannot fit in default window
    if (scale < 1) {
        return scale;
    } else {
        return 1;
    }
}
// idle() function to update animation factor
void idle(void){
    // Increase animation factor by 1
    circle_scale += 1.0f;
    if (circle_scale > K){
        // Reset animation factor to 1 when it reaches K
        circle_scale = 1.0;
    }
    glutPostRedisplay();
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
    draw_circle();
    glEnd();
    glFlush();                            /* render graphics */
    glutSwapBuffers();                    /* swap buffers */
}
// display_circles() function to display multiple circles from file input
void display_circles(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
    glColor3f(1.0f, 0.84f, 0.0f);         /* draw in golden yellow */
    glPointSize(1.0);                     /* size of each point */
    glBegin(GL_POINTS);
    // Calculate scale factor
    float s = scale();
    // Scale, transform, and draw each circle
    for (int i = 0; i < circles; i++){
        pX = round(circles_x[i] * s) + WINDOW_WIDTH/2;
        pY = round(circles_y[i] * s) + WINDOW_HEIGHT/2;
        if (animation_on){
            // Also apply animation when animation_on
            r = floor(circles_r[i] * s) * (circle_scale / K);
        } else {
            r = floor(circles_r[i] * s);
        }
        draw_circle();
    }
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
