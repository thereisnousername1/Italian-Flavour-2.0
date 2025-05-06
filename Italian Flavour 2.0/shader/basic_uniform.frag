#version 460

///
/// In fragment shader
/// for how many variables are displayed as in 
/// same variables should come out from vertex shader
///

// intended to be handled by program, less likely needed to be modified
in vec3 normal;
in vec3 position;
in vec3 n2;
in vec3 pos2;
in vec2 TexCoord;

// according to https://blog.mmorpgtycoon.com/post/opengl-texture-inconsistent/
// lets assume the binding word related to the "slot number" of a texture unit
// so without actually using the relative texture unit with the number binded here
// it gone wrong and causing glitchy texture
//
// also maybe the type have to be the same as well
//
// actual texture to be imported from program
// layout (binding = 0) uniform sampler2D Tex1; // refer to single texture rendering

// without binding = 0
// uniform sampler2D RenderTex;
uniform sampler2D Tex;
layout (binding = 4) uniform sampler2D Tex1;
layout (binding = 5) uniform sampler2D Tex2;
//layout (binding = 0) uniform sampler2D Tex1;  // Transparency since it mixed with BlurTex1
//layout (binding = 1) uniform sampler2D Tex2;  // Transparency since it mixed with BlurTex1

// Final Output
layout (location = 0) out vec4 FragColor;

layout (location = 1) out vec3 HdrColor;

/////////////////// Phong / Blinn-Phong switchable shading model and necessary information ///////////////////

uniform bool Phong; // switch between Phong and Blinn-Phong

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
        vec3 h = normalize(s + v);

        /// SWITCHABLE ///
        
        if(Phong == true)
            // original material color setting in scenebasic_uniform
            //// For Directional Light, use lights[light].Ls in multiple lights scenario
            // final spec                            eye reflection angle
            // specular = Light.Ls * Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);        // reflection -> Phong Shading
            specular = lights[light].Ls * Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);   // specialized multiple lights formula

        else
            // specular = Light.Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.shininess);            // calculate the half vector -> Blinn-Phong Shading
            specular = lights[light].Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.shininess);   // specialized multiple lights formula

        /// SWITCHABLE ///
    }

    // final ouput
    return diffuse + ambient + specular;
}

/////////////////// Phong / Blinn-Phong switchable shading model and necessary information ///////////////////

/////////////////// Phong / Blinn-Phong switchable shading model and necessary information ///////////////////

vec3 phongModel (int light, vec3 n)
{
    vec3 diffuse = vec3(0.0), specular = vec3(0.0);

    vec3 texColor = texture(Tex1, TexCoord).rgb;    // base texture

    vec3 ambient = lights[light].La * texColor;

    // light direction
    vec3 s = normalize(position);

    // According to OpenGL SuperBible p.569 (CH13 - Diffuse Light), this dot product caould be a negative value
    // max() is placed to prevent such situation, it will justify the negative part to zero
    float sDotN = max(dot(s, n), 0.0);    // calculate the dot product of normalized normal and light direction

    diffuse = lights[light].Ld * texColor * sDotN;

    // by default I choose Phong instead of Blinn-Phong
    if (sDotN > 0.0)
    {
        //// this part is different from regular phong model
        vec3 v = normalize(normal);

        /// SWITCHABLE ///

        // reflection -> Phong Shading
        vec3 r = reflect(-s, n);

        // calculate the half vector -> Blinn-Phong Shading
        // vec3 h = normalize(s + v);

        /// SWITCHABLE ///
    
        // final spec                            eye reflection angle
        specular = lights[light].Ls * Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);   // reflection -> Phong Shading
        // specular = Light.Ls * Material.Ks * pow(max(dot(h, n), 0.0), Material.shininess);   // calculate the half vector -> Blinn-Phong Shading
    
        /// SWITCHABLE ///

    }
    // final ouput
    return diffuse + ambient + specular;
}

/////////////////// Image Processing Techniques - Bloom Effect with Gamma Correction (based on HDR) ///////////////////

/////////////////// Image Processing Techniques ///////////////////

///// Bloom Effect with Gamma Correction additional information /////



/// HDR with Tone Mapping additional information ///

uniform float AveLum;
uniform float Exposure = 0.35;
uniform float White = 0.928;
// uniform bool DoToneMap = true;

layout (binding = 0) uniform sampler2D HdrTex;

// XYZ / RGB conversion matrices from:

// http://www.brucelindbloom.com/index.htm?Eqn_RGB_XYZ_Matrix.html

uniform mat3 rgb2xyz = mat3(
0.4124564, 0.2126729, 0.0193339,
0.3575761, 0.7151522, 0.1191920,
0.1804375, 0.0721750, 0.9503041 
);

uniform mat3 xyz2rgb = mat3(
3.2404542, -0.9692660, 0.0556434,
-1.5371385, 1.8760108, -0.2040259,
-0.4985314, 0.0415560, 1.0572252
);

/// HDR with Tone Mapping additional information ///



/// Edge Detection additional information ///

uniform float LumThresh;

