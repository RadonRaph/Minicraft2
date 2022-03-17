#ifndef __YOCTO__ENGINE_TEST__
#define __YOCTO__ENGINE_TEST__

#include "engine/engine.h"

#include "avatar.h"
#include "world.h"
#include "../RaphRender.h"
#include "world.h"

#include <unordered_map>

class MEngineMinicraft : public YEngine {

public :
	int SunShader;
	int ShaderCube;
	int WorldShader;
	YVec3f SunPosition;
	YVec3f SunDirection;
	YColor SkyColor;
	YVec3f SunDir;
	YVbo* VboCube;
	MWorld* World;

	YColor SunColor;
	GLuint postProcessShader;
	YFbo* FBOPostProcess;
	YTexFile* texHatch;


	bool mouseDown;
	YVec3f mousePos_;
	YVec3f mousePos;
	std::unordered_map<int, bool> inputsState;

	MAvatar * avatar;


	//Gestion singleton
	static YEngine * getInstance()
	{
		if (Instance == NULL)
			Instance = new MEngineMinicraft();
		return Instance;
	}

	/*HANDLERS GENERAUX*/
	void loadShaders() {
		SunShader = Renderer->createProgram("shaders/sun");
		ShaderCube = Renderer->createProgram("shaders/cube");
		WorldShader = Renderer->createProgram("shaders/world");
		postProcessShader = Renderer->createProgram("shaders/postprocess");
	}

	void init() 
	{
		YLog::log(YLog::ENGINE_INFO,"Minicraft Started : initialisation");

		Renderer->setBackgroundColor(YColor(0.0f,0.0f,0.0f,1.0f));
		Renderer->Camera->setPosition(YVec3f(10, 10, 10));
		Renderer->Camera->setLookAt(YVec3f());


		World = new MWorld();
		World->init_world(time(nullptr));

		avatar = new MAvatar(Renderer->Camera, World);

		texHatch = YTexManager::getInstance()->loadTexture("textures/hatching.png");
		/*
		//Creation du VBO
		VboCube = new YVbo(3, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		//Définition du contenu du VBO
		VboCube->setElementDescription(0, YVbo::Element(3)); //Sommet
		VboCube->setElementDescription(1, YVbo::Element(3)); //Normale
		VboCube->setElementDescription(2, YVbo::Element(2)); //UV

		//On demande d'allouer la mémoire coté CPU
		VboCube->createVboCpu();


		YVec3f A(0,1,1),
			B(0,1,0),
			C(1,1,0),
			D(1,1,1),
			E(1,0,0), 
			F(1,0,1), 
			G(0,0,0), 
			H(0,0,1);

		Quad quads[6] = {
			Quad(A,B,C,D),
			Quad(D,C,E,F),
			Quad(A,D,F,H),
			Quad(F,E,G,H),
			Quad(H,G,B,A),
			Quad(G,E,C,B)
		};

		
		for (int i = 0; i < 6; i++)
		{
			quads[i].SetTriangle(VboCube, i*6);
		}




		//On envoie le contenu au GPU
		VboCube->createVboGpu();

		//On relache la mémoire CPU
		VboCube->deleteVboCpu();

		*/

		SunColor = YColor(0.7f, 0.5f, 0.0f, 1.0f);

		FBOPostProcess = new YFbo();
		FBOPostProcess->init(Renderer->ScreenWidth, Renderer->ScreenHeight);
	}

	void update(float elapsed) 
	{
		YCamera* cam = Renderer->Camera;
		YVec3f move;
		
		
		if (inputsState['z'])
			move += cam->Direction;
		if (inputsState['s'])
			move -= cam->Direction;

		if (inputsState['d'])
			move += cam->RightVec;
		if (inputsState['q'])
			move -= cam->RightVec;

		if (inputsState[' '])
			move += YVec3f(0, 0, 2);

		

	
		avatar->update(elapsed, move);
		cam->setPosition(avatar->Position + YVec3f(0.5, 0.5, 3));

		updateLights(DeltaTimeCumul*5);

	}

