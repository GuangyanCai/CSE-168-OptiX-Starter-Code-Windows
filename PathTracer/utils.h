#pragma once

// OpenGL 
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

// A library for outputing images
#include <lodepng.h>

// STL
#include <vector>
#include <string>
#include <iostream>

// For compiling shaders
#include "shaders.h"

using std::vector;
using std::string;

/**
 * Output a png image of the rendered result.
 */
void resultToPNG(string outputFilename, int width, int height, const vector<unsigned char> & result);

/**
 * Display the rendered result using OpenGL.
 */
void displayResult(int width, int height, const vector<unsigned char> & result);