// ObjMesh.cpp: implementation of the ObjMesh class.
//
//////////////////////////////////////////////////////////////////////

#include "ObjMesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl/glut.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ObjMesh::ObjMesh()
{
	m_vertices = NULL;
	m_texCoords = NULL;
	m_normals = NULL;
	
	m_Faces = NULL;
	m_TextureFace = NULL;

	m_numTexCoords = m_numVertices = m_numFaces = m_numNormals = 0;

	m_Color[0] = 0.2;
	m_Color[1] = 1;
	m_Color[2] = 0;
}

ObjMesh::~ObjMesh()
{
 	if(m_vertices != NULL)
 		free(m_vertices);
 	if(m_normals != NULL)
 		free(m_normals);
 	if(m_Faces != NULL)
 		free(m_Faces);
 	if(m_texCoords != NULL)
 		free(m_texCoords);
 	if(m_TextureFace != NULL)
 		free(m_TextureFace);
 	/*if(m_TextureID != -1)
 		glDeleteTextures(1, &m_TextureID);*/
}

void ObjMesh::Render()
{
	int i = 0;
	glPushMatrix();	
	
	//면 그리기
	glColor3fv(m_Color);
	float j = 0;
	//printf("first triangle is (%f, %f, %f)\n", m_vertices[m_Faces[i + 0] * 3 + 0], m_vertices[m_Faces[i + 0] * 3 + 1], m_vertices[m_Faces[i + 0] * 3 + 2]);
	/*printf("first triangle vertice's index is:\n");
	printf("(%d, %d, %d) - (%f, %f, %f)\n", m_Faces[i + 0] * 3 + 0, m_Faces[i + 0] * 3 + 1, m_Faces[i + 0] * 3 + 2, m_vertices[m_Faces[i + 0] * 3 + 0], m_vertices[m_Faces[i + 0] * 3 + 1], m_vertices[m_Faces[i + 0] * 3 + 2]);
	printf("(%d, %d, %d) - (%f, %f, %f)\n", m_Faces[i + 1] * 3 + 0, m_Faces[i + 1] * 3 + 1, m_Faces[i + 1] * 3 + 2, m_vertices[m_Faces[i + 1] * 3 + 0], m_vertices[m_Faces[i + 1] * 3 + 1], m_vertices[m_Faces[i + 1] * 3 + 2]);
	printf("(%d, %d, %d) - (%f, %f, %f)\n", m_Faces[i + 2] * 3 + 0, m_Faces[i + 2] * 3 + 1, m_Faces[i + 2] * 3 + 2, m_vertices[m_Faces[i + 2] * 3 + 0], m_vertices[m_Faces[i + 2] * 3 + 1], m_vertices[m_Faces[i + 2] * 3 + 2]);
*/
	glScalef(20, 20, 20);
	for (int i = 0; i < (m_numFaces - 1) * 3; i+=3)	
	{ 	
		glBegin(GL_TRIANGLES);
 		float p1[3] = {m_vertices[m_Faces[i+0] * 3 + 0], m_vertices[m_Faces[i+0] * 3 + 1], m_vertices[m_Faces[i+0] * 3 + 2]};
		float p2[3] = {m_vertices[m_Faces[i+1] * 3 + 0], m_vertices[m_Faces[i+1] * 3 + 1], m_vertices[m_Faces[i+1] * 3 + 2]};
		float p3[3] = {m_vertices[m_Faces[i+2] * 3 + 0], m_vertices[m_Faces[i+2] * 3 + 1], m_vertices[m_Faces[i+2] * 3 + 2]};
		
		glVertex3fv(p1);
		glVertex3fv(p2);
		glVertex3fv(p3);
		
		glEnd();
	}

	
	//외곽선 그리기
	glScalef(4, 4, 4);
	glColor3f(1,0,0);
	for (i = 0; i < (m_numFaces - 1) * 3; i+=3)
	{	
		glBegin(GL_LINES);
		glVertex3f(m_vertices[m_Faces[i+0] * 3 + 0], m_vertices[m_Faces[i+0] * 3 +1], m_vertices[m_Faces[i+0] * 3 + 2]);
		glVertex3f(m_vertices[m_Faces[i+1] * 3 + 0], m_vertices[m_Faces[i+1] * 3 +1], m_vertices[m_Faces[i+1] * 3 + 2]);
		
		glVertex3f(m_vertices[m_Faces[i+1] * 3 + 0], m_vertices[m_Faces[i+1] * 3 +1], m_vertices[m_Faces[i+1] * 3 + 2]);
		glVertex3f(m_vertices[m_Faces[i+2] * 3 + 0], m_vertices[m_Faces[i+2] * 3 +1], m_vertices[m_Faces[i+2] * 3 + 2]);
		
		glVertex3f(m_vertices[m_Faces[i+2] * 3 + 0], m_vertices[m_Faces[i+2] * 3 +1], m_vertices[m_Faces[i+2] * 3 + 2]);
		glVertex3f(m_vertices[m_Faces[i+0] * 3 + 0], m_vertices[m_Faces[i+0] * 3 +1], m_vertices[m_Faces[i+0] * 3 + 2]);
		glEnd();
	}
	glPopMatrix();
}

void ObjMesh::SetColor(float r, float g, float b)
{
	m_Color[0] = r;
	m_Color[1] = g;
	m_Color[2] = b;
}

