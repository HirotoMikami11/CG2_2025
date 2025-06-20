#include "MyMath.h"

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

///																		///
///								float
///																		///

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

float Lerp(const float& min, const float& max, float t) { return min + (max - min) * t; };


float EaseInSine(float x) {
	return 1 - cosf((x * float(M_PI)) / 2);
}

float EaseOutSine(float x) {
	return sinf((x * float(M_PI)) / 2);
}

float EaseInOutSine(float x) {
	return -(cosf(float(M_PI) * x) - 1) / 2;
}


float EaseInQuad(float x) {
	return x * x;
}
float EaseOutQuad(float x) {
	return 1 - (1 - x) * (1 - x);
}

float EaseInOutQuad(float x) {
	return x < 0.5f ? 2 * x * x : 1 - powf(-2 * x + 2, 2) / 2;
}


float EaseInCubic(float x) {
	return x * x * x;
}

float EaseOutCubic(float x) {
	return 1 - powf(1 - x, 3);
}

float EaseInOutCubic(float x) {
	return x < 0.5f ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
}

float EaseInQuart(float x) {
	return x * x * x * x;
}

float EaseOutQuart(float x) {
	return 1 - powf(1 - x, 4);
}

float EaseInOutQuart(float x) {
	return x < 0.5f ? 8 * x * x * x * x : 1 - powf(-2 * x + 2, 4) / 2;
}

float EaseInQuint(float x) {
	return x * x * x * x * x;
}

float EaseOutQuint(float x) {
	return 1 - powf(1 - x, 5);
}

float EaseInOutQuint(float x) {
	return x < 0.5f ? 16 * x * x * x * x * x : 1 - powf(-2 * x + 2, 5) / 2;
}


float EaseInExpo(float x) {
	return x == 0 ? 0 : powf(2, 10 * x - 10);
}

float EaseOutExpo(float x) {
	return x == 1 ? 1 : 1 - powf(2, -10 * x);
}

float EaseInOutExpo(float x) {
	return x == 0
		? 0
		: x == 1
		? 1
		: x < 0.5f ? powf(2, 20 * x - 10) / 2
		: (2 - powf(2, -20 * x + 10)) / 2;
}


float EaseInCirc(float x) {
	return 1 - sqrtf(1 - powf(x, 2));
}

float EaseOutCirc(float x) {
	return sqrtf(1 - powf(x - 1, 2));
}

float EaseInOutCirc(float x) {
	return x < 0.5f
		? (1 - sqrtf(1 - powf(2 * x, 2))) / 2
		: (sqrtf(1 - powf(-2 * x + 2, 2)) + 1) / 2;
}


float EaseInBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	return c3 * x * x * x - c1 * x * x;
}

float EaseOutBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1;

	return 1 + c3 * powf(x - 1, 3) + c1 * powf(x - 1, 2);
}

