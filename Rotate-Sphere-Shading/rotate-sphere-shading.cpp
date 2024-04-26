// Assignment 3
// zy2298 Zijing Yang
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint sphere_smooth_buffer;
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;  /* vertex buffer object id for axis */
GLuint shadow_buffer;   /* vertex buffer object id for shadow */
GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection;  // projection matrix uniform shader variable location

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
bool wireframe = true;
int lighting = 0; // 1: enable lighting; 0: disable lighting
int lighting_source = 2; // 1: point source; 0: spotlight
int shading_type = 0; // 0: Flat Shading; 1: Smooth Shading
int sphere_NumVertices;
std::vector<point4> sphere_points; 
std::vector<color4> sphere_colors;
std::vector<color4> shadow_colors;
std::vector<vec3> sphere_normals;
std::vector<vec3> sphere_smooth_normals;
float radius = 1.0f;
vec4 points[3] = {vec4(3, 1, 5, 1), vec4(-1, 1, -4, 1), vec4(3.5, 1, -2.5, 1)};
int currSegment = 0;
vec4 currDirection;
vec4 currPosition = points[0];
mat4 M = mat4(1.0); // accumulated rotation 

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
point4 quad_vertices[4] = {
    point4( 5.0f, 0.0f, 8.0f, 1.0f ),
    point4( 5.0f, 0.0f, -4.0f, 1.0f ),
    point4( -5.0f, 0.0f, -4.0f, 1.0f ),
    point4( -5.0f, 0.0f, 8.0f, 1.0f ),
};
vec3 floor_normals[floor_NumVertices];

const int axis_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 axis_points[axis_NumVertices]; // positions for all vertices
color4 axis_colors[axis_NumVertices];

/*----- Shadow Parameters -----*/
point4 light_position( -14.0f, 12.0f, -3.0f, 1.0f ); 
          // In World frame.
          // Needs to transform it to Eye Frame
          // before sending it to the shader(s).

color4 shadow_color( 0.25f, 0.25f, 0.25f, 0.65f );
vec4 plane(0.0f, 1.0f, 0.0f, 0.0f);
mat4 shadow_matrix;
bool shadow = false; 

/*----- Shader Lighting Parameters -----*/
    color4 global_ambient( 1.0f, 1.0f, 1.0f, 1.0f );
    // Settings for Direction Light
    color4 dir_ambient( 0.0f, 0.0f, 0.0f, 1.0f );
    color4 dir_diffuse( 0.8f, 0.8f, 0.8f, 1.0f );
    color4 dir_specular( 0.2f, 0.2f, 0.2f, 1.0f );
    vec4 direction( 0.1f, 0.0f, -1.0f, 0.0f);

    // Settsing for Positional Light
    color4 pos_ambient( 0.0f, 0.0f, 0.0f, 1.0f );
    color4 pos_diffuse( 1.0f, 1.0f, 1.0f, 1.0f );
    color4 pos_specular( 1.0f, 1.0f, 1.0f, 1.0f );
    vec4 pos_light(-14.0f, 12.0f, -3.0f, 1.0f);
    float const_att = 2.0f;
    float linear_att = 0.01f;
    float quad_att = 0.001f;

    // Settings for Spot Light
    vec4 spot_light(-6.0f, 0.0f, -4.5f, 1.0f);
    vec4 spot_direction = normalize(pos_light - spot_light) ;
    float spot_exp = 15.0;
    float cutoff_angle = 20.0 * M_PI / 180.0;

    // Floor Lighting
    color4 floor_ambient( 0.2f, 0.2f, 0.2f, 1.0f );
    color4 floor_diffuse( 0.0f, 1.0f, 0.0f, 1.0f );
    color4 floor_specular( 0.0f, 0.0f, 0.0f, 1.0f );
    
    color4 floor_ambient_product = (global_ambient + dir_ambient) * floor_ambient;;
    color4 floor_diffuse_product = dir_diffuse * floor_diffuse;
    color4 floor_specular_product = dir_specular * floor_specular;
    color4 pos_floor_ambient_product = pos_ambient * floor_ambient;;
    color4 pos_floor_diffuse_product = pos_diffuse * floor_diffuse;
    color4 pos_floor_specular_product = pos_specular * floor_specular;

    // Sphere Lighting
    color4 sphere_ambient( 0.2f, 0.2f, 0.2f, 1.0f );
    color4 sphere_diffuse( 1.0f, 0.84f, 0.0f, 1.0f );
    color4 sphere_specular( 1.0f, 0.84f, 0.0f, 1.0f );
    float sphere_shininess = 125.0f;
    
    color4 sphere_ambient_product = (global_ambient + dir_ambient) * sphere_ambient;
    color4 sphere_diffuse_product = dir_diffuse * sphere_diffuse;
    color4 sphere_specular_product = dir_specular * sphere_specular;
    color4 pos_sphere_ambient_product = pos_ambient * sphere_ambient;
    color4 pos_sphere_diffuse_product = pos_diffuse * sphere_diffuse;
    color4 pos_sphere_specular_product = pos_specular * sphere_specular;
