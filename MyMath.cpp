#include "MyMath.h"


/*-----------------------------------------------------------------------*/
//
//								2次元ベクトル
//
/*-----------------------------------------------------------------------*/








/*-----------------------------------------------------------------------*/
//
//								3次元ベクトル
//
/*-----------------------------------------------------------------------*/

Vector3 Vector3Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
		v1.x + v2.x,
		v1.y + v2.y,
		v1.z + v2.z
	};
	return result;
}

Vector3 Vector3Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
	v1.x - v2.x,
	v1.y - v2.y,
	v1.z - v2.z
	};
	return result;
}

Vector3 Vector3Multiply(const Vector3& v, float scalar) {
	Vector3 result = {
	v.x * scalar,
	v.y * scalar,
	v.z * scalar
	};
	return result;
}

float Vector3Dot(const Vector3& v1, const Vector3& v2) {
	float result =
		v1.x * v2.x +
		v1.y * v2.y +
		v1.z * v2.z;

	return result;
}

float Vector3Length(const Vector3& v) {
	float result = sqrtf(
		v.x * v.x +
		v.y * v.y +
		v.z * v.z
	);
	return result;
}

Vector3 Vector3Normalize(const Vector3& v) {
	Vector3 result = { 0,0,0 };
	float length = Vector3Length(v);

	if (length != 0) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}

	return result;
}


//クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
	(v1.y * v2.z) - (v1.z * v2.y),
	(v1.z * v2.x) - (v1.x * v2.z),
	(v1.x * v2.y) - (v1.y * v2.x) };
	return result;
}


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
	Matrix4x4 result = { 0 };
	//|A|の部分の計算
	float A = {
		1 / (
			(m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]) + (m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]) + (m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2])
			- (m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]) - (m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]) - (m.m[0][3] * m.m[1][1] * m.m[2][3] * m.m[3][2])
			- (m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]) - (m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]) - (m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2])
			+ (m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]) + (m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]) + (m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2])
			+ (m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]) + (m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]) + (m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1])
			- (m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][3]) - (m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][1]) - (m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2])
			- (m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]) - (m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]) - (m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0])
			+ (m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]) + (m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]) + (m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0])
			)
	};

	Matrix4x4 m1 = { 0 };

	m1.m[0][0] = (
		(m.m[1][1] * m.m[2][2] * m.m[3][3])
		+ (m.m[1][2] * m.m[2][3] * m.m[3][1])
		+ (m.m[1][3] * m.m[2][1] * m.m[3][2])
		- (m.m[1][3] * m.m[2][2] * m.m[3][1])
		- (m.m[1][2] * m.m[2][1] * m.m[3][2])
		- (m.m[1][1] * m.m[2][3] * m.m[3][2]));
	m1.m[0][1] = (
		-(m.m[0][1] * m.m[2][2] * m.m[3][3])
		- (m.m[0][2] * m.m[2][3] * m.m[3][1])
		- (m.m[0][3] * m.m[2][1] * m.m[3][2])

		+ (m.m[0][3] * m.m[2][2] * m.m[3][1])
		+ (m.m[0][2] * m.m[2][1] * m.m[3][3])
		+ (m.m[0][1] * m.m[2][3] * m.m[3][2])
		);

	m1.m[0][2] = (
		(m.m[0][1] * m.m[1][2] * m.m[3][3])
		+ (m.m[0][2] * m.m[1][3] * m.m[3][1])
		+ (m.m[0][3] * m.m[1][1] * m.m[3][2])
		- (m.m[0][3] * m.m[1][2] * m.m[3][1])
		- (m.m[0][2] * m.m[1][1] * m.m[3][3])
		- (m.m[0][1] * m.m[1][3] * m.m[3][2])
		);

	m1.m[0][3] = (
		-(m.m[0][1] * m.m[1][2] * m.m[2][3])
		- (m.m[0][2] * m.m[1][3] * m.m[2][1])
		- (m.m[0][3] * m.m[1][1] * m.m[2][2])
		+ (m.m[0][3] * m.m[1][2] * m.m[2][1])
		+ (m.m[0][2] * m.m[1][1] * m.m[2][3])
		+ (m.m[0][1] * m.m[1][3] * m.m[2][2])
		);


	m1.m[1][0] = (
		-(m.m[1][0] * m.m[2][2] * m.m[3][3])
		- (m.m[1][2] * m.m[2][3] * m.m[3][0])
		- (m.m[1][3] * m.m[2][0] * m.m[3][2])
		+ (m.m[1][3] * m.m[2][2] * m.m[3][0])
		+ (m.m[1][2] * m.m[2][0] * m.m[3][3])
		+ (m.m[1][0] * m.m[2][3] * m.m[3][2])
		);
	m1.m[1][1] = (
		(m.m[0][0] * m.m[2][2] * m.m[3][3])
		+ (m.m[0][2] * m.m[2][3] * m.m[3][0])
		+ (m.m[0][3] * m.m[2][0] * m.m[3][2])
		- (m.m[0][3] * m.m[2][2] * m.m[3][0])
		- (m.m[0][2] * m.m[2][0] * m.m[3][3])
		- (m.m[0][0] * m.m[2][3] * m.m[3][2])
		);

	m1.m[1][2] = (
		-(m.m[0][0] * m.m[1][2] * m.m[3][3])
		- (m.m[0][2] * m.m[1][3] * m.m[3][0])
		- (m.m[0][3] * m.m[1][0] * m.m[3][2])
		+ (m.m[0][3] * m.m[1][2] * m.m[3][0])
		+ (m.m[0][2] * m.m[1][0] * m.m[3][3])
		+ (m.m[0][0] * m.m[1][3] * m.m[3][2])
		);

	m1.m[1][3] = (
		(m.m[0][0] * m.m[1][2] * m.m[2][3])
		+ (m.m[0][2] * m.m[1][3] * m.m[2][0])
		+ (m.m[0][3] * m.m[1][0] * m.m[2][2])
		- (m.m[0][3] * m.m[1][2] * m.m[2][0])
		- (m.m[0][2] * m.m[1][0] * m.m[2][3])
		- (m.m[0][0] * m.m[1][3] * m.m[2][2])
		);


	m1.m[2][0] = (
		(m.m[1][0] * m.m[2][1] * m.m[3][3])
		+ (m.m[1][1] * m.m[2][3] * m.m[3][0])
		+ (m.m[1][3] * m.m[2][0] * m.m[3][1])
		- (m.m[1][3] * m.m[2][1] * m.m[3][0])
		- (m.m[1][1] * m.m[2][0] * m.m[3][3])
		- (m.m[1][0] * m.m[2][3] * m.m[3][1])
		);

	m1.m[2][1] = (
		-(m.m[0][0] * m.m[2][1] * m.m[3][3])
		- (m.m[0][1] * m.m[2][3] * m.m[3][0])
		- (m.m[0][3] * m.m[2][0] * m.m[3][1])
		+ (m.m[0][3] * m.m[2][1] * m.m[3][0])
		+ (m.m[0][1] * m.m[2][0] * m.m[3][3])
		+ (m.m[0][0] * m.m[2][3] * m.m[3][1])
		);

	m1.m[2][2] = (
		(m.m[0][0] * m.m[1][1] * m.m[3][3])
		+ (m.m[0][1] * m.m[1][3] * m.m[3][0])
		+ (m.m[0][3] * m.m[1][0] * m.m[3][1])
		- (m.m[0][3] * m.m[1][1] * m.m[3][0])
		- (m.m[0][1] * m.m[1][0] * m.m[3][3])
		- (m.m[0][0] * m.m[1][3] * m.m[3][1])
		);

	m1.m[2][3] = (
		-(m.m[0][0] * m.m[1][1] * m.m[2][3])
		- (m.m[0][1] * m.m[1][3] * m.m[2][0])
		- (m.m[0][3] * m.m[1][0] * m.m[2][1])
		+ (m.m[0][3] * m.m[1][1] * m.m[2][0])
		+ (m.m[0][1] * m.m[1][0] * m.m[2][3])
		+ (m.m[0][0] * m.m[1][3] * m.m[2][1])
		);


	m1.m[3][0] = (
		-(m.m[1][0] * m.m[2][1] * m.m[3][2])
		- (m.m[1][1] * m.m[2][2] * m.m[3][0])
		- (m.m[1][2] * m.m[2][0] * m.m[3][1])
		+ (m.m[1][2] * m.m[2][1] * m.m[3][0])
		+ (m.m[1][1] * m.m[2][0] * m.m[3][2])
		+ (m.m[1][0] * m.m[2][2] * m.m[3][1])
		);

	m1.m[3][1] = (
		(m.m[0][0] * m.m[2][1] * m.m[3][2])
		+ (m.m[0][1] * m.m[2][2] * m.m[3][0])
		+ (m.m[0][2] * m.m[2][0] * m.m[3][1])
		- (m.m[0][2] * m.m[2][1] * m.m[3][0])
		- (m.m[0][1] * m.m[2][0] * m.m[3][2])
		- (m.m[0][0] * m.m[2][2] * m.m[3][1])
		);

	m1.m[3][2] = (
		-(m.m[0][0] * m.m[1][1] * m.m[3][2])
		- (m.m[0][1] * m.m[1][2] * m.m[3][0])
		- (m.m[0][2] * m.m[1][0] * m.m[3][1])
		+ (m.m[0][2] * m.m[1][1] * m.m[3][0])
		+ (m.m[0][1] * m.m[1][0] * m.m[3][2])
		+ (m.m[0][0] * m.m[1][2] * m.m[3][1])
		);

	m1.m[3][3] = (
		(m.m[0][0] * m.m[1][1] * m.m[2][2])
		+ (m.m[0][1] * m.m[1][2] * m.m[2][0])
		+ (m.m[0][2] * m.m[1][0] * m.m[2][1])
		- (m.m[0][2] * m.m[1][1] * m.m[2][0])
		- (m.m[0][1] * m.m[1][0] * m.m[2][2])
		- (m.m[0][0] * m.m[1][2] * m.m[2][1])
		);

	//A*m1をする（行列のスカラー倍）
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

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
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

	perspectiveFovMatrix.m[0][0] = (1 / aspectRatio) * (1 / std::tan(fovY / 2));
	perspectiveFovMatrix.m[0][1] = 0;
	perspectiveFovMatrix.m[0][2] = 0;
	perspectiveFovMatrix.m[0][3] = 0;

	perspectiveFovMatrix.m[1][0] = 0;
	perspectiveFovMatrix.m[1][1] = (1 / (std::tan(fovY / 2)));
	perspectiveFovMatrix.m[1][2] = 0;
	perspectiveFovMatrix.m[1][3] = 0;

	perspectiveFovMatrix.m[2][0] = 0;
	perspectiveFovMatrix.m[2][1] = 0;
	perspectiveFovMatrix.m[2][2] = (farClip / (farClip - nearClip));
	perspectiveFovMatrix.m[2][3] = 1;

	perspectiveFovMatrix.m[3][0] = 0;
	perspectiveFovMatrix.m[3][1] = 0;
	perspectiveFovMatrix.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	perspectiveFovMatrix.m[3][3] = 0;

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
	Matrix4x4 projectionMatrix = MakeOrthograpicMatrix(0.0f,0.0f,float(kClientWidth),float(kClientHeight),0.0f,100.0f);
	Matrix4x4 viewProjectionMatrix = Matrix4x4Multiply(viewMatrix, projectionMatrix);

	return viewProjectionMatrix;
}