#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "helper/texture.h" // texture loading related

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

// initialization of mainly 3D models in a scene happen in here
SceneBasic_Uniform::SceneBasic_Uniform() :

    tPrev(0),
    angle(0.0f),
    rotSpeed(glm::pi<float>() / 8.0f),
    plane(20.0f, 50.0f, 1, 1),
    teapot(14, mat4(1.0f)),
    sky(500),
    sphere(2.0f, 50, 50)

{
    // Yee = HDRwithBloom;

    // for instant gauss calculate with sigma2 changeable (Key I -> ++, K -> --)
    temp = Scene::sigma2;   // sigma2 is declared in scene.h
}

float weights[10], sum;  // declared as global for instant gauss calculate

// init(), initialization of everything in a scene happen in here
// Light intensity setting to be placed in here (Well actually that does not matter at all, so as all setting to be import to shader)
void SceneBasic_Uniform::initScene()
{
    compile();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    //// Transparency function ////

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //// Transparency function ////

    // model, view, projection matrix initialization and setup (for 3D scene)
    // model = mat4(1.0f); // moved to render()
    // the view is a bit different from lab cuz I like it more
    // view = glm::lookAt(vec3(1.0f, 1.25f, 1.25f), vec3(0.0f, -0.1f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    model = mat4(1.0f);

    projection = mat4(1.0f);

    // angle = glm::pi<float>() / 4.0f;
    angle = glm::pi<float>() / 2.0f;

    #pragma region Image Processing Techniques

    // Array for full-screen quad
    GLfloat verts[] =
    {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] =
    {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    // set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates

    glBindVertexArray(0);

    
    //// Bloom Effect with Gamma Correction (based on HDR) ////


    
    //// Edge Detection ////

    // prog.setUniform("EdgeThreshold", 0.05f); // renamed and modified in current context
    prog.setUniform("LumThreash", 1.7f);

    //// Edge Detection ////


    
    //// Gaussian Blur ////

    //                        this number decide the level of blur, BUT
    //                        it works in a weird way, number too large / too small
    //                        won't create huge impact
    // float weights[5], sum, sigma2 = 25.0f;    // declared as global for instant gauss calculate, sigma2 was 8.0f

    //Compute and sum the weights
    weights[0] = gauss(0.0f, sigma2);
    sum = weights[0];

    // for (int i = 1; i < 5; i++) {
    for (int i = 1; i < 10; i++) {
        weights[i] = gauss(float(i), sigma2);
        sum += 2 * weights[i];
    }

    //Normalize the weights and set the uniform
    // for (int i = 0; i < 5; i++) {
    for (int i = 0; i < 10; i++) {
        std::stringstream uniName;
        uniName << "Weight[" << i << "]";
        float val = weights[i] / sum;
        prog.setUniform(uniName.str().c_str(), val);
    }

    //// Gaussian Blur ////



    // Set up two sampler objects for linear and nearest filtering
    GLuint samplers[2];
    glGenSamplers(2, samplers);
    linearSampler = samplers[0];
    nearestSampler = samplers[1];
    GLfloat border[] = { 0.0f,0.0f,0.0f,0.0f };

    // Set up the nearest sampler
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameterfv(nearestSampler, GL_TEXTURE_BORDER_COLOR, border);

    // Set up the linear sampler
    glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameterfv(linearSampler, GL_TEXTURE_BORDER_COLOR, border);

    // We want nearest sampling except for the last pass.
    glBindSampler(0, nearestSampler);
    glBindSampler(1, nearestSampler);
    glBindSampler(2, nearestSampler);

    prog.setUniform("Gamma", Scene::Gamma);

    //// Bloom Effect with Gamma Correction (based on HDR) ////

    #pragma endregion

    #pragma region Texture files linking
    
    // reference : https://www.reddit.com/r/opengl/comments/1f1wizb/how_bad_is_it_to_only_use_gl_texture0_and_what_is/
    //             https://stackoverflow.com/questions/8866904/differences-and-relationship-between-glactivetexture-and-glbindtexture
    //             https://community.khronos.org/t/when-to-use-glactivetexture/64913/2

    // Order 1 :
    // Load the texture into program
    GLuint cubeTex = Texture::loadHdrCubeMap("media/texture/cube/hdr/a");


    // if they are not load together, the mixing of both texture will not take place
    GLuint diffTex = Texture::loadTexture("media/lasagnaD.jpeg");
    GLuint normalTex = Texture::loadTexture("media/lasagnaN.jpeg");

    // texture 1 of multiple texture
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, diffTex);

    // texture 2 of multiple texture
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, normalTex);

    // Ensure the correct texture unit is active before binding the cube map
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

    //                 relative file location in my computer            , bool center (according to the IDE)
    mesh = ObjMesh::load("media/lasagna.obj", false, true);

    #pragma endregion

    // since resize() is called in scenerunner run() after initScene(),
    // only 1 setupFBO() is necessary to appears in initScene() and resize()
    // put it in resize() with proper deallocate function could perform a cool ass
    // viewport resize feature
    setupFBO();
}

