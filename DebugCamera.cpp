#include "DebugCamera.h"

void DebugCamera::Initialize()
{
	SetDefaultCamera();
	input_ = InputManager::GetInstance();
}

void DebugCamera::Update()
{
	///入力によってcameraの移動回転
	if (moveDebugCamera_) {
		Move();
		Rotate();
	}

	if (input_->IsKeyTrigger(DIK_TAB)) {
		moveDebugCamera_ = !moveDebugCamera_;  //フラグ反転させる
	}

	///ビュー行列の更新
	UpdateMatrix();
}

void DebugCamera::SetDefaultCamera()
{
	// デフォルト値に設定
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	cameraTransform_.translate = { 0.0f, 0.0f, -10.0f };

	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);

}

void DebugCamera::Move()
{
	//移動速度を定めておく
	const float speed = 0.1f;
	const float wheelSpeed = InputManager::GetInstance()->GetMouseWheel() * 0.005f;

	// カメラのローカル軸を回転から生成
	Matrix4x4 rotateMatrix = MakeRotateXYZMatrix(cameraTransform_.rotate);

	Vector3 forward = TransformDirection({ 0, 0, 1 }, rotateMatrix);	//カメラの向きから見て前方ベクトル
	Vector3 right = TransformDirection({ 1, 0, 0 }, rotateMatrix);		//カメラの向きから見て右方ベクトル
	Vector3 up = TransformDirection({ 0, 1, 0 }, rotateMatrix);			//カメラの向きから見て上方ベクトル

	//移動ベクトルを初期化
	Vector3 move = { 0, 0, 0 };


	///*-----------------------------------------------------------------------*///
	///								キーボードで操作							   ///
	///*-----------------------------------------------------------------------*///
	//								SpaceでY軸移動								//
	if (input_->IsKeyDown(DIK_SPACE)) {
		//					SPACE押されている場合はY軸移動						//

		if (input_->IsKeyDown(DIK_W)) {
			move = Vector3Add(move, up);
		}

		if (input_->IsKeyDown(DIK_S)) {
			move = Vector3Subtract(move, up);
		}

	} else {

		//							通常はXZ軸移動								//
		if (input_->IsKeyDown(DIK_W)) {
			move = Vector3Add(move, forward);
		}
		if (input_->IsKeyDown(DIK_S)) {
			move = Vector3Subtract(move, forward);
		}

		if (input_->IsKeyDown(DIK_D)) {
			move = Vector3Add(move, right);
		}
		if (input_->IsKeyDown(DIK_A)) {
			move = Vector3Subtract(move, right);
		}
	}
	///*-----------------------------------------------------------------------*///
	///							マウスホイールで奥に移動						   ///
	///*-----------------------------------------------------------------------*///

	///マウスホイールが動かされたときのみ
	if (input_->GetMouseWheel()) {
		//マウスホイールの動きに応じて前後移動

		Vector3 wheelMove = Vector3Multiply(forward, wheelSpeed);	//前方方向に、ホイールの前後移動分をかける
		move = Vector3Add(move, wheelMove);							//現状のmoveに加算する。
	}

	// 正規化してスピードを掛ける
	if (Vector3Length(move) > 0.0f) {
		move = Vector3Normalize(move);
		move = Vector3Multiply(move, speed);
		cameraTransform_.translate = Vector3Add(cameraTransform_.translate, move);
	}


}

void DebugCamera::Rotate() {

	///*-----------------------------------------------------------------------*///
	///							マウスホイールで回転							   ///
	///*-----------------------------------------------------------------------*///


	if (input_->IsMouseButtonDown(0)) {
		//マウスを取得
		Vector2 mousePos = input_->GetMousePosition();
		Vector2 preMousePos = input_->GetPreMousePosition();

		//マウスの移動量を取得
		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};
		//カメラ回転に反映させる
		cameraTransform_.rotate.x += mouseDelta.y * 0.001f;
		cameraTransform_.rotate.y += mouseDelta.x * 0.001f;
	}
}




void DebugCamera::ImGui()
{

	ImGui::Text("DubugCamera");
	ImGui::Separator(); // 区切り線

	ImGui::Text("CameraTransform");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", cameraTransform_.translate.x, cameraTransform_.translate.y, cameraTransform_.translate.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", cameraTransform_.rotate.x, cameraTransform_.rotate.y, cameraTransform_.rotate.z);

	ImGui::Separator(); // 区切り線
	// 現在の状態を表示
	ImGui::Text("MoveCamera: %s", moveDebugCamera_ ? "ON" : "OFF");

	// ボタンで切り替え
	if (ImGui::Button(moveDebugCamera_ ? "Move Camera: ON" : "Move Camera: OFF")) {
		moveDebugCamera_ = !moveDebugCamera_;
	}

	ImGui::SameLine();//横並びに設置できる

	if (ImGui::Button("Reset DebugCamera")) {
		SetDefaultCamera();

	}
	ImGui::Separator(); // 区切り線

}



void DebugCamera::UpdateMatrix()
{
	Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1,1,1 }, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);

}