	void renderObjects() 
	{
		FBOPostProcess->setAsOutFBO(true);

#pragma region Object pass



		glPushMatrix();
		glUseProgram(0);
		//Rendu des axes
		glDisable(GL_LIGHTING);
		//glDisable(GL_CULL_FACE);
		glBegin(GL_LINES);
		glColor3d(1, 0, 0);
		glVertex3d(avatar->Position.X, avatar->Position.Y, avatar->Position.Z);
		glVertex3d(SunPosition.X, SunPosition.Y, SunPosition.Z);
		//glVertex3d(0, 0, 0);
		//glVertex3d(1000, 0, 0);
		glEnd();
		glEnable(GL_LIGHTING);

		glPopMatrix();
		
		//

		

		//glUseProgram(SunShader); //Demande au GPU de charger ces shaders
		glPushMatrix();
		glUseProgram(WorldShader);
		


		Renderer->setBackgroundColor(YColor(0.3f, 0.7f, 1.0f, 1.0f));

		GLuint var = glGetUniformLocation(WorldShader, "skyColor");
		glUniform4f(var, SkyColor.R, SkyColor.V, SkyColor.B, 1.0f);

		var = glGetUniformLocation(WorldShader, "camPos");
		glUniform3f(var, Renderer->Camera->Position.X, Renderer->Camera->Position.Y, Renderer->Camera->Position.Z);

		var = glGetUniformLocation(WorldShader, "sunPos");
		glUniform3f(var, SunPosition.X, SunPosition.Y, SunPosition.Z);

		texHatch->setAsShaderInput(WorldShader, GL_TEXTURE0, "hatchingTex");
		

		World->render_world_vbo(false, true, WorldShader, DeltaTimeCumul);
		glPopMatrix();

		glPushMatrix();
		avatar->render(Renderer, WorldShader);
		glPopMatrix();
#pragma endregion


		glPushMatrix();

		FBOPostProcess->setAsOutFBO(false);

		glUseProgram(postProcessShader);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		FBOPostProcess->setColorAsShaderInput(0, GL_TEXTURE0, "TexColor");
		FBOPostProcess->setDepthAsShaderInput(GL_TEXTURE1, "TexDepth");
		var = glGetUniformLocation(postProcessShader, "sunPos");
		glUniform3f(var, SunPosition.X, SunPosition.Y, SunPosition.Z);

		Renderer->sendMatricesToShader(postProcessShader);
		Renderer->sendNearFarToShader(postProcessShader);
		Renderer->sendScreenSizeToShader(postProcessShader);
		Renderer->drawFullScreenQuad();
		glPopMatrix();


		

	}

	void resize(int width, int height) {
		FBOPostProcess->init(width, height);
	}

	/*INPUTS*/

	void keyPressed(int key, bool special, bool down, int p1, int p2) 
	{	
		inputsState[key] = down;
	}


	void mouseClick(int button, int state, int x, int y, bool inUi)
	{
		
		if (button == 2) {
			mouseDown = !state;
			std::cout << state << std::endl;
		}
	}

