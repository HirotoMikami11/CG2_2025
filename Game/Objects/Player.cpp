#define NOMINMAX // C+標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include "Player.h"
#include "Managers/ImGuiManager.h"

void Player::Initialize()
{
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	input_ = InputManager::GetInstance();

	//初期化、座標設定
	Object_ = std::make_unique<Model3D>();
	Object_->Initialize(directXCommon_, "resources/Model/Player", "player.obj");

	// 物理パラメータの初期化
	velocity_ = { 0.0f, 0.0f, 0.0f };
	onGround_ = true;

	//回転方向を右方向
	Object_->SetRotation({ 0.0f,std::numbers::pi_v<float> / 2.0f,0.0f });
	lrDirection_ = LRDirection::kRight;
	turnTimer_ = 0.0f;
	turnFirstRotationY_ = 0.0f;
}

void Player::Update(const Matrix4x4& viewProjectionMatrix)
{
	/// 1. 移動入力
	if (onGround_) {
		// 移動入力
		IsMove();
	} else {
		// 落下中の処理
		IsFall();
	}

	/// 2. 移動量を加味して衝突判定する
	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveVelocity = velocity_;
	// マップ衝突チェック
	if (mapChipField_) {
		CollisonMap(collisionMapInfo);
	}

	/// 3. 判定結果を反映して移動させる
	MoveAfterCollisionCheck(collisionMapInfo);

	/// 4. 天井に接触している場合の処理
	IsCeilingHit(collisionMapInfo);

	/// 5. 壁に設置している場合の処理
	IsWallHit(collisionMapInfo);

	/// 6. 接地状態の切り替え
	SwitchGrounding(collisionMapInfo);

	/// 7. 旋回制御
	TurningControl();

	/// 8. 行列計算
	Object_->Update(viewProjectionMatrix);
}

void Player::IsMove() {
	// 左右移動操作

	if (input_->IsKeyDown(DIK_RIGHT) || input_->IsKeyDown(DIK_LEFT)) {
		// 左右加速
		Vector3 acceleration = {};

		if (input_->IsKeyDown(DIK_RIGHT)) {
			// 左移動中の右入力
			if (velocity_.x < 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - kAcceleration);
			}
			// 加速度を増やす
			acceleration.x += kAcceleration;

			// 方向が違ったら変更する
			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				Vector3 rotation = Object_->GetRotation();
				turnFirstRotationY_ = rotation.y;
				turnTimer_ = kTimeTurn;
			}

		} else if (input_->IsKeyDown(DIK_LEFT)) {
			// 右移動中の左入力
			if (velocity_.x > 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - kAcceleration);
			}
			// 加速度を増やす
			acceleration.x -= kAcceleration;

			// 方向が違ったら変更する
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				Vector3 rotation = Object_->GetRotation();
				turnFirstRotationY_ = rotation.y;
				turnTimer_ = kTimeTurn;
			}
		}
		// 加速減速
		velocity_ = Add(velocity_, acceleration);
		// 最大速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	} else {
		// 非入力時は移動減衰をかける
		velocity_.x *= (1.0f - kAttenuation);
	}

	// ジャンプ入力
	if (input_->IsKeyDown(DIK_UP)) {
		velocity_.y += kJumpAcceleration;
	}
}

void Player::IsFall() {
	// 落下速度
	velocity_.y += -kGravityAcceleration;
	// 落下速度制限
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
}

void Player::CollisonMap(CollisionMapInfo& info) {
	CollisonMapTop(info);
	CollisonMapBottom(info);
	CollisonMapLeft(info);
	CollisonMapRight(info);
}

void Player::CollisonMapTop(CollisionMapInfo& info) {
	// 上昇しているとき以外は抜ける
	if (info.moveVelocity.y <= 0.0f) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, 4> positionNew;
	Vector3 currentPos = Object_->GetPosition();
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(Add(currentPos, Vector3(0, info.moveVelocity.y, 0)), static_cast<Corner>(i));
	}

	MapChipType mapchipType;
	bool hit = false;

	// 左上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒットしたら
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xindex, indexSet.yindex);
		// Y移動量=(ブロックの下端-自キャラの移動前の座標)+(自キャラの半径+余白)
		float moveY = (rect.bottom - currentPos.y) - ((kHeight / 2) + kBlank);
		info.moveVelocity.y = std::max(0.0f, moveY);
		// 天井に当たったことを記録する
		info.isCeilingHit = true;
	}
}

void Player::CollisonMapBottom(CollisionMapInfo& info) {
	// 降下していなければ抜ける
	if (info.moveVelocity.y >= 0.0f) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, 4> positionNew;
	Vector3 currentPos = Object_->GetPosition();
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(Add(currentPos, Vector3(0, info.moveVelocity.y, 0)), static_cast<Corner>(i));
	}

	MapChipType mapchipType;
	bool hit = false;

	// 左下の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒットしたら
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xindex, indexSet.yindex);
		float moveY = (rect.top - currentPos.y) + (kWidth / 2 + kBlank);
		info.moveVelocity.y = std::min(0.0f, moveY);

		// 地面に当たったことを記録する
		info.isLanding = true;
	}
}

