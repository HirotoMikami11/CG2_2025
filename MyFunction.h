#pragma once
#include "MyMath.h"
///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<cassert>
#include<wrl.h>

/// <summary>
/// 球体
/// </summary>
struct Sphere {
	Vector3 center;	///中心点
	float radius;	///半径
};

/// <summary>
/// 直線
/// </summary>
struct Line {
	Vector3 origin;		//始点
	Vector3 diff;		//終点への差分ベクトル
};

/// <summary>
/// 半直線
/// </summary>
struct Ray {
	Vector3 origin;		//始点
	Vector3 diff;		//終点への差分ベクトル
};

/// <summary>
/// 線分
/// </summary>
struct Segment {
	Vector3 origin;		//始点
	Vector3 diff;		//終点への差分ベクトル
};

/// <summary>
/// 平行光源
/// </summary>
struct DirectionalLight {
	Vector4  color;		//色
	Vector3 direction;	//方向
	float intensity;	//強度
};


Microsoft::WRL::ComPtr <ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr <ID3D12Device> device, size_t sizeInBytes);

/// <summary>
/// 矩形スプライトvertexDataに情報を書き込む関数
/// </summary>
/// <returns></returns>
void CreateSpriteVertexData(VertexData vertexDataSprite[], Vector2 center, Vector2 radius);


/// <summary>
/// 三角形リスト方式のVetrtexDataを作成する関数
/// </summary>
/// <param name="vertexData"></param>
//void CreateSphereVertexData(VertexData vertexData[]);

/// <summary>
/// インデックス方式の球体VetrtexDataを作成する関数
/// </summary>
/// <param name="vertexData"></param>
void CreateSphereVertexData(VertexData vertexData[]);

/// <summary>
/// インデックス方式の球体IndexDataを作成する関数
/// </summary>
/// <param name="indexData"></param>
void CreateSphereIndexData(uint32_t indexData[]);



/*-----------------------------------------------------------------------*/
//
//								計算関数
//
/*-----------------------------------------------------------------------*/

//	正射影ベクトルを求める関数
Vector3 Project(const Vector3& v1, const Vector3& v2);
//　最近接点を求める関数
Vector3 ClosestPoint(const Vector3& point, const Segment& segment);
//	行列の更新
void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, TransformationMatrix* matrixData);

void UpdateUVTransform(const Vector3Transform uvtransform, Material* materialData);

/*-----------------------------------------------------------------------*/
//
//								描画関数
//
/*-----------------------------------------------------------------------*/
//グリッド線を描画する関数
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

//球体を表示する関数
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

//線分を描画する関数
void DrawLine(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);




