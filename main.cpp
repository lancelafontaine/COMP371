#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <string>
#include <locale>
#include <sstream>
#include <fstream>
#include <ctime>
#include <map>

#include "objloader.h"
#include "Shaders.h"

// Lance's include files
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL/SOIL.h"

using namespace std;

// Window dimensions
const GLuint WINDOW_WIDTH = 700, WINDOW_HEIGHT = 700;
const GLfloat CAMERA_MOVEMENT_STEP = 1.90f;
const float ANGLE_ROTATION_STEP = 0.15f;

int numberOfrows = 10;

float asteroidRadius = 70.0f;
glm::vec3 asteroidCentre = glm::vec3(0.0f);
glm::vec3 asteroidScale = glm::vec3(4.0f);
vector<glm::vec3> asteroidPositions;

int width, height;
GLFWwindow* window;

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, -25.0f);
glm::mat4 projection_matrix;

int numPlanets = 0;
float xVal;
float yVal;
float zVal;
float radius;

float planetRotationAxisSpeed = 0.2f;

glm::vec3 camera_movement = glm::vec3(0.0f, 0.0f, 15.0f);
double xpos, ypos;
bool mouse_click = false;

GLuint colourVBO;
vector<glm::vec3> planet_positions; // positions of planets in the world
vector<float> planet_radius; // radius of planets
vector<glm::vec3> planet_colors;

float y_rotation_angle = 0.0f, x_rotation_angle = 0.0f;



void IndividualPlanetGeneration() {
        xVal = (float)(rand() % 100 - 50);
        yVal = 0.0f;
        zVal = (float)(rand() % 100 - 50);
        planet_positions.push_back(glm::vec3(xVal, yVal, zVal));
        radius = sqrt(xVal * xVal + yVal * yVal + zVal * zVal);
        planet_radius.push_back(radius);
        planet_colors.push_back(glm::vec3(
                    (GLfloat)(rand()%255)/255,
                    (GLfloat)(rand()%255)/255,
                    (GLfloat)(rand()%255)/255));
}

void IndividualPlanetDestruction() {
        planet_colors.pop_back();
        planet_positions.pop_back();
        planet_radius.pop_back();
}

void planetGeneration() {
        // The sun position and color
        planet_positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        planet_colors.push_back(glm::vec3(1.0f, 1.0f, 0.2f));

	//generate positions for each of the planets at random
	for (int num = 0; num < numPlanets-1; num++)
	{
                IndividualPlanetGeneration();
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	// Update the Projection matrix after a window resize event
	projection_matrix = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//backwards
	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		camera_position.z -= cos(y_rotation_angle)*CAMERA_MOVEMENT_STEP;
		camera_position.x -= sin(y_rotation_angle)*CAMERA_MOVEMENT_STEP;
	}
	//forwards
	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		camera_position.z += cos(y_rotation_angle)*CAMERA_MOVEMENT_STEP;
		camera_position.x += sin(y_rotation_angle)*CAMERA_MOVEMENT_STEP;
	}
	//left
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera_position.x += CAMERA_MOVEMENT_STEP;
	//right
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
		camera_position.x -= CAMERA_MOVEMENT_STEP;

	//rotate cube
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		x_rotation_angle += ANGLE_ROTATION_STEP;

	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		x_rotation_angle -= ANGLE_ROTATION_STEP;

	if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		y_rotation_angle += ANGLE_ROTATION_STEP;

	if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		y_rotation_angle -= ANGLE_ROTATION_STEP;

        // Generate new planets
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
                planet_colors.clear();
                planet_radius.clear();
                planet_positions.clear();
                planetGeneration();
        }

        // planet rotation about their own axes
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
                planetRotationAxisSpeed += 0.1f;
        }
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
                planetRotationAxisSpeed -= 0.1f;
        }

        // create/destroy planets
	if (key == GLFW_KEY_I && action == GLFW_PRESS && numPlanets <= 15) {
                numPlanets++;
                IndividualPlanetGeneration();
        }
	if (key == GLFW_KEY_K && action == GLFW_PRESS && numPlanets >= 2) {
                numPlanets--;
                IndividualPlanetDestruction();
        }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		mouse_click = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mouse_click = false;
	}

}

