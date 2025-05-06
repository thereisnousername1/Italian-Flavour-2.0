#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"

/// <summary>
/// 
/// Base document: https://dle.plymouth.ac.uk/pluginfile.php/3548634/mod_resource/content/2/Basic%20Project%20-%20COMP3015%20Lab%201%201.pdf
/// 
/// In the Project solution explorer we have a folder called ：helper；. This folder contains 
/// all the necessary files to make this project work.The goal of this module is to focus
/// on the shader, so I won・t insist on the OpenGL side.Typically any changes you need
/// to make are to the shaders and the files in the ：Source Files； folder(except glad.c).
/// 
/// A media folder will be used throughout the module to add media files that we・re going
/// to use.
/// 
/// This is the format we・re going to use throughout the module and for submissions. (very important!!)
/// 
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
/// 

int main(int argc, char* argv[])
{
	// initialization of the glfw, glad is done in SceneRunner
	// in another word, SceneRunner is the window itself in this case
	SceneRunner runner("Italian Flavour 2.0");	// Title of the program

	std::unique_ptr<Scene> scene;

	scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());	// scene is a SceneBasic_Uniform class object with the type of Scene

	return runner.run(*scene);
}