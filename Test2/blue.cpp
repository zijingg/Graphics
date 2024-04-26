#include <GL/freeglut.h>
#include <OpenGL/gl.h>

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0); // Set the background color to black
    glColor3f(0.0, 0.0, 1.0); // Set the drawing color to blue
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    glBegin(GL_QUADS); // Start drawing a quad
        glVertex2f(-0.5, -0.5); // Bottom left vertex
        glVertex2f(-0.5,  0.5); // Top left vertex
        glVertex2f( 0.5,  0.5); // Top right vertex
        glVertex2f( 0.5, -0.5); // Bottom right vertex
    glEnd(); // End of quad

    glFlush(); // Ensure all OpenGL commands are executed
}

int main(int argc, char** argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set the display mode
    glutInitWindowSize(500, 500); // Set the window size
    glutInitWindowPosition(100, 100); // Set the window position
    glutCreateWindow("Blue Square using Freeglut"); // Create the window with a title
    init(); // Call the init function
    glutDisplayFunc(display); // Set the display callback function
    glutMainLoop(); // Enter the GLUT main loop
    return 0;
}
