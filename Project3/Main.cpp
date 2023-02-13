// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include "objloader.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable : 4996)

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <control.h>
using namespace glm;
using namespace std;

// Include standard headers
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <string.h>







GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {



	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// If Q in Caps Lock is pressed close the window 
	if (GetKeyState(VK_CAPITAL) & 0x8000 != GLFW_RELEASE) {
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		if (mods == GLFW_MOD_SHIFT) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}
};

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(800, 800, "Solar System", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixPlanet = glGetUniformLocation(programID, "MVP");
	GLuint MatrixSun = glGetUniformLocation(programID, "MVP");

	int width, height, nrChannels;
	unsigned char* dataS;

	GLuint texture[2];
	glGenTextures(2, texture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	dataS = stbi_load("sun.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataS);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(dataS);

	// Get a handle for our "Texture" uniform
	GLuint TextureSun = glGetUniformLocation(programID, "TextureSun");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glUniform1i(TextureSun, 0);

	// Read our sun.obj file
	std::vector<glm::vec3> verticesS;
	std::vector<glm::vec3> normalsS;
	std::vector<glm::vec2> uvsS;
	bool resS = loadOBJ("sun.obj", verticesS, uvsS, normalsS);

	GLuint vertexbufferSun;
	glGenBuffers(1, &vertexbufferSun);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferSun);
	glBufferData(GL_ARRAY_BUFFER, verticesS.size() * sizeof(glm::vec3), &verticesS[0], GL_STATIC_DRAW);

	GLuint uvbufferSun;
	glGenBuffers(1, &uvbufferSun);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferSun);
	glBufferData(GL_ARRAY_BUFFER, uvsS.size() * sizeof(glm::vec2), &uvsS[0], GL_STATIC_DRAW);


	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	dataS = stbi_load("planet.jpg", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataS);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(dataS);

	// Get a handle for our "Texture2" uniform
	GLuint TexturePlanet = glGetUniformLocation(programID, "TexturePlanet");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(TexturePlanet, 0);

	// Read our planet.obj file
	std::vector<glm::vec3> verticesP;
	std::vector<glm::vec3> normalsP;
	std::vector<glm::vec2> uvsP;
	bool resP = loadOBJ("planet.obj", verticesP, uvsP, normalsP);


	GLuint vertexbufferPlanet;
	glGenBuffers(1, &vertexbufferPlanet);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferPlanet);
	glBufferData(GL_ARRAY_BUFFER, verticesP.size() * sizeof(glm::vec3), &verticesP[0], GL_STATIC_DRAW);

	GLuint uvbufferPlanet;
	glGenBuffers(1, &uvbufferPlanet);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferPlanet);
	glBufferData(GL_ARRAY_BUFFER, uvsP.size() * sizeof(glm::vec2), &uvsP[0], GL_STATIC_DRAW);


	glfwSetKeyCallback(window, key_callback);
	glm::vec3 rotationP(0.0f);
	glm::vec3 scaleP(1.0f);
	float planetSpeed = 0.03f;

	// CAMERA
	glm::vec3 camPos = glm::vec3(0.0f,0.0f,120.0f);
	glm::vec3 camFront = glm::vec3(0.0f,0.0f,-1.0f);
	glm::vec3 camUp = glm::vec3(0.0f,1.0f,0.0f);

	// TIMING
	float dTime = 0.0f;
	float lFrame = 0.0f;
	float x = 0.0f;
	float y = 0.0f;

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		float currentFrame = glfwGetTime();
		dTime = currentFrame - lFrame;
		lFrame = currentFrame;

		glm::mat4 modelSun = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 viewSun = glm::lookAt(camPos, camPos + camFront, camUp);
		glm::mat4 projectionSun = glm::perspective(glm::radians(45.0f), 800.0f/800.0f, 0.1f, 200.0f);

		glm::mat4 MVPsun = projectionSun * viewSun * modelSun;
		glUniformMatrix4fv(MatrixSun, 1, GL_FALSE, &MVPsun[0][0]);

		float camSpeed = 10.5 * dTime;
		// Camera Moves
		if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
			camPos += camSpeed*camFront;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
			camPos -= camSpeed * camFront;
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			camPos += camSpeed * camUp;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			camPos -= camSpeed * camUp;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			
		}


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glUniform1i(TexturePlanet, 0);


		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferSun);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferSun);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the Sun !
		glDrawArrays(GL_TRIANGLES, 0, verticesS.size());


		glm::mat4 modelPlanet = glm::mat4(1.0f);

		modelPlanet = glm::translate(modelPlanet, glm::vec3(0.0f,0.0f,0.0f));
		modelPlanet = glm::rotate(modelPlanet, glm::radians(rotationP.x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelPlanet = glm::rotate(modelPlanet, glm::radians(rotationP.y), glm::vec3(0.0f, 1.0f, 0.0f));
		modelPlanet = glm::rotate(modelPlanet, glm::radians(rotationP.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 projectionPlanet = glm::perspective(glm::radians(45.0f), 800.0f/800.0f, 0.1f, 100.0f);

		glm::mat4 MVPplanet = projectionSun * viewSun * modelPlanet;
		glUniformMatrix4fv(MatrixPlanet, 1, GL_FALSE, &MVPplanet[0][0]);

		//Rotate Planet
		rotationP.x = rotationP.x + planetSpeed;
		rotationP.y = rotationP.y + 0.1f;
		rotationP.z = rotationP.z + 0.001f;
		

		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
			planetSpeed = planetSpeed + 0.001f;
		}
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			if (planetSpeed > 0.002f) {
				planetSpeed = planetSpeed - 0.001f;
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glUniform1i(TexturePlanet, 0);


		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferPlanet);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferPlanet);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);


		// Draw the Planet !
		glDrawArrays(GL_TRIANGLES, 0, verticesP.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}while (glfwWindowShouldClose(window) == 0);
	
	
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbufferSun);
	glDeleteBuffers(1, &uvbufferSun);
	glDeleteTextures(1, &texture[0]);
	glDeleteBuffers(1, &vertexbufferPlanet);
	glDeleteBuffers(1, &uvbufferPlanet);
	glDeleteTextures(1, &texture[1]);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
	