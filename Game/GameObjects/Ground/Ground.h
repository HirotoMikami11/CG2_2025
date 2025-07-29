#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"

/// <summary>
/// 地面クラス
/// </summary>
class Ground {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Ground();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Ground();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

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