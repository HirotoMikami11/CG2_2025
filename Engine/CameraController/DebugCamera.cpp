#include "CameraController/DebugCamera.h"

void DebugCamera::Initialize()
{
	input_ = InputManager::GetInstance();

	SetDefaultCamera();

	// 累積回転行列の初期化
	matRot_ = MakeIdentity4x4();
	distance_ = Length(Subtract(cameraTransform_.translate, target_));

	//ピボット回転用のメンバ変数
	target_ = { 0.0f, 0.0f, 0.0f };	// ピボットの中心座標
	matRot_ = MakeIdentity4x4();	// 累積回転行列
	distance_ = 10.0f;				// ターゲットからの距離

	//移動用
	speed = 0.1f;
	wheelSpeed = input_->GetMouseWheel() * 0.005f;

	//移動ベクトル
	move = { 0, 0, 0 };
}

void DebugCamera::Update()
{
	///入力によってcameraの移動回転
	if (moveDebugCamera_) {
		if (rotationMode_ == CameraRotationMode::Default) {
			// オイラー角モード（累積回転行列を使用）
			Move();
			Rotate();
		} else if (rotationMode_ == CameraRotationMode::Pivot) {
			// ピボット回転モード
			PivotMove();
			PivotRotate();
		}
	}

	// TABキーでカメラ移動の切り替え
	if (input_->IsKeyTrigger(DIK_TAB)&&
		!input_->IsKeyDown(DIK_LSHIFT)) {

		moveDebugCamera_ = !moveDebugCamera_;  //フラグ反転させる
	}
	//Rキーで回転モードを切り替え
	if (input_->IsKeyTrigger(DIK_R)) {
		ChangeRotationMode();
	}
	///ビュー行列の更新
	if (rotationMode_ == CameraRotationMode::Default) {
		UpdateMatrix();
	} else {
		UpdatePivotMatrix();
	}
}

void DebugCamera::SetDefaultCamera()
{
	// デフォルト値に設定
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	cameraTransform_.translate = { 0.0f, 0.0f, -10.0f };

	// ピボット回転用もリセット
	target_ = { 0.0f, 0.0f, 0.0f };
	matRot_ = MakeIdentity4x4();// 累積回転行列もリセット
	distance_ = 10.0f;

	// 初期ビュー行列とプロジェクション行列を設定
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

void DebugCamera::Move()
{
	//移動速度を定めておく
	float speed = 0.1f;
	float wheelSpeed = input_->GetMouseWheel() * 0.005f;

	// カメラのローカル軸を累積回転行列から生成
	Vector3 forward = TransformDirection({ 0, 0, 1 }, matRot_);		//カメラの向きから見て前方ベクトル
	Vector3 right = TransformDirection({ 1, 0, 0 }, matRot_);		//カメラの向きから見て右方ベクトル
	Vector3 up = TransformDirection({ 0, 1, 0 }, matRot_);			//カメラの向きから見て上方ベクトル

	//移動ベクトルを初期化
	Vector3 move = { 0, 0, 0 };

	///*-----------------------------------------------------------------------*///
	///								キーボードで操作								///
	///*-----------------------------------------------------------------------*///
	//								SpaceでY軸移動								//
	if (input_->IsKeyDown(DIK_SPACE)) {
		//					SPACE押されている場合はY軸移動							//
		if (input_->IsKeyDown(DIK_W)) {
			move = Add(move, up);
		}
		if (input_->IsKeyDown(DIK_S)) {
			move = Subtract(move, up);
		}
	} else {
		//							通常はXZ軸移動								//
		if (input_->IsKeyDown(DIK_W)) {
			move = Add(move, forward);
		}
		if (input_->IsKeyDown(DIK_S)) {
			move = Subtract(move, forward);
		}
		if (input_->IsKeyDown(DIK_D)) {
			move = Add(move, right);
		}
		if (input_->IsKeyDown(DIK_A)) {
			move = Subtract(move, right);
		}
	}

	///*-----------------------------------------------------------------------*///
	///							マウスホイールで奥に移動							///
	///*-----------------------------------------------------------------------*///

	///マウスホイールが動かされたときのみ
	if (input_->GetMouseWheel()) {
		//マウスホイールの動きに応じて前後移動
		Vector3 wheelMove = Multiply(forward, wheelSpeed);	//前方方向に、ホイールの前後移動分をかける
		move = Add(move, wheelMove);						//現状のmoveに加算する。
	}

	// 正規化してスピードを掛ける
	if (Length(move) > 0.0f) {
		move = Normalize(move);
		move = Multiply(move, speed);
		cameraTransform_.translate = Add(cameraTransform_.translate, move);
	}
}

void DebugCamera::Rotate() {
	///*-----------------------------------------------------------------------*///
	///							マウスで回転（累積回転行列使用）					///
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

		// 今回の回転行列を作成（deltamatRot）
		float deltaYaw = mouseDelta.x * 0.001f;   // Y軸回転
		float deltaPitch = mouseDelta.y * 0.001f; // X軸回転

		// 今回の回転行列を計算
		Matrix4x4 deltaRotY = MakeRotateYMatrix(deltaYaw);
		Matrix4x4 deltaRotX = MakeRotateXMatrix(deltaPitch);
		Matrix4x4 deltamatRot = Matrix4x4Multiply(deltaRotY, deltaRotX);

		// 累積回転行列に今回の回転を合成
		matRot_ = Matrix4x4Multiply(deltamatRot, matRot_);

		/// カメラが傾かないように計算

		// カメラの向きから前方ベクトルを取得
		Vector3 forward = TransformDirection({ 0, 0, 1 }, matRot_);
		Vector3 worldUp = { 0, 1, 0 };

		Vector3 right = Normalize(Cross(worldUp, forward));
		Vector3 up = Cross(forward, right);

		// 回転行列を再構築
		matRot_.m[0][0] = right.x;		matRot_.m[0][1] = right.y;		matRot_.m[0][2] = right.z;
		matRot_.m[1][0] = up.x;			matRot_.m[1][1] = up.y;			matRot_.m[1][2] = up.z;
		matRot_.m[2][0] = forward.x;	matRot_.m[2][1] = forward.y;	matRot_.m[2][2] = forward.z;

		//TODO: 現状カメラのZ軸回転を無効化するだけのコードなので、ピボット回転から変更するとZ軸回転がリセットされる。

	}
}

