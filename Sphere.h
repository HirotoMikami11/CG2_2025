//#pragma once
//#include "GameObject.h"
//#include <cmath>
//class Sphere : public GameObject {
//private:
//	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
//	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
//	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
//	D3D12_INDEX_BUFFER_VIEW indexBufferView;
//	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
//
//	VertexData* vertexData;
//	uint32_t* indexData;
//	DirectionalLight* directionalLightData;
//
//	static const int kSubdivision = 16; // 分割数
//
//public:
//	void Initialize(DirectXCommon* directX) override;
//	void Update() override;
//	void Draw(DirectXCommon* directX) override;
//	void Finalize() override;
//
//	// ライト設定用
//	void SetDirectionalLight(const DirectionalLight& light);
//
//private:
//	void CreateSphereVertices();
//	void CreateSphereIndices();
//};