// Relative luminance, constant for the edge to glow 
const vec3 lum = vec3(0.2126, 0.7152, 0.0722);

float luminance(vec3 color)
{
    return dot(lum, color);
}

/// Edge Detection additional information ///



/// Gaussian Blur additional information ///

uniform float Weight[10];

/// Gaussian Blur additional information ///



layout (binding = 1) uniform sampler2D BlurTex1;
layout (binding = 2) uniform sampler2D BlurTex2;

uniform float PixOffset[10] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);

uniform float Gamma;    // gamma correction

///// Bloom Effect with Gamma Correction additional information /////

// when the uniform pass in from outside, decide what logic it would perform
uniform int Pass;

vec4 pass1()    // always the actual object rendering with lighting model
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < 3; i++){
        color += phongModel(i, pos2, normalize(n2));
    }
        // Gamma correction
        // FragColor = vec4( pow( color, vec3(1.0 / Gamma)), 1.0);

    return vec4(color, 1);
    // return vec4( pow( color, vec3(1.0 / Gamma)), 1.0);
}

vec4 pass1alt()    // always the actual object rendering with lighting model
{
    vec3 color = vec3(0.0);
    vec3 normMap = texture(Tex2, TexCoord).xyz;
    normMap.xy = 2.0 * normMap.xy - 1.0;
    for (int i = 0; i < 3; i++){

        color += phongModel(i, pos2, normalize(n2));

        if (gl_FrontFacing)
        {
            // FragColor = vec4(phongModel(position, normalize(normal)), 1.0);
            color += phongModel(i, normalize(normMap));
        }
        else
        {
            // FragColor = vec4(phongModel(position, normalize(-normal)), 1.0);
            color += phongModel(i, normalize(-normMap));
        }
    }
        // Gamma correction
        // FragColor = vec4( pow( color, vec3(1.0 / Gamma)), 1.0);

    return vec4(color, 1);
    // return vec4( pow( color, vec3(1.0 / Gamma)), 1.0);
}

vec4 pass2()    // Bright-pass filter (write to BlurTex1)
{
    vec4 val = texture(HdrTex, TexCoord);

    if( luminance(val.rgb) > LumThresh )

        return val;

    else

        return vec4(0.0);
}

vec4 pass3()    // First blur pass (read from BlurTex1, write to BlurTex2)
{
    float dy = 1.0 / (textureSize( BlurTex1, 0 )).y;

    vec4 sum = texture(BlurTex1, TexCoord) * Weight[0];

    for( int i = 1; i < 10; i++)
    {
        sum += texture( BlurTex1, TexCoord + vec2(0.0, PixOffset[i]) * dy ) * Weight[i];
        sum += texture( BlurTex1, TexCoord - vec2(0.0, PixOffset[i]) * dy ) * Weight[i];
    }

    return sum;
}

vec4 pass4()    // Second blur pass (read from BlurTex2, write to BlurTex1)
{
    float dx = 1.0 / (textureSize( BlurTex2, 0 )).x;

    vec4 sum = texture(BlurTex2, TexCoord) * Weight[0];

    for( int i = 1; i < 10; i++)
    {
        sum += texture( BlurTex2, TexCoord + vec2(PixOffset[i], 0.0) * dx ) * Weight[i];
        sum += texture( BlurTex2, TexCoord - vec2(PixOffset[i], 0.0) * dx ) * Weight[i];
    }

    return sum;
}

vec4 pass5()    // Read from BlurTex1 and HdrTex, write to default buffer
{
    /////////////// Tone mapping ///////////////

    // Retrieve high-res color from texture
    vec4 color = texture( HdrTex, TexCoord );

    // Convert to XYZ
    vec3 xyzCol = rgb2xyz * vec3(color);
    
    // Convert to xyY
    float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

    // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
    float L = ( Exposure * xyYCol.z ) / AveLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );
    // using L = (L * ( 1 - L / (White * White) )) / ( 1 + L ); can revert the color

    // Using the new luminance, convert back to XYZ
    xyzCol.x = ( L * xyYCol.x ) / (xyYCol.y);
    xyzCol.y = L;
    xyzCol.z = ( L * (1 - xyYCol.x - xyYCol.y)) / xyYCol.y;
    
    /* Convert back to RGB and send to output buffer
    if( DoToneMap )
        FragColor = vec4( xyz2rgb * xyzCol, 1.0);
    else
        FragColor = color;
    */

    // Gamma correction
    // FragColor = vec4( pow( color, vec3(1.0 / Gamma)), 1.0);

    // Convert back to RGB
    vec4 toneMapColor = vec4( xyz2rgb * xyzCol, 1.0);

    /////////////// Tone mapping ///////////////

    ///////////// Combine with blurred texture /////////////

    // We want linear filtering on this texture access so that
    // we get additional blurring.
    vec4 blurTex = texture(BlurTex1, TexCoord);

    // Gamma correction
    // FragColor = vec4( pow( color, vec3(1.0 / Gamma)), 1.0);

    return toneMapColor + blurTex;
    // return toneMapColor + blurTex * vec4( pow( color.rgb, vec3(1.0 / Gamma)), 1.0);
    // return toneMapColor + blurTex * vec4( pow( color.xyz, vec3(1.0 / Gamma)), 1.0);
    // return toneMapColor + blurTex + vec4( pow( color.rgb, vec3(1.0 / Gamma)), 1.0);
    // return toneMapColor + blurTex + vec4( pow( color.xyz, vec3(1.0 / Gamma)), 1.0);

    ///////////// Combine with blurred texture /////////////
}