void SceneBasic_Uniform::compile()
{
    // compile mr prog with the desired shaders to be used in this scene
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");

        // for skybox only
        skyProg.compileShader("shader/skybox.vert");
        skyProg.compileShader("shader/skybox.frag");

		explodeProg.compileShader("shader/wireframe_explosive.vert");
		explodeProg.compileShader("shader/wireframe_explosive.geom");
		explodeProg.compileShader("shader/wireframe_explosive.frag");

        // no need to add skyProg.use()
        skyProg.link();

		explodeProg.link();

        prog.link();
        prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

#pragma region Program mainloop() functions, where everything that has to be updated take place according to program lifespan

bool temp2;

// update should be a while(true) loop as always, or in this case the loop is defined in the scenerunner class function mainloop
// it seems float t can be used as a time factor, restriction to the program lifespan
void SceneBasic_Uniform::update( float t )
{

    #pragma region Spinning logic
    
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;
    angle += 0.1f * deltaT; // this is modified in lab video, angle += rotSpeed * deltaT;

    // if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
    if (this->m_animate)
    {
        // it is just modifing the variable angle more rapidly
        angle += rotSpeed * deltaT;

        if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
    }


    #pragma endregion

    // instant gauss calculate with sigma2 changeable (Key I -> ++, K -> --)
    if (temp != sigma2)
    {
        //Compute and sum the weights
        weights[0] = gauss(0.0f, sigma2);
        sum = weights[0];

        for (int i = 1; i < 10; i++) {
            weights[i] = gauss(float(i), sigma2);
            sum += 2 * weights[i];
        }

        //Normalize the weights and set the uniform
        for (int i = 0; i < 10; i++) {
            std::stringstream uniName;
            uniName << "Weight[" << i << "]";
            float val = weights[i] / sum;
            prog.setUniform(uniName.str().c_str(), val);
        }

        temp = Scene::sigma2;
    }

    // Gamma changeable (Key O -> ++, L -> --)
    prog.setUniform("Gamma", Scene::Gamma);

    explodeProg.setUniform("time", (float)glfwGetTime());
}

LightSwitch temp3;

// this function now call other function(s) awaits to be executed in scene
void SceneBasic_Uniform::render()
{
    // resizing will work this time, but still not ideal
    // it regen the FBO when its necessary to
    // 
    // this is viewport resizing logic (I figure it out by myself)
    // now the content will stick to the window size when resizing happened
    glfwGetFramebufferSize(glfwGetCurrentContext(), &viewportWidth, &viewportHeight);

    if (Scene::showWireframe != showWireframe)
    {
        // clear color buffer and clear color & depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        showWireframe = Scene::showWireframe;
    }

    if (viewportWidth != width || viewportHeight != height)
    {
        byeFBO();
        resize(viewportWidth, viewportHeight);
        setupFBO();
    }

    #pragma region (Light, viewer related) Light and camera settings section in render()

    /// Viewer

    vec3 focus = vec3(7.0f * cos(angle), 4.0f, 7.0f * sin(angle));  // lab 5.1, name is changed to cameraPos in lab

    // view = glm::lookAt(vec3(2.0f, 0.0f, 14.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));   // static camera position
    // view = glm::lookAt(focus, vec3(0.0f, -0.1f, 0.0f), vec3(0.0f, 1.0f, 0.0f));  // camera x is starring at a point in the void, focus

    view = lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);  // mouse & wasd movement control

    //                                                  declared in scene.h
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 1000.0f);

    // prog.setUniform("Light.Position", vec4(view * lightPos));   // Position of light was changing dynamically
    // prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));   // disabled since multiple lights

    /// Viewer

    #pragma endregion

    /*
    pass1();

    computeLogAveLuminance();

    pass2();

    pass3();

    pass4();

    pass5();
    */
    
    if (showWireframe != true)
    {
        if (Yee != temp3)
        {
            switch (Yee)
            {
            case PhongOnly:
                byeFBO();
                setupFBO();
                std::cout << "Current mode: " << "Nothing but lighting" << std::endl;
                break;

            case EdgeDetect:
                byeFBO();
                setupFBO();
                std::cout << "Current mode: " << "Edge Detect" << std::endl;
                break;

            case GaussianBlur:
                byeFBO();
                setupFBO();
                // std::cout << "Current mode: " << "Gassian Blur" + Yee << std::endl;
                std::cout << "Current mode: " << "Gassian Blur" << std::endl;
                break;

            case HDR:
                byeFBO();
                setupFBO();
                std::cout << "Current mode: " << "HDR" << std::endl;
                break;

            case HDRwithBloom:
                byeFBO();
                setupFBO();
                // std::cout << "Current mode: " << "HDR + Bloom + Gamma correct" + Yee << std::endl;
                std::cout << "Current mode: " << "HDR + Bloom + Gamma correct" << std::endl;
                break;
            };

            temp3 = Yee;
        }

        switch (Yee)
        {
        case PhongOnly:

            // clear color buffer and clear color & depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            drawScene();

            break;

        case EdgeDetect:

            // clear color buffer and clear color & depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            pass1();

            glFlush();

            edge2();

            break;

        case GaussianBlur: // won't work

            // clear color buffer and clear color & depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            drawScene();

            gass2();

            gass3();

            break;

        case HDR:

            // clear color buffer and clear color & depth buffers
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            pass1();

            computeLogAveLuminance();

            HDR2();

            break;

        case HDRwithBloom:

            pass1();

            computeLogAveLuminance();

            pass2();

            pass3();

            pass4();

            pass5();

            break;
        };
    }
    else
    {
        // clear color buffer and clear color & depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawScene();
        glFinish();  // flush the buffer and remove the texture loaded
    }
}

