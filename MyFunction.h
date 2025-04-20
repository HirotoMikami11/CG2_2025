#pragma once
#include "MyMath.h"
#include "Easing.h"


void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, Matrix4x4* matrixData);
void SetVertexDataSpriteSquare(VertexData* vertexDataSprite,Vector2 CenterPosition,Vector2 size);

float Lerp(const float& min, const float& max, float t); 
float  Clamp(float& value, float min, float max);
