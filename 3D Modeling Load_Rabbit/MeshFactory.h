#pragma once
#include <stdio.h>
#include "ObjMesh.h"

class CMeshFacotry
{
public: 
	static void LoadObjModel(char* fileDir, ObjMesh* mesh);
};