// this function stored the model / mesh / object declared and will be handled with Phong model
void SceneBasic_Uniform::pass1()    // Render pass function
{
    prog.setUniform("Pass", 1);
    
    glViewport(0, 0, width, height);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    glEnable(GL_DEPTH_TEST);

    // clear color buffer and clear color & depth buffers
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    drawScene();

}

void SceneBasic_Uniform::pass1alt()    // Render pass function
{
    prog.setUniform("Pass", 10);

    // glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    glEnable(GL_DEPTH_TEST);

    // clear color buffer and clear color & depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawScene();

}

void SceneBasic_Uniform::pass2()    // Bright pass function
{
    prog.setUniform("Pass", 2);

    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, intermediateTex1, 0);

    glViewport(0, 0, bloomBufWidth, bloomBufHeight);

    glClearColor(0, 0, 0, 0);

    glDisable(GL_DEPTH_TEST);

    // clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass3()    // First blur pass function
{
    prog.setUniform("Pass", 3);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, intermediateTex2, 0);

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass4()    // Second blur pass function
{
    prog.setUniform("Pass", 4);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, intermediateTex1, 0);

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SceneBasic_Uniform::pass5()    // Composite pass function
{
    prog.setUniform("Pass", 5);

    // revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, width, height);

    glBindSampler(1, linearSampler);

    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // revert to nearest sampling
    glBindSampler(1, nearestSampler);
}

void SceneBasic_Uniform::edge2()
{
    prog.setUniform("Pass", 6); // the pass() function will handle the sampler2D Tex in frag shader

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTex);

    glDisable(GL_DEPTH_TEST);

// clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

}

void SceneBasic_Uniform::gass2()
{
    prog.setUniform("Pass", 7);

    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, intermediateTex1);

    glDisable(GL_DEPTH_TEST);

    // clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    // Render the full-screen quad
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void SceneBasic_Uniform::gass3()
{
    prog.setUniform("Pass", 8);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, intermediateTex2);

    // clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    // Render the full-screen quad
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void SceneBasic_Uniform::HDR2()
{
    prog.setUniform("Pass", 9);

    // revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // clear color buffer and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);   // not sure what does it do, still necessary in the content

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    // Render the full-screen quad
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);

}

