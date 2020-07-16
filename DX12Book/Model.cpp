#include "Model.h"




#include <iostream>
//using std::cout;
#include <sstream>
using std::endl;


void Model::loadModel(string path)
{

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, 
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_RemoveComponent |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_ValidateDataStructure |
		//aiProcess_ImproveCacheLocality | // handled by optimizePostTransform()
		aiProcess_RemoveRedundantMaterials |
		aiProcess_SortByPType |
		aiProcess_FindInvalidData |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_OptimizeGraph
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::stringstream strstr;
		strstr << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
		OutputDebugStringA(strstr.str().c_str());
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);

	const UINT vbByteSize = (UINT)mVertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)mIndices.size() * sizeof(std::uint16_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &VertexBufferCPU));
	CopyMemory(VertexBufferCPU->GetBufferPointer(), mVertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &IndexBufferCPU));
	CopyMemory(IndexBufferCPU->GetBufferPointer(), mIndices.data(), ibByteSize);

	VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCmdList.Get(),
		mVertices.data(), vbByteSize, VertexBufferUploader);

	IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCmdList.Get(),
		mIndices.data(), ibByteSize, IndexBufferUploader);

	VertexByteStride = sizeof(Vertex);
	VertexBufferByteSize = vbByteSize;

	IndexFormat = DXGI_FORMAT_R16_UINT;
	IndexBufferByteSize = ibByteSize;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all meshes in this node.
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	// process all its children 
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	//vector<Vertex> vertices;
	//vector<std::uint16_t> indices;
	//vector<TextureId> textures;

	Mesh rMesh;

	rMesh.BaseVertexLocation = mVertices.size();
	rMesh.StartIndexLocation = mIndices.size();
	rMesh.IndexCount= 0;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		
		// Process position, normal and uv
		vertex.Pos = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		
		// check if mesh has texcoords
		if (mesh->mTextureCoords[0])
		{
			vertex.TexC = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.TexC = XMFLOAT2(0.0f, 0.0f);
		}

		mVertices.push_back(vertex);
	}

	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			mIndices.push_back(face.mIndices[j]);
			++rMesh.IndexCount;
		}
	}


	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<TextureId> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		rMesh.textures.insert(rMesh.textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		//vector<TextureId> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		//rMesh.textures.insert(rMesh.textures.end(), specularMaps.begin(), specularMaps.end());
	}


	return rMesh;
}

#include "DDSTextureLoader.h"

//std::wstring s2ws(const std::string& str)
//{
//	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
//	std::wstring wstrTo(size_needed, 0);
//	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
//	return wstrTo;
//}

#include "DirectXTK12/WICTextureLoader.h"

void CreateWICTextureFromFile12(_In_ ID3D12Device* device,
	_In_ ID3D12GraphicsCommandList* cmdList,
	_In_z_ const wchar_t* szFileName,
	_Out_ ComPtr<ID3D12Resource>& texture,
	_Out_ ComPtr<ID3D12Resource>& textureUploadHeap
	)
{
	std::unique_ptr<uint8_t[]> decodedData;
	D3D12_SUBRESOURCE_DATA subresource;
	ThrowIfFailed(LoadWICTextureFromFile(device, szFileName, texture.ReleaseAndGetAddressOf(), decodedData, subresource));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	ThrowIfFailed(device->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
	));

	UpdateSubresources(cmdList, texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &subresource);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	cmdList->ResourceBarrier(1, &barrier);
}


vector<TextureId> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string defTexPathname)
{
	vector<TextureId> textureIds;

	auto TexHelper = [&](std::string shortName, std::wstring fullPath)
	{
		TextureId id;
		id.shortName = shortName;
		id.type = type;
		textureIds.push_back(id);

		if (mTextures.find(shortName) != mTextures.end())
			return;

		auto tmpTex = std::make_unique<Texture>();
		tmpTex->Name = shortName;
		tmpTex->Filename = fullPath;

		string ext = shortName.substr(shortName.find_last_of('.'));
		if (ext == ".dds" || ext == ".DDS")
		{
			ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
				mCmdList.Get(), tmpTex->Filename.c_str(),
				tmpTex->Resource, tmpTex->UploadHeap));
		}
		else
		{
			CreateWICTextureFromFile12(md3dDevice.Get(),
				mCmdList.Get(), tmpTex->Filename.c_str(),
				tmpTex->Resource, tmpTex->UploadHeap);
		}

		
		tmpTex->type = type;

		mTextures[tmpTex->Name] = std::move(tmpTex);
	};

	//for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	if (mat->GetTextureCount(type) < 1)
	{
		// Use default texture

		TexHelper(defTexPathname.substr(defTexPathname.find_last_of('/')),
			std::wstring(defTexPathname.begin(), defTexPathname.end()));
	}
	else
	{
		aiString str;
		mat->GetTexture(type, 0, &str);
		//mat->Get(AI_MATKEY_TEXTURE(type, 0), str);

		string fileName = str.C_Str();
		
		std::stringstream strstr;
		strstr << "Kagtag " << fileName << endl;
		OutputDebugStringA(strstr.str().c_str());

		fileName = fileName.substr(fileName.find_last_of('\\') + 1);
		fileName = fileName.substr(fileName.find_last_of('/') + 1);

		string fullPath = directory + '/' + fileName;

		TexHelper(fileName, std::wstring(fullPath.begin(), fullPath.end()));
	}

	return textureIds;
}