#pragma once
#include "MyMath/MyMath.h"
///DirectX12
#include<d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include<cassert>
#include<wrl.h>
#include <numbers>
#include<algorithm>

/// <summary>
/// 球体
/// </summary>
struct SphereMath {
	Vector3 center;	///中心点
	float radius;	///半径
};

/// <summary>
/// 直線
/// </summary>
struct LineMath {
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

/// <summary>
/// 平面
/// </summary>
struct PlaneMath {
	Vector3 normal;	//法線
	float distance;//距離

};

/// <summary>
/// 三角形
/// </summary>
struct TriangleMath {
	Vector3 vertices[3];//頂点
};

/// <summary>
/// AABB(軸並行境界箱)
/// </summary>
struct AABB {
	Vector3 min;	//最小点
	Vector3 max;	//最大点

};


Microsoft::WRL::ComPtr <ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr <ID3D12Device> device, size_t sizeInBytes);

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


/// <summary>
/// CatmullRomスプライン曲線上の座標を得る関数
/// </summary>
/// <param name="points"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 CatmullRomPosition(const std::vector<Vector3>& points, float t);

/// <summary>
/// CatMullRom補間
/// </summary>
/// <param name="p0"></param>
/// <param name="p1"></param>
/// <param name="p2"></param>
/// <param name="p3"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);

/*-----------------------------------------------------------------------*/
///								座標関数
/*-----------------------------------------------------------------------*/

/// <summary>
/// ワールドからスクリーン座標に変換する関数
/// </summary>
/// <param name="worldPosition">ワールド座標</param>
/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
/// <returns>スクリーン座標</returns>
Vector3 ConvertWorldToScreenPosition(const Vector3& worldPosition, const Matrix4x4& viewProjectionMatrix);




/*-----------------------------------------------------------------------*/
//
//								当たり判定
//
/*-----------------------------------------------------------------------*/


/// <summary>
/// 球と球の衝突判定
/// </summary>
/// <param name="SphereMath1"></param>
/// <param name="SphereMath2"></param>
/// <returns></returns>
bool IsCollision(const SphereMath& SphereMath1, const SphereMath& SphereMath2);

/// <summary>
/// 球と平面の衝突判定
/// </summary>
/// <param name="SphereMath"> 球体/param>
/// <param name="PlaneMath"> 平面</param>
/// <returns></returns>
bool IsCollision(const SphereMath& SphereMath, const PlaneMath& PlaneMath);


/// <summary>
/// 線分と平面の衝突判定
/// </summary>
/// <param name="segment">線分</param>
/// <param name="PlaneMath">平面</param>
/// <returns></returns>
bool IsCollision(const Segment& segment, const PlaneMath& PlaneMath);



/// <summary>
/// 線分と平面の衝突点の座標を求める
/// </summary>
/// <param name="segment"></param>
/// <param name="normal"></param>
/// <param name="distance"></param>
/// <returns></returns>
Vector3 MakeCollisionPoint(const Segment& segment, const PlaneMath& PlaneMath);

/// <summary>
/// 三角形と線分の衝突判定
/// </summary>
/// <param name="TriangleMath"></param>
/// <param name="segnent"></param>
/// <returns></returns>
bool IsCollision(const TriangleMath& TriangleMath, const Segment& segment);

/// <summary>
/// AABBとAABBの衝突判定
/// </summary>
/// <param name="aabb1"></param>
/// <param name="aabb2"></param>
/// <returns></returns>
bool IsCollision(const AABB& aabb1, const AABB& aabb2);


/// <summary>
/// AABBと球の衝突判定
/// </summary>
/// <param name="aabb"></param>
/// <param name="SphereMath"></param>
/// <returns></returns>
bool IsCollision(const AABB& aabb, SphereMath& SphereMath);

/// <summary>
/// 最大最小を正しくする関数
/// </summary>
/// <param name="aabb"></param>
void FixAABBMinMax(AABB& aabb);


/// <summary>
/// AABBと線分の衝突判定
/// </summary>
/// <param name="aabb"></param>
/// <param name="segment"></param>
/// <returns></returns>
bool IsCollision(const AABB& aabb, const Segment& segment);









/*-----------------------------------------------------------------------*/
//
//								描画関数
//
/*-----------------------------------------------------------------------*/
//グリッド線を描画する関数
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

//球体を表示する関数
void DrawSphere(const SphereMath& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);





