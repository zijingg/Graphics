// CS-GY-6533 Assignment4
// Ahhyun Moon (am12180)
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath> 
#define MAX_NUM_TRI     1024
#define NUM_PARTICLES   300
#define T_MAX           20
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec4  point3;
GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;         /* shader program object id */
GLuint fireworks_program;         /* shader program object id */
GLuint floor_buffer;    /* vertex buffer object id for floor */
GLuint axis_buffer;     /* vertex buffer object id for axis */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint shadow_buffer;   /* vertex buffer object id for shadow */
GLuint model_view;  // model-view matrix uniform shader variable location
GLuint projection;  // projection matrix uniform shader variable location
GLuint fireworks_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;   // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;        // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 30.0;
GLfloat angle = 0.0;    // Angle to control rotation/translation
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;    // current viewer position
vec4  at(0, 0, 0, 1.0); // at = VRP/eye (7,3,-10,1) + VPN (-7.-3,10,0)
vec4  up(0.0, 1.0, 0.0, 0.0);

// Floor variables
const int floor_NumVertices = 6;        //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
point4 floor_vertices[4] = {
    point4( 5.0, 0.0,  8.0, 1.0),   // down R
    point4( 5.0, 0.0, -4.0, 1.0),   // top R
    point4(-5.0, 0.0, -4.0, 1.0),   // top L
    point4(-5.0, 0.0,  8.0, 1.0)    // down L
};
vec3 floor_normals[floor_NumVertices];
vec2 quad_texCoord[floor_NumVertices];
int ground_tex = 1;

// Axis variables
const int axis_NumVertices = 6; // x, y, z axis, each having 2 vertices
point4 axis_points[axis_NumVertices];
color4 axis_colors[axis_NumVertices];

// Sphere variables
int animationFlag = 0;  // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'
int sphere_NumVertices; // Will be initialized in readFile()
int radius = 1;
point4 *sphere_points = new point4[MAX_NUM_TRI * 3];
vec4 move_to;       // Variable for translation 
mat4 M = mat4();    // Variable for accumulated rotation matrix
point4 s_start = point4(-4,1,4,1);  // First segment start point: A
point4 s_end = point4(-1,1,-4,1);   // First segment end point: B
bool rolling_begin = false;
bool wireframe = false;
int shading_type = 1;   //1: smooth, 0: flat
int blending = 1; //1: blending, 0: no blending
int sphereTex = 1; // 1: vertical 2: slanted 0: none
int frameMode = 1; // 
int sphereCB = 0; // 1: Sphere is checkerboard 0: otherwise
int latticeMode = 0;

// Shadow Variables
point4 light_source(-14.0, 12.0, -3.0, 1.0);
color4 shadow_color(0.25, 0.25, 0.25, 0.65);
vec4 plane(0.0, 1.0, 0.0, 0.0);
bool shadow = true;
mat4 shadow_matrix;
// Function to prepare shadow matrix
void getShadowMatrix(point4 light_source, vec4 plane){
    shadow_matrix = mat4();
    shadow_matrix[3][1] = -1.0/light_source[1];
    shadow_matrix[3][3] = 0;  
    // Translate to origin -> perspective(shadow) projection through the origin -> translate to light source
    shadow_matrix = Translate(light_source.x,light_source.y,light_source.z) 
    * shadow_matrix * Translate(-light_source.x,-light_source.y,-light_source.z);
}

