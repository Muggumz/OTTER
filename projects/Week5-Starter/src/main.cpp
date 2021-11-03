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

#include <windows.h>

#include <string>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define LOG_GL_NOTIFICATIONS

unsigned char* image;
int width, height;

void loadImage(const std::string& filename) {
	int channels;
	stbi_set_flip_vertically_on_load(true); //because opengl loads it flipped

	// Load image
	image = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	if (image)
		std::cout << "Image loaded: " << width << " x " << height << std::endl;
	else std::cout << "Failed to load texture!!!!!" << std::endl;

}

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
	
	////////////////////////// DISPLAYING SCORE //////////////////////////////

	static const GLfloat points[] = {//front face, 2 triangles
		-0.5f, -0.5f, 0.5f,//0  front face
		0.5f, -0.5f, 0.5f, //3
		-0.5f, 0.5f, 0.5f, //1
		0.5f, -0.5f, 0.5f, //3
		0.5f, 0.5f, 0.5f, //2
		-0.5f, 0.5f, 0.5f //1
	};

	// Color data
	static const GLfloat colors[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	static const GLfloat normals[] = {
		0.0f, 0.0f, 1.0f, // front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	static const GLfloat uv[] = {
		//Front
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	/// LECTURE 05
	GLfloat cameraPos[] = { 0.0f, 0.0f, 3.0f };
	GLfloat lightPos[] = { 0.0f, 0.0f, 3.0f };
	////////////

	GLuint pos_vbo = 0;
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	GLuint color_vbo = 1;
	glGenBuffers(1, &color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);

	//						index, size, type, normalize?, stride, pointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/////////// LECTURE 05
	GLuint normal_vbo = 2;
	glGenBuffers(1, &normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//////////////


	glEnableVertexAttribArray(0);//pos
	glEnableVertexAttribArray(1);//colors
	/// LEC 05
	glEnableVertexAttribArray(2);//normals
	////////////

	///////////// LETURE 07 ///////////////
	GLuint uv_vbo = 3;
	glGenBuffers(1, &uv_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);

	GLuint textureHandle[2];
	

	loadImage("metalBox.jpg");

	glGenTextures(2, textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	//Texture Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//free image space
	stbi_image_free(image);
	/*
	// Load our shaders
	if (!loadShaders())
		return 1;
	*/
	////////// LECTURE 04 //////////////

	// Projection - FoV, aspect ratio, near, far
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f),
		(float)width / (float)height, 0.001f, 100.0f);

	// View matrix - Camera


	// Model matrix
	glm::mat4 Model = glm::mat4(1.0f);//Identity matrix - resets your matrix

	glm::mat4 mvp;// = Projection * View * Model;

	// Handle for our mvp
	//GLuint matrixMVP = glGetUniformLocation(shader_program, "MVP");
	/*
	////// LEC 05 - uniform variables
	GLuint matrixModel = glGetUniformLocation(shader_program, "Model");
	GLuint lightPosID = glGetUniformLocation(shader_program, "lightPos");
	GLuint cameraPosID = glGetUniformLocation(shader_program, "cameraPos");
	//////////////////////////////// */

	// GL states
	glEnable(GL_DEPTH_TEST);
	// LEC 05
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_FRONT); //GL_BACK, GL_FRONT_AND_BACK

	
	/////////////////////////////////////////////////////////////////////////////



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

	bool collision = false;

	bool boxdestroyed[16]; //bool condition for destroying the box
	bool boxdamaged[16]; 

	for (int counter = 0; counter < 16; counter++) {
		boxdestroyed[counter] = false;
		boxdamaged[counter] = false;
	}

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

	//Texture 0
	
	// Load our shaders
	Shader* shader = new Shader();
	shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", ShaderPartType::Vertex);
	shader->LoadShaderPartFromFile("shaders/frag_shader.glsl", ShaderPartType::Fragment);
	shader->Link();
	/*
	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	*/
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

	///////////////////////BOXES//////////////////
	glm::mat4 boxes[16];

	for (int counter = 0; counter < 16; counter++) {
		boxes[counter] = glm::mat4(1.0f);
	}
	/////////////////////////////////////////////

	glm::mat4 Walls = glm::mat4(1.0f);

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	LOG_INFO("Starting mesh build");

	

	MeshBuilder<VertexPosCol> paddleMesh;
	MeshFactory::AddCube(paddleMesh, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(3.0f,0.5f, 0.5f));
	VertexArrayObject::Sptr paddleVAO = paddleMesh.Bake();
	
	MeshBuilder<VertexPosCol> ballMesh;
	MeshFactory::AddCube(ballMesh, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	VertexArrayObject::Sptr ballVAO = ballMesh.Bake();
	




	//////////////////////////////// BOXES ///////////////////////////////
	/* Box Map  (bracket is how many hits to destroy)
	
			[box0(2)]		[box1(1)]		[box2(2)]		[box3(1)]		[box4(2)]

					[box5(1)]		[box6(1)]		[box7(1)]    [box8(1)]

			[box9(1)]		[box10(2)]		[box11(1)]		[box12(2)]		[box13(1)]

									[box14(2)]		[box15(2)]
	
	
	*/

	MeshBuilder<VertexPosCol> boxMesh[16];

	glm::vec3 boxCoords[16];

	boxCoords[0] = glm::vec3(4.0f, -5.0f, 0.0f);
	boxCoords[1] = glm::vec3(2.0f, -5.0f, 0.0f);
	boxCoords[2] = glm::vec3(0.0f, -5.0f, 0.0f);
	boxCoords[3] = glm::vec3(-2.0f, -5.0f, 0.0f);
	boxCoords[4] = glm::vec3(-4.0f, -5.0f, 0.0f);

	boxCoords[5] = glm::vec3(3.0f, -4.0f, 0.0f);
	boxCoords[6] = glm::vec3(1.0f, -4.0f, 0.0f);
	boxCoords[7] = glm::vec3(-1.0f, -4.0f, 0.0f);
	boxCoords[8] = glm::vec3(-3.0f, -4.0f, 0.0f);
	
	boxCoords[9] = glm::vec3(4.0f, -3.0f, 0.0f);
	boxCoords[10] = glm::vec3(2.0f, -3.0f, 0.0f);
	boxCoords[11] = glm::vec3(0.0f, -3.0f, 0.0f);
	boxCoords[12]= glm::vec3(-2.0f, -3.0f, 0.0f);
	boxCoords[13] = glm::vec3(-4.0f, -3.0f, 0.0f);

	boxCoords[14] = glm::vec3(1.0f, -2.0f, 0.0f);
	boxCoords[15] = glm::vec3(-1.0f, -2.0f, 0.0f);

	VertexArrayObject::Sptr boxVAO[16];

	for (int counter = 0; counter < 16; counter++) {
		MeshFactory::AddCube(boxMesh[counter], boxCoords[counter], glm::vec3(1.75f, 0.5f, 0.1f));

		boxVAO[counter] = boxMesh[counter].Bake();
	}
	/////////////////////////////// WALLS ////////////////////////////////

	MeshBuilder<VertexPosCol> leftWallMesh;
	MeshFactory::AddCube(leftWallMesh, glm::vec3(7.0f, 0.0f, 0.0f), glm::vec3(0.5f, 15.0f, 0.5f));
	VertexArrayObject::Sptr leftWallVAO = leftWallMesh.Bake();

	MeshBuilder<VertexPosCol> rightWallMesh;
	MeshFactory::AddCube(rightWallMesh, glm::vec3(-7.0f, 0.0f, 0.0f), glm::vec3(0.5f, 15.0f, 0.5f));
	VertexArrayObject::Sptr rightWallVAO = rightWallMesh.Bake();

	MeshBuilder<VertexPosCol> ceilingMesh;
	MeshFactory::AddCube(ceilingMesh, glm::vec3(0.0f, -7.0f, 0.0f), glm::vec3(15.0f, 0.5f, 0.5f));
	VertexArrayObject::Sptr ceilingVAO = ceilingMesh.Bake();

	
	/////////////////
	

	/////////////////////////////////////////////////// Game loop ///////////////////////////////////////////////////////////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
				bally = 2.5f;
			}
			//isButtonPressed = true
		}
		else {
			//isButtonPressed = false;
		}

		//***Manually reset ball for testing purposes***
		if (glfwGetKey(window, GLFW_KEY_E)) {

			if (!isButtonPressed == true) {
				// This is the action we want to perform on key press
				ballvelx = 0.0f;
				ballvely = 2.0f;
				ballvelz = 0.0f;
				ballx = paddleX;
				bally = 2.5f;
			}
			//isButtonPressed = true;
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
			ballx -= ballvelx/25;
			bally -= ballvely/25;
			ballz -= ballvelz/25;
			ball = glm::translate(glm::mat4(1.0f), glm::vec3(ballx, bally, ballz));
		}
					///////collision pass 1
		/*
		if (collision == true) {
			ballvelx *= -1;
			ballvely *= -1;
			collision = false;
		}
		*/
		
		//// respawn condition and code
		if (bally > 8.0f)
		{
			respawn = true;
			lives -= 1;

			ballvelx = 0;
			ballvely = 0;
			ballvelz = 0;
			ball = glm::translate(glm::mat4(1.0f), glm::vec3(paddleX, 1.0f, 0.0f));
		}

		//Keep paddle inside play space
		if (paddleX  >= 6.26 - 1.5)
		{
			paddleX = 6.25 - 1.5;
		}
		else if (paddleX <= -6.26 + 1.5)
		{
			paddleX = -6.25 + 1.5;
		}

		//////////////////////////////        UI        ///////////////////////////////////////////
		if (score == 0) {

		 }
		
		///////////////////////////////////////////////////////////////////////////////////////////

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

																								
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* paddle);
		paddleVAO->Draw();

		VertexArrayObject::Unbind();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* ball);
		ballVAO->Draw();

		VertexArrayObject::Unbind(); 


		///////////// WALLS /////////////////
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* Walls);
		leftWallVAO->Draw();
		VertexArrayObject::Unbind();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection()* Walls);
		rightWallVAO->Draw();
		VertexArrayObject::Unbind();

		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * Walls);
		ceilingVAO->Draw();
		VertexArrayObject::Unbind();
		/////////////////////////////////////

		//Yo mon ami, it would appear that the ball does not quite visually match where it actually is in code, that or the blocks do not. I've adjusted some code in the back end.
		//This way the ball believes it is elsewhere to match the boxes being straaangely out of place. Im thinking the boxes may be the strange ones in this scenario.
		//Either way it looks much better now.
		////Boxes can have two lives but they currently have no texture or model and are thus invisibruh, however they currently generate a texture at box[0] (since thats what they do)
		
		float ballXLeft = ballx + 0.25;
		float ballXRight = ballx - 0.25;
		float ballYTop = bally + 1.75f;
		float ballYBottom = bally + 1.5f;

		/////////////////////////////////////////////////// Box Physics

		for (int counter = 0; counter < 16; counter++) {
			
			if (counter == 1 || counter == 3 || counter == 5 || counter == 6 || counter == 7 || counter == 8 || counter == 9 || counter == 11 || counter == 13) {
				// Normal box (destroys in one hit)
				if (boxdestroyed[counter] != true)
				{
					if (((ballXLeft > boxCoords[counter].x - 0.875) && (ballXLeft < boxCoords[counter].x + 0.875) && (ballYTop > boxCoords[counter].y - 0.25) && (ballYTop < boxCoords[counter].y + 0.25)) //Top left corner in brick
						|| ((ballXLeft > boxCoords[counter].x - 0.875) && (ballXLeft < boxCoords[counter].x + 0.875) && (ballYBottom > boxCoords[counter].y - 0.25) && (ballYBottom < boxCoords[counter].y + 0.25)) //Bottom left Corner in brick
						|| ((ballXRight > boxCoords[counter].x - 0.875) && (ballXRight < boxCoords[counter].x + 0.875) && (ballYTop > boxCoords[counter].y - 0.25) && (ballYTop < boxCoords[counter].y + 0.25)) //Top right corner in brick
						|| ((ballXRight > boxCoords[counter].x - 0.875) && (ballXRight < boxCoords[counter].x + 0.875) && (ballYBottom > boxCoords[counter].y - 0.25) && (ballYBottom < boxCoords[counter].y + 0.25))) //Bottom right Corner in brick
					{
						boxdestroyed[counter] = true;
						//collision = true;
						score += 100;

						if (ballXLeft < boxCoords[counter].x - 0.875 || ballXRight > boxCoords[counter].x + 0.875)
						{
							ballvelx *= -1.0f;
						}

						if (ballYTop > boxCoords[counter].y + 0.25 || ballYBottom < boxCoords[counter].y + 0.25)
						{
							ballvely *= -1.0f;
						}

					}
					else
					{
						shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * boxes[counter]);
						boxVAO[counter]->Draw();
					}

				}
			}
			else if (counter == 0 || counter == 2 || counter == 4 || counter == 10 || counter == 12 || counter == 14 || counter == 15) {
				//Box type two (takes 2 hits to destroy)
				if (boxdamaged[counter] != true)
				{
					if ((ballXLeft > boxCoords[counter].x - 0.875 && ballXLeft < boxCoords[counter].x + 0.875 && ballYTop > boxCoords[counter].y - 0.25 && ballYTop < boxCoords[counter].y + 0.25) //Top left corner in brick
						|| (ballXLeft > boxCoords[counter].x - 0.875 && ballXLeft < boxCoords[counter].x + 0.875 && ballYBottom > boxCoords[counter].y - 0.25 && ballYBottom < boxCoords[counter].y + 0.25) //Bottom left Corner in brick
						|| (ballXRight > boxCoords[counter].x - 0.875 && ballXRight < boxCoords[counter].x + 0.875 && ballYTop > boxCoords[counter].y - 0.25 && ballYTop < boxCoords[counter].y + 0.25) //Top right corner in brick
						|| (ballXRight > boxCoords[counter].x - 0.875 && ballXRight < boxCoords[counter].x + 0.875 && ballYBottom > boxCoords[counter].y - 0.25 && ballYBottom < boxCoords[counter].y + 0.25)) //Bottom right Corner in brick
					{
						boxdamaged[counter] = true;
						score += 100;
						if (ballXLeft < boxCoords[counter].x - 0.875 || ballXRight > boxCoords[counter].x + 0.875)
						{
							ballvelx *= -1.0f;
						}

						if (ballYTop > boxCoords[counter].y + 0.25 || ballYBottom < boxCoords[counter].y + 0.25)
						{
							ballvely *= -1.0f;
						}


						/*
						//draw damaged box
						shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * boxes[counter]);
						boxVAO[counter]->Draw();
						*/
					}
					else
					{
						//draw undamaged box
						shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * boxes[counter]);
						boxVAO[counter]->Draw();
					}

				}
				else if (boxdamaged[counter] == true && boxdestroyed[counter] != true)
				{

					if ((ballXLeft > boxCoords[counter].x - 0.875 && ballXLeft < boxCoords[counter].x + 0.875 && ballYTop > boxCoords[counter].y - 0.25 && ballYTop < boxCoords[counter].y + 0.25) //Top left corner in brick
						|| (ballXLeft > boxCoords[counter].x - 0.875 && ballXLeft < boxCoords[counter].x + 0.875 && ballYBottom > boxCoords[counter].y - 0.25 && ballYBottom < boxCoords[counter].y + 0.25) //Bottom left Corner in brick
						|| (ballXRight > boxCoords[counter].x - 0.875 && ballXRight < boxCoords[counter].x + 0.875 && ballYTop > boxCoords[counter].y - 0.25 && ballYTop < boxCoords[counter].y + 0.25) //Top right corner in brick
						|| (ballXRight > boxCoords[counter].x - 0.875 && ballXRight < boxCoords[counter].x + 0.875 && ballYBottom > boxCoords[counter].y- 0.25 && ballYBottom < boxCoords[counter].y + 0.25)) //Bottom right Corner in brick
					{
						boxdestroyed[counter] = true;
						score += 100;
						//don't draw box as it is destroyed
						//collision = true;

						if (ballXLeft < boxCoords[counter].x - 0.875 || ballXRight > boxCoords[counter].x + 0.875)
						{
							ballvelx *= -1.0f;
						}

						if (ballYTop > boxCoords[counter].y + 0.25 || ballYBottom < boxCoords[counter].y + 0.25)
						{
							ballvely *= -1.0f;
						}

					}
					else
					{
						//draw damaged box
						shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * boxes[0]);
						boxVAO[0]->Draw();

					}
				}
			}
			/*
			for (int counter = 0; counter < 16; counter++) {
				if (boxdestroyed[counter] = false) {
					shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * boxes[0]);
					boxVAO[0]->Draw();
				}
			}
			*/
		}
		
		////////////////////////// Paddle Physics


		if (respawn != true)
		{
			if ( (ballx > paddleX - 1.5f) && (ballx < paddleX + 1.5f) && (bally + 0.25f > 3.0f) && (bally - 0.25f < 3.0f) )
			{
				ballvely *= -1.0f;

				if (ballx > paddleX)
				{
					ballvelx = -1 * (ballx - paddleX);
				}
				else if (ballx < paddleX)
				{
					ballvelx = -1 * (ballx - paddleX);
				}

			}
		}

		if (respawn != true)
		{
			if ((ballx > 6.75f) || (ballx < -6.75f))
			{
				ballvelx *= -1.0f;
			}
			else if (bally < -8.0f)
			{
				ballvely *= -1.0f;
			}
		}

		/*
		// Draw OBJ loaded model
		shader->SetUniformMatrix("u_ModelViewProjection", camera->GetViewProjection() * transform3);
		vao4->Draw();
		*/
		VertexArrayObject::Unbind();
		

		//////////////////////////////////Debug Testing Zone

		//std::cout << score << std::endl;
		//std::cout << ballYTop << std::endl;

		//std::cout << boxCoords[9].y << std::endl;

		///////////////////////////////////////
		////// Bind texture 1
		glBindTexture(GL_TEXTURE_2D, textureHandle[0]);
		///// draw 
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
	}



	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}

