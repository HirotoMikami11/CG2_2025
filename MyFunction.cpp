#include "MyFunction.h"

void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix,Matrix4x4* matrixData) {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix = Matrix4x4Multiply(worldMatrix, viewProjectionMatrix);

	*matrixData = worldViewProjectionMatrix;
}