float EaseInOutBack(float x) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;

	return x < 0.5f
		? (powf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
		: (powf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}


float EaseInElastic(float x) {
	const float c4 = (2 * float(M_PI)) / 3;

	return x == 0
		? 0
		: x == 1
		? 1
		: -powf(2, 10 * x - 10) * sinf((x * 10 - 10.75f) * c4);
}

float EaseOutElastic(float x) {
	const float c4 = (2 * float(M_PI)) / 3;

	return x == 0
		? 0
		: x == 1
		? 1
		: powf(2, -10 * x) * sinf((x * 10 - 0.75f) * c4) + 1;
}

float EaseInOutElastic(float x) {
	const float c5 = (2 * float(M_PI)) / 4.5f;

	return x == 0
		? 0
		: x == 1
		? 1
		: x < 0.5f
		? -(powf(2, 20 * x - 10) * sinf((20 * x - 11.125f) * c5)) / 2
		: (powf(2, -20 * x + 10) * sinf((20 * x - 11.125f) * c5)) / 2 + 1;
}





float EaseInBounce(float x) {
	return 1 - EaseOutBounce(1 - x);
}

float EaseOutBounce(float x) {
	const float n1 = 7.5625;
	const float d1 = 2.75;

	if (x < 1 / d1) {
		return n1 * x * x;
	} else if (x < 2 / d1) {
		return n1 * (x -= 1.5f / d1) * x + 0.75f;
	} else if (x < 2.5 / d1) {
		return n1 * (x -= 2.25f / d1) * x + 0.9375f;
	} else {
		return n1 * (x -= 2.625f / d1) * x + 0.984375f;
	}
}

float EaseInOutBounce(float x) {
	return x < 0.5f
		? (1 - EaseOutBounce(1 - 2 * x)) / 2
		: (1 + EaseOutBounce(2 * x - 1)) / 2;
}

void ResetEase(float t, float EaseT, int dir)
{
	t = 0;
	EaseT = 0;
	dir = 1;
}


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

// 長さ（座標から）
float Length(const float x, const float y) {
	float result = sqrtf(x * x + y * y);
	return result;
}

// 長さ（Vector2から）
float Length(const Vector2& v) {
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return result;
}

// 加算（Vector2版）
Vector2 Add(const Vector2& v1, const Vector2& v2) {
	Vector2 result = { v1.x + v2.x, v1.y + v2.y };
	return result;
}

// 減算（Vector2版）
Vector2 Subtract(const Vector2& v1, const Vector2& v2) {
	Vector2 result = { v1.x - v2.x, v1.y - v2.y };
	return result;
}

// スカラー倍（Vector2版）
Vector2 Multiply(float scalar, const Vector2& v) {
	Vector2 result = { v.x * scalar, v.y * scalar };
	return result;
}

// 内積（Vector2版）
float Dot(const Vector2& v1, const Vector2& v2) {
	float result = (v1.x * v2.x) + (v1.y * v2.y);
	return result;
}

// 正規化（Vector2版）
Vector2 Normalize(const Vector2& v) {
	Vector2 result = { 0, 0 };
	float length = Length(v);

	if (length != 0) {
		result.x = v.x / length;
		result.y = v.y / length;
	}

	return result;
}

// 距離（Vector2版）
float Distance(const Vector2& v1, const Vector2& v2) {
	float result = sqrtf(powf(v2.x - v1.x, 2) + powf(v2.y - v1.y, 2));
	return result;
}

// 2Dクロス積（Vector2版 - スカラー値を返す）
float Cross(const Vector2& v1, const Vector2& v2) {
	float result = (v1.x * v2.y) - (v1.y * v2.x);
	return result;
}

// 線形補間（Vector2版）
Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t) {
	return Vector2(v1.x + (v2.x - v1.x) * t, v1.y + (v2.y - v1.y) * t);
}

// 垂直ベクトル（90度回転）
Vector2 Perpendicular(const Vector2& v) {
	Vector2 result = { -v.y, v.x };
	return result;
}

// 回転
Vector2 Rotate(const Vector2& v, float radian) {
	Vector2 result;
	float cosTheta = std::cosf(radian);
	float sinTheta = std::sinf(radian);

	result.x = v.x * cosTheta - v.y * sinTheta;
	result.y = v.x * sinTheta + v.y * cosTheta;

	return result;
}

/*-----------------------------------------------------------------------*/
//
//								3次元ベクトル
//
/*-----------------------------------------------------------------------*/

// 加算（Vector3版）
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
		v1.x + v2.x,
		v1.y + v2.y,
		v1.z + v2.z
	};
	return result;
}

// 減算（Vector3版）
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
	v1.x - v2.x,
	v1.y - v2.y,
	v1.z - v2.z
	};
	return result;
}

// スカラー倍（Vector3版）
Vector3 Multiply(const Vector3& v, float scalar) {
	Vector3 result = {
	v.x * scalar,
	v.y * scalar,
	v.z * scalar
	};
	return result;
}

// 内積（Vector3版）
float Dot(const Vector3& v1, const Vector3& v2) {
	float result =
		v1.x * v2.x +
		v1.y * v2.y +
		v1.z * v2.z;

	return result;
}

// 長さ（Vector3版）
float Length(const Vector3& v) {
	float result = sqrtf(
		v.x * v.x +
		v.y * v.y +
		v.z * v.z
	);
	return result;
}

// 正規化（Vector3版）
Vector3 Normalize(const Vector3& v) {
	Vector3 result = { 0,0,0 };
	float length = Length(v);

	if (length != 0) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}

	return result;
}

// クロス積（Vector3版）
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
	(v1.y * v2.z) - (v1.z * v2.y),
	(v1.z * v2.x) - (v1.x * v2.z),
	(v1.x * v2.y) - (v1.y * v2.x) };
	return result;
}

// 距離（Vector3版）
float Distance(const Vector3& v1, const Vector3& v2) {
	float result = sqrtf(powf(v2.x - v1.x, 2) + powf(v2.y - v1.y, 2) + powf(v2.z - v1.z, 2));
	return result;
}