void SetUp_Lighting_Uniform_Vars(bool isSphere, color4 ambient_product, color4 diffuse_product, color4 specular_product);
void SetUp_Pos_Lighting_Uniform_Vars(mat4 mv, color4 pos_ambient_product, color4 pos_diffuse_product, color4 pos_specular_product);


void Get_Shadow_Matrix(point4 light_position) {
    shadow_matrix = mat4(
        light_position.y, 0.0, 0.0, 0.0,
        -light_position.x, 0.0, -light_position.z, -1.0,
        0.0, 0.0, light_position.y, 0.0,
        0.0, 0.0, 0.0, light_position.y); 
    
}

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

    color4 green(0.0f, 1.0f, 0.0f, 1.0f);
    for (int i = 0; i < floor_NumVertices; i++) {
        floor_colors[i] = green;
    }

    // compute its normal
    vec4 u = quad_vertices[1] - quad_vertices[0];
    vec4 v = quad_vertices[3] - quad_vertices[0];

    vec3 normal = normalize( cross(u, v) );
    for (int i = 0; i < floor_NumVertices; i++) {
        floor_normals[i] = normal;
    }
    
}

// generate x, y, z axis
void axis() 
{
    // x-axis
    axis_colors[0] = color4(1, 0, 0, 1); axis_points[0] = point4(0.0, 0.02, 0.0, 1.0);
    axis_colors[1] = color4(1, 0, 0, 1); axis_points[1] = point4(10.0, 0.02, 0.0, 1.0);
    // y-axis
    axis_colors[2] = color4(1, 0, 1, 1); axis_points[2] = point4(0.0, 0.0, 0.0, 1.0);
    axis_colors[3] = color4(1, 0, 1, 1); axis_points[3] = point4(0.0, 10.0, 0.0, 1.0);
    // z-axis
    axis_colors[4] = color4(0, 0, 1, 1); axis_points[4] = point4(0.0, 0.02, 0.0, 1.0);
    axis_colors[5] = color4(0, 0, 1, 1); axis_points[5] = point4(0.0, 0.02, 10.0, 1.0);
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
    shadow_colors.resize(sphere_NumVertices);
    sphere_normals.resize(sphere_NumVertices);
    sphere_smooth_normals.resize(sphere_NumVertices);

    int k = 0;
    for (const auto& triangle : triangles) {

        // Convert Vertex to vec4 and compute normals
        vec4 v0 = vec4(triangle.vertices[0].x, triangle.vertices[0].y, triangle.vertices[0].z, 1.0);
        vec4 v1 = vec4(triangle.vertices[1].x, triangle.vertices[1].y, triangle.vertices[1].z, 1.0);
        vec4 v2 = vec4(triangle.vertices[2].x, triangle.vertices[2].y, triangle.vertices[2].z, 1.0);

        vec4 u = (v1 - v0); 
        vec4 v = (v2 - v0); 
        vec3 normal = normalize(cross(u, v));
        
        for (int i = 0; i < 3; i++) {
            sphere_points[k].x = triangle.vertices[i].x;
            sphere_points[k].y = triangle.vertices[i].y;
            sphere_points[k].z = triangle.vertices[i].z;
            sphere_points[k].w = 1.0;
            sphere_normals[k] = normal;

            vec3 vertex_normal = normalize(vec3(sphere_points[k].x, sphere_points[k].y, sphere_points[k].z));
            sphere_smooth_normals[k] = vertex_normal;

            k++;
        }
    }

    for (int i = 0; i < sphere_NumVertices; i++) {
        sphere_colors[i] = color4(1.0, 0.84, 0, 1.0);
        shadow_colors[i] = shadow_color;
    }

// Set shadow matrix
    Get_Shadow_Matrix(light_position);

// Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(point4) * sphere_points.size() + 
        sizeof(color4) * sphere_colors.size() + 
        sizeof(vec3) * sphere_normals.size(),
		NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
        sizeof(point4) * sphere_points.size(), sphere_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_points.size(), 
        sizeof(color4) * sphere_colors.size(), sphere_colors.data());
    glBufferSubData(GL_ARRAY_BUFFER, 
        sizeof(point4) * sphere_points.size() + sizeof(color4) * sphere_colors.size(),
        sizeof(vec3) * sphere_normals.size(), sphere_normals.data());

