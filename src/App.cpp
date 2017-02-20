

/** \file App.cpp */
#include "App.h"
#include <iostream>
using namespace std;
using namespace glm;

namespace basicgraphics {

	glm::vec3 tableMin;
	glm::vec3 tableMax;

	App::App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight) : BaseApp(argc, argv, windowName, windowWidth, windowHeight) {
		lastTime = glfwGetTime();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0);

		// Initialize the cylinders that make up the model. We're using unique_ptrs here so they automatically deallocate.
		paddle.reset(new Cylinder(vec3(0, 0, -0.5), vec3(0, 0, 0.5), 8.0, vec4(0.5, 0, 0, 1.0)));
		handle.reset(new Cylinder(vec3(0, -7.5, 0), vec3(0, -16, 0), 1.5, vec4(0.3, 0.4, 0, 1.0)));

		double tableWidth = 152.5;
		double tableLength = 274;
		double netOverhang = 15.25;
		double netDisplacement = (tableWidth / 2) + netOverhang;
		double netHeight = 15.25;

		tableMin = vec3(-76.25, -10, -137);
		tableMax = vec3(76.25, 0, 137);
		tableNormal = vec3(0, 1, 0);
		table.reset(new Box(tableMin, tableMax, vec4(0, 1, 0, 1)));
		net.reset(new Line(vec3(-netDisplacement, netHeight/2, 0), vec3(netDisplacement, netHeight/2, 0), vec3(0, 0, 1), netHeight/2, vec4(1, 1, 1, 1)));
	}

	App::~App()
	{
	}


	void App::onEvent(shared_ptr<Event> event) {
		string name = event->getName();
		if (name == "kbd_ESC_down") {
			glfwSetWindowShouldClose(_window, 1);
		}
		else if (name == "mouse_pointer") {
			vec2 mouseXY = event->get2DData();
            
            int width, height;
            glfwGetWindowSize(_window, &width, &height);

			// This block of code maps the 2D position of the mouse in screen space to a 3D position
			// 20 cm above the ping pong table.  It also rotates the paddle to make the handle move
			// in a cool way.  It also makes sure that the paddle does not cross the net and go onto
			// the opponent's side.
			float xneg1to1 = mouseXY.x / width * 2.0 - 1.0;
			float y0to1 = mouseXY.y / height;
			mat4 rotZ = toMat4(angleAxis(glm::sin(-xneg1to1), vec3(0, 0, 1)));

            glm::vec3 lastPaddlePos = glm::column(paddleFrame, 3);
			paddleFrame = glm::translate(mat4(1.0), vec3(xneg1to1 * 100.0, 20.0, glm::max(y0to1 * 137.0 + 20.0, 0.0))) * rotZ;
			vec3 newPos = glm::column(paddleFrame, 3);

			// This is a weighted average.  Update the velocity to be 10% the velocity calculated 
			// at the previous frame and 90% the velocity calculated at this frame.
			paddleVel = 0.1f*paddleVel + 0.9f*(newPos - lastPaddlePos);
		}
		else if (name == "kbd_SPACE_up") {
			// This is where you can "serve" a new ball from the opponent's side of the net 
			// toward you when the spacebar is released. I found that a good initial position for the ball is: (0, 30, -130).  
			// And, a good initial velocity is (0, 200, 400).  As usual for this program, all 
			// units are in cm.
			ballPos = vec3(0, 30, -130);
			ballRadius = 2.0;
			ball.reset(new Sphere(ballPos, ballRadius, vec4(1, 1, 1, 1)));
			ballVel = vec3(0, 200, 400);
		}
		
	}


	void App::onSimulation(double rdt) {
		// rdt is the change in time (dt) in seconds since the last call to onSimulation
		// So, you can slow down the simulation by half if you divide it by 2.
		rdt *= 0.25;

		// Here are a few other values that you may find useful..
		// Radius of the ball = 2cm
		// Radius of the paddle = 8cm
		// Acceleration due to gravity = 981cm/s^2 in the negative Y direction
		// See the diagram in the assignment handout for the dimensions of the ping pong table
		glm::vec3 gravity = vec3(0, -981.0, 0)*float(rdt); //multiply by time changed
		glm::vec3 ballChange = ballVel*float(rdt);
		ballPos = ballPos + ballChange;
		ballVel = ballVel + gravity;
		//TODO: collision detection
		//detect ball collision with table
		glm::vec3 ballBottom = vec3(ballPos.x, ballPos.y - ballRadius, ballPos.z);
		float ballTop = ballPos.y + ballRadius;
		if (table->contains(ballBottom)) {//collision. Does not 
			ballPos.y = tableMax.y+ballRadius;//corrects ball's position
			//ballVel = (ballVel*glm::vec3(1, -1, 1)) * 0.85f;//corrects ball's velocity. TODO: reflect across normal as opposed to simply reversing y.
			ballVel = ballVel*float(cos(radians(180))) + glm::cross(tableNormal, ballVel)*float(sin(radians(180))) + tableNormal*float((glm::dot(tableNormal, ballVel)*(1 - cos(radians(180)))));
		}
	}


	void App::onRenderGraphics() {

		double curTime = glfwGetTime();
		onSimulation(curTime - lastTime);
		lastTime = curTime;

		// Setup the camera with a good initial position and view direction to see the table
		glm::vec3 eye_world = glm::vec3(0, 100, 250);
		glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		//eye_world = glm::vec3(glm::column(glm::inverse(view), 3));

		// Setup the projection matrix so that things are rendered in perspective
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)_windowWidth / (GLfloat)_windowHeight, 0.1f, 500.0f);
		// Setup the model matrix
		glm::mat4 model = glm::mat4(1.0);

		// Update shader variables
		_shader.setUniform("view_mat", view);
		_shader.setUniform("projection_mat", projection);
		_shader.setUniform("model_mat", model);
		_shader.setUniform("eye_world", eye_world);

		// Draw the paddle using two cylinders
		paddle->draw(_shader, paddleFrame);
		handle->draw(_shader, paddleFrame);
		//draw table and net
		table->draw(_shader, model);
		net->draw(_shader, model);
		//draw ball
		glm::mat4 ballFrame = glm::translate(glm::mat4(1.0), ballPos);
		if (ball) {
			ball->draw(_shader, ballFrame);
		}
        // Check for any opengl errors
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}
	}

}