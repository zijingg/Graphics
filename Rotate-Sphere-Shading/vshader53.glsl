/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
out vec4 color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 PosAmbientProduct, PosDiffuseProduct, PosSpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform float Shininess;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation
uniform vec4 SpotLightPosition; 
uniform vec4 SpotLightDirection;
uniform float SpotCutoff;
uniform float SpotExponent;
uniform int Lighting; 
uniform int LightSource;   // 0: SpotLight; 1: Point Source

void main()
{
    if (Lighting == 1) {
    	// Transform vertex  position into eye coordinates
        vec3 pos = (ModelView * vPosition).xyz;
	
    	vec3 L = normalize( -LightPosition.xyz ); // Default to distant light
    	vec3 E = normalize( -pos );
    	vec3 H = normalize( L + E );

    	// Transform vertex normal into eye coordinates
          // vec3 N = normalize( ModelView*vec4(vNormal, 0.0) ).xyz;
    	vec3 N = normalize(Normal_Matrix * vNormal);

// YJC Note: N must use the one pointing *toward* the viewer
//     ==> If (N dot E) < 0 then N must be changed to -N
//
   	if ( dot(N, E) < 0 ) N = -N;
    	vec4 ambient = AmbientProduct;

    	float d = max( dot(L, N), 0.0 );
    	vec4  diffuse = d * DiffuseProduct;

    	float s = pow( max(dot(N, H), 0.0), Shininess );
    	vec4  specular = s * SpecularProduct;
    
    	if( dot(L, N) < 0.0 ) {
	    specular = vec4(0.0, 0.0, 0.0, 1.0);
    	} 

/*--- To Do: Compute attenuation ---*/
	float attenuation = 0.0; 
	
    	color = ambient + diffuse + specular; 
	
/*--- attenuation below must be computed properly ---*/

 // positional or spotlight
    	if (LightSource == 0 || LightSource == 1) {
            L = normalize( SpotLightPosition.xyz - pos );
            H = normalize( L + E );
	    d = max( dot(L, N), 0.0 );
            s = pow( max(dot(N, H), 0.0), Shininess );
            float distance = length( SpotLightPosition.xyz - pos );
        // Update attenuation
            attenuation = 1.0 / (ConstAtt + LinearAtt * distance + QuadAtt * distance * distance);

            if (LightSource == 0) { // SpotLight
                vec3 Lf = normalize(SpotLightDirection.xyz);
                if (dot(Lf, L) < cos(SpotCutoff)) {
                    attenuation = 0.0;
                } else {
                    attenuation *= pow(dot(Lf, L), SpotExponent);
                }
            }
 	}
        

    // Compute terms in the illumination equation
    	ambient = PosAmbientProduct;

    	diffuse = d * PosDiffuseProduct;
    	
    	specular = s * PosSpecularProduct;
    
    	if( dot(L, N) < 0.0 ) {
	    specular = vec4(0.0, 0.0, 0.0, 1.0);
    	} 
    	gl_Position = Projection * ModelView * vPosition;
    	color += attenuation * (ambient + diffuse + specular);

    } else {
        gl_Position = Projection * ModelView * vPosition;
	color = vColor;
    }
}