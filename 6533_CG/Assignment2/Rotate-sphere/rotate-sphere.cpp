// CS-GY-6533 Assignment2
// Ahhyun Moon (am12180)
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
typedef Angel::vec3  color3;
typedef Angel::vec4  color4;
typedef Angel::vec3  point3;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;         /* shader program object id */
GLuint floor_buffer;    /* vertex buffer object id for floor */
GLuint axis_buffer;     /* vertex buffer object id for axis */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */

// Projection transformation parameters
GLfloat  fovy = 45.0;   // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;        // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 30.0;
GLfloat angle = 0.0;    // Angle to control rotation/translation
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

// Floor variables
const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices
point3 floor_vertices[4] = {
    point3( 5.0, 0.0,  8.0),
    point3( 5.0, 0.0, -4.0),
    point3(-5.0, 0.0, -4.0),
    point3(-5.0, 0.0,  8.0)
};

// Axis variables
const int axis_NumVertices = 6; // x, y, z axis, each having 2 vertices
point3 axis_points[axis_NumVertices];
color3 axis_colors[axis_NumVertices];

// Sphere variables
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'
int sphere_NumVertices; // Will be initialized in readFile()
int radius = 1;
std::vector<std::vector<std::vector<float>>> sphere_triangles; // Sphere vertices
vec4 move_to; // Variable for translation 
mat4 M = mat4(); // Variable for accumulated rotation matrix
point3 s_start = point3(-4,1,4); // First segment start point: A
point3 s_end = point3(-1,1,-4); // First segment end point: B
bool rolling_begin = false;

