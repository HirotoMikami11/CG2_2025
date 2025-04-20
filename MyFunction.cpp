#include "MyFunction.h"

void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, Matrix4x4* matrixData) {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix = Matrix4x4Multiply(worldMatrix, viewProjectionMatrix);

	*matrixData = worldViewProjectionMatrix;
}

void SetVertexDataSpriteSquare(VertexData* vertexDataSprite, Vector2 CenterPosition, Vector2 size)
{
	//左下
	vertexDataSprite[0].position = { CenterPosition.x - size.x,CenterPosition.y + size.y,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//左上
	vertexDataSprite[1].position = { CenterPosition.x - size.x,CenterPosition.y - size.y,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//右下
	vertexDataSprite[2].position = { CenterPosition.x + size.x ,CenterPosition.y + size.y,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };

	//二つ目の三角形
	//左上
	vertexDataSprite[3].position = vertexDataSprite[1].position;
	vertexDataSprite[3].texcoord = vertexDataSprite[1].texcoord;
	//右上
	vertexDataSprite[4].position = { CenterPosition.x + size.x,CenterPosition.y - size.y,0.0f,1.0f };
	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	//右下
	vertexDataSprite[5].position = vertexDataSprite[2].position;
	vertexDataSprite[5].texcoord = vertexDataSprite[2].texcoord;

}

float Lerp(const float& min, const float& max, float t) {
	return min + (max - min) * t;
}

float  Clamp(float& value, float min, float max) {
	// valueが範囲外の場合、範囲内に収める
	if (value < min) {
		return min;
	} else if (value > max) {
		return max;
	} else {
		return value;
	}
}

