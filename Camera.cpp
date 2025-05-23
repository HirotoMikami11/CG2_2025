#include "Camera.h"

void Camera::Initialize()
{
	//デフォルトの設定で初期化
	SetDefaultCamera();

}

void Camera::Update()
{
	// カメラの行列を更新
	UpdateMatrix();
	// スプライト用の行列を更新
	UpdateSpriteMatrix();

}

void Camera::UpdateMatrix()
{
	// 3D用のビュープロジェクション行列を計算
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);

}


void Camera::UpdateSpriteMatrix()
{
	///フラグで使うと判断したときだけスプライトの行列を計算する
	if (useSpriteViewProjectionMatrix_) {
		// スプライト用のビュープロジェクション行列を計算
		Matrix4x4 viewMatrix = MakeIdentity4x4();
		spriteViewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix, spriteProjectionMatrix_);
	}
}

void Camera::SetDefaultCamera()
{
	// デフォルト値に設定
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	cameraTransform_.translate = { 0.0f, 0.0f, -10.0f };

	// カメラパラメータのデフォルト値
	fov_ = 0.45f;
	nearClip_ = 0.1f;
	farClip_ = 100.0f;
	aspectRatio = (float(kClientWidth) / float(kClientHeight));

	//プロジェクション行列は最初に作っておく
	projectionMatrix_ = MakePerspectiveFovMatrix(fov_, aspectRatio, nearClip_, farClip_);
	spriteProjectionMatrix_ = MakeOrthograpicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);

	// 行列を単位行列で初期化
	viewProjectionMatrix_ = MakeIdentity4x4();
	spriteViewProjectionMatrix_ = MakeIdentity4x4();

	// スプライトを画面に表示できるように初期化
	useSpriteViewProjectionMatrix_ = true;

}