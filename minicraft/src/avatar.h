#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "engine/utils/timer.h"
#include "world.h"
#include "../RaphRender.h"

class MAvatar
{
public:
	YVec3f Position;
	YVec3f Speed;

	CONST YVec3f gravity = YVec3f(0, 0, -9.81f*2);

	CONST float moveSpeed = 20;
	CONST float drag = 0.95f;
	CONST float bounciness = 0.1f;

	CONST YColor avatarColor = YColor(0.9f, 0.0f, 0.3f, 1.0f);

	YVbo* avatarVbo;

	bool Move;
	bool Jump;
	float Height;
	float CurrentHeight;
	float Width;
	bool Standing;
	bool InWater;
	bool Crouch;
	bool Run;

	YCamera * Cam;
	MWorld * World;

	YTimer _TimerStanding;

	MAvatar(YCamera * cam, MWorld * world)
	{
		Position = YVec3f((MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_SIZE_METERS) / 2, (MWorld::MAT_HEIGHT_METERS));
		Height = 1.0f;
		CurrentHeight = Height;
		Width = 1.0f;
		Cam = cam;
		Standing = false;
		Jump = false;
		World = world;
		InWater = false;
		Crouch = false;
		Run = false;

		BaseCube cube = BaseCube(YVec3f(0,0,0), Width, Height);
		avatarVbo = cube.CreateVBO(MCube::CUBE_TRONC);
	}

	~MAvatar() {
		delete(avatarVbo);
	}

	void update(float elapsed, YVec3f moveDelta)
	{
		//if (elapsed > 1.0f / 60.0f)
			elapsed = 1.0f / 60.0f;

		Speed += (gravity + moveDelta * moveSpeed - Speed*drag)*elapsed;

		World->getMinCol(Position, Speed, Width * 2.0, Height * 2.0);


		Position = Position + Speed * elapsed;

		/*OLD
		float colMin;
		for (int i = 0; i < 6; ++i) {
			MWorld::MAxis axis = World->getMinCol(Position, Position, Width*2.0, Height*2.0, colMin, false);
			colMin += 0.01f * sign(colMin);
	
			if (axis == MWorld::AXIS_X) {
				//std::cout << "AXIS_X fixed" << std::endl;
				Position.X += colMin;
				Speed.X = -Speed.X*bounciness;
			}
			else if (axis == MWorld::AXIS_Y) {
				//std::cout << "AXIS_Y fixed" << std::endl;
				Position.Y += colMin;
				Speed.Y = -Speed.Y * bounciness;
			}
			else if (axis == MWorld::AXIS_Z) {
				//std::cout << "AXIS_Z fixed" << std::endl;
				Position.Z += colMin;
				Speed.Z = -Speed.Z * bounciness;
			}
			
		}*/
		//}
	}

	void render(YRenderer * Renderer, int  ShaderCube) {
		GLuint var = glGetUniformLocation(ShaderCube, "cube_color");
		glUniform3f(var, avatarColor.R, avatarColor.V, avatarColor.B);
		glTranslatef(Position.X, Position.Y, Position.Z);
		Renderer->updateMatricesFromOgl(); //Calcule toute les matrices à partir des deux matrices OGL
		Renderer->sendMatricesToShader(ShaderCube); //Envoie les matrices au shader

		//avatarVbo->render();
	}
};

#endif