// 線形補間（Vector3版）
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return Vector3(v1.x + (v2.x - v1.x) * t, v1.y + (v2.y - v1.y) * t, v1.z + (v2.z - v1.z) * t);
}

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//

///																		///
///								行列
///																		///

///xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//



Matrix3x3 Matrix3x3Add(Matrix3x3 matrix1, Matrix3x3 matrix2) {
	Matrix3x3 result = { 0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = matrix1.m[i][j] + matrix2.m[i][j];
		}
	}
	return result;
};

Matrix3x3 Matrix3x3Subtract(Matrix3x3 matrix1, Matrix3x3 matrix2) {
	Matrix3x3 result = { 0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = matrix1.m[i][j] - matrix2.m[i][j];
		}
	}
	return result;
};

//行列の積
Matrix3x3 Matrix3x3Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2) {

	Matrix3x3 result = { 0 };

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = (matrix1.m[i][0] * matrix2.m[0][j]) + (matrix1.m[i][1] * matrix2.m[1][j]) + (matrix1.m[i][2] * matrix2.m[2][j]);
		}
	}

	return result;
};





//回転行列
Matrix3x3 Matrix3x3MakeRotateMatrix(float theta) {
	Matrix3x3 RotateMatrix = { 0 };

	RotateMatrix.m[0][0] = cosf(theta);
	RotateMatrix.m[0][1] = sinf(theta);
	RotateMatrix.m[0][2] = 0;
	RotateMatrix.m[1][0] = -sinf(theta);
	RotateMatrix.m[1][1] = cosf(theta);
	RotateMatrix.m[1][2] = 0;
	RotateMatrix.m[2][0] = 0;
	RotateMatrix.m[2][1] = 0;
	RotateMatrix.m[2][2] = 1;
	return RotateMatrix;

}

//平行移動行列
Matrix3x3 Matrix3x3MakeTranslateMatrix(Vector2 translate) {
	Matrix3x3 TranslateMatrix = { 0 };

	TranslateMatrix.m[0][0] = 1;
	TranslateMatrix.m[0][1] = 0;
	TranslateMatrix.m[0][2] = 0;
	TranslateMatrix.m[1][0] = 0;
	TranslateMatrix.m[1][1] = 1;
	TranslateMatrix.m[1][2] = 0;
	TranslateMatrix.m[2][0] = translate.x;
	TranslateMatrix.m[2][1] = translate.y;
	TranslateMatrix.m[2][2] = 1;

	return TranslateMatrix;
};

//拡大縮小行列
Matrix3x3 Matrix3x3MakeScaleMatrix(Vector2 scale) {
	Matrix3x3 ScaleMatrix = { 0 };

	ScaleMatrix.m[0][0] = scale.x;
	ScaleMatrix.m[0][1] = 0;
	ScaleMatrix.m[0][2] = 0;
	ScaleMatrix.m[1][0] = 0;
	ScaleMatrix.m[1][1] = scale.y;
	ScaleMatrix.m[1][2] = 0;
	ScaleMatrix.m[2][0] = 0;
	ScaleMatrix.m[2][1] = 0;
	ScaleMatrix.m[2][2] = 1;

	return ScaleMatrix;
};

//アフィン行列
Matrix3x3 Matrix3x3MakeAffineMatrix(Vector2 scale, float rotate, Vector2 translate) {
	Matrix3x3 AffineMatrix = { 0 };
	AffineMatrix.m[0][0] = scale.x * cosf(rotate);
	AffineMatrix.m[0][1] = scale.x * sinf(rotate);
	AffineMatrix.m[0][2] = 0;
	AffineMatrix.m[1][0] = scale.y * -sinf(rotate);
	AffineMatrix.m[1][1] = scale.y * cosf(rotate);
	AffineMatrix.m[1][2] = 0;
	AffineMatrix.m[2][0] = translate.x;
	AffineMatrix.m[2][1] = translate.y;
	AffineMatrix.m[2][2] = 1;

	return AffineMatrix;
};

//行列変換
Vector2 Matrix3x3Transform(Vector2 vector, Matrix3x3 matrix) {
	Vector2 result = { 0 };//w=1がデカルト座標系であるので(x,y,1)のベクトルとしてmatrixの積をとる
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + 1.0f * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + 1.0f * matrix.m[2][1];
	float w = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + 1.0f * matrix.m[2][2];
	assert(w != 0.0f);//bベクトルに対して基本的な操作を行う秒列ではｗ＝０にならない
	result.x /= w;
	result.y /= w;

	return result;

};