// Createand initialize a vertex buffer object for sphere, to be used in smooth shading
    glGenBuffers(1, &sphere_smooth_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_smooth_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(point4) * sphere_points.size() + 
        sizeof(color4) * sphere_colors.size() + 
        sizeof(vec3) * sphere_smooth_normals.size(),
		NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
        sizeof(point4) * sphere_points.size(), sphere_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_points.size(), 
        sizeof(color4) * sphere_colors.size(), sphere_colors.data());
    glBufferSubData(GL_ARRAY_BUFFER, 
        sizeof(point4) * sphere_points.size() + sizeof(color4) * sphere_colors.size(),
        sizeof(vec3) * sphere_smooth_normals.size(), sphere_smooth_normals.data());

// Create and initialize a vertex buffer object for shadow, to be used in display()
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(point4) * sphere_points.size() + sizeof(color4) * sizeof(shadow_colors), 
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
        sizeof(point4) * sphere_points.size(), sphere_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_points.size(), 
        sizeof(color4) * sizeof(shadow_colors), shadow_colors.data());

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
        sizeof(point4) * floor_NumVertices + 
        sizeof(color4) * floor_NumVertices + 
        sizeof(vec3) * floor_NumVertices,
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * floor_NumVertices, floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * floor_NumVertices, 
        sizeof(color4) * floor_NumVertices, floor_colors);
    glBufferSubData(GL_ARRAY_BUFFER, 
        sizeof(point4) * floor_NumVertices + sizeof(color4) * floor_NumVertices, 
        sizeof(vec3) * floor_NumVertices, floor_normals);

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
    program = InitShader("vshader53.glsl", "fshader53.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.529, 0.807, 0.92, 0.0 ); 
    glLineWidth(2.0);
}

// Set up lighting parameters that are uniform variables in shader for part c.
void SetUp_Lighting_Uniform_Vars(bool isSphere, color4 ambient_product, color4 diffuse_product, color4 specular_product) 
{
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );

   // The Light Position in Eye Frame
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
   		  1, direction);

    if (isSphere) {
        glUniform1f(glGetUniformLocation(program, "Shininess"),
		        sphere_shininess );
    } else {
        glUniform1f(glGetUniformLocation(program, "Shininess"),
		        1.0 );
    }

}

