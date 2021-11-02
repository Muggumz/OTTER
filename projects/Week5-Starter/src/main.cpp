#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArrayObject.h"
#include "Shader.h"
#include "Camera.h"

#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "VertexTypes.h"

#define LOG_GL_NOTIFICATIONS

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
		case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
		case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
			#ifdef LOG_GL_NOTIFICATIONS
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
			#endif
		default: break;
	}
}

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(800, 800);
// The title of our GLFW window
std::string windowTitle = "Midterm Project";

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowSize = glm::ivec2(width, height);
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);
	/*
	static const GLfloat points[] = {
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f
	};

	static const GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	//VBO - Vertex buffer object
	VertexBuffer::Sptr posVbo = VertexBuffer::Create();
	posVbo->LoadData(points, 9);

	VertexBuffer::Sptr color_vbo = VertexBuffer::Create();
	color_vbo->LoadData(colors, 9);

	VertexArrayObject::Sptr vao = VertexArrayObject::Create();
	vao->AddVertexBuffer(posVbo, {
		BufferAttribute(0, 3, AttributeType::Float, 0, NULL, AttribUsage::Position)
	});
	vao->AddVertexBuffer(color_vbo, {
		{ 1, 3, AttributeType::Float, 0, NULL, AttribUsage::Color }
	});
	*/
	

	///////////////Odd Definitions

	// ie. int a = 5 or something

	int lives = 3; // for ball count down the road
	int score = 0; // for box score down the road
	bool respawn = true; // setup condition for making the player launch the ball at start

	// the balls current pos relative to axis
	float ballx = 0.0f;
	float bally = 0.0f;
	float ballz = 0.0f;

	// the balls velocity relative to axis
	float ballvelx = 0.0f;
	float ballvely = 0.0f;
	float ballvelz = 0.0f;

	bool box1destroyed = false; //bool condition for destroying the box

	//debug items
	bool test1 = false;
	bool test2 = true;

	bool isMoving = true;
	bool isButtonPressed = false;

	GLfloat paddleX = 0.0f;

	/////////////////////////// Camera and Shaders //////////////////////////////////
	/*
	static const float interleaved[] = {
		// X      Y    Z       R     G     B
		 0.5f, -0.5f, 0.5f,   0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,   0.3f, 0.2f, 0.5f,
		-0.5f,  0.5f, 0.5f,   1.0f, 1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,   1.0f, 1.0f, 1.0f
	};
	VertexBuffer::Sptr interleaved_vbo = VertexBuffer::Create();
	interleaved_vbo->LoadData(interleaved, 6 * 4);

	static const uint16_t indices[] = {
		3, 0, 1,
		3, 1, 2
	};
	IndexBuffer::Sptr interleaved_ibo = IndexBuffer::Create();
	interleaved_ibo->LoadData(indices, 3 * 2);

	size_t stride = sizeof(float) * 6;
	VertexArrayObject::Sptr vao2 = VertexArrayObject::Create();
	vao2->AddVertexBuffer(interleaved_vbo, {
		BufferAttribute(0, 3, AttributeType::Float, stride, 0, AttribUsage::Position),
		BufferAttribute(1, 3, AttributeType::Float, stride, sizeof(float) * 3, AttribUsage::Color),
	});
	vao2->SetIndexBuffer(interleaved_ibo);
	*/

	// Load our shaders
	Shader* shader = new Shader();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
	shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
	shader->Link();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// Get uniform location for the model view projection
	Camera::Sptr camera = Camera::Create();
	camera->SetPosition(glm::vec3(0, 0, 5));
	camera->LookAt(glm::vec3(0.0f));
	
	//////////////////////////////////////////////////////The Place we Make objects/////////////////////////////////////


	// Create a mat4 to store our mvp (for now)
	//glm::mat4 transform = glm::mat4(1.0f);
	glm::mat4 paddle = glm::mat4(1.0f);
	//glm::mat4 transform3 = glm::mat4(1.0f);
	glm::mat4 ball = glm::mat4(1.0f);
	glm::mat4 box1 = glm::mat4(1.0f);

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	LOG_INFO("Starting mesh build");

	

	MeshBuilder<VertexPosCol> mesh;
	MeshFactory::AddCube(mesh, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(3.0f,0.5f, 0.5f));
	VertexArrayObject::Sptr vao3 = mesh.Bake();

	MeshBuilder<VertexPosCol> mesh2;
	MeshFactory::AddCube(mesh, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	VertexArrayObject::Sptr vao4 = mesh.Bake();

	MeshBuilder<VertexPosCol> mesh3;
	MeshFactory::AddCube(mesh, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	VertexArrayObject::Sptr vao5 = mesh.Bake();

	/// //////////////
	

	/////////////////////////////////////////////////// Game loop ///////////////////////////////////////////////////////////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// WEEK 5: Input handling
		if (glfwGetKey(window, GLFW_KEY_A)) {
			
			if (!isButtonPressed == true) {
				// This is the action we want to perform on key press
				isMoving = !isMoving;
				paddleX += .1f;
			}
			//isButtonPressed = true;
		}
		else {
			//isButtonPressed = false;
			
		}
		
		if (glfwGetKey(window, GLFW_KEY_D)) {

			if (!isButtonPressed == true) {
				// This is the action we want to perform on key press
				isMoving = !isMoving;
				paddleX -= 0.1f;
			}
			//isButtonPressed = true;
		}
		else {
			//isButtonPressed = false;

		}
		
		if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			if (!isButtonPressed == true && respawn == true) {
				respawn = false;
				ballvelx = 0.0f;
				ballvely = 1.0f;
				ballvelz = 0.0f;
				ballx = paddleX;
				bally = 0.1f;
			}
			//isButtonPressed = true
		}
		else {
			//isButtonPressed = false;
		}

		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// TODO: Week 5 - toggle code

		// Rotate our models around the z axis
		
		if (isMoving) {
			//transform  = glm::rotate(glm::mat4(1.0f), static_cast<float>(thisFrame), glm::vec3(0, 0, 1));
			paddle = glm::translate(glm::mat4(1.0f), glm::vec3(paddleX, 0.0f, 0.0f));
		}

		//////////////////////// Ball Movement ////////////////////////////////


		if (respawn == true) {
			ball = glm::translate(glm::mat4(1.0f), glm::vec3(paddleX, 1.0f, 0.0f));
		}
		else
		{
			ballx -= ballvelx/50;
			bally -= ballvely/50;
			ballz -= ballvelz/50;
			ball = glm::translate(glm::mat4(1.0f), glm::vec3(ballx, bally, ballz));
		}
		
		//// respawn condition and code
		if (bally > 2.0f)
		{
			respawn = true;
			lives -= 1;

			ballvelx = 0;
			ballvely = 0;
			ballvelz = 0;
			ball = glm::translate(glm::mat4(1.0f), glm::vec3(paddleX, 1.0f, 0.0f));
		}

		
		
		//transform3 = glm::rotate(glm::mat4(1.0f), -static_cast<float>(thisFrame), glm::vec3(1, 0, 0)) * glm::translate(glm::mat4(1.0f), glm::vec3(0, glm::sin(static_cast<float>(thisFrame)), 0.0f));
		
		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind our shader and upload the uniform
		shader->Bind();
		/*
		// Draw spinny triangle
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform);
		vao->Draw();
		*/
		// Draw MeshFactory Sample

																								// bug with this part. Making a new object loads a copy of all the old objects already made. 
																								// I guess it needs to maybe clear the old creations but idk how we do that
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* paddle);
		vao3->Draw();

		VertexArrayObject::Unbind();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* ball);
		vao4->Draw();

		VertexArrayObject::Unbind(); 
		
		
		////// Box destory code
		///// let it be known, i fucking hate this syntax
		///// this was very bruh for 2 hours and like 4 lines of code
		/////curse you coding grammer

		if (box1destroyed != true)
		{
			if (((0.5 > ballx + 0.125 && ballx + 0.125 > -0.5) || (0.5 > ballx - 0.125 && ballx - 0.125 > -0.5)) && ((-1.5 > bally - 0.125 && bally - 0.125 > -2.5) || (-1.5 > bally + 0.125 && bally + 0.125 > 2.5)))
			{
				box1destroyed = true;
			}
			else
			{
				shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* box1);
				vao5->Draw();
			}
				
		}
	
		std::cout << test1 << std::endl;

		/*
		// Draw OBJ loaded model
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform3);
		vao4->Draw();
		*/
		VertexArrayObject::Unbind();

		//checking DT viability
		//std::cout << dt << std::endl;

		glfwSwapBuffers(window);
	}



	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}
