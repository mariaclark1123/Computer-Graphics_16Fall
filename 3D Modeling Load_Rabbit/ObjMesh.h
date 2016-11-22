// ObjMesh.h: interface for the ObjMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJMESH_H__796231AB_E521_4A69_8FDA_D1E69EE353C0__INCLUDED_)
#define AFX_OBJMESH_H__796231AB_E521_4A69_8FDA_D1E69EE353C0__INCLUDED_

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

class ObjMesh  
{
public:
	ObjMesh();
	virtual ~ObjMesh();
	void Render();
	void SetColor(float r, float g, float b);

	float* m_vertices;		//���� �迭 
	float* m_texCoords;		//�ؽ��� �迭
	float* m_normals;
	
	int* m_Faces;			//��(�ﰢ���� �̷�� ���� 3��)
	int* m_TextureFace;		//��(�ﰢ���� �̷�� �ؽ��� 3��)
	
	float m_Color[3];

	int m_numVertices;
	int m_numFaces;
	int m_numNormals;
	int m_numTexCoords;
};

#endif // !defined(AFX_OBJMESH_H__796231AB_E521_4A69_8FDA_D1E69EE353C0__INCLUDED_)
