#pragma once

#include <glm/glm.hpp>

/// <summary>
/// 
/// class Scene define what is a scene(Bullshit)
/// most importantly, this structure prevent a messy pile of codes
/// 
/// in this class it only define what is inside the scene
/// nothing less, nothing more
/// as init of the window is done in the scenerunner class
/// details is defined in its subclass, scenebasic_uniform class
/// 
/// </summary>

enum LightSwitch
{
    PhongOnly,  // default
    EdgeDetect,
    GaussianBlur,
    HDR,
    HDRwithBloom
};

class Scene
{
protected:
	glm::mat4 model, view, projection;

public:
    int width;
    int height;

    /// features section ///

    bool showWireframe = false; // Set true to show wireframe

    // lighting model switch

    enum LightSwitch Yee = PhongOnly;

    bool Phong = true;  // by default = true, if false -> Blinn-Phong

    //Relative position within world space
    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    //The direction of travel
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    //Up position within world space
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float sigma2 = 25.0f;   // declared as global for instant gauss calculate

    float Gamma = 2.2f;     // gamma correction

    /// features section ///

    // by default 800 and 600 will be the value once every scene object is created and constructor is executed
	Scene() : m_animate(true), width(800), height(600) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}

    /**
      Load textures, initialize shaders, etc.
      */
    virtual void initScene() = 0;

    /**
      This is called prior to every frame.  Use this
      to update your animation.
      */
    virtual void update( float t ) = 0;

    /**
      Draw your scene.
      */
    virtual void render() = 0;

    /**
      Called when screen is resized
      */
    virtual void resize(int, int) = 0;
    
    void animate( bool value ) { m_animate = value; }
    bool animating() { return m_animate; }
    
protected:
	bool m_animate;
};