// Lighting Variables
int lighting_on = 1;    //1: true, 0: false
int light_src = 1;      //1: point source, 0: spot light
// Settings for directional light
color4 global_ambient(1.0, 1.0, 1.0, 1.0);
color4 dist_light(0.0, 0.0, 0.0, 1.0);
color4 diffuse(0.8, 0.8, 0.8, 1.0);
color4 specular(0.2, 0.2, 0.2, 1.0);
vec4 direction(0.1, 0.0, -1.0, 0.0); // in eye coord
// Settings for positional light
color4 pos_ambient(0.0, 0.0, 0.0, 1.0);
color4 pos_diffuse(1.0, 1.0, 1.0, 1.0);
color4 pos_specular(1.0, 1.0, 1.0, 1.0);
vec4 pos_light(-14.0, 12.0, -3.0, 1.0); // in world coord
float const_att = 2.0f;
float linear_att = 0.01f;
float quad_att = 0.001f;
// Settings for spotlight
vec4 spot_light_dir(-6.0, 0.0, -4.5, 1.0); // in world coord
float spot_light_exp = 15.0;
float spot_light_ang = 20.0 / 180 * M_PI; // angle in radian
// Floor lighting 
color4 floor_diffuse(0.0, 1.0, 0.0, 1.0);
color4 floor_ambient(0.2, 0.2, 0.2, 1.0);
color4 floor_specular(0.0, 0.0, 0.0, 1.0);
color4 floor_ambient_product = (global_ambient + dist_light) * floor_ambient;
color4 floor_diffuse_product = diffuse * floor_diffuse;
color4 floor_specular_product = specular * floor_specular;
color4 floor_ambient_product_pos = pos_ambient * floor_ambient;
color4 floor_diffuse_product_pos = pos_diffuse * floor_diffuse;
color4 floor_specular_product_pos = pos_specular * floor_specular;
// Sphere lighting 
color4 sphere_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_ambient(0.2, 0.2, 0.2, 1.0);
color4 sphere_specular(1.0, 0.84, 0.0, 1.0);
float sphere_shininess = 125.0;
color4 sphere_ambient_product = (global_ambient + dist_light) * sphere_ambient;
color4 sphere_diffuse_product = diffuse * sphere_diffuse;
color4 sphere_specular_product = specular * sphere_specular;
color4 sphere_ambient_product_pos = pos_ambient * sphere_ambient;
color4 sphere_diffuse_product_pos = pos_diffuse * sphere_diffuse;
color4 sphere_specular_product_pos = pos_specular * sphere_specular;

// Function to setup lighting uniform variables for part c
void SetUp_Lighting_Uniform_Vars(bool isSphere, color4 ambient_product, color4 diffuse_product, color4 specular_product){    
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, specular_product );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, direction); // Already in Eye Frame
    if (isSphere) glUniform1f(glGetUniformLocation(program, "Shininess"), sphere_shininess );
    else glUniform1f(glGetUniformLocation(program, "Shininess"), 1.0 );
}

// Function to setup lighting uniform variables for part d
void SetUp_Pos_Lighting_Uniform_Vars(mat4 mv, color4 pos_ambient_product, color4 pos_diffuse_product, color4 pos_specular_product){ 
    glUniform4fv( glGetUniformLocation(program, "PosAmbientProduct"), 1, pos_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "PosDiffuseProduct"), 1, pos_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "PosSpecularProduct"), 1, pos_specular_product );  
    // Convert light position & direction to eye frame
    glUniform4fv( glGetUniformLocation(program, "SpotLightPosition"), 1,  mv * pos_light); 
    mat3 normal_matrix = NormalMatrix(mv, 1);
	mat4 nm = mat4WithUpperLeftMat3(normal_matrix); 
    glUniform4fv( glGetUniformLocation(program, "SpotLightDirection"), 1, nm * spot_light_dir);
    glUniform1f( glGetUniformLocation(program, "SpotLightExponent"), spot_light_exp);
    glUniform1f( glGetUniformLocation(program, "SpotLightAngle"), spot_light_ang);
    glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
}
// Fireworks Variables
struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float     Life;
  
    Particle() 
      : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};    
float timespan; // Get current time
int firework = 0;
vec4 p_velocity[NUM_PARTICLES];
vec4 p_color[NUM_PARTICLES];
vec4 init_pos[NUM_PARTICLES];
float t_sub;
float t = 0;
int i = 0;
float findMod(float a, float b){
    float mod;
    // Handling negative values
    if (a < 0) mod = -a;
    else mod =  a;
    if (b < 0) b = -b;
    // Finding mod by repeated subtraction
    while (mod >= b) mod = mod - b;
    return mod;
}
float getTime(){
    t_sub = (float)glutGet(GLUT_ELAPSED_TIME);
    t = t - t_sub;
    return fmod(t,T_MAX); 

}
void setFirework(){
	for(int i=0; i < NUM_PARTICLES; i++){
		p_velocity[i].x = 2.0 * ((rand()%256) / 256.0 - 0.5);
		p_velocity[i].y = 1.2 * 2.0 * ((rand()%256) / 256.0);
		p_velocity[i].z = 2.0 * ((rand()%256) / 256.0 - 0.5);
		p_velocity[i].w = 0;
		p_color[i].x = (rand()%256)/256.0;
		p_color[i].y = (rand()%256)/256.0;
		p_color[i].z = (rand()%256)/256.0;
		p_color[i].w = 1;
        init_pos[i] = vec4(0.0, 0.1, 0.0, 1);
	}
}
// Fog Variables
int fog = 0; // Fog Option
float linear_start = 0.0;
float linear_end = 18.0;
float exp_density = 0.09;
color4 fog_color(0.7, 0.7, 0.7, 0.5);

