// install a GLSL tool to edit any shader more easily
// for they are written in GLSL language
#version 460

// under the setting of using a Phong lighting, main mechanic should take place in fragment shader

// according to https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// using a layout modifier, assign attrib in program is no longer necessary
// intended to be handled by program, less likely needed to be modified
layout (location = 0) in vec3 VertexPosition;

out vec3 skyposition;

// intended to be handled by user, do what you want, also they could be left empty
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform mat3 NormalMatrix;

void main()
{
    skyposition = VertexPosition;
    gl_Position = MVP * vec4(VertexPosition,1.0);

}