//3x3行列の逆行列を生成
Matrix3x3 Matrix3x3Inverse(Matrix3x3 matrix) {
	float scalar = 1 /
		((matrix.m[0][0] * matrix.m[1][1] * matrix.m[2][2]) +
			(matrix.m[0][1] * matrix.m[1][2] * matrix.m[2][0]) +

			(matrix.m[0][2] * matrix.m[1][0] * matrix.m[2][1]) -
			(matrix.m[0][2] * matrix.m[1][1] * matrix.m[2][0]) -

			(matrix.m[0][1] * matrix.m[1][0] * matrix.m[2][2]) -
			(matrix.m[0][0] * matrix.m[1][2] * matrix.m[2][1]));

	Matrix3x3 m1 = { 0 };
	m1.m[0][0] = matrix.m[1][1] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][1];
	m1.m[0][1] = -(matrix.m[0][1] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[2][1]);
	m1.m[0][2] = matrix.m[0][1] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][1];

	m1.m[1][0] = -(matrix.m[1][0] * matrix.m[2][2] - matrix.m[1][2] * matrix.m[2][0]);
	m1.m[1][1] = matrix.m[0][0] * matrix.m[2][2] - matrix.m[0][2] * matrix.m[2][0];
	m1.m[1][2] = -(matrix.m[0][0] * matrix.m[1][2] - matrix.m[0][2] * matrix.m[1][0]);

	m1.m[2][0] = matrix.m[1][0] * matrix.m[2][1] - matrix.m[1][1] * matrix.m[2][0];
	m1.m[2][1] = -(matrix.m[0][0] * matrix.m[2][1] - matrix.m[0][1] * matrix.m[2][0]);
	m1.m[2][2] = matrix.m[0][0] * matrix.m[1][1] - matrix.m[0][1] * matrix.m[1][0];


	Matrix3x3 result = { 0 };

	//行列のスカラー倍
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.m[i][j] = scalar * m1.m[i][j];
		}
	}

	return result;
};

//3x3転置行列を求める
Matrix3x3 Matrix3x3Transpose(Matrix3x3 matrix) {
	Matrix3x3 result = { 0 };
	//対称になるように変更
	result.m[0][1] = matrix.m[1][0];
	result.m[0][2] = matrix.m[2][0];
	result.m[1][0] = matrix.m[0][1];
	result.m[1][2] = matrix.m[2][1];
	result.m[2][1] = matrix.m[1][2];
	result.m[2][0] = matrix.m[0][2];

	//軸なので変わらない
	result.m[0][0] = matrix.m[0][0];
	result.m[1][1] = matrix.m[1][1];
	result.m[2][2] = matrix.m[2][2];

	return result;
};











/*-----------------------------------------------------------------------*/
//
//								4x4行列
//
/*-----------------------------------------------------------------------*/

Matrix4x4 Matrix4x4Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}


Matrix4x4 Matrix4x4Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = {
				(m1.m[i][0] * m2.m[0][j]) +
				(m1.m[i][1] * m2.m[1][j]) +
				(m1.m[i][2] * m2.m[2][j]) +
				(m1.m[i][3] * m2.m[3][j])
			};
		}
	}
	return result;

}

