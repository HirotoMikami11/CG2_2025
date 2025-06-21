#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "../DirectXCommon.h"
#include "MyMath/MyFunction.h"


/// <summary>
/// ライトクラス
/// </summary>
class Light
{
public:
	/// <summary>
	/// ライトの種類
	/// </summary>
	enum class Type {
		DIRECTIONAL,	// 平行光源
		//以下未実装
		POINT,			//点光源
		SPOT			//スポットライト
	};

	Light() = default;
	~Light() = default;

	/// <summary>
	/// ライトを初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="type">ライトの種類</param>
	/// 現状は何も設定しないで平行光源にする
	void Initialize(DirectXCommon* dxCommon, Type type = Type::DIRECTIONAL);

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	void SetDefaultSettings();

	// Getter
	Vector4 GetColor() const { return lightData_->color; }
	Vector3 GetDirection() const { return lightData_->direction; }
	float GetIntensity() const { return lightData_->intensity; }
	ID3D12Resource* GetResource() const { return lightResource_.Get(); }
	Type GetType() const { return type_; }

	// Setter
	void SetColor(const Vector4& color) { lightData_->color = color; }
	void SetDirection(const Vector3& direction) { lightData_->direction = direction; }
	void SetIntensity(float intensity) { lightData_->intensity = intensity; }

	/// <summary>
	/// ImGui用の編集UI
	/// </summary>
	/// <param name="label">UIのラベル</param>
	void ImGui(const std::string& label);

private:
	Type type_ = Type::DIRECTIONAL;
	Microsoft::WRL::ComPtr<ID3D12Resource> lightResource_;
	DirectionalLight* lightData_ = nullptr;
};