vec3 intense = vec3(1.0f);
vec4 lightPos = vec4(0.0f, 4.0f, 2.5f, 1.0f);

void SceneBasic_Uniform::drawScene()
{
    if (showWireframe != true)
    {
        prog.setUniform("lights[0].Ld", intense);
        prog.setUniform("lights[0].Ls", intense);
        prog.setUniform("lights[1].Ld", intense);
        prog.setUniform("lights[1].Ls", intense);
        prog.setUniform("lights[2].Ld", intense);
        prog.setUniform("lights[2].Ls", intense);

        prog.setUniform("lights[0].La", 0.05f);
        prog.setUniform("lights[1].La", 0.05f);
        prog.setUniform("lights[2].La", 0.05f);

        lightPos.x = -7.0f;
        prog.setUniform("lights[0].Position", view * lightPos);

        lightPos.x = 0.0f;
        prog.setUniform("lights[1].Position", view * lightPos);

        lightPos.x = 7.0f;
        prog.setUniform("lights[2].Position", view * lightPos);

        prog.setUniform("Material.Kd", vec3(0.9f, 0.3f, 0.2f));
        prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
        prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
        prog.setUniform("Material.shininess", 100.0f);

        model = mat4(1.0f);

        // The backdrop plane
        model = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        setMatrices(prog);
        plane.render();

        // The bottom plane
        model = glm::translate(mat4(1.0f), vec3(0.0f, -5.0f, 0.0f));
        setMatrices(prog);
        plane.render();

        prog.setUniform("Pass", 10);

        prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
        prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
        prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f));
        prog.setUniform("Material.shininess", 100.0f);

        model = mat4(1.0f);

        model = glm::scale(model, vec3(0.05f, 0.05f, 0.05f));

        mv = view * model;

        // besides of setUniform, there are also setMat4, and etc. (From COMP3016)
        // set the ModelViewMatrix uniform to mv
        setMatrices(prog);

        mesh->render();

        skyProg.use();

        model = mat4(1.0f);

        // setMatrices();
        setMatrices(skyProg);

        sky.render();

        prog.use();
    }
    else
    {
        explodeProg.use();

        explodeProg.setUniform("lights[0].Ld", intense);
        explodeProg.setUniform("lights[0].Ls", intense);
        explodeProg.setUniform("lights[1].Ld", intense);
        explodeProg.setUniform("lights[1].Ls", intense);
        explodeProg.setUniform("lights[2].Ld", intense);
        explodeProg.setUniform("lights[2].Ls", intense);

        explodeProg.setUniform("lights[0].La", 0.05f);
        explodeProg.setUniform("lights[1].La", 0.05f);
        explodeProg.setUniform("lights[2].La", 0.05f);

        lightPos.x = -7.0f;
        explodeProg.setUniform("lights[0].Position", view * lightPos);

        lightPos.x = 0.0f;
        explodeProg.setUniform("lights[1].Position", view * lightPos);

        lightPos.x = 7.0f;
        explodeProg.setUniform("lights[2].Position", view * lightPos);

        explodeProg.setUniform("Material.Kd", vec3(0.9f, 0.3f, 0.2f));
        explodeProg.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
        explodeProg.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
        explodeProg.setUniform("Material.shininess", 100.0f);

        explodeProg.setUniform("Line.Width", 0.75f);
        explodeProg.setUniform("Line.Color", vec4(0.05f, 0.0f, 0.05f, 1.0f));

        model = mat4(1.0f);

        explodeProg.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
        explodeProg.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
        explodeProg.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f));
        explodeProg.setUniform("Material.shininess", 100.0f);

        model = mat4(1.0f);

        model = glm::scale(model, vec3(1.0f, 1.0f, 1.0f));

        mv = view * model;

        // besides of setUniform, there are also setMat4, and etc. (From COMP3016)
        // set the ModelViewMatrix uniform to mv
        setMatrices(explodeProg);

        mesh->render();

        explodeProg.use();
    }

    #pragma region (Material related) Multiple imported model setting section (Will be modified in numerous labs)

    /* replaced with the drawScene()

    // the color and shininess setting of the material that mapped to the model
    // that going to be effected by the light(s) setting in the scene soon in shader(s)



    //////////////////// Model render template ////////////////////

    /* standard template of setting up a model
    prog.setUniform("Material.Kd", vec3(?f, ?f, ?f));
    prog.setUniform("Material.Ks", vec3(?f, ?f, ?f));
    prog.setUniform("Material.Ka", vec3(?f, ?f, ?f));
    prog.setUniform("Material.shininess", ?f);

    model = mat4(1.0f);

    /// these 3 are transform related, could be most commonly used
    // translation            obj  ,along x , y , z
    // model = glm::translate(model, vec3(?f, ?f, ?f));

    // rotation            obj  , how much degree ,along  x,  y, z
    // model = glm::rotate(model, glm::radians(?f), vec3(?f, ?f, ?f));

    // scaling            obj  ,along x , y , z
    // model = glm::scale(model, vec3(?f, ?f, ?f));
    /// more examples could refer to COMP3016 CW2

    setMatrices();

    // it seems as a std::unique_ptr<ObjMesh> mesh
    // -> has to be use instead of . notation and so it could be render()
    // ?->render();
    ?.render();
    */

    //////////////////// Model render template ////////////////////

    #pragma endregion
}