// Function to setup fog variables in f-shader
void SetUp_fog_Uniform_Vars(){    
    glUniform1f( glGetUniformLocation(program, "linear_start"), linear_start);
    glUniform1f( glGetUniformLocation(program, "linear_end"), linear_end);
    glUniform1f( glGetUniformLocation(program, "fog_density"), exp_density);
    glUniform4fv( glGetUniformLocation(program, "fog_color"), 1, fog_color); 
}
#define ImageWidth  32
#define ImageHeight 32
#define	stripeImageWidth 32
GLubyte Image[ImageHeight][ImageWidth][4];
GLubyte stripeImage[4*stripeImageWidth];
// static GLuint textureName[2];
GLuint textureName1,textureName2;
/*************************************************************
void image_set_up(void):
  generate checkerboard and stripe images. 

* Inside init(), call this function and set up texture objects
  for texture mapping.
  (init() is called from main() before calling glutMainLoop().)
***************************************************************/
void image_set_up(void)
{
 int i, j, c; 
 
 /* --- Generate checkerboard image to the image array ---*/
  for (i = 0; i < ImageHeight; i++)
    for (j = 0; j < ImageWidth; j++)
      {
       c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

       if (c == 1) /* white */
	{
         c = 255;  
	 Image[i][j][0] = (GLubyte) c;
         Image[i][j][1] = (GLubyte) c;
         Image[i][j][2] = (GLubyte) c;
	}
       else  /* green */
	{
         Image[i][j][0] = (GLubyte) 0;
         Image[i][j][1] = (GLubyte) 150;
         Image[i][j][2] = (GLubyte) 0;
	}

       Image[i][j][3] = (GLubyte) 255;
      }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

/*--- Generate 1D stripe image to array stripeImage[] ---*/
  for (j = 0; j < stripeImageWidth; j++) {
     /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
        When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
      */
    stripeImage[4*j] = (GLubyte)    255;
    stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
    stripeImage[4*j+2] = (GLubyte) 0; 
    stripeImage[4*j+3] = (GLubyte) 255;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
/*----------- End 1D stripe image ----------------*/

/*--- texture mapping set-up is to be done in 
      init() (set up texture objects),
      display() (activate the texture object to be used, etc.)
      and in shaders.
 ---*/

} /* end function */

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
    int triangles;
    // Read first value for number of triangles
    file >> triangles;
    sphere_NumVertices = triangles * 3;
    int v;
    float x,y,z;
    // Read x,y,r of each row and store them in vectors
    for (int i = 0; i < triangles; i++){
        file >> v;
        for (int j = 0; j < v; j++){   
            file >> x >> y >> z;
            sphere_points[i*3+j] = point4(x,y,z,1.0);
        }
    }

}
//----------------------------------------------------------------------------
// Generate rotation axis based on start and end points and return rotation matrix
mat4 rolling(point4 start, point4 end){
    vec4 direction = end - start; 
    vec4 rAxis = cross(vec4(0,1,0,0), direction);
    return Rotate(angle, rAxis.x, rAxis.y, rAxis.z);
}
//----------------------------------------------------------------------------
// Take start and end points of a segment and return translation matrix
mat4 rolling_move(point4 start, point4 end){
    vec4 direction = end - start;
    vec4 start_point = start - point4(0,0,0,1);
    move_to = start_point + angle * (2 * M_PI * radius / 360) * normalize(direction);
    return Translate(move_to.x, move_to.y, move_to.z);
}
//----------------------------------------------------------------------------
// Generate 2 triangles: 6 vertices and 6 colors
void floor() {
    floor_colors[0] = color4(0,1,0,1); floor_points[0] = floor_vertices[1];
    floor_colors[1] = color4(0,1,0,1); floor_points[1] = floor_vertices[2];
    floor_colors[2] = color4(0,1,0,1); floor_points[2] = floor_vertices[3];
    floor_colors[3] = color4(0,1,0,1); floor_points[3] = floor_vertices[3];
    floor_colors[4] = color4(0,1,0,1); floor_points[4] = floor_vertices[0];
    floor_colors[5] = color4(0,1,0,1); floor_points[5] = floor_vertices[1];

    vec4 u = floor_vertices[2] - floor_vertices[1];
    vec4 v = floor_vertices[0] - floor_vertices[1];
    vec3 normal = normalize( cross(u, v) );
	for(int i=0; i<floor_NumVertices; i++){
		floor_normals[i] = normal;
	}
	quad_texCoord[0] = vec2(1.0,1.0); // 24 X 20 = 6 X 5 in v-shader
	quad_texCoord[1] = vec2(1.0,0.0);
	quad_texCoord[2] = vec2(0.0,0.0);
	quad_texCoord[3] = vec2(0.0,0.0);
	quad_texCoord[4] = vec2(0.0,1.0);
	quad_texCoord[0] = vec2(1.0,1.0);
}
//----------------------------------------------------------------------------
// Generate x,y,z axis
void axes() {
	axis_colors[0] = color4(1,0,0,1); axis_points[0] = point4(0.0,0.02,0.0,1.0);	//x-axis
	axis_colors[1] = color4(1,0,0,1); axis_points[1] = point4(10.0,0.02,0.0,1.0);	//x-axis
	axis_colors[2] = color4(1,0,1,1); axis_points[2] = point4(0.0,0.0,0.0,1.0);	    //y-axis
	axis_colors[3] = color4(1,0,1,1); axis_points[3] = point4(0.0,10.0,0.0,1.0);	//y-axis
	axis_colors[4] = color4(0,0,1,1); axis_points[4] = point4(0.0,0.02,0.0,1.0);	//z-axis
	axis_colors[5] = color4(0,0,1,1); axis_points[5] = point4(0.0,0.02,10.0,1.0);	//z-axis
}
//----------------------------------------------------------------------------
// OpenGL initialization
void init(){
    // Texture Mapping
    image_set_up();
    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &textureName1);      // Generate texture obj name(s)
    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0 
    glBindTexture(GL_TEXTURE_2D, textureName1); // Bind the texture to this texture unit
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
	/*--- Create and Initialize a texture for strip ---*/
	glGenTextures(1, &textureName2);      // Generate texture obj name(s)
	glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 1 
	glBindTexture(GL_TEXTURE_1D, textureName2); // Bind the texture to this texture unit
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
    // Set sphere normals 
    vec3* sphere_normals = new vec3[sphere_NumVertices];
    for (int n = 0; n < sphere_NumVertices / 3; n++){
        point4 u = sphere_points[n*3+1] - sphere_points[n*3];
        point4 v = sphere_points[n*3+2] - sphere_points[n*3+1];
        vec3 normal = cross(u,v);
        for (int m = 0; m < 3; m++){
            sphere_normals[n*3+m] = normalize(normal);
        }
    }
    // Set sphere color & shadow color
    color4* sphere_colors = new color4[sphere_NumVertices];
    color4* shadow_colors = new color4[sphere_NumVertices];
    for (int s = 0; s < sphere_NumVertices; s++){
        sphere_colors[s] = color4(1.0, 0.84, 0.0, 1.0);
        shadow_colors[s] = shadow_color;
    }
    // Set shadow matrix
    getShadowMatrix(light_source, plane);
    // Create and initialize a vertex buffer object for sphere with colors, to be used in display()
    //// Updated for sphere normals
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices + sizeof(sphere_colors) + sizeof(sphere_normals), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices, sizeof(sphere_colors) , sphere_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices + sizeof(sphere_colors) , sizeof(sphere_normals), sphere_normals);
    // Create and initialize a vertex buffer object for shadow, to be used in display()
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices + sizeof(shadow_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*sphere_NumVertices, sizeof(shadow_colors), shadow_colors);
    // Create and initialize a vertex buffer object for floor, to be used in display()
    //// Updated for floor normals
    floor();     
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors)  + sizeof(floor_normals) , NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_normals) , floor_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals), sizeof(quad_texCoord), quad_texCoord);    
    // Create and initialize a vertex buffer object for axes, to be used in display()
    axes();
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);
    // Create and initialize a vertex buffer object for fireworks, to be used in display()
    setFirework();
    glGenBuffers(1, &fireworks_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, fireworks_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(init_pos) + sizeof(p_color) + sizeof(p_velocity), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(init_pos), init_pos);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(init_pos), sizeof(p_color), p_color);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(init_pos) + sizeof(p_color), sizeof(p_velocity), p_velocity);
    // Load shaders and create a shader program (to be used in display())
    program = InitShader("../vshader53.glsl", "../fshader53.glsl");
    fireworks_program =  InitShader("../vFireworks.glsl", "../fFireworks.glsl");
    glClearColor(0.529, 0.807, 0.92, 0.0); 
    glLineWidth(2.0);
}
void drawFirework(){
	glPointSize(3.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
    glBindBuffer(GL_ARRAY_BUFFER, fireworks_buffer);
    GLuint vPosition = glGetAttribLocation(fireworks_program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	GLuint vColor = glGetAttribLocation(fireworks_program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4) * NUM_PARTICLES)); 
	GLuint vVelocity = glGetAttribLocation(fireworks_program, "vVelocity");
	glEnableVertexAttribArray(vVelocity);
	glVertexAttribPointer(vVelocity, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4) * NUM_PARTICLES * 2)); 
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vVelocity);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vPosition);
}
//----------------------------------------------------------------------------
//   Draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//   Use color attribute
void drawObj(GLuint buffer, int num_vertices, GLuint drawMode){
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices) ); 
    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices * 2)); 
    glDrawArrays(drawMode, 0, num_vertices);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
    glDisableVertexAttribArray(vTexCoord);
}