Matrix4x4 Matrix4x4Inverse(const Matrix4x4& m) {
	//|A|を求める
	float A = {
		1 /
		((m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]) +
		(m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]) +
		(m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2])

		- (m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]) -
			(m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]) -
			(m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2])

		- (m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]) -
			(m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]) -
			(m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2])

		+ (m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]) + (m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]) + (m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2])

		+ (m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]) + (m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]) + (m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2])

		- (m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]) - (m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]) - (m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2])

		- (m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]) - (m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]) - (m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0])

		+ (m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]) + (m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]) + (m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0])
			)
	};


	//00_02のP11　１/｜A｜の後ろを書く
	Matrix4x4 m1 = { 0 };
	m1.m[0][0] = (
		(m.m[1][1] * m.m[2][2] * m.m[3][3]) + (m.m[1][2] * m.m[2][3] * m.m[3][1]) + (m.m[1][3] * m.m[2][1] * m.m[3][2])
		- (m.m[1][3] * m.m[2][2] * m.m[3][1]) - (m.m[1][2] * m.m[2][1] * m.m[3][3]) - (m.m[1][1] * m.m[2][3] * m.m[3][2])
		);
	m1.m[0][1] = (
		-(m.m[0][1] * m.m[2][2] * m.m[3][3]) - (m.m[0][2] * m.m[2][3] * m.m[3][1]) - (m.m[0][3] * m.m[2][1] * m.m[3][2])
		+ (m.m[0][3] * m.m[2][2] * m.m[3][1]) + (m.m[0][2] * m.m[2][1] * m.m[3][3]) + (m.m[0][1] * m.m[2][3] * m.m[3][2])
		);
	m1.m[0][2] = (
		(m.m[0][1] * m.m[1][2] * m.m[3][3]) + (m.m[0][2] * m.m[1][3] * m.m[3][1]) + (m.m[0][3] * m.m[1][1] * m.m[3][2])
		- (m.m[0][3] * m.m[1][2] * m.m[3][1]) - (m.m[0][2] * m.m[1][1] * m.m[3][3]) - (m.m[0][1] * m.m[1][3] * m.m[3][2])
		);

	m1.m[0][3] = (
		-(m.m[0][1] * m.m[1][2] * m.m[2][3]) - (m.m[0][2] * m.m[1][3] * m.m[2][1]) - (m.m[0][3] * m.m[1][1] * m.m[2][2])
		+ (m.m[0][3] * m.m[1][2] * m.m[2][1]) + (m.m[0][2] * m.m[1][1] * m.m[2][3]) + (m.m[0][1] * m.m[1][3] * m.m[2][2])
		);


	m1.m[1][0] = (
		-(m.m[1][0] * m.m[2][2] * m.m[3][3]) - (m.m[1][2] * m.m[2][3] * m.m[3][0]) - (m.m[1][3] * m.m[2][0] * m.m[3][2])
		+ (m.m[1][3] * m.m[2][2] * m.m[3][0]) + (m.m[1][2] * m.m[2][0] * m.m[3][3]) + (m.m[1][0] * m.m[2][3] * m.m[3][2])
		);

	m1.m[1][1] = (
		(m.m[0][0] * m.m[2][2] * m.m[3][3]) + (m.m[0][2] * m.m[2][3] * m.m[3][0]) + (m.m[0][3] * m.m[2][0] * m.m[3][2])
		- (m.m[0][3] * m.m[2][2] * m.m[3][0]) - (m.m[0][2] * m.m[2][0] * m.m[3][3]) - (m.m[0][0] * m.m[2][3] * m.m[3][2])
		);

	m1.m[1][2] = (
		-(m.m[0][0] * m.m[1][2] * m.m[3][3]) - (m.m[0][2] * m.m[1][3] * m.m[3][0]) - (m.m[0][3] * m.m[1][0] * m.m[3][2])
		+ (m.m[0][3] * m.m[1][2] * m.m[3][0]) + (m.m[0][2] * m.m[1][0] * m.m[3][3]) + (m.m[0][0] * m.m[1][3] * m.m[3][2])
		);

	m1.m[1][3] = (
		(m.m[0][0] * m.m[1][2] * m.m[2][3]) + (m.m[0][2] * m.m[1][3] * m.m[2][0]) + (m.m[0][3] * m.m[1][0] * m.m[2][2])
		- (m.m[0][3] * m.m[1][2] * m.m[2][0]) - (m.m[0][2] * m.m[1][0] * m.m[2][3]) - (m.m[0][0] * m.m[1][3] * m.m[2][2])
		);


	m1.m[2][0] = (
		(m.m[1][0] * m.m[2][1] * m.m[3][3]) + (m.m[1][1] * m.m[2][3] * m.m[3][0]) + (m.m[1][3] * m.m[2][0] * m.m[3][1])
		- (m.m[1][3] * m.m[2][1] * m.m[3][0]) - (m.m[1][1] * m.m[2][0] * m.m[3][3]) - (m.m[1][0] * m.m[2][3] * m.m[3][1])
		);

	m1.m[2][1] = (
		-(m.m[0][0] * m.m[2][1] * m.m[3][3]) - (m.m[0][1] * m.m[2][3] * m.m[3][0]) - (m.m[0][3] * m.m[2][0] * m.m[3][1])
		+ (m.m[0][3] * m.m[2][1] * m.m[3][0]) + (m.m[0][1] * m.m[2][0] * m.m[3][3]) + (m.m[0][0] * m.m[2][3] * m.m[3][1])
		);

	m1.m[2][2] = (
		(m.m[0][0] * m.m[1][1] * m.m[3][3]) + (m.m[0][1] * m.m[1][3] * m.m[3][0]) + (m.m[0][3] * m.m[1][0] * m.m[3][1])
		- (m.m[0][3] * m.m[1][1] * m.m[3][0]) - (m.m[0][1] * m.m[1][0] * m.m[3][3]) - (m.m[0][0] * m.m[1][3] * m.m[3][1])
		);

	m1.m[2][3] = (
		-(m.m[0][0] * m.m[1][1] * m.m[2][3]) - (m.m[0][1] * m.m[1][3] * m.m[2][0]) - (m.m[0][3] * m.m[1][0] * m.m[2][1])
		+ (m.m[0][3] * m.m[1][1] * m.m[2][0]) + (m.m[0][1] * m.m[1][0] * m.m[2][3]) + (m.m[0][0] * m.m[1][3] * m.m[2][1])
		);


	m1.m[3][0] = (
		-(m.m[1][0] * m.m[2][1] * m.m[3][2]) - (m.m[1][1] * m.m[2][2] * m.m[3][0]) - (m.m[1][2] * m.m[2][0] * m.m[3][1])
		+ (m.m[1][2] * m.m[2][1] * m.m[3][0]) + (m.m[1][1] * m.m[2][0] * m.m[3][2]) + (m.m[1][0] * m.m[2][2] * m.m[3][1])
		);

	m1.m[3][1] = (
		(m.m[0][0] * m.m[2][1] * m.m[3][2]) + (m.m[0][1] * m.m[2][2] * m.m[3][0]) + (m.m[0][2] * m.m[2][0] * m.m[3][1])
		- (m.m[0][2] * m.m[2][1] * m.m[3][0]) - (m.m[0][1] * m.m[2][0] * m.m[3][2]) - (m.m[0][0] * m.m[2][2] * m.m[3][1])
		);

	m1.m[3][2] = (
		-(m.m[0][0] * m.m[1][1] * m.m[3][2]) - (m.m[0][1] * m.m[1][2] * m.m[3][0]) - (m.m[0][2] * m.m[1][0] * m.m[3][1])
		+ (m.m[0][2] * m.m[1][1] * m.m[3][0]) + (m.m[0][1] * m.m[1][0] * m.m[3][2]) + (m.m[0][0] * m.m[1][2] * m.m[3][1])
		);

	m1.m[3][3] = (
		(m.m[0][0] * m.m[1][1] * m.m[2][2]) + (m.m[0][1] * m.m[1][2] * m.m[2][0]) + (m.m[0][2] * m.m[1][0] * m.m[2][1])
		- (m.m[0][2] * m.m[1][1] * m.m[2][0]) - (m.m[0][1] * m.m[1][0] * m.m[2][2]) - (m.m[0][0] * m.m[1][2] * m.m[2][1])
		);


	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = A * m1.m[i][j];
		}
	}

	return result;


}