#pragma endregion

// unlikely to be edit very often
void SceneBasic_Uniform::resize(int w, int h)
{

    glViewport(0, 0, w, h);

    width = w;
    height = h;

    // it replace the one in initScene()
    //setupFBO();

    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewport = mat4(vec4(w2, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, h2, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(w2 + 0, h2 + 0, 0.0f, 1.0f));

    // setting the aspect ratio for the model according to the window size
    // without this line it will not render
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 1000.0f);
}

// to be called for rendering 3d models, unlikely to be edit very often
void SceneBasic_Uniform::setMatrices()
{
    mv = view * model;

    // besides of setUniform, there are also setMat4, and etc.
    // set the ModelViewMatrix uniform to mv
    prog.setUniform("ModelViewMatrix", mv);

    // p.setUniform("ModelMatrix", mv);    // this provide a variation of reflection
    prog.setUniform("ModelMatrix", model);    // this provide another variation of reflection

    // set the NormalMatrix uniform to following structure
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    // set uniform for model, view, projection (MVP) and pass in the projection matrix * model view matrix
    prog.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& p)
{
    mv = view * model;

    // besides of setUniform, there are also setMat4, and etc.
    // set the ModelViewMatrix uniform to mv
    p.setUniform("ModelViewMatrix", mv);

    // p.setUniform("ModelMatrix", mv);    // this provide a variation of reflection
    p.setUniform("ModelMatrix", model);    // this provide another variation of reflection

    // set the NormalMatrix uniform to following structure
    p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    // set uniform for model, view, projection (MVP) and pass in the projection matrix * model view matrix
    p.setUniform("MVP", projection * mv);

    p.setUniform("ViewportMatrix", viewport);
}

