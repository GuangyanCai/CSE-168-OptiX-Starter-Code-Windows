// Windows only
#ifdef _WIN64
#define NOMINMAX 
#endif

// Optix related 
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

// Structs to be used for passing info from host to device
#include "Geometries.h"
#include "Lights.h"

#include "Constants.h"

// STL
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace optix;
// Avoid "using namespace std"
using std::string;
using std::vector;
using std::unordered_map;
using std::stack;
using std::ifstream;
using std::stringstream;
using std::runtime_error;

/**
 * OptixApp can read a scene file, construct the graph for Optix to traverse
 * and render the image described by the scene file.
 */

class OptixApp {
	Context context; // an Optix conext that encapsulates all OptiX resources
	Buffer resultBuffer; // a buffer that stores the rendered image
	unordered_map<string, Program> programs; // a map that maps a name to a program
	Material material; // the material used by all objects

	// Some info about the output image
	string outputFilename = "raytrace.png";
	unsigned int width, height;

	/**
	 * A helper function for readability. One should initialize all the Optix
	 * programs here.
	 */
	void initPrograms();

	/**
	 * Read the scene file and build the Optix Graph. This can be rewritten, or
	 * even separated out as an individual class, to support other scene file
	 * formats.
	 */
	void buildScene(string sceneFilename);

	/**
	 * A helper function to create a buffer and fill it with data.
	 */
	template <class T>
	Buffer createBuffer(vector<T> data);
public:
	/**
	 * The constructor.
	 */
	OptixApp(string sceneFilename);

	/**
	 * Run the Optix engine with the constructed graph. The resultBuffer should
	 * contain the rendered image as the function terminates.
	 */
	void run();

	/**
	 * Get the rendered result as a 1D array of bytes.
	 */
	vector<unsigned char> getResult();

	// Some getters
	string getOutputFilename() { return outputFilename; }
	int getWidth() { return width; }
	int getHeight() { return height; }
};