Matrix4x4 Matrix4x4Transpose(const Matrix4x4& m) {
	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			//上下反転させる
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;

}
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			//行と列が同じ時だけ１を入れる
			if (i == j) {
				result.m[i][j] = 1;
			}
		}
	}
	return result;
}



Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 TranslateMatrix = { 0 };
	TranslateMatrix.m[0][0] = 1;
	TranslateMatrix.m[0][1] = 0;
	TranslateMatrix.m[0][2] = 0;
	TranslateMatrix.m[0][3] = 0;

	TranslateMatrix.m[1][0] = 0;
	TranslateMatrix.m[1][1] = 1;
	TranslateMatrix.m[1][2] = 0;
	TranslateMatrix.m[1][3] = 0;

	TranslateMatrix.m[2][0] = 0;
	TranslateMatrix.m[2][1] = 0;
	TranslateMatrix.m[2][2] = 1;
	TranslateMatrix.m[2][3] = 0;

	TranslateMatrix.m[3][0] = translate.x;
	TranslateMatrix.m[3][1] = translate.y;
	TranslateMatrix.m[3][2] = translate.z;
	TranslateMatrix.m[3][3] = 1;

	return TranslateMatrix;
}

Matrix4x4 MakeScaleMatrix(const Vector3& Scale) {
	Matrix4x4 ScaleMatrix = { 0 };

	ScaleMatrix.m[0][0] = Scale.x;
	ScaleMatrix.m[0][1] = 0;
	ScaleMatrix.m[0][2] = 0;
	ScaleMatrix.m[0][3] = 0;

	ScaleMatrix.m[1][0] = 0;
	ScaleMatrix.m[1][1] = Scale.y;
	ScaleMatrix.m[1][2] = 0;
	ScaleMatrix.m[1][3] = 0;

	ScaleMatrix.m[2][0] = 0;
	ScaleMatrix.m[2][1] = 0;
	ScaleMatrix.m[2][2] = Scale.z;
	ScaleMatrix.m[2][3] = 0;

	ScaleMatrix.m[3][0] = 0;
	ScaleMatrix.m[3][1] = 0;
	ScaleMatrix.m[3][2] = 0;
	ScaleMatrix.m[3][3] = 1;

	return ScaleMatrix;

}

