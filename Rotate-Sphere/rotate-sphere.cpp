// Assignment 2
// zy2298 Zijing Yang
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;  /* vertex buffer object id for axis */

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 30.0;
GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex vertices[3];
};

std::vector<Triangle> triangles;
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int begin = 0; // 1: "begin" key is hit; 0: "begin" key is not hit
int sphere_NumVertices;
std::vector<point3> sphere_points; 
std::vector<color3> sphere_colors;
float radius = 1.0f;
vec4 points[3] = {vec4(3, 1, 5, 1), vec4(-1, 1, -4, 1), vec4(3.5, 1, -2.5, 1)};
int currSegment = 0;
vec4 currDirection;
vec4 currPosition = points[0];
mat4 M = mat4(1.0); // accumulated rotation 

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices
point3 quad_vertices[4] = {
    point3( 5.0, 0.0,  8.0),
    point3( 5.0, 0.0,  -4.0),
    point3( -5.0, 0.0,  -4.0),
    point3( -5.0, 0.0,  8.0),
};

const int axis_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 axis_points[axis_NumVertices]; // positions for all vertices
color3 axis_colors[axis_NumVertices];

//----------------------------------------------------------------------------
// file input function
void file_in(void)
{
    // get the file name
    std::string fileName;
    std::cout << "what filename? ";
    std::cin >> fileName;

    // open file
    std::ifstream inFile;
    inFile.open(fileName);
    while (!inFile.is_open()) {
        std::cout << "FILE '" << fileName << "' NOT FOUND.\n";
		std::cout << "What filename? ";
		std::cin >> fileName;
		inFile.clear();
		inFile.open(fileName);
    }

    // read the file
    int numberOfTriangles;
    inFile >> numberOfTriangles;

    int n;
    Triangle triangle;
    for (int i = 0; i < numberOfTriangles; i++) {
        inFile >> n;
        
        for (int j = 0; j < n; j++) {
            inFile >> triangle.vertices[j].x >> triangle.vertices[j].y >> triangle.vertices[j].z;
        }
        triangles.push_back(triangle);
    }

    inFile.close();
}

//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_points[0] = quad_vertices[0];
    floor_points[1] = quad_vertices[1];
    floor_points[2] = quad_vertices[2];

    floor_points[3] = quad_vertices[2];
    floor_points[4] = quad_vertices[3];
    floor_points[5] = quad_vertices[0];

    color3 green(0, 1, 0);
    for (int i = 0; i < floor_NumVertices; i++) {
        floor_colors[i] = green;
    }
}

// generate x, y, z axis
void axis() 
{
    // x-axis
    axis_colors[0] = color3(1, 0, 0); axis_points[0] = point3(0.0, 0.02, 0.0);
    axis_colors[1] = color3(1, 0, 0); axis_points[1] = point3(10.0, 0.02, 0.0);
    // y-axis
    axis_colors[2] = color3(1, 0, 1); axis_points[2] = point3(0.0, 0.0, 0.0);
    axis_colors[3] = color3(1, 0, 1); axis_points[3] = point3(0.0, 10.0, 0.0);
    // z-axis
    axis_colors[4] = color3(0, 0, 1); axis_points[4] = point3(0.0, 0.02, 0.0);
    axis_colors[5] = color3(0, 0, 1); axis_points[5] = point3(0.0, 0.02, 10.0);
}