void Player::CollisonMapLeft(CollisionMapInfo& info) {
	// 左に移動していないければ抜ける
	if (info.moveVelocity.x >= 0.0f) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, 4> positionNew;
	Vector3 currentPos = Object_->GetPosition();
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(Add(currentPos, Vector3(info.moveVelocity.x, 0, 0)), static_cast<Corner>(i));
	}

	MapChipType mapchipType;
	bool hit = false;

	// 左上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒットしたら
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xindex, indexSet.yindex);
		float moveX = (rect.right - currentPos.x) + (kWidth / 2 + kBlank);
		info.moveVelocity.x = std::min(0.0f, moveX);

		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}

void Player::CollisonMapRight(CollisionMapInfo& info) {
	// 右に移動していないければ抜ける
	if (info.moveVelocity.x <= 0.0f) {
		return;
	}

	// 移動後の４つの角の座標
	std::array<Vector3, 4> positionNew;
	Vector3 currentPos = Object_->GetPosition();
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(Add(currentPos, Vector3(info.moveVelocity.x, 0, 0)), static_cast<Corner>(i));
	}

	MapChipType mapchipType;
	bool hit = false;

	// 右上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
	if (mapchipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒットしたら
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xindex, indexSet.yindex);
		float moveX = (rect.left - currentPos.x) - ((kWidth / 2) + kBlank);
		info.moveVelocity.x = std::max(0.0f, moveX);

		// 壁に当たったことを記録する
		info.isWallHit = true;
	}
}

Vector3& Player::CornerPosition(const Vector3& center, Corner corner) {
	static Vector3 result;

	Vector3 offsetTable[kNumCorner] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},	//右下
		{-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},	//左下
		{+kWidth / 2.0f, +kHeight / 2.0f, 0.0f},	//右上
		{-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}		//左上
	};
	// テーブルを使ってif文使わないで角の座標を求める
	result = Add(center, offsetTable[static_cast<uint32_t>(corner)]);

	return result;
}

void Player::MoveAfterCollisionCheck(const CollisionMapInfo& info) {
	Vector3 currentPos = Object_->GetPosition();
	Vector3 newPos = Add(currentPos, info.moveVelocity);
	Object_->SetPosition(newPos);
}

void Player::IsCeilingHit(const CollisionMapInfo& info) {
	if (info.isCeilingHit) {
		velocity_.y = 0;
	}
}

void Player::IsWallHit(const CollisionMapInfo& info) {
	if (info.isWallHit) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::SwitchGrounding(const CollisionMapInfo& info) {
	if (onGround_) {
		/// 接地状態の処理
		/// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		} else {
			/// 落下判定
			// 移動後の４つの角の座標
			std::array<Vector3, 4> positionNew;
			Vector3 currentPos = Object_->GetPosition();
			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(Add(currentPos, info.moveVelocity), static_cast<Corner>(i));
			}

			MapChipType mapchipType;
			bool hit = false;

			// 左下の判定
			MapChipField::IndexSet indexSet;
			Vector3 leftBottomCheckPos = Add(positionNew[kLeftBottom], Vector3(0.0f, -kGroundCheckOffset, 0.0f));
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottomCheckPos);
			mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
			if (mapchipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下の判定
			Vector3 rightBottomCheckPos = Add(positionNew[kRightBottom], Vector3(0.0f, -kGroundCheckOffset, 0.0f));
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottomCheckPos);
			mapchipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xindex, indexSet.yindex);
			if (mapchipType == MapChipType::kBlock) {
				hit = true;
			}

			// ブロックに当たっていない
			// 落下中なら落下状態に切り替え
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}
	} else {
		/// 空中状態の処理
		// 接地フラグ
		if (info.isLanding) {
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にx速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// y速度はゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::TurningControl() {
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= (1.0f / 60.0f);
		turnTimer_ = std::clamp(turnTimer_, 0.0f, kTimeTurn);

		// イージングt
		float t = 1.0f - (turnTimer_ / kTimeTurn);
		t = std::clamp(t, 0.0f, 1.0f);
		float easeT = EaseInOutQuad(t);

		// 旋回制御
		// 左右のキャラ角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,		// 右
			std::numbers::pi_v<float> *3.0f / 2.0f	// 左
		};

		// 状況に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定
		float currentRotationY = Lerp(turnFirstRotationY_, destinationRotationY, easeT);
		Vector3 rotation = Object_->GetRotation();
		rotation.y = currentRotationY;
		Object_->SetRotation(rotation);
	}
}

void Player::Draw(const Light& directionalLight)
{
	Object_->Draw(directionalLight);
}

void Player::ImGui()
{
#ifdef _DEBUG
	ImGui::Text("Player");
	Object_->ImGui();

	// 物理パラメータの表示
	ImGui::Text("Velocity: %.3f, %.3f, %.3f", velocity_.x, velocity_.y, velocity_.z);
	ImGui::Text("On Ground: %s", onGround_ ? "true" : "false");
	ImGui::Text("Direction: %s", lrDirection_ == LRDirection::kRight ? "Right" : "Left");

	ImGui::Spacing();
#endif
}