Vector3 Matrix4x4Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result = { 0 };

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];

	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}

	return result;
}


//1.X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 RotateXMatrix = { 0 };
	RotateXMatrix.m[0][0] = 1;
	RotateXMatrix.m[0][1] = 0;
	RotateXMatrix.m[0][2] = 0;
	RotateXMatrix.m[0][3] = 0;

	RotateXMatrix.m[1][0] = 0;
	RotateXMatrix.m[1][1] = std::cos(radian);
	RotateXMatrix.m[1][2] = std::sin(radian);
	RotateXMatrix.m[1][3] = 0;

	RotateXMatrix.m[2][0] = 0;
	RotateXMatrix.m[2][1] = -std::sin(radian);
	RotateXMatrix.m[2][2] = std::cos(radian);
	RotateXMatrix.m[2][3] = 0;

	RotateXMatrix.m[3][0] = 0;
	RotateXMatrix.m[3][1] = 0;
	RotateXMatrix.m[3][2] = 0;
	RotateXMatrix.m[3][3] = 1;
	return RotateXMatrix;
}
//2.Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 RotateYMatrix = { 0 };
	RotateYMatrix.m[0][0] = std::cos(radian);
	RotateYMatrix.m[0][1] = 0;
	RotateYMatrix.m[0][2] = -std::sin(radian);
	RotateYMatrix.m[0][3] = 0;

	RotateYMatrix.m[1][0] = 0;
	RotateYMatrix.m[1][1] = 1;
	RotateYMatrix.m[1][2] = 0;
	RotateYMatrix.m[1][3] = 0;

	RotateYMatrix.m[2][0] = std::sin(radian);
	RotateYMatrix.m[2][1] = 0;
	RotateYMatrix.m[2][2] = std::cos(radian);
	RotateYMatrix.m[2][3] = 0;

	RotateYMatrix.m[3][0] = 0;
	RotateYMatrix.m[3][1] = 0;
	RotateYMatrix.m[3][2] = 0;
	RotateYMatrix.m[3][3] = 1;
	return RotateYMatrix;
}

//3.Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 RotateZMatrix = { 0 };
	RotateZMatrix.m[0][0] = std::cos(radian);
	RotateZMatrix.m[0][1] = std::sin(radian);
	RotateZMatrix.m[0][2] = 0;
	RotateZMatrix.m[0][3] = 0;

	RotateZMatrix.m[1][0] = -std::sin(radian);
	RotateZMatrix.m[1][1] = std::cos(radian);
	RotateZMatrix.m[1][2] = 0;
	RotateZMatrix.m[1][3] = 0;

	RotateZMatrix.m[2][0] = 0;
	RotateZMatrix.m[2][1] = 0;
	RotateZMatrix.m[2][2] = 1;
	RotateZMatrix.m[2][3] = 0;

	RotateZMatrix.m[3][0] = 0;
	RotateZMatrix.m[3][1] = 0;
	RotateZMatrix.m[3][2] = 0;
	RotateZMatrix.m[3][3] = 1;
	return RotateZMatrix;
}

Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotate)
{
	Matrix4x4 roatateXYZMatrix = { 0 };

	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

	roatateXYZMatrix = Matrix4x4Multiply(rotateXMatrix, Matrix4x4Multiply(rotateYMatrix, rotateZMatrix));
	return roatateXYZMatrix;

}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Matrix4x4Multiply(rotateXMatrix, Matrix4x4Multiply(rotateYMatrix, rotateZMatrix));
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	Matrix4x4 worldMatrix = Matrix4x4Multiply(scaleMatrix, Matrix4x4Multiply(rotateXYZMatrix, translateMatrix));


	return worldMatrix;
}


Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 perspectiveFovMatrix = { 0 };

	perspectiveFovMatrix.m[0][0] = (1.0f / aspectRatio) * (1.0f / std::tan(fovY / 2.0f));
	perspectiveFovMatrix.m[0][1] = 0.0f;
	perspectiveFovMatrix.m[0][2] = 0.0f;
	perspectiveFovMatrix.m[0][3] = 0.0f;

	perspectiveFovMatrix.m[1][0] = 0.0f;
	perspectiveFovMatrix.m[1][1] = (1.0f / (std::tan(fovY / 2.0f)));
	perspectiveFovMatrix.m[1][2] = 0.0f;
	perspectiveFovMatrix.m[1][3] = 0.0f;

	perspectiveFovMatrix.m[2][0] = 0.0f;
	perspectiveFovMatrix.m[2][1] = 0.0f;
	perspectiveFovMatrix.m[2][2] = (farClip / (farClip - nearClip));
	perspectiveFovMatrix.m[2][3] = 1.0f;

	perspectiveFovMatrix.m[3][0] = 0.0f;
	perspectiveFovMatrix.m[3][1] = 0.0f;
	perspectiveFovMatrix.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	perspectiveFovMatrix.m[3][3] = 0.0f;

	return perspectiveFovMatrix;
}



