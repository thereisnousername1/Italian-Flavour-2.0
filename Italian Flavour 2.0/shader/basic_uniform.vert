// install a GLSL tool to edit any shader more easily
// for they are written in GLSL language
#version 460

// under the setting of using a Phong lighting, main mechanic should take place in fragment shader

// according to https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// using a layout modifier, assign attrib in program is no longer necessary
// intended to be handled by program, less likely needed to be modified
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

layout (location = 3) in vec4 VertexTangent;

uniform struct LightInfo
{
    vec4 Position;  // could be static or moving, depends on logic in the program

    vec3 Ld;    // diffuse light intensity
    vec3 La;    // ambient light intensity
    vec3 Ls;    // specular light intensity

}lights[3];

// send to fragment shader or other shader
out vec3 normal;
out vec3 position;
out vec3 n2;
out vec3 pos2; 
out vec2 TexCoord;

// intended to be handled by user, do what you want, also they could be left empty
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform mat3 NormalMatrix;

// this function is called in main()
//                     return value stored in the imported variables
void getCamSpaceValues(out vec3 n, out vec3 pos)
{
    // convert the vertex normal to eye coordinates
    n = normalize(NormalMatrix * VertexNormal);

    // convert the vertex position to eye coordinates
    pos = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
}

void main()
{
    vec3 norm = normalize(NormalMatrix * VertexNormal);
    vec3 tangent = normalize(NormalMatrix * vec3(VertexTangent));

    // calculate the bitangent vector from the normal and
    // tangent vectors - OpenGL SuperBible p.584
    vec3 binormal = normalize(cross(norm, tangent)) * VertexTangent.w;

    // this I don't know
    mat3 toObjectLocal = mat3(
        tangent.x, binormal.x, norm.x,
        tangent.y, binormal.y, norm.y,
        tangent.z, binormal.z, norm.z
    );

    // convert the vertex position to eye coordinates
    vec3 Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
    
    // to calculate this both shader needs the Light struct
    // LightDir = toObjectLocal * (Light.Position.xyz - position);
    for(int light = 0; light < 3; light++)
    {
        position = toObjectLocal * (lights[light].Position.xyz - Position);
    }

    getCamSpaceValues(n2, pos2);
    
    // ViewDir = toObjectLocal * normalize(-position);
    normal = toObjectLocal * normalize(-Position);

    TexCoord = VertexTexCoord;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}