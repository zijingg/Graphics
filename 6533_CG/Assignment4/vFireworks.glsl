// Vertex shader for fireworks
#version 150
in vec4 vVelocity;
in vec4 vColor;
in vec4 vPosition;
out vec4 color;
out vec4 Position;
uniform mat4 model_view;
uniform mat4 projection;
uniform float t;
uniform vec4 pos;

void main() {
	// Update firework position
	float a = -0.00000049;
	vec4 pos = vec4(0,0.1,0,1); 
	float x,y,z;
	// x = pos.x + 0.001 * velocity.x * t;
	// y = pos.y + 0.001 * velocity.y * t + 0.5 * a * t * t; 
	// z = pos.z + 0.001 * velocity.z * t;
	// Set firework color
	color = vColor;
	gl_Position =  pos;
	Position = pos;
} 