void DebugCamera::PivotMove()
{
	float speed = 0.1f;
	float wheelSpeed = input_->GetMouseWheel() * 0.005f;

	//移動ベクトルを初期化
	Vector3 move = { 0, 0, 0 };

	//								SpaceでY軸移動								//
	if (input_->IsKeyDown(DIK_SPACE)) {
		//					SPACE押されている場合はY軸移動							//
		if (input_->IsKeyDown(DIK_W)) {
			move.y += 1.0f;
		}
		if (input_->IsKeyDown(DIK_S)) {
			move.y -= 1.0f;
		}
	} else {
		//							通常はXZ軸移動								//
		if (input_->IsKeyDown(DIK_W)) {
			move.z += 1.0f;
		}
		if (input_->IsKeyDown(DIK_S)) {
			move.z -= 1.0f;
		}
		if (input_->IsKeyDown(DIK_D)) {
			move.x += 1.0f;
		}
		if (input_->IsKeyDown(DIK_A)) {
			move.x -= 1.0f;
		}
	}

	// 正規化してスピードを掛ける
	if (Length(move) > 0.0f) {
		move = Normalize(move);
		move = Multiply(move, speed);
		target_ = Add(target_, move);
	}

	// マウスホイールで距離調整
	if (input_->GetMouseWheel()) {
		distance_ += wheelSpeed;
		// 距離の制限
		distance_ = std::clamp(distance_, 1.0f, 100.0f);  // 距離制限
	}
}

void DebugCamera::PivotRotate()
{
	///*-----------------------------------------------------------------------*///
	///							マウスでピボット回転								///
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

		// 今回の回転行列を作成（deltamatRot）
		float deltaYaw = mouseDelta.x * 0.002f;   // Y軸回転（水平方向）
		float deltaPitch = mouseDelta.y * 0.002f; // X軸回転（垂直方向）

		// 今回の回転行列を計算
		Matrix4x4 deltaRotY = MakeRotateYMatrix(deltaYaw);
		Matrix4x4 deltaRotX = MakeRotateXMatrix(deltaPitch);

		// 今回の回転行列を合成（Y軸回転 * X軸回転）
		Matrix4x4 deltamatRot = Matrix4x4Multiply(deltaRotY, deltaRotX);

		// 累積回転行列に今回の回転を合成
		matRot_ = Matrix4x4Multiply(deltamatRot, matRot_);
	}
}

