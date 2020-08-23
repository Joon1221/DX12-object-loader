#pragma once

#include "stdafx.h"
#include "component.h"
#include "renderable.h"

//using namespace std;

struct Vertex;
struct TriFace;
struct QuadFace;

class  Mesh : public Renderable {
public:
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Variables
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------

	vector<Vertex> vertices;      // vertices values (x, y, z)
	vector<Vertex> uvVertices;    // uv vertices values (x, y)
	int *visited;

	vector<Vertex> finalVertices; // combination of two datasets above

	vector<QuadFace> quadFaces;   // used to store 4 sided faces from .obj file
	vector<TriFace> triFaces;     // final dataset of all faces (quadfaces data is transformed into 2 trifaces)

	bool usesQuadFace;

	//-------------------------------------------------------------------------
	// Init()/Render() related
	//-------------------------------------------------------------------------
	ID3D12DescriptorHeap *mainDescriptorHeap; 
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// 아래의 variable들은 원래 init()에 있던 것을, vertex정보를 바꿀때 사용하려고 mv로 올림.
	ID3D12Resource *vertexBuffer;
	ID3D12Resource* vBufferUploadHeap;
	//Vertex *vList;
	//int vBufferSize;

	string imagePath; // 이 object전용 imagePath.

	int numIndices; // the number of indices to draw the cube

	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	// Member Functions
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	//-------------------------------------------------------------------------
	Mesh();
	Mesh(string filePath);
	virtual ~Mesh();

	virtual bool Init();
	virtual bool loadFile(string filePath);
	virtual void Render();

	bool loadMtlFile(string mtlFilePathWithFilename);
};
