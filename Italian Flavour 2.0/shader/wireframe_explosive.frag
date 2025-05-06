#version 460

in vec3 n2;
in vec3 pos2;

noperspective in vec3 GEdgeDistance;

// line struct for rendering the wireframe
uniform struct LineInfo
{
    float Width;
    vec4 color;
} Line;

// Final Output
layout (location = 0) out vec4 FragColor;

/////////////////// Phong / Blinn-Phong switchable shading model and necessary information ///////////////////

uniform struct LightInfo
{
    vec4 Position;  // could be static or moving, depends on logic in the program

    vec3 Ld;    // diffuse light intensity
    vec3 La;    // ambient light intensity
    vec3 Ls;    // specular light intensity

// }Light;
}lights[3]; // multiple lights logic

uniform struct MaterialInfo
{
    vec3 Kd;    // diffuse material reflected color
    vec3 Ka;    // ambient material reflected color
    vec3 Ks;    // specular material reflected color

    float shininess;

}Material;

vec3 phongModel (int light, vec3 position, vec3 n)   // multiple directional lights
// vec3 phongModel (vec3 position, vec3 n)
{

//// diffuse lighting is directional lighting
    vec3 diffuse = vec3(0.0);

//// specular lighting is meant to represent glare upon an object's surface
    vec3 specular = vec3(0.0);

    // combined the actual texture with the coordinate
    // to output the color of that texture
    // vec3 texColor = texture(Tex, TexCoord).rgb;

//// ambient lighting is a constant & omnipresent light source
    // vec3 ambient = Light.La * Material.Ka;
    vec3 ambient = lights[light].La * Material.Ka;

    // light direction
    // calculate the direction from the light position to the vertex point with normalize
    // vec3 s = normalize(Light.Position.xyz - position);       // find out s vector
    vec3 s = normalize(lights[light].Position.xyz - position);  // specialized multiple lights formula

    // According to OpenGL SuperBible p.569 (CH13 - Diffuse Light), this dot product caould be a negative value
    // max() is placed to prevent such situation, it will justify the negative part to zero
    float sDotN = max(dot(s, n), 0.0);    // calculate the dot product of normalized normal and light direction

    // diffuseColor = diffuseMaterial * DiffuseLight * the dot product from normal and light direction
    // diffuse = Light.Ld * Material.Kd * sDotN;
    diffuse = lights[light].Ld * Material.Kd * sDotN;

    // by default I choose Phong instead of Blinn-Phong
    if (sDotN > 0.0)
    {
        // p.569 of opengl superbible
        // eye normal
        vec3 v = normalize(-position.xyz);

        /// SWITCHABLE ///

        // reflection -> Phong Shading
        vec3 r = reflect(-s, n);

        // calculate the half vector -> Blinn-Phong Shading
        // vec3 h = normalize(s + v);

        /// SWITCHABLE ///
        
        
        // original material color setting in scenebasic_uniform
        //// For Directional Light, use lights[light].Ls in multiple lights scenario
        // final spec                            eye reflection angle
        // specular = Light.Ls * Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);        // reflection -> Phong Shading
        specular = lights[light].Ls * Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);   // specialized multiple lights formula

        // specular = Light.Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.shininess);            // calculate the half vector -> Blinn-Phong Shading
        // specular = lights[light].Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.shininess);   // specialized multiple lights formula

        /// SWITCHABLE ///
    }

    // final ouput
    return diffuse + ambient + specular;
}

/////////////////// Phong / Blinn-Phong switchable shading model and necessary information ///////////////////

void main()
{
    vec4 color = vec4(0.0);
    for (int i = 0; i < 3; i++)
    {
        color += (phongModel(i, pos2, n2), 1.0);
    }

    float d = min( GEdgeDistance.x, GEdgeDistance.y);
    d = min(d, GEdgeDistance.z);

    float mixVal;
    if( d < Line.Width )
    {
        mixVal = 1.0;
    }
    else if( d < Line.Width + 1 )
    {
        mixVal = 0.0;
    }
    else
    {
        float x = d - (Line.Width - 1);
        mixVal = exp2(-2.0 * (x * x));
    }

    FragColor = mix( color, Line.color, mixVal);
}
