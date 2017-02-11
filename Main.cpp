#include "Main.h"
#include "opengl-wrapper\GLEW\include\GL\glew.h"
#include "opengl-wrapper\GLFW\include\GLFW\glfw3.h"
#include "Mat4.h"
#include <iostream>
#include <vector>
#include "ShaderProgram.h"
#include "Camera.h"
#include "Object.h"
#include "OrcTree.h"

#define NUMBER_OF_BODIES 100
#define USE_OPENGL 1


GLFWwindow * window;
GLuint VAO, VBO_positions, VBO_size;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
bool keys[1024];

void initBuffers(std::vector<Object*> objects, float *pos, float *size);
void updateBuffers(std::vector<Object*> objects, float *pos, float *size);
void updateObjects(std::vector<Object*> objects, float dt);
void input(Camera* camera, float dt);
void createWindow();
void initOpenGL();

int main() {


	float *pos = new float[NUMBER_OF_BODIES * 3];
	float *size = new float[NUMBER_OF_BODIES];

	std::vector<Object*> objects;
	Camera camera;

	createWindow();
	initOpenGL();


	Mat4 projectionMatrix;
	projectionMatrix.createProjectionMatrix(70.0f, 400.0f/400.0f, 0.1f, 100.0f);


	ShaderProgram shader;
	shader.compileShader("vertexShader.txt", NULL, "fragmentShader.txt");
	shader.loadMat4("ProjectionMatrix", &projectionMatrix);

	for (int i = 0; i < NUMBER_OF_BODIES; i++) {
		Vector3 random = Vector3((float)rand()/RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		objects.push_back(new Object(random, (float)rand() / RAND_MAX));
	}

	OrcTree tree = OrcTree(objects);

	initBuffers(objects, pos, size);

	glfwSetTime(0);
	float current_time;
	float last_time = glfwGetTime();
	float dt;


	while (!glfwWindowShouldClose(window)) {

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		current_time = glfwGetTime();
		dt = current_time - last_time;
		last_time = current_time;


		input(&camera, dt);
		//updateObjects(objects, dt);
		//updateBuffers(objects, pos, size);
		

		camera.update();
		shader.loadMat4("ViewMatrix", &camera.viewMatrix);

		glDrawArraysInstanced(GL_POINTS, 0, 1, NUMBER_OF_BODIES);

		glfwPollEvents();
		glfwSwapBuffers(window);


	}


	for (int i = 0; i < NUMBER_OF_BODIES; i++) {
		delete objects[i];
	}

	delete[] pos;
	delete[] size;

	return 0;


}


void updateObjects(std::vector<Object*> objects, float dt) {


	for (int i = 0; i < NUMBER_OF_BODIES; i++) {
		Vector3 force;
		for (int j = 0; j < NUMBER_OF_BODIES; j++) {
			if (i != j) {

				Vector3 dir = (objects[j]->position - objects[i]->position).normalize();
				float dist = 1 * (objects[i]->position.distance(objects[j]->position));

				force += 0.05f*(dir * objects[j]->size * (1.0f / (dist*dist))) * (1 / objects[i]->size);


				if (objects[i]->position.distance(objects[j]->position) < 0.01f) {
					objects[i]->size += objects[j]->size;
				}

			}
		}

		objects[i]->force = force;
		objects[i]->velocity += objects[i]->force*dt;
		objects[i]->position += objects[i]->velocity * dt;

	}
}


void createWindow() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	window = glfwCreateWindow(800, 800, "N-Body Simulation", 0, 0);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

}

void initOpenGL() {

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
		std::cout << "Error:" << glewGetErrorString(err);
	glClearColor(.5, .5, .5, 1);
	glEnable(GL_PROGRAM_POINT_SIZE);

}

void initBuffers(std::vector<Object*> objects, float *pos, float *size) {

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	for (int i = 0; i < NUMBER_OF_BODIES; i++) {
		pos[3 * i + 0] = objects[i]->position.x;
		pos[3 * i + 1] = objects[i]->position.y;
		pos[3 * i + 2] = objects[i]->position.z;

		size[i] = objects[i]->size;
	}


	glGenBuffers(1, &VBO_positions);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * NUMBER_OF_BODIES, pos, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glVertexAttribDivisor(0, 1);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_size);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_size);
	glBufferData(GL_ARRAY_BUFFER, 4 * NUMBER_OF_BODIES, size, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, 0);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);

}

void updateBuffers(std::vector<Object*> objects, float *pos, float *size) {

	for (int i = 0; i < NUMBER_OF_BODIES; i++) {
		
		pos[3 * i + 0] = objects[i]->position.x;
		pos[3 * i + 1] = objects[i]->position.y;
		pos[3 * i + 2] = objects[i]->position.z;

		size[i] = objects[i]->size;
		
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO_positions);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * 3 * NUMBER_OF_BODIES, pos);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_size);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * NUMBER_OF_BODIES, size);
	
}

void input(Camera* camera, float dt) {

	if (keys[GLFW_KEY_SPACE]) {
		camera->position += dt * 1.0f *camera->up;
	}
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		camera->position += dt * -1.0f * camera->up;
	}
	if (keys[GLFW_KEY_Q]) {
		camera->horzAngle += dt * -1.0f;
	}
	if (keys[GLFW_KEY_E]) {
		camera->horzAngle += dt * 1.0f;
	}
	if (keys[GLFW_KEY_W]) {
		camera->position += dt * 1.0f*camera->direction;
	}
	if (keys[GLFW_KEY_S]) {
		camera->position += dt * -1.0f*camera->direction;
	}
	if (keys[GLFW_KEY_A]) {
		camera->position += dt * -1.0f*camera->right;
	}
	if (keys[GLFW_KEY_D]) {
		camera->position += dt * 1.0f*camera->right;
	}

}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {


	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
		return;
	}

	if (action == GLFW_PRESS)
		keys[key] = true;

	if (action == GLFW_RELEASE)
		keys[key] = false;

}