#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <string>
#include <vector>
#include <memory>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <array>


#include "d3dx12.h"
#include <unordered_map>
#include "MathHelper.h"

using Microsoft::WRL::ComPtr;


inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}


class d3dUtil
{
public:


	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes). So round up to nearest 
		// multiple of 256
		return (byteSize + 255) & ~255;
	}

	static ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer);

	static ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target
	);
};


class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};



// Defines a subrange of geometry in a MeshGeometry. This is for when multiple 
// geometries are stored in 1 vertex and index buffer.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	std::string Name;

	// System memory copies. Use Blobs because the vertex/index format can be generic
	// It is up to the client to cast appropriately
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

	// A MeshGeometry may store multiple geometries in 1 vertex/index buffer
	// Use this container to define the Submesh geometries so we can draw the 
	// Submeshes individually
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

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

struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f,0.5f,0.5f };
	float FalloffStart = 1.0f;
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
	float FalloffEnd = 10.0f;
	DirectX::XMFLOAT3 Position = { 0.0f,0.0f,0.0f };
	float SpotPower = 64.0f;
};

#define MaxLights 16

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	// Used in texture mapping
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

extern const int gNumFrameResources;

struct Material
{
	// Unique material name for lookup
	std::string Name;

	// Index into constant buffer corresponding to this material
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture
	int NormalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the
	// constant buffer. Because we have a material constant buffer for each Frameresource
	int NumFramesDirty = gNumFrameResources;

	// Material constant buffer data used for shading
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct TextureId
{
	std::string shortName;
	std::string type;
	//std::string fullPath;

	//unsigned int SRVHeapIndex;
};

struct Texture
{
	// Unique material name for lookup
	std::string Name;

	std::wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;

	std::string type;

	UINT SRVIndex;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)\
{ \
	HRESULT hr__ = (x); \
	std::wstring wfn = AnsiToWString(__FILE__); \
	if (FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__);  } \
}

#endif

#ifndef ReleaseCom
#define ReleaseCom(x) {if(x) {x->Release(); x = 0;}}
#endif