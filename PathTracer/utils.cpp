#include "utils.h"

void resultToPNG(string outputFilename, int width, int height, const vector<unsigned char> & result) {
	// Attempt to use lodepng to make the png image
	unsigned error = lodepng::encode(outputFilename, result, width, height);
	if (error) {
		throw std::runtime_error(lodepng_error_text(error));
	}
}

void displayResult(int width, int height, const vector<unsigned char> & result) {
	// Initialize GLFW
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize glfw.");

	// Initialize a GLFWwindow
	GLFWwindow * window = glfwCreateWindow(width, height, "Result", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		throw std::runtime_error("Failed to initialize the window.");
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		throw std::runtime_error("Failed to initialize glew.");

	// Setup OpenGL stuff

	GLuint program, vert, frag, vao, texID;
	GLuint vbos[2];
	vector<float> vertices {
		-1, -1,
		1, -1,
		-1, 1,
		1, 1,
		-1, 1,
		1, -1,
	}; 
	vector<float> texCoords {
		0, 1,
		1, 1,
		0, 0,
		1, 0,
		0, 0,
		1, 1,
	};

	// Compile shaders
	vert = initshaders(GL_VERTEX_SHADER, "texture.vert.glsl");
	frag = initshaders(GL_FRAGMENT_SHADER, "texture.frag.glsl");
	program = initprogram(vert, frag);
	glUseProgram(program);

	// Setup buffers to draw a quad
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, vbos);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	// Use the result to create a texture
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glClearColor(0, 0, 0, 1);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Draw the quad
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Create a callback function to handle resizing
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int width, int height) {
		// Draw the quad again with the new width and height
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
	});

	glfwSwapBuffers(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwTerminate();
}