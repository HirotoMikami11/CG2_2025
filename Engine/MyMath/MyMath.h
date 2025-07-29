#pragma once
#define _USE_MATH_DEFINES
#include<math.h>
#include <cmath>
#include<assert.h>
#include <vector>
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
#include<sstream>
#include<numbers>

#include <algorithm>

///ウィンドウサイズ
#include"BaseSystem/GraphicsConfig.h"

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

///																		///
///								float
///																		///

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

float Lerp(const float& min, const float& max, float t);


///Easing関連

float EaseInSine(float x);
float EaseOutSine(float x);
float EaseInOutSine(float x);

float EaseInQuad(float x);
float EaseOutQuad(float x);
float EaseInOutQuad(float x);

float EaseInCubic(float x);
float EaseOutCubic(float x);
float EaseInOutCubic(float x);

float EaseInQuart(float x);
float EaseOutQuart(float x);
float EaseInOutQuart(float x);

float EaseInQuint(float x);
float EaseOutQuint(float x);
float EaseInOutQuint(float x);

float EaseInExpo(float x);
float EaseOutExpo(float x);
float EaseInOutExpo(float x);

float EaseInCirc(float x);
float EaseOutCirc(float x);
float EaseInOutCirc(float x);

float EaseInBack(float x);
float EaseOutBack(float x);
float EaseInOutBack(float x);

float EaseInElastic(float x);
float EaseOutElastic(float x);
float EaseInOutElastic(float x);

float EaseInBounce(float x);
float EaseOutBounce(float x);
float EaseInOutBounce(float x);





///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

///																		///
///								ベクトル
///																		///

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//
/*-----------------------------------------------------------------------*/
//
//								2次元ベクトル
//
/*-----------------------------------------------------------------------*/

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;
};

// 長さ（座標から）
float Length(const float x, const float y);

// 長さ（Vector2から）
float Length(const Vector2& v);

// 加算
Vector2 Add(const Vector2& v1, const Vector2& v2);

// 減算
Vector2 Subtract(const Vector2& v1, const Vector2& v2);

// スカラー倍
Vector2 Multiply(float scalar, const Vector2& v);

// 内積
float Dot(const Vector2& v1, const Vector2& v2);

// 正規化
Vector2 Normalize(const Vector2& v);

// 距離
float Distance(const Vector2& v1, const Vector2& v2);

// 2Dクロス積（スカラー値を返す）
float Cross(const Vector2& v1, const Vector2& v2);

// 線形補間
Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t);

// 垂直ベクトル（90度回転）
Vector2 Perpendicular(const Vector2& v);

// 回転
Vector2 Rotate(const Vector2& v, float radian);


/*-----------------------------------------------------------------------*/
//
//						Vector2 演算子オーバーロード
//
/*-----------------------------------------------------------------------*/

// ===== 二項演算子 =====

// 加算 (v1 + v2)
inline Vector2 operator+(const Vector2& v1, const Vector2& v2) { return { v1.x + v2.x, v1.y + v2.y }; }

// 減算 (v1 - v2)
inline Vector2 operator-(const Vector2& v1, const Vector2& v2) { return { v1.x - v2.x, v1.y - v2.y }; }

// スカラー倍 (scalar * v)
inline Vector2 operator*(float scalar, const Vector2& v) { return { v.x * scalar, v.y * scalar }; }

// スカラー倍 (v * scalar)
inline Vector2 operator*(const Vector2& v, float scalar) { return { v.x * scalar, v.y * scalar }; }

// スカラー除算 (v / scalar)
inline Vector2 operator/(const Vector2& v, float scalar) {
	assert(scalar != 0.0f); // ゼロ除算チェック
	return { v.x / scalar, v.y / scalar };
}

// ===== 単項演算子 =====

// 単項マイナス (-v)
inline Vector2 operator-(const Vector2& v) { return { -v.x, -v.y }; }

// 単項プラス (+v)
inline Vector2 operator+(const Vector2& v) { return v; }

