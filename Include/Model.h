#pragma once


#include "d3dUtil.h"
#include <map>
#include <unordered_map>

using namespace DirectX;

using std::vector;
using std::string;


struct Vertex
{
	XMFLOAT3 Pos;
	//XMFLOAT4 Color;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
};

class Mesh
{


public:
	//vector<Vertex> vertices;
	//vector<std::uint16_t> indices;
	vector<TextureId> textures;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;



	//std::string Name;


//private:

	//void setupMesh();

};

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(string path, ComPtr<ID3D12Device> Device, ComPtr<ID3D12GraphicsCommandList> CmdList)
		:md3dDevice(Device), mCmdList(CmdList)
	{
		loadModel(path);
	}


	vector<Mesh> meshes;

	vector<Vertex> mVertices;
	vector<std::uint16_t> mIndices;


	std::unordered_map<string, std::unique_ptr<Texture>> mTextures;

public:
	string directory;

	ComPtr<ID3D12Device> md3dDevice;
	ComPtr<ID3D12GraphicsCommandList> mCmdList;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<TextureId> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);



private:

	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;

	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

public:

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}

};