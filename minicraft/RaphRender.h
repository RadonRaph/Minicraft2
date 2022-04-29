#pragma once
#include "engine/engine.h"

class Face {
public:
	YVec3f A, B, C;

	Face() {};
	Face(YVec3f a, YVec3f b, YVec3f c) : A(a), B(b), C(c) {}

	void SetTriangle(YVbo* vbo, int index) {

		YVec3f dir = (B - A).cross(C - A);
		YVec3f norm = dir.normalize();

		YVec3f min = getMin(getMin(A, B),C);
		YVec3f max = getMax(getMax(A, B),C);

		vbo->setElementValue(0, index, A.X, A.Y, A.Z);
		vbo->setElementValue(1, index, norm.X, norm.Y, norm.Z);
		vbo->setElementValue(2, index, 0,0);
		

		vbo->setElementValue(0, index+1, B.X, B.Y, B.Z);
		vbo->setElementValue(1, index+1, norm.X, norm.Y, norm.Z);
		vbo->setElementValue(2, index+1, 0, 0);

		vbo->setElementValue(0, index + 2, C.X, C.Y, C.Z);
		vbo->setElementValue(1, index + 2, norm.X, norm.Y, norm.Z);
		vbo->setElementValue(2, index + 2, 0, 0);
	}

	void SetTriangle(YVbo* vbo, int index, int blockId) {
		SetTriangle(vbo, index);
		vbo->setElementValue(3, index, blockId);
		vbo->setElementValue(3, index+1, blockId);
		vbo->setElementValue(3, index+2, blockId);
	}


	YVec3f getMin(YVec3f A, YVec3f B) {
		if (A.getSqrSize() < B.getSqrSize()) {
			return B;
		}
		else {
			return A;
		}
	}

	YVec3f getMax(YVec3f A, YVec3f B) {
		if (A.getSqrSize() > B.getSqrSize()) {
			return B;
		}
		else {
			return A;
		}
	}
};

class Quad {
public:
	YVec3f A, B, C, D;
	Face North;
	Face South;

	Quad() {

	}

	Quad(YVec3f a, YVec3f b, YVec3f c, YVec3f d) : A(a), B(b), C(c), D(d) {
		North = Face(D, C, A);
		South = Face(C, B, A);
	}

	void SetTriangle(YVbo* vbo, int index) {
		North.SetTriangle(vbo, index);
		South.SetTriangle(vbo, index+3);
	}

	void SetTriangle(YVbo* vbo, int index, int blockId) {
		North.SetTriangle(vbo, index, blockId);
		South.SetTriangle(vbo, index + 3, blockId);
	}
};

/*
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
		};*/

class BaseCube {
public:
	Quad quadFaces[6];
	/*
	cubeXPrev 0 Back
	cubeXNext 1 Front
	cubeYPrev 2 Left
	cubeYNext 3 Right
	cubeZPrev 4 Down
	cubeZNext 5 Up
	$*/

	YVec3f A, B, C, D, E, F, G, H;

	BaseCube(YVec3f position,float width, float heigth) {

		A = position + YVec3f(0, width, heigth);
		B = position + YVec3f(0, width, 0);
		C = position + YVec3f(width, width, 0);
		D = position + YVec3f(width, width, heigth);
		E = position + YVec3f(width, 0, 0);
		F = position + YVec3f(width, 0, heigth);
		G = position + YVec3f(0, 0, 0);
		H = position + YVec3f(0, 0, heigth);

		quadFaces[3] = Quad(A, B, C, D);//right
		quadFaces[1] = Quad(D, C, E, F);//front
		quadFaces[5] = Quad(A, D, F, H);//up
		quadFaces[2] = Quad(F, E, G, H);//left
		quadFaces[0] = Quad(H, G, B, A);//back
		quadFaces[4] = Quad(G, E, C, B);//Down
	};

	YVbo * CreateVBO(int blockId) {
		YVbo * vbo = new YVbo(4, 36, YVbo::PACK_BY_ELEMENT_TYPE);

		vbo->setElementDescription(0, YVbo::Element(3)); //Sommet
		vbo->setElementDescription(1, YVbo::Element(3)); //Normale
		vbo->setElementDescription(2, YVbo::Element(2)); //UV
		vbo->setElementDescription(3, YVbo::Element(1)); //Blco
		vbo->createVboCpu();
		for (int i = 0; i < 6; i++)
		{
			quadFaces[i].SetTriangle(vbo, i * 6, blockId);
		}

		vbo->createVboGpu();
		vbo->deleteVboCpu();

		return vbo;
	}
};


bool getSunDirFromDayTime(YVec3f & sunDir, float mnLever, float mnCoucher, float boostTime)
{
	bool nuit = false;

	SYSTEMTIME t;
	GetLocalTime(&t);

	//On borne le tweak time à une journée (cyclique)
	while (boostTime > 24 * 60)
		boostTime -= 24 * 60;

	//Temps écoulé depuis le début de la journée
	float fTime = (float)(t.wHour * 60 + t.wMinute);
	fTime += boostTime;
	while (fTime > 24 * 60)
		fTime -= 24 * 60;

	//Si c'est la nuit
	if (fTime < mnLever || fTime > mnCoucher)
	{
		nuit = true;
		if (fTime < mnLever)
			fTime += 24 * 60;
		fTime -= mnCoucher;
		fTime /= (mnLever + 24 * 60 - mnCoucher);
		fTime *= (float)M_PI;
	}
	else
	{
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
	sunDir.normalize();

	return nuit;
}