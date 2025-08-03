#include "RailCameraMovement.h"

void RailCameraMovement::Initialize(RailTrack* track) {
	track_ = track;
	t_ = 0.0f;
	isAtEnd_ = false;
}

void RailCameraMovement::Update() {
	if (!isMoving_ || !track_ || !track_->IsValid()) {
		return;
	}

	// 終端フラグをリセット（移動が再開された場合）
	if (isAtEnd_ && isMoving_) {
		isAtEnd_ = false;
	}

	if (uniformSpeedEnabled_ && track_->GetTotalLength() > 0.0f) {
		// 等間隔移動：距離ベースでの移動
		float currentLength = t_ * track_->GetTotalLength();
		currentLength += speed_ * track_->GetTotalLength();

		// ループ処理
		if (currentLength >= track_->GetTotalLength()) {
			if (loopEnabled_) {
				currentLength = 0.0f;
				isAtEnd_ = false;
			} else {
				currentLength = track_->GetTotalLength();
				isMoving_ = false;
				isAtEnd_ = true;
			}
		}

		// 進行度を更新
		t_ = currentLength / track_->GetTotalLength();
	} else {
		// 従来の移動：tパラメータベース
		t_ += speed_;
		if (t_ >= 1.0f) {
			if (loopEnabled_) {
				t_ = 0.0f;
				isAtEnd_ = false;
			} else {
				t_ = 1.0f;
				isMoving_ = false;
				isAtEnd_ = true;
			}
		}
	}
}

void RailCameraMovement::ResetPosition() {
	t_ = 0.0f;
	isMoving_ = false;
	isAtEnd_ = false;
}

void RailCameraMovement::SetProgress(float progress) {
	t_ = std::clamp(progress, 0.0f, 1.0f);
	isAtEnd_ = (t_ >= 1.0f && !loopEnabled_);
}

Vector3 RailCameraMovement::GetCurrentPosition() const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}
	return track_->CalculatePosition(t_);
}

Vector3 RailCameraMovement::GetLookAtTarget() const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	// 終端近くでの特別処理
	if (t_ >= 0.99f) {
		float safeT = std::max(0.0f, t_ - 0.01f);
		Vector3 currentPos = track_->CalculatePosition(t_);
		Vector3 prevPos = track_->CalculatePosition(safeT);

		Vector3 direction = currentPos - prevPos;
		if (Length(direction) > 0.001f) {
			direction = Normalize(direction);
			return currentPos + direction * (lookAheadDistance_ * track_->GetTotalLength());
		}
	}

	float lookAheadT = t_ + lookAheadDistance_;

	if (uniformSpeedEnabled_ && track_->GetTotalLength() > 0.0f) {
		// 等間隔移動の場合は距離ベースで先読み
		float currentLength = t_ * track_->GetTotalLength();
		float lookAheadLength = currentLength + (lookAheadDistance_ * track_->GetTotalLength());

		if (lookAheadLength >= track_->GetTotalLength()) {
			lookAheadLength = track_->GetTotalLength();
		}

		lookAheadT = track_->GetTFromLength(lookAheadLength);
	} else {
		if (lookAheadT >= 1.0f) {
			lookAheadT = 1.0f;
		}
	}

	return track_->CalculatePosition(lookAheadT);
}

int RailCameraMovement::GetCurrentFrameFromStart() const {
	if (speed_ <= 0.0f || !track_) {
		return 0;
	}

	if (uniformSpeedEnabled_ && track_->GetTotalLength() > 0.0f) {
		float currentDistance = t_ * track_->GetTotalLength();
		float distancePerFrame = speed_ * track_->GetTotalLength();
		return static_cast<int>(currentDistance / distancePerFrame);
	} else {
		return static_cast<int>(t_ / speed_);
	}
}

float RailCameraMovement::GetProgressFromFrame(int frame) const {
	if (frame <= 0 || speed_ <= 0.0f || !track_) {
		return 0.0f;
	}

	if (uniformSpeedEnabled_ && track_->GetTotalLength() > 0.0f) {
		float distancePerFrame = speed_ * track_->GetTotalLength();
		float targetDistance = frame * distancePerFrame;
		return std::clamp(targetDistance / track_->GetTotalLength(), 0.0f, 1.0f);
	} else {
		return std::clamp(frame * speed_, 0.0f, 1.0f);
	}
}

void RailCameraMovement::SetProgressFromFrame(int frame) {
	float newProgress = GetProgressFromFrame(frame);
	SetProgress(newProgress);
}

int RailCameraMovement::GetMaxFrames() const {
	if (speed_ <= 0.0f || !track_) {
		return 0;
	}

	if (uniformSpeedEnabled_ && track_->GetTotalLength() > 0.0f) {
		float distancePerFrame = speed_ * track_->GetTotalLength();
		return static_cast<int>(track_->GetTotalLength() / distancePerFrame);
	} else {
		return static_cast<int>(1.0f / speed_);
	}
}