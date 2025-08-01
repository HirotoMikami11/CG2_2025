#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"

/// <summary>
/// 天球クラス
/// </summary>
class Skydome {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Skydome();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Skydome();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);


	/// <summary>
	/// imgui
	/// </summary>
	void ImGui();
private:
	// ゲームオブジェクト
	std::unique_ptr<Model3D> gameObject_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
};