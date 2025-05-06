#version 460

///
/// In fragment shader
/// for how many variables are displayed as in 
/// same variables should come out from vertex shader
///

// intended to be handled by program, less likely needed to be modified
in vec3 skyposition;

// according to https://blog.mmorpgtycoon.com/post/opengl-texture-inconsistent/
// lets assume the binding word related to the "slot number" of a texture unit
// so without actually using the relative texture unit with the number binded here
// it gone wrong and causing glitchy texture
//
// also maybe the type have to be the same as well
//
// actual texture to be imported from program
layout (binding = 7) uniform samplerCube SkyTex;

// Final Output
layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 texColor = texture(SkyTex, normalize(skyposition)).rgb;

    texColor = pow(texColor, vec3(1.0 / 2.2));

    FragColor = vec4(texColor, 1.0);
    
}