	void mouseMove(int x, int y, bool pressed, bool inUi)
	{
		static int lastx = -1;
		static int lasty = -1;

		if (!pressed)
		{
			lastx = x;
			lasty = y;
			showMouse(true);
		}
		else
		{
			if (lastx == -1 && lasty == -1)
			{
				lastx = x;
				lasty = y;
			}

			int dx = x - lastx;
			int dy = y - lasty;

			if (dx == 0 && dy == 0)
				return;

			lastx = x;
			lasty = y;

			if (MouseBtnState & GUI_MRBUTTON)
			{
				showMouse(false);
				if (GetKeyState(VK_LCONTROL) & 0x80)
				{
					Renderer->Camera->rotateAround((float)-dx / 300.0f);
					Renderer->Camera->rotateUpAround((float)-dy / 300.0f);
					glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
					lastx = Renderer->ScreenWidth / 2;
					lasty = Renderer->ScreenHeight / 2;
				}
				else {
					showMouse(false);
					Renderer->Camera->rotate((float)-dx / 300.0f);
					Renderer->Camera->rotateUp((float)-dy / 300.0f);
					glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
					lastx = Renderer->ScreenWidth / 2;
					lasty = Renderer->ScreenHeight / 2;
				}
			}

			if (MouseBtnState & GUI_MMBUTTON)
			{
				showMouse(false);
				if (GetKeyState(VK_LCONTROL) & 0x80)
				{
					YVec3f strafe = Renderer->Camera->RightVec;
					strafe.Z = 0;
					strafe.normalize();
					strafe *= (float)-dx / 2.0f;

					YVec3f avance = Renderer->Camera->Direction;
					avance.Z = 0;
					avance.normalize();
					avance *= (float)dy / 2.0f;

					Renderer->Camera->move(avance + strafe);
				}
				else {
					YVec3f strafe = Renderer->Camera->RightVec;
					strafe.Z = 0;
					strafe.normalize();
					strafe *= (float)-dx / 5.0f;

					Renderer->Camera->move(Renderer->Camera->UpRef * (float)dy / 5.0f);
					Renderer->Camera->move(strafe);
					glutWarpPointer(Renderer->ScreenWidth / 2, Renderer->ScreenHeight / 2);
					lastx = Renderer->ScreenWidth / 2;
					lasty = Renderer->ScreenHeight / 2;
				}
			}
		}
	}

	void mouseWheel(int wheel, int dir, int x, int y, bool inUi)
	{
		Renderer->Camera->move(Renderer->Camera->Direction * 10.0f * dir);
	}



	static bool getSunDirFromDayTime(YVec3f& sunDir, float mnLever, float mnCoucher, float boostTime) {
		bool nuit;

		SYSTEMTIME t;
		GetLocalTime(&t);

		//On borne le tweak time à une journée (cyclique)
		while (boostTime > 24 * 60)
			boostTime -= 24 * 60;

		//Temps écoulé depuis le début de la journée
		float fTime = t.wHour * 60.0f + t.wMinute;
		fTime += boostTime;
		while (fTime > 24 * 60)
			fTime -= 24 * 60;

		//Si c'est la nuit
		if (fTime < mnLever || fTime > mnCoucher) {
			nuit = true;
			if (fTime < mnLever)
				fTime += 24 * 60;
			fTime -= mnCoucher;
			fTime /= (mnLever + 24 * 60 - mnCoucher);
			fTime *= (float)M_PI;
		}
		else {
			//c'est le jour
			nuit = false;
			fTime -= mnLever;
			fTime /= (mnCoucher - mnLever);
			fTime *= (float)M_PI;
		}

		//Direction du soleil en fonction de l'heure
		sunDir.X = cos(fTime);
		sunDir.Y = 0.2f;
		sunDir.Z = sin(fTime);
		//sunDir.X = 1;
		//sunDir.Z = 0.1f;
		sunDir.normalize();

		return nuit;
	}

	void updateLights(float boostTime = 0) {
		//On recup la direction du soleil
		bool nuit = getSunDirFromDayTime(SunDirection, 6.0f * 60.0f, 19.0f * 60.0f, boostTime);
		SunPosition = SunDirection * 500.0f;

		//Pendant la journée
		if (!nuit) {
			//On definit la couleur
			SunColor = YColor(1.0f, 1.0f, 0.8f, 1.0f);
			SkyColor = YColor(0.0f, 181.f / 255.f, 221.f / 255.f, 1.0f);
			YColor downColor(0.9f, 0.5f, 0.1f, 1);

			SunColor = SunColor.interpolate(downColor, (abs(SunDirection.X)));
			SkyColor = SkyColor.interpolate(downColor, (abs(SunDirection.X)));
		}
		else {
			//La nuit : lune blanche et ciel noir
			SunColor = YColor(1, 1, 1, 1);
			SkyColor = YColor(0.1, 0.1, 0.3, 1);
		}

		Renderer->setBackgroundColor(SkyColor);
	}
	
};


#endif