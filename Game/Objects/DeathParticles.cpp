#include "DeathParticles.h"
#include "Managers/ImGui/ImGuiManager.h"

DeathParticles::DeathParticles() {}

DeathParticles::~DeathParticles() {}

void DeathParticles::Initialize(const Vector3& position) {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// パーティクルの初期化
	for (auto& particle : particles_) {
		particle = std::make_unique<Model3D>();
		particle->Initialize(directXCommon_, "deathParticle");
		particle->SetPosition(position);
	}

	// 初期状態の設定
	isFinished_ = false;
	counter_ = 0.0f;
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void DeathParticles::Update(const Matrix4x4& viewProjectionMatrix) {
	// 終了していたら更新しない
	if (isFinished_) {
		return;
	}

	// パーティクルの移動処理
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = { kSpeed, 0.0f, 0.0f };

		// 回転角を計算する
		float angle = kAngleUnit * i;

		// Z軸まわり回転行列を作成
		Matrix4x4 rotationMatrix = MakeRotateZMatrix(angle);

		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = TransformDirection(velocity, rotationMatrix);

		// 現在の位置を取得
		Vector3 currentPos = particles_[i]->GetPosition();

		// 移動処理
		Vector3 newPos = Add(currentPos, velocity);
		particles_[i]->SetPosition(newPos);
	}

	/// 寿命の計算
	// カウンターを１フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	// フェードアウトの計算
	float colorT = 1.0f - (counter_ / kDuration);
	color_.w = std::clamp(colorT, 0.0f, 1.0f);

	// 各パーティクルに色を設定
	for (auto& particle : particles_) {
		particle->SetColor(color_);
		// 行列の更新
		particle->Update(viewProjectionMatrix);
	}
}

void DeathParticles::Draw(const Light& directionalLight) {
	// 終了していたら描画しない
	if (isFinished_) {
		return;
	}

	// 各パーティクルの描画
	for (auto& particle : particles_) {
		particle->Draw(directionalLight);
	}
}

void DeathParticles::Start(const Vector3& position) {
	// パーティクルをリセットして開始
	counter_ = 0.0f;
	isFinished_ = false;
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 全パーティクルの位置を設定
	for (auto& particle : particles_) {
		particle->SetPosition(position);
		particle->SetColor(color_);
	}
}