#version 400

in vec2 textureCoord;
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 fogColor;
uniform float fogDensity;

void main()
{
	vec4 texColor = texture(texture1, textureCoord);
	if(texColor.a < 0.1) discard;

	float ambientStrength = 0.5; 
	vec3 ambient = ambientStrength * lightColor;

	// illumination
	vec3 normDir = normalize(norm);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(normDir, lightDir), 0.0);

	vec3 diffuse = diff * lightColor * 1.5; 

	float specularStrength = 0.3;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normDir);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16); 
	vec3 specular = specularStrength * spec * lightColor;

	vec3 lighting = (ambient + diffuse + specular);
	vec3 objectColor = lighting * vec3(texColor);

	//fog
	float distance = length(viewPos - fragPos);
	float fogFactor = 1.0 / exp((distance * fogDensity) * (distance * fogDensity));
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	vec3 finalColor = mix(fogColor, objectColor, fogFactor);

	fragColor = vec4(finalColor, 1.0);
}
