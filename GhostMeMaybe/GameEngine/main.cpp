#include <iostream>
#include <vector>
#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

//constants

//terrain
const float TERRAIN_FREQ = 0.1f;
const float TERRAIN_AMP = 4.0f;
const float TERRAIN_OFFSET = -3.0f;

//courtyard
const int COURTYARD_SIZE = 6;
const float FENCE_POS_STEP = 10.0f;
float fenceScale = 15.0f;
float fenceStretch = 7.5f;
float fenceYOffset = 12.0f;
float groundOffset = 12.0f; // Pentru fata

//atmosphere
glm::vec3 skyColor = glm::vec3(0.15f, 0.15f, 0.25f);
glm::vec3 lightColor = glm::vec3(0.7f, 0.7f, 0.9f);
glm::vec3 lightPos = glm::vec3(50.0f, 100.0f, -100.0f);
float fogDensity = 0.015f;

//player
glm::vec3 playerPos = glm::vec3(0.0f, 15.0f, 0.0f);
float playerSpeed = 15.0f;
float playerRotation = 0.0f;
bool isMoving = false;

//physics
float playerVelocityY = 0.0f;
float gravity = 35.0f;
float jumpForce = 15.0f;
bool isJumping = false;

//camera
float cameraAngle = 0.0f;
float cameraDistance = 25.0f;
float cameraHeight = 15.0f;

//time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Window window("Game Engine - Graveyard Final", 800, 800);

void processKeyboardInput();
float getTerrainHeight(float x, float z);