float EdgeThreshold = 0.05;

vec4 pass6()
{
    ivec2 pix = ivec2(gl_FragCoord.xy); //we grab a pixel to check if edge

    //pick neighbouring pixels for convolution filter

    //check lecture slides

    float s00 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(-1, 1) ).rgb);
    float s10 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(-1, 0) ).rgb);
    float s20 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(-1, -1) ).rgb);
    float s01 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(0, 1) ).rgb);
    float s21 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(0, -1) ).rgb);
    float s02 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(1, 1) ).rgb);
    float s12 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(1, 0) ).rgb);
    float s22 = luminance(texelFetchOffset( Tex, pix, 0, ivec2(1, -1) ).rgb);

    float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
    float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

    float g = sx * sx + sy * sy;
    
    if( g > EdgeThreshold )
        return vec4(1.0); // edge
    else
        // return vec4(0.0, 0.0, 0.0, 1.0); // no edge
        return texelFetch(Tex, pix, 0);
}

int[5] Weight2;

vec4 pass7()
{
    ivec2 pix = ivec2( gl_FragCoord.xy );

    vec4 sum = texelFetch( Tex, pix, 0 ) * Weight2[0];

    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, 1)) * Weight2[1];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, -1)) * Weight2[1];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, 2)) * Weight2[2];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, -2)) * Weight2[2];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, 3)) * Weight2[3];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, -3)) * Weight2[3];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, 4)) * Weight2[4]; 
    sum += texelFetchOffset( Tex, pix, 0, ivec2(0, -4)) * Weight2[4];
    
    return sum;
}

vec4 pass8()
{
    ivec2 pix= ivec2( gl_FragCoord.xy );

    vec4 sum = texelFetch( Tex, pix, 0 ) * Weight2[0];

    sum += texelFetchOffset( Tex, pix, 0, ivec2(1, 0)) * Weight2[1];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(-1, 0)) * Weight2[1];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(2, 0)) * Weight2[2];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(-2, 0)) * Weight2[2];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(3, 0)) * Weight2[3];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(-3, 0)) * Weight2[3];
    sum += texelFetchOffset( Tex, pix, 0, ivec2(4, 0)) * Weight2[4]; 
    sum += texelFetchOffset( Tex, pix, 0, ivec2(-4, 0)) * Weight2[4];
    
    return sum;
}

uniform bool DoToneMap = true;

vec4 pass9()    // this pass computes the sum of the luminance of all pixels
{

    // Retrieve high-res color from texture
    vec4 color = texture( HdrTex, TexCoord );

    // Convert to XYZ
    vec3 xyzCol = rgb2xyz * vec3(color);
    
    // Convert to xyY
    float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

    // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
    float L = ( Exposure * xyYCol.z) / AveLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );
    // using L = (L * ( 1 - L / (White * White) )) / ( 1 + L ); can revert the color

    // Using the new luminance, convert back to XYZ
    xyzCol.x = ( L * xyYCol.x ) / (xyYCol.y);
    xyzCol.y = L;
    xyzCol.z = ( L * (1 - xyYCol.x - xyYCol.y)) / xyYCol.y;
    
    // Convert back to RGB and send to output buffer
    if( DoToneMap )
        return vec4( xyz2rgb * xyzCol, 1.0);
    else
        return color;
}

/////////////////// Image Processing Techniques ///////////////////

bool temp;
// edge detection takes 2 pass()
// gaussian blur takes 3 pass()
// hdr with tone mapping takes 2 pass()
// bloom effect with gamma correction (based on hdr) takes 5 pass()
void main()
{
    vec4 color;

    temp = Phong;
    if(Phong != temp)
    {
        color = pass1();
        temp = Phong;    
    }

    if (Pass == 1) color = pass1();
    if (Pass == 10) color = pass1alt();
    if (Pass == 2) color = pass2();
    if (Pass == 3) color = pass3();
    if (Pass == 4) color = pass4();
    // if (Pass == 5) color = pass5();
    if (Pass == 5) color = vec4( pow( vec3(pass5()), vec3(1.0 / Gamma)), 1.0);

    if (Pass == 6) color = pass6();

    if (Pass == 7) color = pass7();
    if (Pass == 8) color = pass8();

    if (Pass == 9) color = pass9();

    // FragColor = vec4(phongModel(position, normalize(normal)), 1.0);

    // Gamma correction
    FragColor = color;
    // FragColor = color * vec4(texColor, 1.0);
    // FragColor = vec4( pow( color.rgb, vec3(1.0 / Gamma)), 1.0);
    // FragColor = pow( color, vec4(1.0 / Gamma));
}