void DebugCamera::ImGui()
{
#ifdef _DEBUG
	ImGui::Text("DebugCamera");
	ImGui::Separator();		//区切り線

	// 現在のモード表示
	ImGui::Text("Mode: %s", rotationMode_ == CameraRotationMode::Default ? "Free Camera" : "pivot Camera");

	//カメラモードの切り替えボタン
	if (ImGui::Button(rotationMode_ == CameraRotationMode::Default ? "Switch to pivot" : "Switch to Free")) {
		ChangeRotationMode();
	}

	ImGui::Separator();		//区切り線

	// モード別のパラメータ表示
	if (rotationMode_ == CameraRotationMode::Default) {
		ImGui::Text("Free Camera Parameters");
		ImGui::Text("Position: (%.2f, %.2f, %.2f)",
			cameraTransform_.translate.x, cameraTransform_.translate.y, cameraTransform_.translate.z);
	} else {
		ImGui::Text("pivot Camera Parameters");
		ImGui::Text("Target: (%.2f, %.2f, %.2f)", target_.x, target_.y, target_.z);
		ImGui::Text("Distance: %.2f", distance_);

		// パラメータ調整UI
		ImGui::SliderFloat3("Target Position", &target_.x, -20.0f, 20.0f);
		ImGui::SliderFloat("Distance", &distance_, 1.0f, 50.0f);
	}

	ImGui::Separator();		//区切り線

	// カメラを動かすかどうか
	ImGui::Text("MoveCamera: %s", moveDebugCamera_ ? "ON" : "OFF");
	if (ImGui::Button(moveDebugCamera_ ? "Move Camera: ON" : "Move Camera : OFF")) {
		moveDebugCamera_ = !moveDebugCamera_;
	}

	ImGui::SameLine();		//区切り線

	// カメラリセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera();
	}

	ImGui::Separator();		//区切り線

	// 操作説明
	ImGui::Text("Controls:");
	ImGui::Text("TAB: Switch camera control");
	ImGui::Text("R: Switch camera mode");

	if (rotationMode_ == CameraRotationMode::Default) {
		ImGui::Text("Free Camera:");
		ImGui::Text("  WASD: Move camera");
		ImGui::Text("  Mouse drag: Rotate view");
		ImGui::Text("  Mouse wheel: Forward/backward");
		ImGui::Text("  SPACE+WS: Vertical movement");
	} else {
		ImGui::Text("pivot Camera:");
		ImGui::Text("  WASD: Move target");
		ImGui::Text("  Mouse drag: Rotate around target");
		ImGui::Text("  Mouse wheel: Change distance");
		ImGui::Text("  SPACE+WS: Vertical movement");
	}
#endif
}

void DebugCamera::UpdateMatrix()
{
	// 累積回転行列を使用してカメラのワールド行列を作成
	Matrix4x4 translateMatrix = MakeTranslateMatrix(cameraTransform_.translate);
	Matrix4x4 cameraMatrix = Matrix4x4Multiply(matRot_, translateMatrix);

	// ビュー行列はワールド行列の逆行列
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);

	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

void DebugCamera::UpdatePivotMatrix()
{
	// ターゲットからカメラへの相対位置ベクトル（初期状態：後ろにdistance_分離れた位置）
	Vector3 relativePos = { 0.0f, 0.0f, -distance_ };

	// 累積回転行列でカメラの相対位置を回転
	relativePos = TransformDirection(relativePos, matRot_);

	// ターゲット座標に回転後の相対位置を加算してカメラの最終位置を決定
	Vector3 cameraPos = Add(target_, relativePos);

	// 平行移動行列を作成
	Matrix4x4 translateMatrix = MakeTranslateMatrix(cameraPos);

	// ワールド行列を作成（回転行列 * 平行移動行列）
	Matrix4x4 cameraMatrix = Matrix4x4Multiply(matRot_, translateMatrix);

	// ビュー行列はワールド行列の逆行列
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix);

	// プロジェクション行列を更新
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight)), 0.1f, 100.0f);

	// ビュープロジェクション行列を計算
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

void DebugCamera::ChangeRotationMode()
{
	if (rotationMode_ == CameraRotationMode::Default) {
		// 通常回転からピボット回転への切り替え
		rotationMode_ = CameraRotationMode::Pivot;

		// 現在のカメラ位置の先にターゲットを計算
		Vector3 forward = TransformDirection({ 0, 0, 1 }, matRot_);		// カメラの前方向を定める
		distance_ = 10.0f;												// デフォルト距離
		//ピボットポイント(target)の位置を決める
		target_ = Add(cameraTransform_.translate, Multiply(forward, distance_));

	} else {
		// ピボット回転から通常回転への切り替え
		rotationMode_ = CameraRotationMode::Default;

		// 現在のピボット回転で計算されているカメラ位置を取得
		Vector3 relativePos = { 0.0f, 0.0f, -distance_ };
		relativePos = TransformDirection(relativePos, matRot_);

		//ピボットの位置からカメラの位置を逆算しそこを通常回転のカメラ座標にする
		cameraTransform_.translate = Add(target_, relativePos);

	}
}