int main()
{
	std::cout << "--- START ---" << std::endl;
	glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);

	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //load resources
	GLuint texGirl = loadBMP("Resources/Textures/girl.bmp");
	GLuint texGrass = loadBMP("Resources/Textures/grass.bmp");
	GLuint texMoon = loadBMP("Resources/Textures/rock.bmp");
	GLuint texFence = loadBMP("Resources/Textures/fence.bmp");

	std::vector<Texture> tGirl; tGirl.push_back(Texture()); tGirl[0].id = texGirl; tGirl[0].type = "texture_diffuse";
	std::vector<Texture> tGrass; tGrass.push_back(Texture()); tGrass[0].id = texGrass; tGrass[0].type = "texture_diffuse";
	std::vector<Texture> tFence; tFence.push_back(Texture()); tFence[0].id = texFence; tFence[0].type = "texture_diffuse";

	MeshLoaderObj loader;
	Mesh playerMesh = loader.loadObj("Resources/Models/girl.obj", tGirl);
	Mesh moonMesh = loader.loadObj("Resources/Models/sphere.obj");
	Mesh grassTile = loader.loadObj("Resources/Models/grass.obj", tGrass);
	Mesh fenceMesh = loader.loadObj("Resources/Models/fence.obj", tFence);

	shader.use();
	GLuint loc_MVP = glGetUniformLocation(shader.getId(), "MVP");
	GLuint loc_Model = glGetUniformLocation(shader.getId(), "model");
	GLuint loc_View = glGetUniformLocation(shader.getId(), "view");
	GLuint loc_Proj = glGetUniformLocation(shader.getId(), "projection");

	GLuint loc_LightColor = glGetUniformLocation(shader.getId(), "lightColor");
	GLuint loc_LightPos = glGetUniformLocation(shader.getId(), "lightPos");
	GLuint loc_ViewPos = glGetUniformLocation(shader.getId(), "viewPos");
	GLuint loc_FogColor = glGetUniformLocation(shader.getId(), "fogColor");
	GLuint loc_FogDensity = glGetUniformLocation(shader.getId(), "fogDensity");

	GLuint loc_EnableTerrain = glGetUniformLocation(shader.getId(), "enableTerrain");
	GLuint loc_TerrFreq = glGetUniformLocation(shader.getId(), "terrainFreq");
	GLuint loc_TerrAmp = glGetUniformLocation(shader.getId(), "terrainAmp");
	GLuint loc_TerrOffset = glGetUniformLocation(shader.getId(), "terrainOffset");

	sunShader.use();
	GLuint loc_SunMVP = glGetUniformLocation(sunShader.getId(), "MVP");

	lastFrame = (float)glfwGetTime();

	while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (deltaTime > 0.1f) deltaTime = 0.1f;

		processKeyboardInput();

		// physics
		playerVelocityY -= gravity * deltaTime;
		playerPos.y += playerVelocityY * deltaTime;
		float h = getTerrainHeight(playerPos.x, playerPos.z);
		float correctY = h + groundOffset;
		if (playerPos.y < correctY) { playerPos.y = correctY; playerVelocityY = 0.0f; isJumping = false; }

		//camera
		float camX = (float)sin(cameraAngle) * cameraDistance;
		float camZ = (float)cos(cameraAngle) * cameraDistance;
		glm::vec3 currentCameraPos = glm::vec3(playerPos.x + camX, playerPos.y + cameraHeight, playerPos.z + camZ);

		glm::mat4 Proj = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 View = glm::lookAt(currentCameraPos, playerPos, glm::vec3(0.0f, 1.0f, 0.0f));

		//moon
		sunShader.use();
		glm::mat4 MVP_Moon = Proj * View * glm::translate(glm::mat4(1.0), lightPos);
		glUniformMatrix4fv(loc_SunMVP, 1, GL_FALSE, &MVP_Moon[0][0]);
		moonMesh.draw(sunShader);

		//scene
		shader.use();
		glUniformMatrix4fv(loc_View, 1, GL_FALSE, &View[0][0]);
		glUniformMatrix4fv(loc_Proj, 1, GL_FALSE, &Proj[0][0]);
		glUniform3f(loc_LightColor, lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(loc_LightPos, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(loc_ViewPos, currentCameraPos.x, currentCameraPos.y, currentCameraPos.z);
		glUniform3f(loc_FogColor, skyColor.x, skyColor.y, skyColor.z);
		glUniform1f(loc_FogDensity, fogDensity);

		// player
		glUniform1i(loc_EnableTerrain, 0);
		float walkOffset = 0.0f;
		if (isMoving && !isJumping) walkOffset = (float)sin(glfwGetTime() * 15.0f) * 0.2f;
		glm::mat4 ModelP = glm::translate(glm::mat4(1.0), glm::vec3(playerPos.x, playerPos.y + walkOffset, playerPos.z));
		ModelP = glm::rotate(ModelP, glm::radians(playerRotation), glm::vec3(0, 1, 0));
		ModelP = glm::scale(ModelP, glm::vec3(1.0f));
		glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &ModelP[0][0]);
		glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * ModelP)[0][0]);
		playerMesh.draw(shader);

		// grass
		glUniform1i(loc_EnableTerrain, 1);
		glUniform1f(loc_TerrFreq, TERRAIN_FREQ);
		glUniform1f(loc_TerrAmp, TERRAIN_AMP);
		glUniform1f(loc_TerrOffset, TERRAIN_OFFSET);
		int mapSize = 15;
		float tileSize = 10.0f;
		for (int i = -mapSize; i < mapSize; i++) {
			for (int j = -mapSize; j < mapSize; j++) {
				glm::mat4 ModelG = glm::translate(glm::mat4(1.0), glm::vec3(i * tileSize, 0.0f, j * tileSize));
				glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &ModelG[0][0]);
				glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * ModelG)[0][0]);
				grassTile.draw(shader);
			}
		}

		// fences
		glUniform1i(loc_EnableTerrain, 0);

		//back fence
		for (int i = -COURTYARD_SIZE; i <= COURTYARD_SIZE; i++) {
			float x = i * FENCE_POS_STEP; float z = -COURTYARD_SIZE * FENCE_POS_STEP;
			float y = getTerrainHeight(x, z) + fenceYOffset;

			glm::mat4 M = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));
			M = glm::scale(M, glm::vec3(fenceStretch, fenceScale, fenceScale));

			glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * M)[0][0]);
			fenceMesh.draw(shader);
		}

		// front fence
		for (int i = -COURTYARD_SIZE; i <= COURTYARD_SIZE; i++) {
			float x = i * FENCE_POS_STEP; float z = COURTYARD_SIZE * FENCE_POS_STEP;
			float y = getTerrainHeight(x, z) + fenceYOffset;
			if (i == 0) continue;

			glm::mat4 M = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));

			if (i == -1) M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));
			if (i == 1) M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0, 1, 0));

			M = glm::scale(M, glm::vec3(fenceStretch, fenceScale, fenceScale));

			glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * M)[0][0]);
			fenceMesh.draw(shader);
		}

		// left fence
		for (int i = -COURTYARD_SIZE; i <= COURTYARD_SIZE; i++) {
			float x = -COURTYARD_SIZE * FENCE_POS_STEP; float z = i * FENCE_POS_STEP;
			float y = getTerrainHeight(x, z) + fenceYOffset;

			glm::mat4 M = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));
			M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));

			M = glm::scale(M, glm::vec3(fenceStretch, fenceScale, fenceScale));

			glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * M)[0][0]);
			fenceMesh.draw(shader);
		}

		//right fence
		for (int i = -COURTYARD_SIZE; i <= COURTYARD_SIZE; i++) {
			float x = COURTYARD_SIZE * FENCE_POS_STEP; float z = i * FENCE_POS_STEP;
			float y = getTerrainHeight(x, z) + fenceYOffset;

			glm::mat4 M = glm::translate(glm::mat4(1.0), glm::vec3(x, y, z));
			M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0, 1, 0));

			M = glm::scale(M, glm::vec3(fenceStretch, fenceScale, fenceScale));

			glUniformMatrix4fv(loc_Model, 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(loc_MVP, 1, GL_FALSE, &glm::mat4(Proj * View * M)[0][0]);
			fenceMesh.draw(shader);
		}

		window.update();
	}
}

float getTerrainHeight(float x, float z) {
	if (abs(x) < 5.0f && abs(z) < 5.0f) return TERRAIN_OFFSET;
	return (float)(sin(x * TERRAIN_FREQ) * cos(z * TERRAIN_FREQ) * TERRAIN_AMP + TERRAIN_OFFSET);
}

void processKeyboardInput() {
	float s = playerSpeed * deltaTime;
	float rs = 2.0f * deltaTime;
	isMoving = false;
	if (window.isPressed(GLFW_KEY_W)) { playerPos.z += s; playerRotation = 180.0f; isMoving = true; }
	if (window.isPressed(GLFW_KEY_S)) { playerPos.z -= s; playerRotation = 0.0f; isMoving = true; }
	if (window.isPressed(GLFW_KEY_A)) { playerPos.x += s; playerRotation = -90.0f; isMoving = true; }
	if (window.isPressed(GLFW_KEY_D)) { playerPos.x -= s; playerRotation = 90.0f; isMoving = true; }
	if (window.isPressed(GLFW_KEY_SPACE) && !isJumping) { playerVelocityY = jumpForce; isJumping = true; }
	if (window.isPressed(GLFW_KEY_R)) { cameraAngle -= rs; }
	if (window.isPressed(GLFW_KEY_F)) { cameraAngle += rs; }
}
