#include "RailCamera.h"
#include "Managers/ImGui/ImGuiManager.h"

RailCamera::RailCamera()
	: t_(0.0f)
	, speed_(0.001f)
	, isMoving_(true)
	, lookAheadDistance_(0.01f)
	, showRailTrack_(true)
	, railTrackColor_({ 1.0f, 0.0f, 0.0f, 1.0f })  // 赤色
	, railTrackSegments_(100)  // KamataEngineと同じ
	, directXCommon_(nullptr) {
}

RailCamera::~RailCamera() {
	// デストラクタでも安全にリソースを解放
	ReleaseResources();
}

void RailCamera::Initialize(const Vector3& position, const Vector3& rotation) {
	// 初期値を保存
	initialPosition_ = position;
	initialRotation_ = rotation;

	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// トランスフォームの初期化
	transform_.Initialize(directXCommon_);
	SetDefaultCamera(position, rotation);

	// デバッグ用カメラモデルの初期化
	cameraModel_ = std::make_unique<Model3D>();
	cameraModel_->Initialize(directXCommon_, "camera");
	cameraModel_->SetName("RailCamera");

	// 線描画システムの初期化
	lineRenderer_ = std::make_unique<LineRenderer>();
	lineRenderer_->Initialize(directXCommon_);

	// スプライン曲線の制御点を設定（KamataEngineと同じ）
	controlPoints_ = {
		{0,  0,   0 },
		{10, 10,  0 },
		{10, 15,  0 },
		{20, 15,  0 },
		{20, 0,   0 },
		{20, 0,   10},
		{30, -10, 5 },
		{20, -10, 0 },
		{20, -15, 5 },
		{10, -15, 0 },
		{10, -10, 0 },
		{0,  0,   0 },
	};

	// レール移動パラメータの初期化
	t_ = 0.0f;
	speed_ = 0.001f;            // 移動速度（1フレームあたりのtの増分）
	isMoving_ = true;           // 初期状態で移動開始
	lookAheadDistance_ = 0.01f; // 注視点の先読み距離

	// 軌道の線分を生成
	GenerateRailTrackLines();
}

void RailCamera::Update() {
	// レール移動が有効な場合
	if (isMoving_) {
		UpdateCameraPosition();
	}

	// 修正: 循環参照を回避するため、単位行列を使用してワールド行列を更新
	Matrix4x4 identityMatrix = MakeIdentity4x4();
	transform_.UpdateMatrix(identityMatrix);
}

void RailCamera::UpdateCameraModel() {
	// カメラモデルの更新を独立して実行
	if (cameraModel_) {
		// カメラモデルの位置をレールカメラの位置に同期
		cameraModel_->SetPosition(transform_.GetPosition());
		cameraModel_->SetRotation(transform_.GetRotation());

		// カメラモデル用のビュープロジェクション行列を取得
		// アクティブなカメラのビュープロジェクション行列を使用
		CameraController* cameraController = CameraController::GetInstance();
		Matrix4x4 viewProjectionMatrix = cameraController->GetViewProjectionMatrix();
		cameraModel_->Update(viewProjectionMatrix);
	}
}

void RailCamera::GenerateRailTrackLines() {
	if (!lineRenderer_) {
		return;
	}

	// 線分リストをクリア
	lineRenderer_->Reset();

	// KamataEngineと同じように線分を生成
	std::vector<Vector3> pointsDrawing;

	// 線分の数+1個分の頂点座標を計算
	for (int i = 0; i < railTrackSegments_ + 1; i++) {
		float t = 1.0f / railTrackSegments_ * i;
		Vector3 point = CatmullRomPosition(controlPoints_, t);
		// 描画用にリストに保存する
		pointsDrawing.push_back(point);
	}

	// 配列の範囲内でループして隣接する2点を結ぶ線分を追加
	for (size_t i = 0; i < pointsDrawing.size() - 1; ++i) {
		lineRenderer_->AddLine(pointsDrawing[i], pointsDrawing[i + 1], railTrackColor_);
	}
}

