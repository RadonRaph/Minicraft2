#version 400

uniform vec3 camPos;
uniform vec4 skyColor;
uniform vec3 sunPos;
uniform sampler2D hatchingTex;

//Variables en entree
in vec3 normal;
in vec4 color;
in vec2 uv;
flat in float type;
in vec4 wPos;
in vec4 vPos;

out vec4 color_out;


#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_EAU 4.0
#define CUBE_SABLE_01 17.0
//Globales
const float ambientLevel = 0.15;

void main()
{
	

	vec3 pos = vPos.xyz;

	vec3 toLight = normalize(sunPos-pos);
	// Diffuse
	float diffuse = max(0, dot(toLight, normal))*((skyColor.r+skyColor.g+skyColor.b)/3.0);

	// Specular
	vec3 viewDir = normalize(camPos - wPos.xyz);
	vec3 halfVec = normalize(toLight + viewDir);
	float specular = max(0, dot(halfVec, normal));
	specular = pow(specular, 1000) * 2;

	vec4 specularColor = vec4(0.8, 0.7, 0.2, 1);

	vec4 ambient = skyColor * ambientLevel;

	vec4 c = color;

	if (type == CUBE_EAU) {
		//color_out = vec4(sqrt(c.xyz * max(0, dot(toLight, normal)) * 0.97 + 0.03 * vec3(0.8, 0.9, 1)), c.a);
		c = diffuse*color+specular*skyColor+ambient;
		//color_out = vec4(1, 0, 0, 1);
		
	}
	else {
		c = diffuse*color+ambient;

		float numTex = floor(diffuse * 5) / 5;
		vec2 uvHatch = uv;
		uvHatch.x /= 5;
		uvHatch.x += numTex;

	//	c *= texture2D(hatchingTex, uvHatch);
	}


	


	color_out = c;
}