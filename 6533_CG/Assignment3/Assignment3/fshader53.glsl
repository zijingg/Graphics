/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version
in  vec4 ePosition;
in  vec4 color;
in vec2 GtexCoord;
in vec2 StexCoord;
in vec2 LattCoord;
out vec4 fColor;

uniform sampler2D texture_2D;
uniform sampler1D texture_1D;
uniform float linear_start;
uniform float linear_end;
uniform float fog_density;
uniform vec4 fog_color, shadow_color;
uniform int fog_mode;
uniform int tex_mode; // Ground texture
uniform int sphereTex;
uniform int sphereCB;
uniform int lattice_mode;
uniform int LightingOn;

void main() {   
    if (LightingOn == 1){ // Sphere, ground
        vec4 newColor = color;
        if (tex_mode == 1){
            newColor = color * texture(texture_2D, GtexCoord);
        }
        if (sphereTex == 1 || sphereTex == 2 ){
            newColor = color * texture(texture_1D, StexCoord.x);
        } 
        if (sphereCB == 1){
            newColor = color * texture(texture_2D, StexCoord);
            if (newColor.x == 0) newColor = color * vec4(0.9, 0.1, 0.1, 1.0);
        }
        if ( (lattice_mode == 1 || lattice_mode == 2) 
                && (fract(4 * LattCoord.x) < 0.35 
                &&  fract(4 * LattCoord.y) < 0.35)) discard;

        float z = length(ePosition.xyz);
        float fog_factor;
        switch(fog_mode){
        case 0:
            fog_factor = 1;  // No fog
            break;
        case 1:
            fog_factor = (linear_end - z)/(linear_end - linear_start) ;   // Linear fog
            break;
        case 2:
            fog_factor = exp(-(fog_density * z));   // Exp fog
            break;
        case 3:
            fog_factor = exp(-pow((fog_density * z), 2));   // Exp Sqr fog
            break;
        }
        fog_factor = clamp(fog_factor, 0.0, 1.0);
        fColor = mix(fog_color, newColor, fog_factor);
    }
    else if (LightingOn == 2 && fog_mode == 0){
        fColor = vec4(0.25, 0.25, 0.25, 0.65); // Shadow when no fog
    } else if (LightingOn == 2 && fog_mode != 0){
        vec4 shadowColor = vec4(0.25, 0.25, 0.25, 0.65); // Shadow when fog
        float z = length(ePosition.xyz);
        float fog_factor;
        switch(fog_mode){
        case 0:
            fog_factor = 1;  // No fog
            break;
        case 1:
            fog_factor = (linear_end - z)/(linear_end - linear_start) ;   // Linear fog
            break;
        case 2:
            fog_factor = exp(-(fog_density * z));   // Exp fog
            break;
        case 3:
            fog_factor = exp(-pow((fog_density * z), 2));   // Exp Sqr fog
            break;
        }
        fog_factor = clamp(fog_factor, 0.0, 1.0);
        fColor = mix(shadowColor, fog_color, fog_factor);
    } 
    else {
        fColor = color;
    } 
} 

