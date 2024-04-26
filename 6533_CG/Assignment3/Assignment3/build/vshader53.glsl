/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version
in  vec2 vTexCoord;
in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
out vec4 color;
out vec4 ePosition; // For fog in f-shader: position in eye frame
out vec2 GtexCoord; // Ground Texture
out vec2 StexCoord; // Sphere Texture
out vec2 LattCoord; // Lattice
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 model_view, projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;
uniform float Shininess;
uniform vec4 PosAmbientProduct, PosDiffuseProduct, PosSpecularProduct;
uniform vec4 SpotLightPosition, SpotLightDirection;
uniform float SpotLightExponent, SpotLightAngle;
uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation
uniform int LightingOn, ShadingType, LightSource;
uniform int fog_mode;
uniform int sphereTex;
uniform int tex_mode;
uniform int frame_mode;
uniform int sphereCB;
uniform int lattice_mode;
void main() {
    if (LightingOn == 1){
    if (tex_mode == 1){
      GtexCoord.x = vTexCoord.x * 6;
      GtexCoord.y = vTexCoord.y * 5;
    }
    vec4 framePos;
    ePosition = model_view * vPosition;
    if (frame_mode == 1) {
        framePos = vPosition; // o or O
    } else if (frame_mode == 2) {
        framePos = ePosition; // e or E
    }
    if (sphereTex == 1) {
        StexCoord.x = framePos.x * 2.5; // vertical
    } else if (sphereTex == 2) {
        StexCoord.x = (framePos.x + framePos.y + framePos.z) * 1.5; // slanted
    }
    if (sphereCB == 1 && sphereTex == 1){
      StexCoord.x = 0.45 * (framePos.x + framePos.y + framePos.z);
      StexCoord.y = 0.45 * (framePos.x - framePos.y + framePos.z);
    } else if (sphereCB == 1 && sphereTex == 2) {
      StexCoord.x = 0.75 * (framePos.x + 1);
      StexCoord.y = 0.75 * (framePos.y + 1);
    }
    switch(lattice_mode){
      case 1: // Upright
        LattCoord.x = 0.5 * (vPosition.x + 1);
        LattCoord.y = 0.5 * (vPosition.y + 1);
        break; 
      case 2: // Tilted
        LattCoord.x = 0.3 * (vPosition.x + vPosition.y + vPosition.z);
        LattCoord.y = 0.3 * (vPosition.x - vPosition.y + vPosition.z);
        break; 
      }
    if (sphereTex == 0) StexCoord = vec2(framePos.x, framePos.y);
    vec3 N = normalize( Normal_Matrix * vNormal.xyz);
    if(ShadingType == 1) {
      N = normalize( Normal_Matrix * vPosition.xyz);
    }

      vec3 pos = (model_view * vPosition).xyz;
      // When Light is a distant (directional light)
      vec3 L = normalize( -LightPosition.xyz );
      vec3 E = normalize( -pos );
      vec3 H = normalize( L + E );
      // YJC Note: N must use the one pointing *toward* the viewer
      //     ==> If (N dot E) < 0 then N must be changed to -N
      //
      if ( dot(N, E) < 0 ) N = -N;
      vec4 ambient = AmbientProduct;
      float d = max( dot(L, N), 0.0 );
      vec4 diffuse = d * DiffuseProduct;
      float s = pow( max(dot(N, H), 0.0), Shininess );
      vec4 specular = s * SpecularProduct;
      if( dot(L, N) < 0.0 ) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
      } 
      // Skip multiplyting att, since attenuation = 1 for directional light
      color = (ambient + diffuse + specular);

      // Upadte for positional light source - section c & d
      L = normalize( SpotLightPosition.xyz - pos ); // Distance from point p to the light source
      d = max( dot(L, N), 0.0 );
      // s = pow( max(dot(N, H), 0.0), Shininess );
      // Get distance and attenuation
      float distance = length(SpotLightPosition.xyz - pos);
      // Attenuation when point source
      float attenuation = 1 / (ConstAtt + LinearAtt * distance + QuadAtt * distance * distance);
      // Update Attenuation when spotlight
      if(LightSource == 0){	
        vec3 Lf = normalize(SpotLightDirection.xyz);
        if( dot(Lf,-L) < cos(SpotLightAngle) ){
          attenuation = 0;
        } else{
          attenuation *= pow(dot(Lf,-L),SpotLightExponent);
        }
      }
      // Update ambient, diffuse, specular with attenuation
      ambient = attenuation * PosAmbientProduct;
      diffuse = attenuation * d * PosDiffuseProduct;
      specular = attenuation * s * PosSpecularProduct;
      if( dot(L, N) < 0.0 ) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
      } 

      // Update color output
      color += (ambient + diffuse + specular);
      gl_Position = projection * model_view * vPosition;
  } else if (LightingOn == 2) {
    color = vColor;
    gl_Position = projection * model_view * vPosition;
  } else {
    color = vColor;
    gl_Position = projection * model_view * vPosition;
  }
}