//----------------------------------------------------------------------------
// Read input file for storing sphere vertices
void readFile(){
    // Get user input file name
    std::string fileName;
    std::cout << "Enter your file name:  ";
    std::cin >> fileName;
    // Open file
    std::ifstream file;
    file.open(fileName);
    // Get correct file name when cannot find the file
    while (!file.is_open()){
        std::cout << "File does not exist. Re-enter your file name:  ";
        std::cin >> fileName;
        file.open(fileName);
    }
    // Read first value for number of triangles
    int numOfTriangles;
    file >> numOfTriangles;
    int v;
    float x,y,z;
    // Read x,y,r of each row and store them in vectors
    for (int i = 0; i < numOfTriangles; i++){
        file >> v;
        std::vector<std::vector<float>> polygon;
        for (int j = 0; j < v; j++){   
            file >> x >> y >> z;    
            std::vector<float> v = {x,y,z};
            polygon.push_back(v);
        }
        sphere_triangles.push_back(polygon);
    }
}
//----------------------------------------------------------------------------
// Generate rotation axis based on start and end points and return rotation matrix
mat4 rolling(point3 start, point3 end){
    vec4 direction = end - start; 
    vec4 rAxis = cross(vec4(0,1,0,0), direction);
    return Rotate(angle, rAxis.x, rAxis.y, rAxis.z);
}
//----------------------------------------------------------------------------
// Take start and end points of a segment and return translation matrix
mat4 rolling_move(point3 start, point3 end){
    vec4 direction = end - start;
    vec4 start_point = start - point3(0,0,0);
    move_to = start_point + angle * (2 * M_PI * radius / 360) * normalize(direction);
    return Translate(move_to.x, move_to.y, move_to.z);
}
//----------------------------------------------------------------------------
// Generate 2 triangles: 6 vertices and 6 colors
void floor() {
    floor_colors[0] = color3(0,1,0); floor_points[0] = floor_vertices[2];
    floor_colors[1] = color3(0,1,0); floor_points[1] = floor_vertices[1];
    floor_colors[2] = color3(0,1,0); floor_points[2] = floor_vertices[0];

    floor_colors[3] = color3(0,1,0); floor_points[3] = floor_vertices[2];
    floor_colors[4] = color3(0,1,0); floor_points[4] = floor_vertices[3];
    floor_colors[5] = color3(0,1,0); floor_points[5] = floor_vertices[0];
}
//----------------------------------------------------------------------------
// Generate x,y,z axis
void axes() {
	axis_colors[0] = color3(1,0,0); axis_points[0] = point3(0.0,0.02,0.0);	//x-axis
	axis_colors[1] = color3(1,0,0); axis_points[1] = point3(10.0,0.02,0.0);	//x-axis
	axis_colors[2] = color3(1,0,1); axis_points[2] = point3(0.0,0.0,0.0);	//y-axis
	axis_colors[3] = color3(1,0,1); axis_points[3] = point3(0.0,10.0,0.0);	//y-axis
	axis_colors[4] = color3(0,0,1); axis_points[4] = point3(0.0,0.02,0.0);	//z-axis
	axis_colors[5] = color3(0,0,1); axis_points[5] = point3(0.0,0.02,10.0);	//z-axis
}
//----------------------------------------------------------------------------
// OpenGL initialization
void init(){
    // Set position and color for each vertex in sphere
    sphere_NumVertices = sphere_triangles.size() * 3;
    point3 sphere_points[sphere_NumVertices];
    for (int s = 0; s < sphere_triangles.size(); s++){
        for (int p = 0; p < sphere_triangles[s].size(); p++){
            sphere_points[s*3 + p] = point3(sphere_triangles[s][p][0],
            sphere_triangles[s][p][1],sphere_triangles[s][p][2]);
        }
    }
    color3 sphere_colors[sphere_NumVertices];
    for (int s = 0; s < sphere_NumVertices; s++){
        sphere_colors[s] =  color3(1.0, 0.84, 0.0);
    }
    // Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_points) + sizeof(sphere_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sphere_points), sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphere_points), sizeof(sphere_colors),
                    sphere_colors);
    // Create and initialize a vertex buffer object for floor, to be used in display()
    floor();     
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);
    // Create and initialize a vertex buffer object for axes, to be used in display()
    axes();
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
                    axis_colors);
    // Load shaders and create a shader program (to be used in display())
    program = InitShader("../vshader42.glsl", "../fshader42.glsl");
    glEnable( GL_DEPTH_TEST );
    glClearColor(0.529, 0.807, 0.92, 0.0); 
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
//   Draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
void drawObj(GLuint buffer, int num_vertices, GLuint drawMode){
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
    glDrawArrays(drawMode, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}

//----------------------------------------------------------------------------
void display( void ){
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
    vec4  at(0, 0, 0, 1.0); // at = VRP/eye (7,3,-10,1) + VPN (-7.-3,10,0)
    vec4  up(0.0, 1.0, 0.0, 0.0);
    // Translate the center of the sphere to the point A = (−4, 1, 4, 1)
    mat4  mv = LookAt(eye, at, up) * Translate(-4.0, 1.0, 4.0); 

/*----- Set up the Mode-View matrix for the sphere -----*/
    // Roll the sphere on the x-z plane by using translations and rotations
    // First rolling - center goes from the point A = (−4, 1, 4, 1) to the point B = (−1, 1, −4, 1)
    mv =  LookAt(eye, at, up) * rolling_move(s_start, s_end) * rolling(s_start,s_end) * M; 

    // Center passes through B
    // Roll the sphere so that its center goes from the point B to the point C = (3, 1, 5, 1)
    if (move_to.x >= -1 && move_to.z <= -4){
        M = rolling(s_start,s_end);
        s_start = point3(-1 ,1,-4);
        s_end = point3(3,1,5);
        angle = 1;
    }
    // Center passes through C
    // Roll the sphere so that its center goes from the point B to the point C = (3, 1, 5, 1)
    if (move_to.x >= 3 && move_to.z >= 5){
        M = rolling(s_start,s_end);
        s_start = point3(3,1,5);
        s_end = point3(-4,1,4);
        angle = 1;
    }
    // Center passes through A
    // Roll the sphere so that its center goes from the point A to the point B
    if (move_to.x <= -4 && move_to.z <= 4){
        M = rolling(s_start,s_end);
        s_start = point3(-4,1,4);
        s_end = point3(-1 ,1,-4);
        angle = 1;
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

/*----- Set up the Mode-View matrix for the x,y,z axis -----*/
    mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, axis_NumVertices, GL_LINES);  // draw the axes

    glutSwapBuffers();
}
//---------------------------------------------------------------------------
// Update angle to control rotation and translation animation
void idle (void) {
    angle += 5.0f;    //YJC: change this value to adjust the cube rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
// ‘b’ or the ‘B’ key begins rolling the sphere
// ‘x’ and the ‘X’ keys respectively decrease and increase the viewer x-coordinate by 1.0
// and similarly for the viewer y- and z-coordinates (with ‘y’, ‘Y’, ‘z’ and ‘Z’ keys)
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
    case 'b': case 'B':
        glutIdleFunc(idle); // Begin rolling
        animationFlag = 1;  
        rolling_begin = true;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
// Right click to stop and start rolling the sphere
// Only works after b/B is pressed
void myMouse(int button, int state, int x, int y){
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        if(rolling_begin && animationFlag == 1){
            animationFlag = 0;
            glutIdleFunc(NULL);
        } else if (rolling_begin && animationFlag == 0) {
            animationFlag = 1;
            glutIdleFunc(idle);
        }
	}
}
//----------------------------------------------------------------------------
// Create menu for Default View Point and Quit
void myMenu(int id){
	switch(id){
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
    glutCreateWindow("Assignment2");

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
    readFile();
    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL); // Initially, show the sphere standing still
    glutKeyboardFunc(keyboard);
	glutMouseFunc(myMouse);
    int menu_ID;
    menu_ID = glutCreateMenu(myMenu);
    // glutSetMenuFont(menu_ID, GLUT_BITMAP_HELVETICA_18);  // Fails to set font in Apple M1 and generates below error:
                                                            // Undefined symbols for architecture arm64:
                                                            // "_glutSetMenuFont", referenced from:
                                                            // _main in rotate-sphere.cpp.o
                                                            // ld: symbol(s) not found for architecture arm64
                                                            // clang: error: linker command failed with exit code 1 (use -v to see invocation)
	glutAddMenuEntry(" Default View Point ",1);
	glutAddMenuEntry(" Quit ",2);
	glutAttachMenu(GLUT_LEFT_BUTTON);
    init();
    glutMainLoop();
    return 0;
}