// ===== 複合代入演算子 =====

// 加算代入 (v1 += v2)
inline Vector2& operator+=(Vector2& v1, const Vector2& v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	return v1;
}

// 減算代入 (v1 -= v2)
inline Vector2& operator-=(Vector2& v1, const Vector2& v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	return v1;
}

// スカラー倍代入 (v *= scalar)
inline Vector2& operator*=(Vector2& v, float scalar) {
	v.x *= scalar;
	v.y *= scalar;
	return v;
}

// スカラー除算代入 (v /= scalar)
inline Vector2& operator/=(Vector2& v, float scalar) {
	assert(scalar != 0.0f); // ゼロ除算チェック
	v.x /= scalar;
	v.y /= scalar;
	return v;
}


/*-----------------------------------------------------------------------*/
//
//								3次元ベクトル
//
/*-----------------------------------------------------------------------*/
/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;
};

/// <summary>
/// トランスフォーム
/// </summary>
struct Vector3Transform final {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

//加算
Vector3 Add(const Vector3& v1, const Vector3& v2);
//減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);
//スカラー倍
Vector3 Multiply(const Vector3& v, float scalar);
//内積
float Dot(const Vector3& v1, const Vector3& v2);
//長さ
float Length(const Vector3& v);
//正規化
Vector3 Normalize(const Vector3& v);

//クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);
//距離
float Distance(const Vector3& v1, const Vector3& v2);
//線形補間
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

//球面線形補間
//min,max
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

/*-----------------------------------------------------------------------*/
//
//							Vector3 演算子オーバーロード
//
/*-----------------------------------------------------------------------*/

///二項演算子

// 加算 (v1 + v2)
inline Vector3 operator+(const Vector3& v1, const Vector3& v2) { return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z }; }

// 減算 (v1 - v2)
inline Vector3 operator-(const Vector3& v1, const Vector3& v2) { return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z }; }

// スカラー倍 (scalar * v)
inline Vector3 operator*(float scalar, const Vector3& v) { return { v.x * scalar, v.y * scalar, v.z * scalar }; }

// スカラー倍 (v * scalar)
inline Vector3 operator*(const Vector3& v, float scalar) { return { v.x * scalar, v.y * scalar, v.z * scalar }; }

// スカラー除算 (v / scalar)
inline Vector3 operator/(const Vector3& v, float scalar) {
	assert(scalar != 0.0f); // ゼロ除算チェック
	return { v.x / scalar, v.y / scalar, v.z / scalar };
}

///単項演算子

// 単項マイナス (-v)
inline Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }

// 単項プラス (+v)
inline Vector3 operator+(const Vector3& v) { return v; }

/// 代入演算子

// 加算代入 (v1 += v2)
inline Vector3& operator+=(Vector3& v1, const Vector3& v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	return v1;
}

// 減算代入 (v1 -= v2)
inline Vector3& operator-=(Vector3& v1, const Vector3& v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	return v1;
}

// スカラー倍代入 (v *= scalar)
inline Vector3& operator*=(Vector3& v, float scalar) {
	v.x *= scalar;
	v.y *= scalar;
	v.z *= scalar;
	return v;
}

// スカラー除算代入 (v /= scalar)
inline Vector3& operator/=(Vector3& v, float scalar) {
	assert(scalar != 0.0f); // ゼロ除算チェック
	v.x /= scalar;
	v.y /= scalar;
	v.z /= scalar;
	return v;
}
/*-----------------------------------------------------------------------*/
//
//								4次元ベクトル
//
/*-----------------------------------------------------------------------*/
/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
};


/// <summary>
/// 頂点データ
/// </summary>
struct VertexData final {
	Vector4 position;//座標
	Vector2 texcoord;//UV座標系(テクスチャ座標系)
	Vector3 normal;	//法線
};



/// <summary>
/// mtlファイルを読んで使えるようにする
/// </summary>
struct MaterialDataModel {
	std::string textureFilePath;	//テクスチャファイルのパス
};


