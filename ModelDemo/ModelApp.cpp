//#include "d3dApp.h"
//
//using namespace DirectX;
//using std::vector;
//using std::string;
//
//
//struct Vertex
//{
//	XMFLOAT3 Position;
//	XMFLOAT3 Normal;
//	XMFLOAT2 TexCoords;
//};
//
//struct Texture
//{
//	unsigned int id;
//	string type;  // Diffuse? Specular
//};
//
//class Mesh
//{
//public:
//	// Mesh data
//	vector<Vertex> vertices;
//	vector<unsigned int> indices;
//	vector<Texture> textures;
//
//	Mesh(vector<Vertex> _vertices, vector<unsigned int> _indices, vector<Texture> _textures)
//		:vertices(_vertices),indices(_indices), textures(_textures)
//	{
//		setupMesh();
//	}
//	void Draw(class Shader shader);
//
//private:
//	unsigned int VAO, VBO, EBO;
//
//	void setupMesh();
//
//};
//
//#include "assimp/Importer.hpp"
//#include "assimp/scene.h"
//#include "assimp/postprocess.h"
//
//#include <iostream>
//
//using std::cout;
//using std::endl;
//
//void loadModel(string path)
//{
//	Assimp::Importer imp;
//	const aiScene* scene = imp.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | 
//	aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
//
//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//	{
//		cout << "ERROR::ASSIMP::" << imp.GetErrorString() << endl;
//		return;
//	}
//
//	
//
//}
//
//void processNode(aiNode* node, const aiScene* scene)
//{
//	// process all meshes of the node, if there is any.
//	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
//	{
//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//		meshes.push_back()
//	}
//
//	// do the same thing to all its child nodes.
//	for (unsigned int i = 0; i < node->mNumChildren; ++i)
//	{
//		processNode(node->mChildren[i], scene);
//	}
//}
//
//vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
//{
//	vector<Texture> textures;
//
//	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
//	{
//		aiString str;
//		mat->GetTexture(type, i, &str);
//		Texture texture;
//		texture.id = TextureFromFil
//	}
//}
//
//Mesh processMesh(aiMesh* mesh, const aiScene* scene)
//{
//	vector<Vertex> vertices;
//	vector<unsigned int> indices;
//	vector<Texture> textures;
//
//	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
//	{
//		Vertex vertex; 
//		// processing position, normal, and uv
//
//		vertex.Position = XMFLOAT3(
//			mesh->mVertices[i].x,
//			mesh->mVertices[i].y,
//			mesh->mVertices[i].z
//			);
//
//		vertex.Normal = XMFLOAT3(
//			mesh->mNormals[i].x,
//			mesh->mNormals[i].y,
//			mesh->mNormals[i].z
//		);
//
//		if (mesh->mTextureCoords[0]) // does texture coordinates exist ?
//		{
//			vertex.TexCoords = XMFLOAT2(
//				mesh->mTextureCoords[0][i].x,
//				mesh->mTextureCoords[0][i].y
//			);
//		}
//		else
//		{
//			vertex.TexCoords = XMFLOAT2(0.0f, 0.0f);
//		}
//
//		vertices.push_back(vertex);
//	}
//
//	// processing index
//	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
//	{
//		aiFace face = mesh->mFaces[i];
//		for (unsigned int j = 0; j < face.mNumIndices; ++j)
//		{
//			indices.push_back(face.mIndices[j]);
//		}
//	}
//
//
//	// processing material
//	if (mesh->mMaterialIndex >= 0)
//	{
//		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
//		vector<Texture> diffuseMaps 
//	}
//
//	return Mesh(vertices, indices, textures);
//}