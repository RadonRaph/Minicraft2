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

bool isPointOnLine(vec2 A, vec2 B, vec2 C){
	return distance(A,C) + distance(B,C)==distance(A,B);
}

void main (void)
{
	float xstep = 1.0/screen_width;
	float ystep = 1.0/screen_height;
	float ratio = screen_width / screen_height;

	vec4 color = texture2D( TexColor , uv );
	float depth = texture2D( TexDepth , uv ).r;	
	//Permet de scaler la profondeur
	depth = LinearizeDepth(depth);

	vec4 sun = vec4(sunPos.xyz, 1);
	vec4 sunPosView = p*v*sun;
	float sunSign = sign(sunPosView.z);
	//sunPosView *= sunSign;
	sunPosView /= sunPosView.w;
	sunPosView.xy = (sunPosView.xy+vec2(1,1))/2;

	vec2 directSunDist = sunPosView.xy - uv;
	float sunPosDist =  pow(clamp( 1-length(directSunDist),0,1), 20)*3;

	if (sunSign < 0 || depth < 0.9)
		sunPosDist = 0;
	
	vec4 rayColor = vec4(1,0,1,1);
	vec2 sunUV = sunPosView.xy;
	
	if (sunSign < 0){
		sunUV.y *= -1;
		sunUV.x *= -1;
		rayColor = vec4(1,0,0,1);
	}
	sunUV.x = clamp(sunUV.x, 0,1);
	sunUV.y = clamp(sunUV.y, 0,1);
	



    

	vec3 colorBlur = color.rgb;
	float blurSt = 2;
	colorBlur += texture2D(TexColor, uv + vec2(xstep * blurSt, 0)).rgb;
	colorBlur += texture2D(TexColor, uv + vec2(-xstep * blurSt, 0)).rgb;
	colorBlur += texture2D(TexColor, uv + vec2(0, ystep * blurSt)).rgb;
	colorBlur += texture2D(TexColor, uv + vec2(0, -ystep * blurSt)).rgb;
	colorBlur /= 5;


	vec3 colorFinal = mix(color.rgb, colorBlur.rgb,  depth);


	//vec3 colorFinal = color.rgb + length(colorBlur)*0;

	//Gamma correction
	colorFinal.r = pow(colorFinal.r, 1.0 / 2.2);
	colorFinal.g = pow(colorFinal.g, 1.0 / 2.2);
	colorFinal.b = pow(colorFinal.b, 1.0 / 2.2);

	/*
	float sunRay = 0;
	
	vec2 path = sunUV-uv;
	float stepsX = min(floor(path.x/xstep),100);
	float stepsY = min(floor(path.y/ystep),100);
	path = normalize(path);
	for(int x = 0; x < stepsX; x++){
		for(int y = 0; y < stepsY; y++){
			vec2 pos = uv + vec2(x*xstep*path.x, y*ystep*path.y);

			float d = LinearizeDepth(texture2D(TexDepth, pos).r);
			if (d>0.9){
				sunRay+=0.00001;
			}
		}
	}*/

	
	float sunRay = 0;
	vec2 pos = uv;
	int i =0;
	if (depth < 0.9){
		while (distance(pos,sunUV)>0.1 && i < 2000){
			float d = LinearizeDepth(texture2D(TexDepth, pos).r);
			if (d>0.9){
				sunRay+=0.00050*(sun.z/1000);
			}

			vec2 path = normalize(sunUV-pos);
			pos+= vec2(path.x*xstep, path.y*ystep);
			i++;
		}
	}
	

	

	color_out = vec4(colorFinal.rgb, 1);

	color_out += vec4(sunPosDist, sunPosDist,0,1)+sunRay;
	if (isPointOnLine(vec2(0.5,0.5), sunUV, uv)){
		color_out = rayColor;
	}
}