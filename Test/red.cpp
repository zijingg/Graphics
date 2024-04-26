#include <GLUT/glut.h>

// Function to initialize the OpenGL environment
void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0); // Set the background color to black
    glColor3f(1.0, 0.0, 0.0); // Set the drawing color to red
}

// Function to display the triangle
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen
    glBegin(GL_TRIANGLES); // Start drawing a triangle
        glVertex2f(-0.5, -0.5); // Vertex 1
        glVertex2f(0.5, -0.5); // Vertex 2
        glVertex2f(0.0, 0.5); // Vertex 3
    glEnd(); // End of triangle
    glFlush(); // Ensure all OpenGL commands are executed
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set the display mode
    glutInitWindowSize(500, 500); // Set the window size
    glutInitWindowPosition(100, 100); // Set the window position
    glutCreateWindow("Red Triangle using Freeglut"); // Create the window with a title
    init(); // Call the init function
    glutDisplayFunc(display); // Set the display callback function
    glutMainLoop(); // Enter the GLUT main loop
    return 0;
}
