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
	//PivotRotate();

	if (input_->IsKeyTrigger(DIK_TAB)) {
		moveDebugCamera_ = !moveDebugCamera_;  //フラグ反転させる
	}


	///ビュー行列の更新
	UpdateMatrix();
}

void DebugCamera::SetDefaultCamera()
{
	//X,Y,Z軸周りのローカル回転角
	rotation_ = { 0,0,0 };
	//ローカル座標
	translation_ = { 0,0,-10.0f };
	Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1,1,1 }, rotation_, translation_);
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
	Matrix4x4 rotateMatrix = MakeRotateXYZMatrix(rotation_);

	Vector3 forward = TransformDirection({ 0, 0, 1 }, rotateMatrix);	//カメラの向きから見て前方ベクトル
	Vector3 right = TransformDirection({ 1, 0, 0 }, rotateMatrix);		//カメラの向きから見て右方ベクトル
	Vector3 up = TransformDirection({ 0, 1, 0 }, rotateMatrix);			//カメラの向きから見て上方ベクトル

	//移動ベクトルを初期化
	Vector3 move = { 0, 0, 0 };


	///*-----------------------------------------------------------------------*///
	///								キーボードで操作							   ///
	///*-----------------------------------------------------------------------*///
	//								SpaceでY軸移動								//
	if (InputManager::GetInstance()->IsKeyDown(DIK_SPACE)) {
		//					SPACE押されている場合はY軸移動						//

		if (InputManager::GetInstance()->IsKeyDown(DIK_W)) {
			move = Vector3Add(move, up);
		}

		if (InputManager::GetInstance()->IsKeyDown(DIK_S)) {
			move = Vector3Subtract(move, up);
		}

	} else {

		//							通常はXZ軸移動								//
		if (InputManager::GetInstance()->IsKeyDown(DIK_W)) {
			move = Vector3Add(move, forward);
		}
		if (InputManager::GetInstance()->IsKeyDown(DIK_S)) {
			move = Vector3Subtract(move, forward);
		}

		if (InputManager::GetInstance()->IsKeyDown(DIK_D)) {
			move = Vector3Add(move, right);
		}
		if (InputManager::GetInstance()->IsKeyDown(DIK_A)) {
			move = Vector3Subtract(move, right);
		}
	}
	///*-----------------------------------------------------------------------*///
	///							マウスホイールで奥に移動						   ///
	///*-----------------------------------------------------------------------*///

	///マウスホイールが動かされたときのみ
	if (InputManager::GetInstance()->GetMouseWheel()) {
		//マウスホイールの動きに応じて前後移動

		Vector3 wheelMove = Vector3Multiply(forward, wheelSpeed);	//前方方向に、ホイールの前後移動分をかける
		move = Vector3Add(move, wheelMove);							//現状のmoveに加算する。
	}

	// 正規化してスピードを掛ける
	if (Vector3Length(move) > 0.0f) {
		move = Vector3Normalize(move);
		move = Vector3Multiply(move, speed);
		translation_ = Vector3Add(translation_, move);
	}


}

void DebugCamera::Rotate() {

	///*-----------------------------------------------------------------------*///
	///							マウスホイールで回転							   ///
	///*-----------------------------------------------------------------------*///


	if (InputManager::GetInstance()->IsMouseButtonDown(0)) {
		//マウスを取得
		Vector2 mousePos = InputManager::GetInstance()->GetMousePosition();
		Vector2 preMousePos = InputManager::GetInstance()->GetPreMousePosition();

		//マウスの移動量を取得
		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};
		//カメラ回転に反映させる
		rotation_.x += mouseDelta.y * 0.001f;
		rotation_.y += mouseDelta.x * 0.001f;

		//rotation_.x = std::clamp(rotation_.x, -1.5f, 1.5f);
		//rotation_.y = std::clamp(rotation_.y, -1.5f, 1.5f);

	}
}


void DebugCamera::PivotRotate() {
	if (InputManager::GetInstance()->IsMouseButtonDown(1)) { // 右クリックでピボット回転
		// マウスの移動量を取得
		Vector2 mousePos = InputManager::GetInstance()->GetMousePosition();
		Vector2 preMousePos = InputManager::GetInstance()->GetPreMousePosition();

		Vector2 mouseDelta = {
			(mousePos.x - preMousePos.x),
			(mousePos.y - preMousePos.y)
		};

		// ピボット回転角を更新
		const float sensitivity = 0.005f;
		pivotRotation_.x += mouseDelta.y * sensitivity;
		pivotRotation_.y += mouseDelta.x * sensitivity;

		// 回転制限（必要に応じて）
		pivotRotation_.x = std::clamp(pivotRotation_.x, -1.5f, 1.5f);

		// ピボット点を中心とした球面座標でカメラ位置を計算
		float cosX = cos(pivotRotation_.x);
		float sinX = sin(pivotRotation_.x);
		float cosY = cos(pivotRotation_.y);
		float sinY = sin(pivotRotation_.y);

		// カメラの新しい位置を計算
		Vector3 offset = {
			pivotDistance_ * cosX * sinY,
			pivotDistance_ * sinX,
			pivotDistance_ * cosX * cosY
		};

		translation_ = Vector3Add(pivotPoint_, offset);

		// カメラをピボット点に向ける
		Vector3 forward = Vector3Normalize(Vector3Subtract(pivotPoint_, translation_));
		Vector3 up = { 0, 1, 0 };
		Vector3 right = Vector3Normalize(Cross(up, forward));
		up = Cross(forward, right);

		// 回転行列から回転角を計算（簡易版）
		rotation_.y = atan2(forward.x, forward.z);
		rotation_.x = asin(-forward.y);
		rotation_.z = 0; // ロール回転は0に固定
	}

	// マウスホイールで距離調整
	if (InputManager::GetInstance()->GetMouseWheel()) {
		const float wheelSpeed = InputManager::GetInstance()->GetMouseWheel() * 0.0005f;
		pivotDistance_ += wheelSpeed;
		pivotDistance_ = std::max(1.0f, pivotDistance_); // 最小距離を設定

		// 距離変更後のカメラ位置を再計算
		float cosX = cos(pivotRotation_.x);
		float sinX = sin(pivotRotation_.x);
		float cosY = cos(pivotRotation_.y);
		float sinY = sin(pivotRotation_.y);

		Vector3 offset = {
			pivotDistance_ * cosX * sinY,
			pivotDistance_ * sinX,
			pivotDistance_ * cosX * cosY
		};

		translation_ = Vector3Add(pivotPoint_, offset);
	}
}



void DebugCamera::ImGui()
{

	ImGui::Text("DubugCamera");

	// 現在の状態を表示
	ImGui::Text("MoveCamera: %s", moveDebugCamera_ ? "ON" : "OFF");

	// ボタンで切り替え
	if (ImGui::Button(moveDebugCamera_ ? "Move Camera: ON" : "Move Camera: OFF")) {
		moveDebugCamera_ = !moveDebugCamera_;
	}
	ImGui::SameLine();
}



void DebugCamera::UpdateMatrix()
{
	Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1,1,1 }, rotation_, translation_);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);

}