//----------------------------------------------------------------------------
// Rotation Transformation
mat4 rotate(int start, int end) {
    currDirection = points[end] - points[start];
    vec4 rotationAxis = cross(vec4(0, 1, 0, 0), currDirection);
    return Rotate(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
}

// Translation Transformation
mat4 translate(int start, int end) {
    currDirection = points[end] - points[start];
    vec4 start_vec = points[start] - vec4(0, 0, 0, 1);
    currPosition = start_vec + angle * (2 * M_PI * radius / 360) * normalize(currDirection);
    return Translate(currPosition.x, currPosition.y, currPosition.z);
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
// Populate sphere_points with vertices from triangles
    sphere_NumVertices = triangles.size() * 3;
    sphere_points.resize(sphere_NumVertices);
    sphere_colors.resize(sphere_NumVertices);

    int k = 0;
    for (const auto& triangle : triangles) {
        for (int i = 0; i < 3; i++) {
            sphere_points[k].x = triangle.vertices[i].x;
            sphere_points[k].y = triangle.vertices[i].y;
            sphere_points[k].z = triangle.vertices[i].z;
            k++;
        }
    }

    for (int i = 0; i < sphere_NumVertices; i++) {
        sphere_colors[i] = color3(1.0, 0.84, 0);
    }

// Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sphere_points.size() * sizeof(point3) + sphere_colors.size() * sizeof(color3),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sphere_points.size() * sizeof(point3), sphere_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sphere_points.size() * sizeof(point3), sphere_colors.size() * sizeof(color3),
                    sphere_colors.data());

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);

    axis();     
 // Create and initialize a vertex buffer object for axis, to be used in display()
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
                    axis_colors);

 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader42.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 0.0 ); 
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, GLuint mode)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(sizeof(point3) * num_vertices) ); 
      // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(mode, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}
//----------------------------------------------------------------------------
void display( void )
{
  GLuint  model_view;  // model-view matrix uniform shader variable location
  GLuint  projection;  // projection matrix uniform shader variable location

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

    // Translate the center of the sphere to the point A
    mat4  mv = LookAt(eye, at, up) * Translate(3.0, 1.0, 5.0);

/*----- Set Up the Model-View matrix for the sphere -----*/
    mv = LookAt(eye, at, up) *  translate(currSegment, (currSegment + 1) % 3) 
            * rotate(currSegment, (currSegment + 1) % 3) * M;

    if ((currPosition.x < -1 && currPosition.z < -4) || 
           (currPosition.x > 3.5 && currPosition.z > -2.5) || 
           (currPosition.x < 3 && currPosition.z > 5)) {
        M = rotate(currSegment, (currSegment + 1) % 3) * M;
        currSegment = (currSegment + 1) % 3;
        angle = 0.0f;
    }

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere

/*----- Set up the Mode-View matrix for the floor -----*/
 // The set-up below gives a new scene (scene 2), using Correct LookAt() function
    mv = LookAt(eye, at, up);

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);  // draw the floor

/*----- Set Up the Model-View matrix for the axis -----*/
    mv = LookAt(eye, at, up);

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, axis_NumVertices, GL_LINES);  // draw the axis

    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    angle += 1.0f;
      // angle += 0.8f;    //YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

        case 'b': case 'B': // Toggle between animation and non-animation
            animationFlag = 1; 
            begin = 1;
            glutIdleFunc(idle);
            break;

    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void myMouse(int button, int state, int x, int y) 
{
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (begin == 1) {
            if (animationFlag == 1) {
                animationFlag = 0;
                glutIdleFunc(NULL);
            } else {
                animationFlag = 1;
                glutIdleFunc(idle);
            }
            
        } 
    }
}
//----------------------------------------------------------------------------
void myMenu(int id) 
{
    switch(id)
    {
        case 1:
            eye = init_eye;
	        break;
        case 2:
            exit( EXIT_SUCCESS );
            break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Sphere");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    { 
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif

    // Read sphere file
    file_in();

    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(myMouse);

    // set menu
    int menu_ID;
    menu_ID = glutCreateMenu(myMenu);
    //glutSetMenuFont(menu_ID, GLUT_BITMAP_HELVETICA_18);
    glutAddMenuEntry(" Default View Point ", 1);
    glutAddMenuEntry(" Quit ", 2);
    glutAttachMenu(GLUT_LEFT_BUTTON);
    
    init();
    glutMainLoop();
    return 0;
}