void SceneBasic_Uniform::setupFBO() // each image processing technique has unique setupFBO()
{
    
    ///// Necessary part /////
    
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    // Create the texture object
    glGenTextures(1, &fboTex);

    if (Yee == HDR || Yee == HDRwithBloom)
        glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, fboTex);

    if (Yee == HDRwithBloom || Yee == HDR)
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
    else
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    if (Yee == GaussianBlur || Yee == HDR)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else if (Yee == EdgeDetect)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (Yee == GaussianBlur || Yee == EdgeDetect)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fboTex, 0);
    

    /// Beginning of depth buffer ///


    // Create the depth buffer
    // GLuint depthBuf; // declared as global variable, go to header file
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    
    // Bind the depth buffer to the FBO 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                              GL_RENDERBUFFER, depthBuf);
    
    ///// Necessary part /////

    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    // GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0 };

    if (Yee == HDR)
    {
        GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(2, drawBuffers);
    }
	else if (Yee == HDRwithBloom)
	{
        glDrawBuffers(1, drawBuffers);
	}
	else
    {
        glDrawBuffers(1, drawBuffers);
        // glDrawBuffers(2, drawBuffers); 

        // Unbind the framebuffer, and revert to default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    

    if (Yee == HDRwithBloom || Yee == GaussianBlur)
    {
        ///// Bloom Effect with Gamma Correction (based on HDR) /////

        // Create an FBO for the bright-pass filter and blur
        glGenFramebuffers(1, &intermediateFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

        if (Yee == HDRwithBloom)
        {
            // Create two texture objects to ping-pong for the bright-pass filter
            // and the two-pass blur
            bloomBufWidth = width / 8;
            bloomBufHeight = height / 8;

            glGenTextures(1, &intermediateTex1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, intermediateTex1);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
            glActiveTexture(GL_TEXTURE2);

            glGenTextures(1, &intermediateTex2);
            glBindTexture(GL_TEXTURE_2D, intermediateTex2);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
        }
        
        if(Yee == GaussianBlur)
        {
            glGenTextures(1, &intermediateTex1);

            glActiveTexture(GL_TEXTURE0);   // Use texture unit 0

            glBindTexture(GL_TEXTURE_2D, intermediateTex1);

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, intermediateTex1, 0);

        glDrawBuffers(1, drawBuffers);

        ///// Bloom Effect with Gamma Correction (based on HDR) /////
    }

    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
/*
	///// Necessary part /////

    // Generate and bind the framebuffer
    // glDeleteFramebuffers(1, &hdrFBO);            // Deallocate to free the slot
    // glGenFramebuffers(1, &hdrFBO);
    glGenFramebuffers(1, &fboHandle);

    // glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    // Create the texture object
    // glGenTextures(1, &hdrTex);
    // glDeleteTextures(1, &hdrTex);                // Deallocate to free the slot
    glGenTextures(1, &fboTex);

    glActiveTexture(GL_TEXTURE0); // Use texture unit0
    
    // glBindTexture(GL_TEXTURE_2D, renderTex);
    glBindTexture(GL_TEXTURE_2D, fboTex);

    // glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
    
    // Bind the texture to the FBO
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, fboTex, 0);
    

    /// Beginning of depth buffer ///


    // Create the depth buffer
    // GLuint depthBuf; // declared as global variable, go to header file
    // glDeleteRenderbuffers(1, &depthBuf);         // Deallocate to free the slot
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    
    // Bind the depth buffer to the FBO 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                              GL_RENDERBUFFER, depthBuf);
    
    ///// Necessary part /////

    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    // GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0 };
    
    glDrawBuffers(1, drawBuffers);



    ///// Bloom Effect with Gamma Correction (based on HDR) /////

    // Create an FBO for the bright-pass filter and blur
    // glDeleteFramebuffers(1, &blurFbo);           // deallocate
    glGenFramebuffers(1, &intermediateFBO);

    // glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    // Create two texture objects to ping-pong for the bright-pass filter
    // and the two-pass blur
    bloomBufWidth = width / 8;
    bloomBufHeight = height / 8;

    // glDeleteTextures(1, &tex1);                  // deallocate
    glGenTextures(1, &intermediateTex1);

    glActiveTexture(GL_TEXTURE1);

    glBindTexture(GL_TEXTURE_2D, intermediateTex1);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
    glActiveTexture(GL_TEXTURE2);

    // glDeleteTextures(1, &tex2);                  // deallocate
    glGenTextures(1, &intermediateTex2);

    glBindTexture(GL_TEXTURE_2D, intermediateTex2);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);

    // Bind tex1 to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, intermediateTex1, 0);

    glDrawBuffers(1, drawBuffers);

    ///// Bloom Effect with Gamma Correction (based on HDR) /////



    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

*/
}

// this is self-made deallocator, it serve for one reason
// to free the slots and save some memory (at least I hope it can)
// it is called when you need to resize the viewport
void SceneBasic_Uniform::byeFBO()   // each glDelete... correspond to each glGen...
{
    glDeleteFramebuffers(1, &fboHandle);

    glDeleteTextures(1, &fboTex);

    glDeleteRenderbuffers(1, &depthBuf);

    glDeleteFramebuffers(1, &intermediateFBO);

    glDeleteTextures(1, &intermediateTex1);

    glDeleteTextures(1, &intermediateTex2);
}

// being called since initScene()
float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}

void SceneBasic_Uniform::computeLogAveLuminance()
{
    int size = width * height;
    std::vector<GLfloat> texData(size * 3);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, fboTex);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData.data());
    
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {

        float lum = glm::dot(vec3(texData[i * 3 + 0],
                                  texData[i * 3 + 1],
                                  texData[i * 3 + 2]),
                             vec3(0.2126f, 0.7152f, 0.0722f));
        sum += logf(lum + 0.00001f);
    }

    prog.setUniform("AveLum", expf(sum / size));
}