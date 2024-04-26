#version 150
in vec3 vVelocity;
in vec3 vColor;
out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;
uniform float t; // pass the time in miliseconds
void main() 
{
    vec3 position = vec3(0.0, 0.1, 0.0);
	float x,y,z;
	float a = -0.00000049;

	x = position.x + 0.001 * vVelocity.x * t;
	y = position.y + 0.001 * vVelocity.y * t + 0.5 * a * t * t; 
	z = position.z + 0.001 * vVelocity.z * t;

	color = vec4(vColor, 1.0);
	gl_Position = projection * model_view * vec4(x, y, z, 1.0);
} 
