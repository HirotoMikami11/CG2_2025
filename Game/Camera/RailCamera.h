#pragma once
#include <vector>
#include "CameraController/BaseCamera.h"
#include "Objects/GameObject/GameObject.h"
#include "Objects/Line/LineRenderer.h"
#include "Engine.h"

/// <summary>
/// レールカメラコントローラー
/// スプライン曲線に沿って等間隔で移動するカメラ
/// </summary>
class RailCamera : public BaseCamera {
public:
	/// <summary>
	/// 長さテーブルのエントリ
	/// </summary>
	struct LengthTableEntry {
		float t;            // パラメータt
		float length;       // 累積長さ
		float segmentLength; // このセグメントの長さ
	};

	/// <summary>
	/// 視錐台の頂点データ
	/// </summary>
	struct ViewFrustum {
		Vector3 nearCorners[4];  // 近クリップ面の4つの角
		Vector3 farCorners[4];   // 遠クリップ面の4つの角
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	RailCamera();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~RailCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転</param>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;

	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;

	// BaseCamera interface
	Matrix4x4 GetViewProjectionMatrix() const override;
	Matrix4x4 GetSpriteViewProjectionMatrix() const override;
	Vector3 GetPosition() const override { return transform_.GetPosition(); }
	void SetPosition(const Vector3& position) override { transform_.SetPosition(position); }
	Vector3 GetRotation() const override { return transform_.GetRotation(); }
	std::string GetCameraType() const override { return "Rail"; }

	// RailCamera固有機能
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }

	/// <summary>
	/// レールカメラの軌道描画
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="directionalLight">平行光源</param>
	void DrawRailTrack(const Matrix4x4& viewProjectionMatrix, const Light& directionalLight);

	// レール移動制御
	void StartMovement() { isMoving_ = true; }
	void StopMovement() { isMoving_ = false; }
	void ResetPosition() { t_ = 0.0f; isMoving_ = false; }
	bool IsMoving() const { return isMoving_; }

	/// <summary>
	/// ループ移動を有効/無効にする
	/// </summary>
	/// <param name="enabled">有効フラグ</param>
	void SetLoopEnabled(bool enabled) { loopEnabled_ = enabled; }

	/// <summary>
	/// ループ移動が有効かどうか
	/// </summary>
	/// <returns>有効フラグ</returns>
	bool IsLoopEnabled() const { return loopEnabled_; }

	/// <summary>
	/// 移動速度を設定
	/// </summary>
	/// <param name="speed">移動速度</param>
	void SetSpeed(float speed) { speed_ = speed; }

	/// <summary>
	/// 移動速度を取得
	/// </summary>
	/// <returns>移動速度</returns>
	float GetSpeed() const { return speed_; }

	/// <summary>
	/// 制御点を設定（エディタ用）
	/// </summary>
	/// <param name="controlPoints">制御点のリスト</param>
	void SetControlPoints(const std::vector<Vector3>& controlPoints);

	/// <summary>
	/// 制御点を取得
	/// </summary>
	/// <returns>制御点のリスト</returns>
	const std::vector<Vector3>& GetControlPoints() const { return controlPoints_; }

	/// <summary>
	/// 軌道の線分を生成（エディタから呼び出し用）
	/// </summary>
	void GenerateRailTrackLines();

	/// <summary>
	/// 視錐台線分を生成（動的描画用）
	/// </summary>
	void GenerateViewFrustumLines();

	/// <summary>
	/// リソースの明示的解放
	/// </summary>
	void ReleaseResources();

	/// <summary>
	/// 等間隔移動を有効/無効にする
	/// </summary>
	/// <param name="enabled">有効フラグ</param>
	void SetUniformSpeedEnabled(bool enabled) { uniformSpeedEnabled_ = enabled; }

	/// <summary>
	/// 等間隔移動が有効かどうか
	/// </summary>
	/// <returns>有効フラグ</returns>
	bool IsUniformSpeedEnabled() const { return uniformSpeedEnabled_; }

