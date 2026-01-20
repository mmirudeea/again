#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 textureCoord;
out vec3 norm;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 MVP; 

uniform int enableTerrain;
uniform float terrainFreq;
uniform float terrainAmp;
uniform float terrainOffset;

void main()
{
	textureCoord = texCoord;

	vec4 worldPosition = model * vec4(pos, 1.0f);

	if (enableTerrain == 1) 
	{
		
		float displacement = sin(worldPosition.x * terrainFreq) * cos(worldPosition.z * terrainFreq) * terrainAmp;

		if (abs(worldPosition.x) < 5.0 && abs(worldPosition.z) < 5.0) {
			displacement = 0.0;
		}

		worldPosition.y += displacement + terrainOffset;
	}

	fragPos = vec3(worldPosition);

	norm = mat3(transpose(inverse(model))) * normals;

	gl_Position = projection * view * worldPosition;
}
