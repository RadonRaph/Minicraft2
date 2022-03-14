#pragma once

#include "engine/render/renderer.h"
#include "engine/render/vbo.h"
#include "cube.h"
#include "../RaphRender.h"

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class MChunk
{
	public :

		static const int CHUNK_SIZE = 64; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		MCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		YVbo * VboOpaque = NULL;
		YVbo * VboTransparent = NULL;

		MChunk * Voisins[6];

		int _XPos, _YPos, _ZPos; ///< Position du chunk dans le monde

		MChunk(int x, int y, int z)
		{
			memset(Voisins, 0x00, sizeof(void*)* 6);
			_XPos = x;
			_YPos = y;
			_ZPos = z;
		}

		/*
		Creation des VBO
		*/

		//On met le chunk ddans son VBO
		void toVbos(void)
		{
			SAFEDELETE(VboOpaque);
			SAFEDELETE(VboTransparent);

			//Compter les sommets
			int nbVertOpaque = 0;
			int nbVertTransp = 0;

			foreachVisibleTriangle(true, &nbVertOpaque, &nbVertTransp, VboOpaque, VboTransparent);



			//Créer les VBO

			VboOpaque = new YVbo(4, nbVertOpaque, YVbo::PACK_BY_ELEMENT_TYPE);

			//Définition du contenu du VBO
			VboOpaque->setElementDescription(0, YVbo::Element(3)); //Sommet
			VboOpaque->setElementDescription(1, YVbo::Element(3)); //Normale
			VboOpaque->setElementDescription(2, YVbo::Element(2)); //UV
			VboOpaque->setElementDescription(3, YVbo::Element(1)); //Type

			//On demande d'allouer la mémoire coté CPU
			VboOpaque->createVboCpu();

			VboTransparent = new YVbo(4, nbVertTransp, YVbo::PACK_BY_ELEMENT_TYPE);

			//Définition du contenu du VBO
			VboTransparent->setElementDescription(0, YVbo::Element(3)); //Sommet
			VboTransparent->setElementDescription(1, YVbo::Element(3)); //Normale
			VboTransparent->setElementDescription(2, YVbo::Element(2)); //UV
			VboTransparent->setElementDescription(3, YVbo::Element(1)); //Type

			//On demande d'allouer la mémoire coté CPU
			VboTransparent->createVboCpu();

			//Remplir les VBO
			foreachVisibleTriangle(false, &nbVertOpaque, &nbVertTransp, VboOpaque, VboTransparent);

			VboOpaque->createVboGpu();
			VboTransparent->createVboGpu();
			//On relache la mémoire CPU
			VboOpaque->deleteVboCpu();
			VboTransparent->deleteVboCpu();

		}

		//Ajoute un quad du cube. Attention CCW
		int addQuadToVbo(YVbo * vbo, int iVertice, YVec3f & a, YVec3f & b, YVec3f & c, YVec3f & d, float type) {

			return 6;
		}


		//Permet de compter les triangles ou des les ajouter aux VBO
		void foreachVisibleTriangle(bool countOnly, int * nbVertOpaque, int * nbVertTransp, YVbo * VboOpaque, YVbo * VboTrasparent) {
			int currentTriangleOpaque = 0;
			int currentTriangleTrans = 0;
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				for (int y = 0; y < CHUNK_SIZE; y++)
				{
					for (int z = 0; z < CHUNK_SIZE; z++)
					{
						MCube cube = _Cubes[x][y][z];
						//std::cout << cube.getType() << std::endl;

						if (!cube.getDraw() || cube.getType() == 39)
							continue;

						int faces[6];
						getVisibleFaces(x, y, z, faces, &cube);
						int count = faceCount(faces);

						BaseCube bCube = BaseCube(YVec3f(x, y, z)* MCube::CUBE_SIZE + YVec3f(_XPos*CHUNK_SIZE*2, _YPos * CHUNK_SIZE*2, _ZPos), MCube::CUBE_SIZE, MCube::CUBE_SIZE);

						if (cube.isOpaque()) {
							*nbVertOpaque += 6 * count;

							if (!countOnly) {
								for (int i = 0; i < 6; i++)
								{
									if (faces[i]) {
										bCube.quadFaces[i].SetTriangle(VboOpaque, currentTriangleOpaque, cube.getType());
										currentTriangleOpaque += 6;
									}
								}
							}
						}
						else {
							*nbVertTransp += 6 * count;

							if (!countOnly) {
								for (int i = 0; i < 6; i++)
								{
									if (faces[i]) {
										bCube.quadFaces[i].SetTriangle(VboTrasparent,  currentTriangleTrans, cube.getType());
										currentTriangleTrans += 6;
									}
								}
							}
						}
					}
				}
			}
		}

		int faceCount(int faces[]) {
			int sum = 0;
			for (int i = 0; i < 6; i++)
			{
				sum += faces[i];
			}
			return sum;
		}

		bool getVisibleFaces(int x, int y, int z, int faces[], MCube * cube ) {
			MCube* cubeXPrev;
			MCube* cubeXNext;
			MCube* cubeYPrev;
			MCube* cubeYNext;
			MCube* cubeZPrev;
			MCube* cubeZNext;

			get_surrounding_cubes(x, y, z, &cubeXPrev, &cubeXNext, &cubeYPrev, &cubeYNext, &cubeZPrev, &cubeZNext);

			/*
			faces[0] = 1;
			faces[1] = 1;
			faces[2] = 1;
			faces[3] = 1;
			faces[4] = 1;
			faces[5] = 1;
			return 1;
			*/

			/*
			faces[0] = (cubeXPrev != nullptr && cubeXPrev->isOpaque()) ? 0 : 1;
			faces[1] = (cubeXNext != nullptr && cubeXNext->isOpaque()) ? 0 : 1;
			faces[2] = (cubeYPrev != nullptr && cubeYPrev->isOpaque()) ? 0 : 1;
			faces[3] = (cubeYNext != nullptr && cubeYNext->isOpaque()) ? 0 : 1;
			faces[4] = (cubeZPrev != nullptr && cubeZPrev->isOpaque()) ? 0 : 1;
			faces[5] = (cubeZNext != nullptr && cubeZNext->isOpaque()) ? 0 : 1;*/

			faces[0] = faceTest(cubeXPrev, cube);
			faces[1] = faceTest(cubeXNext, cube);
			faces[2] = faceTest(cubeYPrev, cube);
			faces[3] = faceTest(cubeYNext, cube);
			faces[4] = faceTest(cubeZPrev, cube);
			faces[5] = faceTest(cubeZNext, cube);

			return faces[0] + faces[1] + faces[2] + faces[3] + faces[4] + faces[5] > 0;
		}

		int faceTest(MCube* other, MCube* self) {
			if (other == nullptr)
				return 0;

			if (self->getType() == MCube::CUBE_EAU) {
				if (other->getType() != MCube::CUBE_AIR) {
					return 0;
				}
				else {
					return 1;
				}
			}
			else if (other->isOpaque()) {
				return 0;
			}
			else {
				return 1;
			}
		}

		/*
		Gestion du chunk
		*/

		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(false);
						_Cubes[x][y][z].setType(MCube::CUBE_AIR);
					}
		}

		void setVoisins(MChunk * xprev, MChunk * xnext, MChunk * yprev, MChunk * ynext, MChunk * zprev, MChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		void get_surrounding_cubes(int x, int y, int z, MCube ** cubeXPrev, MCube ** cubeXNext,
			MCube ** cubeYPrev, MCube ** cubeYNext,
			MCube ** cubeZPrev, MCube ** cubeZNext)
		{

			*cubeXPrev = NULL;
			*cubeXNext = NULL;
			*cubeYPrev = NULL;
			*cubeYNext = NULL;
			*cubeZPrev = NULL;
			*cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				*cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				*cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == CHUNK_SIZE - 1 && Voisins[1] != NULL)
				*cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < CHUNK_SIZE - 1)
				*cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				*cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE - 1][z]);
			else if (y > 0)
				*cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == CHUNK_SIZE - 1 && Voisins[3] != NULL)
				*cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < CHUNK_SIZE - 1)
				*cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				*cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE - 1]);
			else if (z > 0)
				*cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == CHUNK_SIZE - 1 && Voisins[5] != NULL)
				*cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < CHUNK_SIZE - 1)
				*cubeZNext = &(_Cubes[x][y][z + 1]);
		}

		void render(bool transparent)
		{
			if (transparent)
				VboTransparent->render();
			else
				VboOpaque->render();
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			MCube * cubeXPrev = NULL; 
			MCube * cubeXNext = NULL; 
			MCube * cubeYPrev = NULL; 
			MCube * cubeYNext = NULL; 
			MCube * cubeZPrev = NULL; 
			MCube * cubeZNext = NULL; 

			if(x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE-1][y][z]);
			else if(x > 0)
				cubeXPrev = &(_Cubes[x-1][y][z]);

			if(x == CHUNK_SIZE-1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if(x < CHUNK_SIZE-1)
				cubeXNext = &(_Cubes[x+1][y][z]);

			if(y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE-1][z]);
			else if(y > 0)
				cubeYPrev = &(_Cubes[x][y-1][z]);

			if(y == CHUNK_SIZE-1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if(y < CHUNK_SIZE-1)
				cubeYNext = &(_Cubes[x][y+1][z]);

			if(z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE-1]);
			else if(z > 0)
				cubeZPrev = &(_Cubes[x][y][z-1]);

			if(z == CHUNK_SIZE-1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if(z < CHUNK_SIZE-1)
				cubeZNext = &(_Cubes[x][y][z+1]);

			if( cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL )
				return false;

			if (cubeXPrev->isOpaque() == true && //droite
				cubeXNext->isOpaque() == true && //gauche
				cubeYPrev->isOpaque() == true && //haut
				cubeYNext->isOpaque() == true && //bas
				cubeZPrev->isOpaque() == true && //devant
				cubeZNext->isOpaque() == true)  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z].setDraw(true);
						if(test_hidden(x,y,z))
							_Cubes[x][y][z].setDraw(false);
					}
		}


};