struct ModelData {
	std::vector<VertexData> vertices;
	MaterialDataModel material;
	std::string materialName = "";    // マテリアル名
	size_t materialIndex = 0;         // マテリアルインデックス
};


///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

///																		///
///								行列
///																		///

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//



/*-----------------------------------------------------------------------*/
//
//								3x3
//
/*-----------------------------------------------------------------------*/


//3x3の行列を表す
struct Matrix3x3 {
	float m[3][3];
};

Matrix3x3 Matrix3x3Add(Matrix3x3 matrix1, Matrix3x3 matrix2);

Matrix3x3 Matrix3x3Subtract(Matrix3x3 matrix1, Matrix3x3 matrix2);
//行列の積
Matrix3x3 Matrix3x3Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2);


//回転行列
Matrix3x3 Matrix3x3MakeRotateMatrix(float theta);

//平行移動行列
//平行移動行列
Matrix3x3 Matrix3x3MakeTranslateMatrix(Vector2 translate);

//拡大縮小行列
Matrix3x3 Matrix3x3MakeScaleMatrix(Vector2 scale);

//アフィン行列
Matrix3x3 Matrix3x3MakeAffineMatrix(Vector2 scale, float rotate, Vector2 translate);

//行列変換
Vector2 Matrix3x3Transform(Vector2 vector, Matrix3x3 matrix);

//3x3行列の逆行列を生成
Matrix3x3 Matrix3x3Inverse(Matrix3x3 matrix);

//3x3転置行列を求める
Matrix3x3 Matrix3x3Transpose(Matrix3x3 matrix);



/*-----------------------------------------------------------------------*/
//
//								4x4
//
/*-----------------------------------------------------------------------*/
/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 final {
	float m[4][4];
};


/// <summary>
/// マテリアル
/// </summary>
struct  MaterialData final {
	Vector4 color;						//色
	int32_t enableLighting;				//ライティングするか
	int32_t useLambertianReflectance;	//ランバート反射させるか
	float padding[2];					//隙間埋める
	Matrix4x4 uvTransform;
};



/// <summary>
/// 座標変換行列
/// </summary>
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;

};

//4x4行列の加算
Matrix4x4 Matrix4x4Add(const Matrix4x4& m1, const Matrix4x4& m2);
//4x4行列の減算
Matrix4x4 Matrix4x4Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
//4x4行列の積
Matrix4x4 Matrix4x4Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
//4x4行列の逆行列
Matrix4x4 Matrix4x4Inverse(const Matrix4x4& m);
//4x4行列の転置
Matrix4x4 Matrix4x4Transpose(const Matrix4x4& m);
//4x4行列の単位行列の生成
Matrix4x4 MakeIdentity4x4();


//4x4行列の平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
//4x4行列の拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& Scale);
//4x4行列の座標変換
Vector3 Matrix4x4Transform(const Vector3& vector, const Matrix4x4& matrix);
// 4x4行列方向ベクトル変換
Vector3 Matrix4x4TransformNormal(const Vector3& vector, const Matrix4x4& matrix);


//X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
//Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
//Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);

//XYZ軸回転行列
Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotate);

//アフィン返還行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

//透視射影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
//正射影行列
Matrix4x4 MakeOrthograpicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
//ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

//
Matrix4x4 MakeViewProjectionMatrix(const Vector3Transform& camera, float aspectRatio);
//矩形Sprite用のカメラを原点としたviewProjecton
Matrix4x4 MakeViewProjectionMatrixSprite();

/// <summary>
/// 方向ベクトルを行列で変換する関数
/// 平行移動成分は無視し、回転・スケール成分のみを適用する
/// </summary>
/// <param name="v">変換したい方向ベクトル（ローカル座標）</param>
/// <param name="m">変換行列（回転・スケール・平行移動を含む4x4行列）</param>
/// <returns>変換された方向ベクトル（ワールド座標）</returns>
Vector3 TransformDirection(const Vector3& vector, const Matrix4x4& matrix);