//----------------------------------------------------------------------------
//   Draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//   Use normals attribute
void drawNormal(GLuint buffer, int num_vertices, GLuint drawMode){
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    GLuint vNormal = glGetAttribLocation(program, "vNormal"); 
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( (sizeof(point4) * num_vertices))); 
    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * num_vertices * 2)); 
    glDrawArrays(drawMode, 0, num_vertices);
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
    glDisableVertexAttribArray(vTexCoord);
}
//----------------------------------------------------------------------------
//  Draw the floor: 
//  use drawNormal when lighting is enabled
//  use drawObj when disabled
void drawFloor(mat4 mv, mat4 p){
    glUniform1i( glGetUniformLocation(program, "tex_mode"), ground_tex);
    glUniform1i(glGetUniformLocation(program, "fog_mode"), fog );
    glUniform1i(glGetUniformLocation(program, "ShadingType"), 0); 
    glUniform1i(glGetUniformLocation(program, "LightingOn"), lighting_on); 
    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (lighting_on == 1){
        SetUp_Lighting_Uniform_Vars(false, floor_ambient_product, floor_diffuse_product, floor_specular_product);
        SetUp_Pos_Lighting_Uniform_Vars(mv, floor_ambient_product_pos, floor_diffuse_product_pos, floor_specular_product_pos);
        mat3 normal_matrix = NormalMatrix(mv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
        drawNormal(floor_buffer, floor_NumVertices, GL_TRIANGLES);
    } else if (lighting_on == 0){
        drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);
    }
    glUniform1i( glGetUniformLocation(program, "tex_mode"), 0);
    glUniform1i(glGetUniformLocation(program, "ShadingType"), shading_type); 
    glEnable(GL_DEPTH_TEST);

}
//----------------------------------------------------------------------------
void display() {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram(program); // Use the shader program
    // Pass current lighting, shading, light source settings to shader
    glUniform1i(glGetUniformLocation(program, "texture_2D"), 0 );
    glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
    glUniform1i(glGetUniformLocation(program, "lattice_mode"), latticeMode );
    glUniform1i(glGetUniformLocation(program, "frame_mode"), frameMode );
    glUniform1i(glGetUniformLocation(program, "sphereCB"), sphereCB );
    glUniform1i(glGetUniformLocation(program, "fog_mode"), fog );
    glUniform1i(glGetUniformLocation(program, "LightingOn"), lighting_on);
    glUniform1i(glGetUniformLocation(program, "ShadingType"), shading_type);
    glUniform1i(glGetUniformLocation(program, "LightSource"), light_src);
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    vec4  at(0, 0, 0, 1.0); // at = VRP/eye (7,3,-10,1) + VPN (-7.-3,10,0)
    vec4  up(0.0, 1.0, 0.0, 0.0);
    /*----- Draw the sphere -----*/
    // Translate the center of the sphere to the point A = (−4, 1, 4, 1)
    mat4  mv = LookAt(eye, at, up) * Translate(-4.0, 1.0, 4.0); 
    if (lighting_on == 1){
        SetUp_Lighting_Uniform_Vars(true, sphere_ambient_product, sphere_diffuse_product, sphere_specular_product);
        SetUp_Pos_Lighting_Uniform_Vars(mv, sphere_ambient_product_pos, sphere_diffuse_product_pos, sphere_specular_product_pos);
        SetUp_fog_Uniform_Vars();
    }
    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );
    glUniform1i( glGetUniformLocation(program, "sphereTex"), sphereTex);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    // Roll the sphere on the x-z plane by using translations and rotations
    // First rolling - center goes from the point A = (−4, 1, 4, 1) to the point B = (−1, 1, −4, 1)
    mv =  LookAt(eye, at, up) * rolling_move(s_start, s_end) * rolling(s_start,s_end) * M; 
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    // Center passes through B
    // Roll the sphere so that its center goes from the point B to the point C = (3, 1, 5, 1)
    if (move_to.x >= -1 && move_to.z <= -4){
        M = rolling(s_start,s_end);
        s_start = point4(-1 ,1,-4,1);
        s_end = point4(3,1,5,1);
        angle = 1;
    }
    // Center passes through C
    // Roll the sphere so that its center goes from the point B to the point C = (3, 1, 5, 1)
    if (move_to.x >= 3 && move_to.z >= 5){
        M = rolling(s_start,s_end);
        s_start = point4(3,1,5,1);
        s_end = point4(-4,1,4,1);
        angle = 1;
    }
    // Center passes through A
    // Roll the sphere so that its center goes from the point A to the point B
    if (move_to.x <= -4 && move_to.z <= 4){
        M = rolling(s_start,s_end);
        s_start = point4(-4,1,4,1);
        s_end = point4(-1 ,1,-4,1);
        angle = 1;
    }
    // Change fill/line drawing mode based on current setting
    if (wireframe){
        glUniform1i(glGetUniformLocation(program, "LightingOn"), 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);
    } else if (lighting_on == 0) {
        glUniform1i(glGetUniformLocation(program, "LightingOn"), 0); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);
    } else {
        glUniform1i(glGetUniformLocation(program, "LightingOn"), lighting_on);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        mat3 normal_matrix = NormalMatrix(mv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix );
        drawNormal(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);
    }
    glUniform1i( glGetUniformLocation(program, "sphereTex"), 0);
    glUniform1i(glGetUniformLocation(program, "sphereCB"), 0);
    glUniform1i(glGetUniformLocation(program, "lattice_mode"), 0);

    /*----- Draw the floor -----*/
    mv = LookAt(eye, at, up);   // Set mv to center
    glDepthMask(GL_FALSE);      // Disable writing to z-buffer
    drawFloor(mv, p);           // Draw ground only to frame buffer
    /*----- Draw the shadow only when eye frame y > 0 -----*/
    if (shadow && eye[1] > 0) {
        if(blending == 1){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } 
        glUniform1i(glGetUniformLocation(program, "LightingOn"), 2); // Turn off lighting for shadow
        model_view = glGetUniformLocation(program, "model_view" );
        projection = glGetUniformLocation(program, "projection" );
        glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
        mv = LookAt(eye, at, up);
        // Apply shadow matrix to rolling sphere(shadow)
        mv =  mv * shadow_matrix * rolling_move(s_start, s_end) * rolling(s_start,s_end) * M;
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        // Set drawing mode based on wireframe mode on/off
        if (wireframe == true){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        if (blending == 0){
            glDepthMask(GL_TRUE);   // Enable writing to z-buffer
            drawObj(shadow_buffer, sphere_NumVertices, GL_TRIANGLES);  // Draw the sphere shadow
        } else if (blending == 1){
            drawObj(shadow_buffer, sphere_NumVertices, GL_TRIANGLES);  // Draw the sphere shadow
            glDepthMask(GL_TRUE);   // Enable writing to z-buffer
            glDisable(GL_BLEND);
        } 
        glUniform1i(glGetUniformLocation(program, "LightingOn"), lighting_on); 
    } else {
        glDepthMask(GL_TRUE);   // Enable writing to z-buffer
    }
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);    // Disable writing to frame buffer
    /*----- Draw the floor -----*/
    mv = LookAt(eye, at, up); 
    drawFloor(mv, p);   // Draw floor only to z-buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    // Enable writing to frame buffer
    /*----- Draw x,y,z axis -----*/
    glUniform1i(glGetUniformLocation(program, "LightingOn"), 0); // Turn off lighting for axes
    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );
    glUniformMatrix4fv(projection, 1, GL_TRUE, p);
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, axis_NumVertices, GL_LINES);  // draw the axes
    /*----- Draw Fireworks -----*/  
    if (firework == 1){
        glUseProgram(fireworks_program); // Use the shader program
        if (i == NUM_PARTICLES) i = 0;
        timespan = getTime();
        std::cout << timespan << std::endl;
        std::cout << p_velocity[i++] << std::endl;
        vec4 newPos(0,0.1,0,1);
        newPos.x += 0.001 * p_velocity[i].x * t;
        newPos.y += 0.001 * p_velocity[i].y * t + 0.5 * -0.00000049 * t * t; 
        newPos.z += 0.001 * p_velocity[i].x * t;
        Translate(newPos.x, newPos.y, newPos.z) * vec4(0,0.1,0,1);
        glUniform4fv(glGetUniformLocation(fireworks_program, "pos"),1, newPos); // Get t mod t max
        glUniform1f(glGetUniformLocation(fireworks_program, "t"), timespan); // Get t mod t max
        drawFirework();
    }
    glutSwapBuffers();
}
//---------------------------------------------------------------------------
// Update angle to control rotation and translation animation
void idle (void) {
    angle += 5.0f;    //YJC: change this value to adjust the rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
// ‘b’ or the ‘B’ key begins rolling the sphere
// ‘x’ and the ‘X’ keys respectively decrease and increase the viewer x-coordinate by 1.0
// and similarly for the viewer y- and z-coordinates (with ‘y’, ‘Y’, ‘z’ and ‘Z’ keys)
void keyboard(unsigned char key, int x, int y) {
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
    case 'V': case 'v': 
        sphereTex = 1;
        break;
    case 's': case 'S': 
        sphereTex = 2;
        break;
    case 'o': case 'O': 
        frameMode = 1;
        break;
    case 'e': case 'E': 
        frameMode = 2;
        break;
    case 'u': case 'U': 
        latticeMode = 1; // Upright
        break;
    case 't': case 'T': 
        latticeMode = 2; // Tilted
        break;
    case 'l': case 'L': 
        latticeMode = 0; // No lattice
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
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
// Create menu for Default View Point and Quit
void main_menu(int id){
	switch(id){
	case 1:
		eye = init_eye; 
		break;
	case 2:
		exit( EXIT_SUCCESS );
		break;
    case 3: 
        if (wireframe) {
            wireframe = false;
        } else {
            wireframe = true;
        }
        break;
	}
	glutPostRedisplay();
}
//----------------------------------------------------------------------------
// Create submenus
void shadow_menu(int id){
	switch(id){
	case 1:
		shadow = false;
        break;
	case 2:
		shadow = true;
        break;
	}
    glutPostRedisplay();
}
void lighting_menu(int id){
	switch(id){
	case 1:
		lighting_on = 0;
        shading_type = 0;
        break;
	case 2:
		lighting_on = 1;
        break;
	}
    glutPostRedisplay();
}

void wireframe_menu(int id){
	switch(id){
	case 1:
		wireframe = false;
        break;
	case 2:
		wireframe = true;
        break;
	}
    glutPostRedisplay();
}
void shading_menu(int id){
	switch(id){
	case 1:
		shading_type = 0;  // Flat Shading
        break;
	case 2:
		shading_type = 1;   // Smooth Shading
        break;
	}
    wireframe = false;
    glutPostRedisplay();
}
void light_src_menu(int id){
	switch(id){
	    case 1: light_src = 0; break;  // Spot Light 
	    case 2: light_src = 1; break;  // Point Source
	}
    glutPostRedisplay();
}
void fog_menu(int id){
	switch(id){ 
        case 1: fog = 0; break; // No fog break;
	    case 2: fog = 1; break;   // Linear fog
	    case 3: fog = 2; break;   // Exp fog   
    	case 4: fog = 3; break;  // Exp Sqr fog
	}
    glutPostRedisplay();
}
void blending_menu(int id){
	switch(id){
	    case 1: blending = 1; break; // blending
	    case 2: blending = 0; break; // No blending
	}
    glutPostRedisplay();
}
void ground_tex_menu(int id){
	switch(id){
	    case 1: ground_tex = 1; break; // Texture On
	    case 2: ground_tex = 0; break;  // Off
	}
    glutPostRedisplay();
}
void sphere_tex_menu(int id){
	switch(id){
	    case 1: 
            sphereTex = 0; 
            sphereCB = 0;
            break; // No
	    case 2:
            sphereTex = 1;
		    sphereCB = 0;   // Yes - Contour Lines
            break;
	    case 3: sphereCB = 1; break;  // Yes - Checkerboard 
	}
    glutPostRedisplay();
}
void firework_menu(int id){
	switch(id){
	    case 1: 
            firework = 0; 
            break; // No
	    case 2:
            firework = 1;
            break;
	}
    glutPostRedisplay();
}

// Put together the menu
void menu(){
    GLuint shadow_sub = glutCreateMenu(shadow_menu);
	glutAddMenuEntry(" No ",1);
	glutAddMenuEntry(" Yes ",2);
    GLuint lighting_sub = glutCreateMenu(lighting_menu);
	glutAddMenuEntry(" No ",1);
	glutAddMenuEntry(" Yes ",2);
    GLuint wireframe_sub = glutCreateMenu(wireframe_menu);
	glutAddMenuEntry(" No ",1);
	glutAddMenuEntry(" Yes ",2);
    GLuint shading_sub = glutCreateMenu(shading_menu);
	glutAddMenuEntry(" Flat Shading ",1);
	glutAddMenuEntry(" Smooth Shading ",2);
    GLuint light_src_sub = glutCreateMenu(light_src_menu);
	glutAddMenuEntry(" Spot Light ",1);
	glutAddMenuEntry(" Point Source ",2);
    GLuint fog_sub = glutCreateMenu(fog_menu);
	glutAddMenuEntry(" No Fog ",1);
	glutAddMenuEntry(" Linear  ",2);
	glutAddMenuEntry(" Exponential  ",3);
	glutAddMenuEntry(" Exponential Square ",4);
    GLuint blending_sub = glutCreateMenu(blending_menu);
	glutAddMenuEntry(" Yes ",1);
	glutAddMenuEntry(" No ",2);
    GLuint ground_tex_sub = glutCreateMenu(ground_tex_menu);
	glutAddMenuEntry(" Yes ",1);
	glutAddMenuEntry(" No ",2);
    GLuint sphere_tex_sub = glutCreateMenu(sphere_tex_menu);
	glutAddMenuEntry(" No ",1);
	glutAddMenuEntry(" Yes - Contour Lines ",2);
	glutAddMenuEntry(" Yes - Checkerboard ",3);
    GLuint firework_sub = glutCreateMenu(firework_menu);
	glutAddMenuEntry(" No ",1);
	glutAddMenuEntry(" Yes ",2);

    int menu_ID = glutCreateMenu(main_menu);
    // glutSetMenuFont(menu_ID, GLUT_BITMAP_HELVETICA_18);  // Fails to set font in Apple M1 and generates below error:
                                                            // Undefined symbols for architecture arm64:
                                                            // "_glutSetMenuFont", referenced from:
                                                            // _main in rotate-sphere.cpp.o
                                                            // ld: symbol(s) not found for architecture arm64
                                                            // clang: error: linker command failed with exit code 1 (use -v to see invocation)
    glutAddSubMenu(" Shadow ", shadow_sub);
    glutAddSubMenu(" Enable Lighting ", lighting_sub);
    glutAddSubMenu(" Wire Frame Sphere ", wireframe_sub);
    glutAddSubMenu(" Shading ", shading_sub);
    glutAddSubMenu(" Light Source ", light_src_sub);
    glutAddSubMenu(" Fog Options ", fog_sub);
    glutAddSubMenu(" Blending Shadow ", blending_sub);
    glutAddSubMenu(" Texture Mapped Ground ", ground_tex_sub);
    glutAddSubMenu(" Texture Mapped Sphere ", sphere_tex_sub);
    glutAddSubMenu(" Fireworks ", firework_sub);
    glutAddMenuEntry(" Default View Point ",1);
    glutAddMenuEntry(" Quit ",2);
	glutAttachMenu(GLUT_LEFT_BUTTON);
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
    glutCreateWindow("Assignment4");

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
    menu();
    init();
    glutMainLoop();
    return 0;
}