GLuint loadCubemap(std::vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);

		SOIL_free_image_data(image); //free resources
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}


void programInit() {
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);
	glEnable(GL_MULTISAMPLE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Open Space GLadiators", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		getchar();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback); //ADDED FOR PROJECT
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		getchar();
		exit(-1);
	}

	// Define the viewport dimensions
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

}

int main() {
	programInit();

	///////////////////
	// SKYBOX CONFIG //
	///////////////////

	// Reading and compiling vertex and fragment shaders
	GLuint skyboxVertexShader = compileShader("vertex", readShaderFile("vertexSkybox.shader"));
	GLuint skyboxFragmentShader = compileShader("fragment", readShaderFile("fragmentSkybox.shader"));
	// Linking shaders
	GLuint skyboxShaderProgram = linkShaders(skyboxVertexShader, skyboxFragmentShader);

	vector<const GLchar*> faces;
	srand(time(0));
	int skybox = rand() % 3 + 1;
	if (skybox == 1)
	{
		faces.push_back("skybox/stars_right.jpg");
		faces.push_back("skybox/stars_left.jpg");
		faces.push_back("skybox/stars_up.jpg");
		faces.push_back("skybox/stars_down.jpg");
		faces.push_back("skybox/stars_back.jpg");
		faces.push_back("skybox/stars_front.jpg");
	}
	else if (skybox == 2)
	{
		faces.push_back("skybox/galaxy_right.jpg");
		faces.push_back("skybox/galaxy_left.jpg");
		faces.push_back("skybox/galaxy_up.jpg");
		faces.push_back("skybox/galaxy_down.jpg");
		faces.push_back("skybox/galaxy_back.jpg");
		faces.push_back("skybox/galaxy_front.jpg");
	}
	else
	{
		faces.push_back("skybox/galaxy2_right.jpg");
		faces.push_back("skybox/galaxy2_left.jpg");
		faces.push_back("skybox/galaxy2_up.jpg");
		faces.push_back("skybox/galaxy2_down.jpg");
		faces.push_back("skybox/galaxy2_back.jpg");
		faces.push_back("skybox/galaxy2_front.jpg");
	}
	glActiveTexture(GL_TEXTURE1);
	GLuint cubemapTexture = loadCubemap(faces);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);


	// For some weird reason, the skybox disappears unless the vertices for an object are being loaded. So we load a simple cube here.
	std::vector<glm::vec3> verticesSkybox;
	std::vector<glm::vec3> normalsSkybox;
	std::vector<glm::vec2> UVsSkybox;

	loadOBJ("cube.obj", verticesSkybox, normalsSkybox, UVsSkybox);

	GLuint VAOSkybox, vertices_VBOSkybox;
	glGenVertexArrays(1, &VAOSkybox);
	glGenBuffers(1, &vertices_VBOSkybox);
	glBindVertexArray(VAOSkybox);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOSkybox);
	glBufferData(GL_ARRAY_BUFFER, verticesSkybox.size() * sizeof(glm::vec3), &verticesSkybox.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/////////////////////////////
	// MILLENIUM FALCON CONFIG //
	/////////////////////////////

	// Reading and compiling vertex and fragment shaders
	GLuint milleniumVertexShader = compileShader("vertex", readShaderFile("vertexMillenium.shader"));
	GLuint milleniumFragmentShader = compileShader("fragment", readShaderFile("fragmentMillenium.shader"));
	// Linking shaders
	GLuint milleniumShaderProgram = linkShaders(milleniumVertexShader, milleniumFragmentShader);

	std::vector<glm::vec3> verticesMillenium;
	std::vector<glm::vec3> normalsMillenium;
	std::vector<glm::vec2> UVsMillenium;

	loadOBJ("Falcon-Only.obj", verticesMillenium, normalsMillenium, UVsMillenium);

	GLuint VAOMillenium, vertices_VBOMillenium, normals_VBOMillenium, UVs_VBOMillenium;

	glGenVertexArrays(1, &VAOMillenium);
	glGenBuffers(1, &vertices_VBOMillenium);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAOMillenium);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOMillenium);
	glBufferData(GL_ARRAY_BUFFER, verticesMillenium.size() * sizeof(glm::vec3), &verticesMillenium.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &normals_VBOMillenium);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBOMillenium);
	glBufferData(GL_ARRAY_BUFFER, normalsMillenium.size() * sizeof(glm::vec3), &normalsMillenium.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	//////////////////////////
	// SPHERE/PLANET CONFIG //
	//////////////////////////

	// Reading and compiling vertex and fragment shaders
	GLuint planetVertexShader = compileShader("vertex", readShaderFile("vertexPlanet.shader"));
	GLuint planetFragmentShader = compileShader("fragment", readShaderFile("fragmentPlanet.shader"));
	// Linking shaders
	GLuint planetShaderProgram = linkShaders(planetVertexShader, planetFragmentShader);

	std::vector<glm::vec3> verticesPlanet;
	std::vector<glm::vec3> normalsPlanet;
	std::vector<glm::vec2> UVsPlanet;

	loadOBJ("sphere.obj", verticesPlanet, normalsPlanet, UVsPlanet);

	GLuint VAOPlanet, vertices_VBOPlanet, normals_VBOPlanet, UVs_VBOPlanet;

	glGenVertexArrays(1, &VAOPlanet);
	glGenBuffers(1, &vertices_VBOPlanet);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAOPlanet);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOPlanet);
	glBufferData(GL_ARRAY_BUFFER, verticesPlanet.size() * sizeof(glm::vec3), &verticesPlanet.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &normals_VBOPlanet);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBOPlanet);
	glBufferData(GL_ARRAY_BUFFER, normalsPlanet.size() * sizeof(glm::vec3), &normalsPlanet.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &UVs_VBOPlanet);
	glBindBuffer(GL_ARRAY_BUFFER, UVs_VBOPlanet);
	glBufferData(GL_ARRAY_BUFFER, UVsPlanet.size() * sizeof(glm::vec2), &UVsPlanet.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	/////////////////////
	// ASTEROID CONFIG //
	////////////////////

	// Reading and compiling vertex and fragment shaders
	GLuint asteroidVertexShader = compileShader("vertex", readShaderFile("vertexAsteroid.shader"));
	GLuint asteroidFragmentShader = compileShader("fragment", readShaderFile("fragmentAsteroid.shader"));
	// Linking shaders
	GLuint asteroidShaderProgram = linkShaders(asteroidVertexShader, asteroidFragmentShader);

	std::vector<glm::vec3> verticesAsteroid;
	std::vector<glm::vec3> normalsAsteroid;
	std::vector<glm::vec2> UVsAsteroid;

	loadOBJ("cube.obj", verticesAsteroid, normalsAsteroid, UVsAsteroid);

	GLuint VAOAsteroidBelt, vertices_VBOAsteroid, normals_VBOAsteroid, UVs_VBOAsteroid, vertices_VBOAsteroidBelt;
        
        glGenVertexArrays(1, &VAOAsteroidBelt);
        glGenBuffers(1, &vertices_VBOAsteroid);
        glGenBuffers(1, &vertices_VBOAsteroidBelt);
//
        glBindVertexArray(VAOAsteroidBelt);
//
        glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOAsteroid);
        glBufferData(GL_ARRAY_BUFFER, verticesAsteroid.size() * sizeof(glm::vec3), &verticesAsteroid.front(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
       
        glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOAsteroidBelt);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribDivisor(1, 1);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

	/////////////////
	// GAME CONFIG //
	/////////////////

	//FOR PROJECT
	glfwGetCursorPos(window, &xpos, &ypos);
	double xpos_old, ypos_old;

	float currentPlanetRotation = 0.0f;

	do
	{
		cout << "Please enter the number of planets you would like to see ( between 1 and 15) : \n";
		cin >> numPlanets;
	} while (numPlanets <= 0 || numPlanets >= 16);

        planetGeneration();





    glm::vec3 point;
    const int MAX_ASTEROIDS = (360.0f / asteroidRadius);
    float steps = floor(numberOfrows / 2);
    int row = 0;

    for(float zValue = steps * -5.0f; zValue <= steps * 5.0f; zValue += 5.0f)
    {
        for (int col = 0; col < 5; col++)
        {
            asteroidRadius += 5.0f;

            bool r0c0c4 = ((row == 0) && ((col == 0) || (col == 4)));
            bool r4c0c4 = ((row == numberOfrows - 1) && ((col == 0) || (col == 4)));

            if(!r0c0c4 && !r4c0c4)
            {
                for(float i = 0.0f; i < 360.0f; i += (rand() % MAX_ASTEROIDS))
                {
                    if((rand() % 100 + 1) > 60) // 40% of the time.
                    {
                        //circle
                        point.x = (float) (asteroidCentre.x + asteroidRadius * cos(glm::radians(i)));
                        point.y = (float) (asteroidCentre.y + asteroidRadius * sin(glm::radians(i)));
                        point.z = zValue;

                        cout << "Asteroid Position: (" << point.x << ", " << point.y << ", " << point.z << ")" << endl;

                        asteroidPositions.push_back(point);

                        glBindBuffer(GL_ARRAY_BUFFER, vertices_VBOAsteroidBelt);
                        glBufferData(GL_ARRAY_BUFFER, asteroidPositions.size() * sizeof(glm::vec3), &asteroidPositions.front(), GL_STATIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind for good measure
                    }
                }
            }
        }
        cout << asteroidPositions.size();
        row++;
    }

	///////////////
	// GAME LOOP //
	///////////////

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (mouse_click)
		{
			xpos_old = xpos;
			ypos_old = ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			camera_movement += glm::vec3(0.0f, 0.0f, 1.0f) * ((GLfloat)(ypos - ypos_old)*0.025f);

			if ((ypos - ypos_old) > 0)
				x_rotation_angle += 0.01f;
			else if ((ypos - ypos_old) < 0)
				x_rotation_angle -= 0.01f;
			if ((xpos - xpos_old) > 0)
				y_rotation_angle -= 0.01f;
			else if ((xpos - xpos_old) < 0)
				y_rotation_angle += 0.01f;
		}

		//HERE
		glm::mat4 view_matrix;
		view_matrix = glm::rotate(view_matrix, -y_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));


		//Draw the skybox
		glUseProgram(skyboxShaderProgram);
		glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skybox"), 1); //sky box should read from texture unit 1
		GLuint projectionLoc = glGetUniformLocation(skyboxShaderProgram, "projection_matrix");
		GLuint viewMatrixLoc = glGetUniformLocation(skyboxShaderProgram, "view_matrix");
		glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix)); //remove the translation data
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

		glBindVertexArray(VAOSkybox);
		glDepthMask(GL_FALSE);
		glDrawArrays(GL_TRIANGLES, 0, verticesSkybox.size());
		glDepthMask(GL_TRUE);
		glBindVertexArray(0);

		// Draw the Millenium Falcon
		glm::mat4 model_matrix_millenium = glm::mat4(1.0f);
		model_matrix_millenium = glm::translate(model_matrix_millenium, camera_position);
		model_matrix_millenium = glm::rotate(model_matrix_millenium, glm::radians(360.f - 50.0f) + y_rotation_angle, glm::vec3(0, 1, 0));
		model_matrix_millenium = glm::scale(model_matrix_millenium, glm::vec3(0.1f));

		view_matrix = glm::lookAt(camera_position + glm::vec3(0.0f, 0.2f, 0.0f), camera_position + glm::vec3(sin(y_rotation_angle), 0.2f, cos(y_rotation_angle)), glm::vec3(0.0f, 1.0f, 0.0f));

		glUseProgram(milleniumShaderProgram);

		GLuint transformLocMillenium = glGetUniformLocation(milleniumShaderProgram, "model_matrix_millenium");
		projectionLoc = glGetUniformLocation(milleniumShaderProgram, "projection_matrix");
		viewMatrixLoc = glGetUniformLocation(milleniumShaderProgram, "view_matrix");

		glUniformMatrix4fv(transformLocMillenium, 1, GL_FALSE, value_ptr(model_matrix_millenium));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));

		glBindVertexArray(VAOMillenium);
		glDrawArrays(GL_TRIANGLES, 0, verticesMillenium.size());
		glBindVertexArray(0);
	

		// Draw the main planet
                glUseProgram(planetShaderProgram);

                for (int x = 0; x < numPlanets; x++)
                {
                        //makeMultiplePlanets(x);
                        GLuint transformLocPlanet = glGetUniformLocation(planetShaderProgram, "model_matrix_planet");
                        projectionLoc = glGetUniformLocation(planetShaderProgram, "projection_matrix");
                        viewMatrixLoc = glGetUniformLocation(planetShaderProgram, "view_matrix");
                        GLuint sphereColorLoc = glGetUniformLocation(planetShaderProgram, "sphereColor");
                        GLuint planetBumpFactorXLoc = glGetUniformLocation(planetShaderProgram, "planetBumpFactorX");
                        GLuint planetBumpFactorYLoc = glGetUniformLocation(planetShaderProgram, "planetBumpFactorY");
                        GLuint planetBumpFactorZLoc = glGetUniformLocation(planetShaderProgram, "planetBumpFactorZ");


                        glm::mat4 model_matrix_planet = glm::mat4(1.0f);
                        model_matrix_planet = glm::scale(model_matrix_planet, glm::vec3(10.0f));

                        model_matrix_planet = glm::translate(model_matrix_planet, planet_positions.at(x));
                        model_matrix_planet = glm::rotate(model_matrix_planet, glm::radians(currentPlanetRotation), glm::vec3(0, 1, 0));

                        float planetBumpFactorX = (float)(rand() % 1000)/50000 + 1;
                        float planetBumpFactorY = (float)(rand() % 1000)/50000 + 1;
                        float planetBumpFactorZ = (float)(rand() % 1000)/50000 + 1;

                        glUniformMatrix4fv(transformLocPlanet, 1, GL_FALSE, value_ptr(model_matrix_planet));
                        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
                        glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
                        glUniform3fv(sphereColorLoc, 1, glm::value_ptr(planet_colors.at(x)));
                        glUniform1f(planetBumpFactorXLoc, planetBumpFactorX);
                        glUniform1f(planetBumpFactorYLoc, planetBumpFactorY);
                        glUniform1f(planetBumpFactorZLoc, planetBumpFactorZ);

                        glBindVertexArray(VAOPlanet);
                        glDrawArrays(GL_TRIANGLES, 0, verticesPlanet.size());
                        glBindVertexArray(0);
                }





                // Draw asteroids
                        glUseProgram(asteroidShaderProgram);
                        GLuint transformLocAsteroid = glGetUniformLocation(asteroidShaderProgram, "model_matrix_asteroid");
                        projectionLoc = glGetUniformLocation(asteroidShaderProgram, "projection_matrix");
                        viewMatrixLoc = glGetUniformLocation(asteroidShaderProgram, "view_matrix");

                        glm::mat4 model_matrix_asteroid = glm::mat4(1.0f);
                        model_matrix_asteroid = glm::rotate(model_matrix_asteroid, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                        model_matrix_asteroid = glm::rotate(model_matrix_asteroid, glm::radians(90.0f) , glm::vec3(1.0f, 0.0f, 0.0f));
                        model_matrix_asteroid = glm::scale(model_matrix_asteroid, asteroidScale);

                        glUniformMatrix4fv(transformLocAsteroid, 1, GL_FALSE, value_ptr(model_matrix_asteroid));
                        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
                        glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));

                        glBindVertexArray(VAOAsteroidBelt);
                        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, verticesPlanet.size());
                        glBindVertexArray(0);



		currentPlanetRotation += planetRotationAxisSpeed;
		if (currentPlanetRotation >= 360.0f) {
			currentPlanetRotation = 0.0f;
		}
                if (currentPlanetRotation < 0.0f) {
                        currentPlanetRotation = 360.0f;
                }

		//glm::mat4 projection_matrix;
		projection_matrix = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
