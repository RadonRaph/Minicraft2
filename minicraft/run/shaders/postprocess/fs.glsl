#version 400

in vec2 uv;

uniform sampler2D TexColor;
uniform sampler2D TexDepth;
uniform float screen_width;
uniform float screen_height;
uniform vec2 near_far;
uniform vec3 sunPos;

uniform mat4 v;
uniform mat4 p;

out vec4 color_out;

float LinearizeDepth(float z)
{
	float n = near_far.x; // camera z near
  	float f = near_far.y; // camera z far
  	return (2.0 * n) / (f + n - z * (f - n));
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( TexColor , uv );
	float depth = texture2D( TexDepth , uv ).r;	

	vec4 sun = vec4(sunPos.xyz, 1);
	vec4 sunPosView = p*v*sun;
	sunPosView /= sunPosView.w;

	float sunPosDist =  pow(clamp( 1-length(sunPosView.xy - uv),0,1), 20)*3;

	if (sunPosView.z < 0)
		sunPosDist = 0;
	
	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);



    

	vec3 colorBlur = color.rgb*4;
	float blurSt = 1;
	colorBlur -= texture2D(TexColor, uv + vec2(xstep * blurSt, 0)).rgb;
	colorBlur -= texture2D(TexColor, uv + vec2(-xstep * blurSt, 0)).rgb;
	colorBlur -= texture2D(TexColor, uv + vec2(0, ystep * blurSt)).rgb;
	colorBlur -= texture2D(TexColor, uv + vec2(0, -ystep * blurSt)).rgb;
	//colorBlur /= 5;

	float centerDepth = LinearizeDepth(texture2D(TexDepth, vec2(0.5, 0.5)).r);

	//vec3 colorFinal = mix(color.rgb, colorBlur.rgb, abs(centerDepth - depth));


	vec3 colorFinal = color.rgb + length(colorBlur)*0;

	//Gamma correction
	colorFinal.r = pow(colorFinal.r, 1.0 / 2.2);
	colorFinal.g = pow(colorFinal.g, 1.0 / 2.2);
	colorFinal.b = pow(colorFinal.b, 1.0 / 2.2);

	

	color_out = vec4(colorFinal.rgb, 1);

	color_out += vec4(sunPosDist, sunPosDist,0,1);
}