	/// <summary>
	/// 現在の移動進行度を取得（0.0-1.0）
	/// </summary>
	/// <returns>進行度</returns>
	float GetProgress() const { return t_; }

	/// <summary>
	/// 移動進行度を設定
	/// </summary>
	/// <param name="progress">進行度（0.0-1.0）</param>
	void SetProgress(float progress) { t_ = std::clamp(progress, 0.0f, 1.0f); }

	/// <summary>
	/// 現在の進行方向ベクトルを取得
	/// </summary>
	/// <returns>正規化された進行方向ベクトル</returns>
	Vector3 GetForwardDirection() const;

	/// <summary>
	/// 選択されたポイントのインデックスを設定（エディタ用）
	/// </summary>
	/// <param name="index">選択されたポイントのインデックス（-1で選択解除）</param>
	void SetSelectedPointIndex(int index) { selectedPointIndex_ = index; }

	/// <summary>
	/// 選択されたポイントのインデックスを取得
	/// </summary>
	/// <returns>選択されたポイントのインデックス（-1は選択なし）</returns>
	int GetSelectedPointIndex() const { return selectedPointIndex_; }

	// ======================== デバッグ機能 ========================

	/// <summary>
	/// 開始時点から現在の位置までのフレーム数を取得
	/// </summary>
	/// <returns>フレーム数</returns>
	int GetCurrentFrameFromStart() const;

	/// <summary>
	/// 指定したフレーム数での進行度を計算
	/// </summary>
	/// <param name="frame">フレーム数</param>
	/// <returns>進行度（0.0-1.0）</returns>
	float GetProgressFromFrame(int frame) const;

	/// <summary>
	/// フレーム数から位置を設定
	/// </summary>
	/// <param name="frame">フレーム数</param>
	void SetProgressFromFrame(int frame);

	/// <summary>
	/// 最大フレーム数を取得（ループなしで終端まで）
	/// </summary>
	/// <returns>最大フレーム数</returns>
	int GetMaxFrames() const;

	/// <summary>
	/// 視錐台表示の有効/無効を設定
	/// </summary>
	/// <param name="visible">表示フラグ</param>
	void SetViewFrustumVisible(bool visible) { showViewFrustum_ = visible; }

	/// <summary>
	/// 視錐台表示の状態を取得
	/// </summary>
	/// <returns>表示フラグ</returns>
	bool IsViewFrustumVisible() const { return showViewFrustum_; }

	/// <summary>
	/// 視錐台の色を設定
	/// </summary>
	/// <param name="color">色</param>
	void SetViewFrustumColor(const Vector4& color) { viewFrustumColor_ = color; }

	/// <summary>
	/// 視錐台の色を取得
	/// </summary>
	/// <returns>視錐台の色</returns>
	Vector4 GetViewFrustumColor() const { return viewFrustumColor_; }

	/// <summary>
	/// 視錐台の距離を設定
	/// </summary>
	/// <param name="distance">描画距離</param>
	void SetViewFrustumDistance(float distance) { viewFrustumDistance_ = distance; }

	/// <summary>
	/// 視錐台の描画距離を取得
	/// </summary>
	/// <returns>描画距離</returns>
	float GetViewFrustumDistance() const { return viewFrustumDistance_; }

	/// <summary>
	/// カメラのFOVを取得（デバッグ用）
	/// </summary>
	/// <returns>FOV値</returns>
	float GetFOV() const { return 0.45f; }

	/// <summary>
	/// カメラのアスペクト比を取得（デバッグ用）
	/// </summary>
	/// <returns>アスペクト比</returns>
	float GetAspectRatio() const {
		return (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));
	}

	/// <summary>
	/// カメラのnearクリップ距離を取得（デバッグ用）
	/// </summary>
	/// <returns>nearクリップ距離</returns>
	float GetNearClip() const { return 0.1f; }

	/// <summary>
	/// カメラのfarクリップ距離を取得（デバッグ用）
	/// </summary>
	/// <returns>farクリップ距離</returns>
	float GetFarClip() const { return 1000.0f; }

