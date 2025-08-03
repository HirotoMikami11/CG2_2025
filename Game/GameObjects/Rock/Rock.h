#pragma once
#include "Objects/GameObject/GameObject.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"
#include "Objects/Light/Light.h"

/// <summary>
/// 岩のタイプ
/// </summary>
enum class RockType {
	Rock1,
	Rock2,
	Rock3
};

/// <summary>
/// 岩クラス（Model3Dを継承）
/// </summary>
class Rock : public Model3D {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Rock();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Rock();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="rockType">岩のタイプ</param>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転</param>
	/// <param name="scale">初期スケール</param>
	void Initialize(DirectXCommon* dxCommon, RockType rockType,
		const Vector3& position = { 0.0f, 0.0f, 0.0f },
		const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
		const Vector3& scale = { 1.0f, 1.0f, 1.0f });

	/// <summary>
	/// ImGui（オーバーライド）
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// 岩タイプの設定
	/// </summary>
	/// <param name="rockType">岩のタイプ</param>
	void SetRockType(RockType rockType);

	/// <summary>
	/// 岩タイプの取得
	/// </summary>
	/// <returns>岩のタイプ</returns>
	RockType GetRockType() const { return rockType_; }

	/// <summary>
	/// 岩タイプから文字列に変換
	/// </summary>
	/// <param name="rockType">岩タイプ</param>
	/// <returns>文字列</returns>
	static std::string RockTypeToString(RockType rockType);

	/// <summary>
	/// 文字列から岩タイプに変換
	/// </summary>
	/// <param name="str">文字列</param>
	/// <returns>岩タイプ</returns>
	static RockType StringToRockType(const std::string& str);

private:
	/// <summary>
	/// 岩タイプに応じたモデル名を取得
	/// </summary>
	/// <param name="rockType">岩のタイプ</param>
	/// <returns>モデル名</returns>
	std::string GetModelNameFromType(RockType rockType);

	// 岩のタイプ
	RockType rockType_;
};