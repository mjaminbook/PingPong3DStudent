/**
@file App.h

The default starter app is configured for OpenGL 3.3 and relatively recent
GPUs.
*/
#ifndef App_h
#define App_h

#include "BaseApp.h"
#include <vector>
#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace basicgraphics {
	class App : public BaseApp {
	public:

		App(int argc, char** argv, std::string windowName, int windowWidth, int windowHeight);
		~App();

	protected:

		void onRenderGraphics() override;
		void onEvent(std::shared_ptr<Event> event) override;
		void onSimulation(double rdt);
        
        // Use these functions to access the current state of the paddle!
        glm::vec3 getPaddlePosition() { return glm::column(paddleFrame, 3); }
        glm::vec3 getPaddleNormal() { return glm::vec3(0,0,-1); }
        glm::vec3 getPaddleVelocity() { return paddleVel; }

		// The paddle is drawn with two cylinders
		std::unique_ptr<Cylinder> paddle;
		std::unique_ptr<Cylinder> handle;

		// This 4x4 matrix stores position and rotation data for the paddle.
		glm::mat4 paddleFrame;

		// This vector stores the paddle's current velocity.
		glm::vec3 paddleVel;

		//The model for the table
		std::unique_ptr<Box> table;
		std::unique_ptr<Line> net;
		
		//model for ball
		std::unique_ptr<Sphere> ball;
		float ballRadius;
		//position of ball
		glm::vec3 ballPos;
		//current movement vector of ball
		glm::vec3 ballVel;

        // This holds the time value for the last time onSimulate was called
		double lastTime;
	};
}

#endif
