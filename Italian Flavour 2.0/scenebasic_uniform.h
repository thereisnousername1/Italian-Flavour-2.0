#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"
#include "helper/sphere.h"
#include "helper/skybox.h"

// SceneBasic_Uniform is a subclass of the Scene class
class SceneBasic_Uniform : public Scene
{
private:

    int viewportWidth, viewportHeight;

    GLSLProgram prog, explodeProg;

    GLSLProgram skyProg;

    GLSLProgram textureProg;

    float angle;

    float tPrev;

    float rotSpeed;

    glm::mat4 mv, viewport;
    
    std::unique_ptr<ObjMesh> mesh;

    // Torus torus;
    Sphere sphere;

    Plane plane;
    Teapot teapot;

    SkyBox sky;

    GLuint fsQuad, fboHandle, fboTex;           // basics requires for image processing (setupFBO())
    GLuint depthBuf;                            // depth logic related, also requires for edge detection
    // GLuint intermediateTex, intermediateFBO; // requires for gaussian blur
    GLuint intermediateTex1, intermediateTex2, intermediateFBO;
    
    GLuint linearSampler, nearestSampler;       // requires for bloom effect with gamma correction
    int bloomBufWidth, bloomBufHeight;          // requires for bloom effect with gamma correction

    bool showWireframe = false; // Set true to show wireframe

    float temp;     // declared as global for instant gauss calculate (perhaps)

    void setMatrices();

    void setMatrices(GLSLProgram&);

    void compile();
    
    void setupFBO();

    void byeFBO();

    // void renderToTexture();
    void pass1();

    void pass1alt();

    // void renderScene();
    void pass2();

    void pass3();

    void pass4();

    void pass5();

    void edge2();

    void gass2();
    void gass3();

    void HDR2();

    float gauss(float, float);

    void computeLogAveLuminance();
    void drawScene();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
