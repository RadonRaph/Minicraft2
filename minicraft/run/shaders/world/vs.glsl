#version 400

uniform float elapsed;
uniform mat4 m;
uniform mat4 v;
uniform mat4 p;
uniform mat4 nmat;

layout(location=0) in vec3 vs_position_in;
layout(location=1) in vec3 vs_normal_in;
layout(location=2) in vec2 vs_uv_in;
layout(location=3) in float vs_type_in;

//Variables en sortie
out vec3 normal;
out vec4 color;
out vec2 uv;
out vec4 wPos;
out vec4 vPos;
flat out float type;

#define CUBE_HERBE 0.0
#define CUBE_TERRE 1.0
#define CUBE_EAU 4.0
#define CUBE_SABLE_01 17.0

void main()
{
	vec4 vecIn = vec4(vs_position_in,1.0);
	type = vs_type_in;

	 wPos = m * vecIn;
	
		
	normal = (nmat * vec4(vs_normal_in,1.0)).xyz; 

	uv = vs_uv_in;

	//Couleur par défaut violet
	color = vec4(1.0,1.0,0.0,1.0);

	//Couleur fonction du type
	if(vs_type_in == CUBE_HERBE)
		color = vec4(0.247, 0.776, 0.341,1);
	if(vs_type_in == CUBE_TERRE)
		color = vec4(0.3,0.2,0.1,1);
	if (vs_type_in == CUBE_EAU) {
		color = vec4(0.2, 0.2, 1.0, 0.85);
		//wPos.z += 1 * sin(wPos.x/15 + elapsed);

		vec4 _wPos = wPos;
		vec3 px = vec3(_wPos.x + 0.1f, _wPos.yz);
		vec3 py = vec3(_wPos.x, _wPos.y + 0.1f, _wPos.z);

		float h = sin(_wPos.x/20.0+_wPos.y/2.0+elapsed)*1;
		float hx = sin(px.x / 20.0 + _wPos.y / 2.0 + elapsed)*1;
		float hy = sin(py.x /20.0 + _wPos.y / 2.0 + elapsed)*1;
		_wPos.z -= h;
		px.z -= hx;
		py.z -= hy;
		vec3 v1 = normalize(px - _wPos.xyz);
		vec3 v2 = normalize(py - _wPos.xyz);
		normal = cross(v1, v2)*1;

		wPos.z -= abs(h)*0.7;

		//normal = vs_normal_in+ vec3(1,0,0) * sin(vecIn.x / 10.0 + elapsed * 1);
		//color = vec4(1, 0, sin(vecIn.x / 10.0 + elapsed * 1), 1);
	}
	if (vs_type_in == CUBE_SABLE_01)
		color = vec4(0.921, 0.886, 0.580, 0.7);

	vPos = v * wPos;


	gl_Position = p*vPos;
}