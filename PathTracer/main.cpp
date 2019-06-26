#include "OptixApp.h"
#include "utils.h"

int main(int argc, char** argv) {
	try {
		// Create the optix app by passing in the path to the scene file
		OptixApp app(argv[1]);
		app.run();

		string outputFilename = app.getOutputFilename();
		int width = app.getWidth();
		int height = app.getHeight();
		vector<unsigned char> result = app.getResult();

		// Output a png image
		resultToPNG(outputFilename, width, height, result);

		// Display the result 
		displayResult(width, height, result);
	}
	catch (const std::exception & ex) {
		// Print out the exception for debugging
		std::cerr << ex.what() << std::endl;

	}
	return 0;
}