// Set up lighting parameters that are uniform variables in shader for part d.
void SetUp_Pos_Lighting_Uniform_Vars(mat4 mv, color4 pos_ambient_product, color4 pos_diffuse_product, color4 pos_specular_product)
{
    glUniform4fv( glGetUniformLocation(program, "PosAmbientProduct"),
		  1, pos_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "PosDiffuseProduct"),
		  1, pos_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "PosSpecularProduct"),
		  1, pos_specular_product );
    
   // The Light Position in Eye Frame
    vec4 light_position_eyeFrame = mv * pos_light;
    glUniform4fv( glGetUniformLocation(program, "SpotLightPosition"),
   		  1, light_position_eyeFrame);
    
    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
		        const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
		        linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
		        quad_att);
    
    mat3 normal_matrix = NormalMatrix(mv, 1);
	mat4 nm = mat4WithUpperLeftMat3(normal_matrix); 
    glUniform4fv(glGetUniformLocation(program, "SpotLightDirection"),
		      1, nm * spot_direction);
    glUniform1f(glGetUniformLocation(program, "SpotExponent"),
		        spot_exp );
    glUniform1f(glGetUniformLocation(program, "SpotCutoff"),
		        cutoff_angle );
    
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
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(sizeof(point4) * num_vertices) ); 

    GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices)); 
      // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(mode, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vNormal);
}
//----------------------------------------------------------------------------
void drawFloor(mat4 mv, mat4 p)
{
    glUniform1i(glGetUniformLocation(program, "Lighting"), lighting);
    glUniform1i(glGetUniformLocation(program, "LightSource"), lighting_source);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (lighting == 1) { // use floor_normals when lighting is enabled
        SetUp_Lighting_Uniform_Vars(false, floor_ambient_product, floor_diffuse_product, floor_specular_product);
        SetUp_Pos_Lighting_Uniform_Vars(mv, pos_floor_ambient_product, pos_floor_diffuse_product, pos_floor_specular_product);
        mat3 normal_matrix = NormalMatrix(mv, 1);
        glUniformMatrix3fv( glGetUniformLocation(program, "Normal_Matrix"),
		  1, GL_TRUE, normal_matrix );
        drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES); // draw the floor
    } else {
        
        drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);  // draw the floor
    }

}
//----------------------------------------------------------------------------
void display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    glUniform1i(glGetUniformLocation(program, "LightSource"), lighting_source);
    glUniform1i(glGetUniformLocation(program, "Lighting"), lighting); 

    model_view = glGetUniformLocation(program, "ModelView" );
    projection = glGetUniformLocation(program, "Projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

    // Translate the center of the sphere to the point A
    mat4 mv = LookAt(eye, at, up);

    if (lighting) {
        
        SetUp_Lighting_Uniform_Vars(true, sphere_ambient_product, sphere_diffuse_product, sphere_specular_product);
        SetUp_Pos_Lighting_Uniform_Vars(mv, pos_sphere_ambient_product, pos_sphere_diffuse_product, pos_sphere_specular_product);
    }

/*----- Set Up the Model-View matrix for the sphere -----*/
    mv = LookAt(eye, at, up) * translate(currSegment, (currSegment + 1) % 3) 
            * rotate(currSegment, (currSegment + 1) % 3) * M;

    if ((currPosition.x < -1 && currPosition.z < -4) || 
           (currPosition.x > 3.5 && currPosition.z > -2.5) || 
           (currPosition.x < 3 && currPosition.z > 5)) {
        M = rotate(currSegment, (currSegment + 1) % 3) * M;
        currSegment = (currSegment + 1) % 3;
        angle = 0.0f;
    }
    
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

    if (wireframe) {
        glUniform1i(glGetUniformLocation(program, "Lighting"), 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
    } else if (lighting == 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
    } else {
        glUniform1i(glGetUniformLocation(program, "Lighting"), lighting);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        mat3 normal_matrix = NormalMatrix(mv, 0);
        glUniformMatrix3fv( glGetUniformLocation(program, "Normal_Matrix"),
		  1, GL_TRUE, normal_matrix );
        
        if (shading_type == 0) {
            drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
        } else {
            drawObj(sphere_smooth_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere
        }
        
    }
    

/*----- Set up the Mode-View matrix for the floor -----*/
    mv = LookAt(eye, at, up);
    glDepthMask(GL_FALSE); // Disable writing to z-buffer
    drawFloor(mv, p); // Draw ground only to frame buffer

    if (shadow && eye[1] > 0) {
        glUniform1i(glGetUniformLocation(program, "Lighting"), 0); // disable lighting/shading
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);
        mv = LookAt(eye, at, up) * shadow_matrix * translate(currSegment, (currSegment + 1) % 3) 
            * rotate(currSegment, (currSegment + 1) % 3) * M;
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

        if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        //glDepthMask(GL_TRUE); // Enable writing to z-buffer
        drawObj(shadow_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the shadow
        glDepthMask(GL_TRUE); // Enable writing to z-buffer
        glUniform1i(glGetUniformLocation(program, "Lighting"), lighting);
    } else {
        glDepthMask(GL_TRUE); // Enable writing to z-buffer
    }

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable writing to the color buffer
    mv = LookAt(eye, at, up);
    drawFloor(mv, p); // Draw ground only to z-buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable writing to the color buffer

    
/*----- Set Up the Model-View matrix for the axis -----*/
    mv = LookAt(eye, at, up);
    // Disable lighting when drawing axes
    glUniform1i(glGetUniformLocation(program, "Lighting"), 0);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); 
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
void mainMenu(int id) 
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
void shadow_menu(int id) 
{
    switch (id)
    {
    case 1:
        shadow = false;
        break;
    case 2:
        shadow = true;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void lighting_menu(int id) 
{
    switch (id)
    {
    case 1:
        lighting = 0;
        shading_type = 0;
        break;
    case 2:
        lighting = 1;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void wireframe_menu(int id) 
{
    switch (id)
    {
    case 1:
        wireframe = false;
        break;
    case 2:
        wireframe = true;
        break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void shading_menu(int id) 
{
    switch (id)
    {
    case 1:
        shading_type = 0; // Flat Shading
        break;
    case 2:
        shading_type = 1; // Smooth Shading
        break;
    }
    wireframe = false;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void light_souce_menu(int id)
{
    switch (id)
    {
    case 1:
        lighting_source = 0; // spotlight
        break;
    case 2:
        lighting_source = 1; // Point Source
        break;
    }

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void myMenu()
{
    GLuint shadow_sub = glutCreateMenu(shadow_menu);
    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    GLuint lighting_sub = glutCreateMenu(lighting_menu);
    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    GLuint wireframe_sub = glutCreateMenu(wireframe_menu);
    glutAddMenuEntry(" No ", 1);
    glutAddMenuEntry(" Yes ", 2);

    GLuint shading_sub = glutCreateMenu(shading_menu);
    glutAddMenuEntry(" Flat Shading ", 1);
    glutAddMenuEntry(" Smooth Shading ", 2);

    GLuint lighting_source_sub = glutCreateMenu(light_souce_menu);
    glutAddMenuEntry(" Spot Light ", 1);
    glutAddMenuEntry(" Point Source ", 2);

    // set menu
    int menu_ID;
    menu_ID = glutCreateMenu(mainMenu);
    //glutSetMenuFont(menu_ID, GLUT_BITMAP_HELVETICA_18);
    glutAddSubMenu(" Shadow ", shadow_sub);
    glutAddSubMenu(" Enable Lighting ", lighting_sub);
    glutAddSubMenu(" Wire Frame Sphere ", wireframe_sub);
    glutAddSubMenu(" Shading ", shading_sub);
    glutAddSubMenu(" Light Source ", lighting_source_sub);

    glutAddMenuEntry(" Default View Point ", 1);
    glutAddMenuEntry(" Quit ", 2);
    glutAttachMenu(GLUT_LEFT_BUTTON);
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
    glutCreateWindow("Sphere Shading");

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
    myMenu();
    
    init();
    glutMainLoop();
    return 0;
}