private:
	// トランスフォーム
	Transform3D transform_;

	// カメラパラメータ
	Matrix4x4 projectionMatrix_;
	Matrix4x4 spriteProjectionMatrix_;

	// デバッグ用のカメラモデル
	std::unique_ptr<Model3D> cameraModel_;

	// 線描画システム
	std::unique_ptr<LineRenderer> lineRenderer_;

	/// スプライン曲線制御点(通過点)
	std::vector<Vector3> controlPoints_;

	// レール移動用
	float t_;                 // スプライン曲線上の位置パラメータ (0.0 - 1.0)
	float speed_;             // 移動速度
	bool isMoving_;           // 移動中フラグ
	bool loopEnabled_;        // ループ移動フラグ
	float lookAheadDistance_; // 注視点の先読み距離

	// 等間隔移動用
	bool uniformSpeedEnabled_;     // 等間隔移動有効フラグ
	std::vector<LengthTableEntry> lengthTable_; // 長さテーブル
	float totalLength_;            // 曲線の総長さ
	int lengthTableResolution_;    // 長さテーブルの解像度

	// 軌道描画設定
	bool showRailTrack_;        // 軌道表示フラグ
	Vector4 railTrackColor_;    // 軌道の色
	int railTrackSegments_;     // 軌道の分割数

	// 制御点描画設定
	bool showControlPoints_;    // 制御点表示フラグ
	Vector4 controlPointColor_; // 制御点の色
	Vector4 selectedPointColor_; // 選択された制御点の色
	float controlPointSize_;    // 制御点のサイズ

	// エディタ用
	int selectedPointIndex_;    // 選択された制御点のインデックス

	// 初期値保存用
	Vector3 initialPosition_;
	Vector3 initialRotation_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// デバッグ機能用
	bool showViewFrustum_ = false;              // 視錐台表示フラグ
	Vector4 viewFrustumColor_ = { 1.0f, 1.0f, 0.0f, 1.0f }; // 視錐台の色（黄色）
	float viewFrustumDistance_ = 50.0f;         // 視錐台描画距離
	int debugFrameInput_ = 0;                   // デバッグ用フレーム入力

	/// <summary>
	/// カメラモデルの更新（描画時に呼び出し）
	/// </summary>
	void UpdateCameraModel();

	/// <summary>
	/// スプライン曲線上の位置を計算してカメラを更新
	/// </summary>
	void UpdateCameraPosition();

	/// <summary>
	/// 注視点を計算する
	/// </summary>
	/// <param name="currentT">現在のtパラメータ</param>
	/// <returns>注視点の座標</returns>
	Vector3 CalculateLookAtTarget(float currentT);

	/// <summary>
	/// 指定座標・回転でデフォルト値を設定
	/// </summary>
	/// <param name="position">初期座標</param>
	/// <param name="rotation">初期回転</param>
	void SetDefaultCamera(const Vector3& position, const Vector3& rotation);

	/// <summary>
	/// 長さテーブルを構築する
	/// </summary>
	void BuildLengthTable();

	/// <summary>
	/// 長さからtパラメータを取得する
	/// </summary>
	/// <param name="targetLength">目標長さ</param>
	/// <returns>対応するtパラメータ</returns>
	float GetTFromLength(float targetLength) const;

	/// <summary>
	/// 曲線上の2点間の距離を計算する
	/// </summary>
	/// <param name="t1">開始tパラメータ</param>
	/// <param name="t2">終了tパラメータ</param>
	/// <param name="subdivisions">分割数</param>
	/// <returns>距離</returns>
	float CalculateSegmentLength(float t1, float t2, int subdivisions = 10) const;

	/// <summary>
	/// 制御点を描画用の線分として追加
	/// </summary>
	void DrawControlPoints();

	/// <summary>
	/// 視錐台を計算する
	/// </summary>
	/// <param name="cameraPos">カメラ位置</param>
	/// <param name="cameraRot">カメラ回転</param>
	/// <param name="distance">描画距離</param>
	/// <returns>視錐台データ</returns>
	ViewFrustum CalculateViewFrustum(const Vector3& cameraPos, const Vector3& cameraRot, float distance) const;
};