void RailCamera::ImGui() {
#ifdef _DEBUG
	ImGui::Text("RailCamera");
	ImGui::Separator();

	Vector3 position = transform_.GetPosition();
	Vector3 rotation = transform_.GetRotation();

	if (ImGui::DragFloat3("Position", &position.x, 0.01f)) {
		transform_.SetPosition(position);
	}
	if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f)) {
		transform_.SetRotation(rotation);
	}
	ImGui::Separator();

	// レール移動制御
	ImGui::Text("Rail Movement:");
	ImGui::Checkbox("Moving", &isMoving_);
	ImGui::SameLine();
	if (ImGui::Button("Start")) {
		StartMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		StopMovement();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
		ResetPosition();
	}

	ImGui::DragFloat("Speed", &speed_, 0.0001f, 0.0001f, 0.01f);
	ImGui::DragFloat("Look Ahead Distance", &lookAheadDistance_, 0.001f);

	ImGui::Separator();

	// 軌道描画設定
	ImGui::Text("Rail Track Display:");
	bool trackSettingsChanged = false;

	if (ImGui::Checkbox("Show Rail Track", &showRailTrack_)) {
		trackSettingsChanged = true;
	}

	if (ImGui::ColorEdit4("Track Color", &railTrackColor_.x)) {
		trackSettingsChanged = true;
	}

	if (ImGui::DragInt("Track Segments", &railTrackSegments_, 1, 10, 500)) {
		trackSettingsChanged = true;
	}

	// 軌道設定が変更された場合は再生成
	if (trackSettingsChanged) {
		GenerateRailTrackLines();
	}

	// 線描画システムのImGui
	if (lineRenderer_) {
		lineRenderer_->ImGui();
	}

	ImGui::Separator();

	// リセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera(initialPosition_, initialRotation_);
	}
#endif
}

Matrix4x4 RailCamera::GetViewProjectionMatrix() const {
	// ワールド行列の逆行列をビュー行列として使用
	Matrix4x4 viewMatrix = Matrix4x4Inverse(transform_.GetWorldMatrix());
	return Matrix4x4Multiply(viewMatrix, projectionMatrix_);
}

Matrix4x4 RailCamera::GetSpriteViewProjectionMatrix() const {
	// スプライト用行列を作成して返す
	Matrix4x4 spriteViewMatrix = MakeIdentity4x4();
	return Matrix4x4Multiply(spriteViewMatrix, spriteProjectionMatrix_);
}

void RailCamera::DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight) {
	// カメラモデルの更新（デバッグカメラでも表示されるように）
	UpdateCameraModel();

	// デバッグ用カメラモデルの描画
	if (cameraModel_) {
		cameraModel_->Draw(directionalLight);
	}

	// 軌道の一括描画
	if (lineRenderer_ && showRailTrack_) {
		lineRenderer_->Draw(viewProjectionMatrix);
	}
}

void RailCamera::ReleaseResources() {
	// 1. 線描画システムを先に解放
	if (lineRenderer_) {
		lineRenderer_.reset();
	}

	// 2. カメラモデルを解放
	if (cameraModel_) {
		cameraModel_.reset();
	}

	// 3. システム参照をクリア
	directXCommon_ = nullptr;

	Logger::Log(Logger::GetStream(), "RailCamera: Resources released\n");
}

void RailCamera::SetDefaultCamera(const Vector3& position, const Vector3& rotation) {
	// デフォルト値に設定（座標・回転は引数で指定）
	transform_.SetPosition(position);
	transform_.SetRotation(rotation);
	transform_.SetScale({ 1.0f, 1.0f, 1.0f });

	// カメラパラメータのデフォルト値
	float fov = 0.45f;
	float nearClip = 0.1f;
	float farClip = 100.0f;
	float aspectRatio = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));

	// プロジェクション行列の初期化
	projectionMatrix_ = MakePerspectiveFovMatrix(fov, aspectRatio, nearClip, farClip);
	spriteProjectionMatrix_ = MakeOrthograpicMatrix(
		0.0f, 0.0f,
		float(GraphicsConfig::kClientWidth),
		float(GraphicsConfig::kClientHeight),
		0.0f, 100.0f
	);
}

void RailCamera::UpdateCameraPosition() {
	// tパラメータを更新
	t_ += speed_;

	// ループ処理
	if (t_ >= 1.0f) {
		t_ = 0.0f;
	}

	// 現在の位置を計算（カメラの視点）
	Vector3 currentPosition = CatmullRomPosition(controlPoints_, t_);

	// 注視点を計算（進行方向の少し先の点）
	Vector3 lookAtTarget = CalculateLookAtTarget(t_);

	// カメラの位置を設定
	transform_.SetPosition(currentPosition);

	// 視点から注視点への方向ベクトルを計算
	Vector3 forward = lookAtTarget - currentPosition;
	forward = Normalize(forward);

	// 方向ベクトルから回転角を計算
	Vector3 rotation = transform_.GetRotation();

	// Y軸回転（ヨー角）
	rotation.y = std::atan2(forward.x, forward.z);

	// X軸回転（ピッチ角）
	float horizontalDistance = std::sqrt(forward.x * forward.x + forward.z * forward.z);
	rotation.x = -std::atan2(forward.y, horizontalDistance);

	// Z軸回転は0のまま
	rotation.z = 0.0f;

	transform_.SetRotation(rotation);
}

Vector3 RailCamera::CalculateLookAtTarget(float currentT) {
	// 少し先のtパラメータを計算
	float lookAheadT = currentT + lookAheadDistance_;

	// tが1.0を超える場合の処理
	if (lookAheadT >= 1.0f) {
		lookAheadT = 1.0f;
	}

	// 先読み位置を計算
	return CatmullRomPosition(controlPoints_, lookAheadT);
}