Matrix4x4 MakeOrthograpicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 orthpgrapicMatrix = { 0 };

	orthpgrapicMatrix.m[0][0] = 2 / (right - left);
	orthpgrapicMatrix.m[0][1] = 0;
	orthpgrapicMatrix.m[0][2] = 0;
	orthpgrapicMatrix.m[0][3] = 0;

	orthpgrapicMatrix.m[1][0] = 0;
	orthpgrapicMatrix.m[1][1] = 2 / (top - bottom);
	orthpgrapicMatrix.m[1][2] = 0;
	orthpgrapicMatrix.m[1][3] = 0;

	orthpgrapicMatrix.m[2][0] = 0;
	orthpgrapicMatrix.m[2][1] = 0;
	orthpgrapicMatrix.m[2][2] = 1 / (farClip - nearClip);
	orthpgrapicMatrix.m[2][3] = 0;

	orthpgrapicMatrix.m[3][0] = (left + right) / (left - right);
	orthpgrapicMatrix.m[3][1] = (top + bottom) / (bottom - top);
	orthpgrapicMatrix.m[3][2] = nearClip / (nearClip - farClip);
	orthpgrapicMatrix.m[3][3] = 1;

	return orthpgrapicMatrix;

}


Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 viewportMatrix = { 0 };

	viewportMatrix.m[0][0] = width / 2;
	viewportMatrix.m[0][1] = 0;
	viewportMatrix.m[0][2] = 0;
	viewportMatrix.m[0][3] = 0;

	viewportMatrix.m[1][0] = 0;
	viewportMatrix.m[1][1] = -(height / 2);
	viewportMatrix.m[1][2] = 0;
	viewportMatrix.m[1][3] = 0;

	viewportMatrix.m[2][0] = 0;
	viewportMatrix.m[2][1] = 0;
	viewportMatrix.m[2][2] = maxDepth - minDepth;
	viewportMatrix.m[2][3] = 0;

	viewportMatrix.m[3][0] = left + (width / 2);
	viewportMatrix.m[3][1] = top + (height / 2);
	viewportMatrix.m[3][2] = minDepth;
	viewportMatrix.m[3][3] = 1;

	return viewportMatrix;
}


Matrix4x4 MakeViewProjectionMatrix(const Vector3Transform& cameraTransform, float aspectRatio) {

	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Matrix4x4Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f,
		aspectRatio, 0.1f, 100.0f);
	Matrix4x4 viewProjectionMatrix = Matrix4x4Multiply(viewMatrix, projectionMatrix);

	return viewProjectionMatrix;
}


Matrix4x4 MakeViewProjectionMatrixSprite() {

	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthograpicMatrix(0.0f, 0.0f, float(GraphicsConfig::kClientWidth), float(GraphicsConfig::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 viewProjectionMatrix = Matrix4x4Multiply(viewMatrix, projectionMatrix);

	return viewProjectionMatrix;
}

Vector3 TransformDirection(const Vector3& v, const Matrix4x4& m)
{
	Vector3 result;
	// 行列の回転・スケール部分（左上3x3）のみを使用
	// 平行移動成分（m[3][0], m[3][1], m[3][2]）は無視
	//カメラを向いている方向に移動させるために使用

	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2];
	return result;
}

