#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <array>

/// <summary>
/// ブレンドモードの定義
/// </summary>
enum class BlendMode {
	None,			// ブレンドなし
	AlphaBlend,		// アルファブレンド
	Add,			// 加算合成
	Subtract,		// 減算合成
	Multiply,		// 乗算合成
	Screen          // スクリーン合成
};

/// <summary>
/// カリングモードの定義
/// </summary>
enum class CullMode {
	None,			// カリングなし（両面描画）
	Back,			// 背面カリング（通常）
	Front			// 前面カリング
};

/// <summary>
/// PSO作成用の設定を保持するクラス
/// パターンで簡単に設定可能
/// </summary>
class PSODescriptor {
public:
	/// <summary>
	/// シェーダー情報
	/// </summary>
	struct ShaderInfo {
		std::wstring filePath;					// シェーダーファイルパス
		std::wstring entryPoint = L"main";		// エントリーポイント名
		std::wstring target;					// シェーダーターゲット（"vs_6_0", "ps_6_0"など）
	};

	/// <summary>
	/// InputLayout要素（頂点レイアウト定義）
	/// </summary>
	struct InputElement {
		std::string semanticName;				// セマンティック名（POSITION, TEXCOORD等）
		uint32_t semanticIndex = 0;				// セマンティックインデックス
		DXGI_FORMAT format;						// データフォーマット
		uint32_t inputSlot = 0;					// 入力スロット
		uint32_t alignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT; // バイトオフセット
		D3D12_INPUT_CLASSIFICATION inputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // 頂点データかインスタンスデータか
		uint32_t instanceDataStepRate = 0;		// インスタンスデータのステップレート
	};

public:

	///		 ファクトリメソッド（よく使う設定のプリセット）			///

	/// <summary>
	/// 3Dオブジェクト用のデフォルト設定を作成
	/// </summary>
	static PSODescriptor Create3D();

	/// <summary>
	/// スプライト用のデフォルト設定を作成
	/// </summary>
	static PSODescriptor CreateSprite();

	/// <summary>
	/// 線分描画用のデフォルト設定を作成
	/// </summary>
	static PSODescriptor CreateLine();

	/// <summary>
	/// ポストエフェクト用のデフォルト設定を作成
	/// </summary>
	static PSODescriptor CreatePostEffect();

	/// <summary>
	/// 深度ありのポストエフェクト用のデフォルト設定を作成
	/// </summary>
	/// <returns></returns>
	static PSODescriptor CreatePostEffectWithDepth();

	/// <summary>
	/// カラーテクスチャのみのポストエフェクト用のデフォルト設定を作成
	/// </summary>
	/// <returns></returns>
	static PSODescriptor CreatePostEffectColorOnly();



	///			ビルダーパターンによる設定メソッド		///

	/// <summary>
	/// 頂点シェーダーを設定
	/// </summary>
	PSODescriptor& SetVertexShader(const std::wstring& path, const std::wstring& entryPoint = L"main");

	/// <summary>
	/// ピクセルシェーダーを設定
	/// </summary>
	PSODescriptor& SetPixelShader(const std::wstring& path, const std::wstring& entryPoint = L"main");

	/// <summary>
	/// ブレンドモードを設定
	/// </summary>
	PSODescriptor& SetBlendMode(BlendMode mode);

	/// <summary>
	/// カリングモードを設定
	/// </summary>
	PSODescriptor& SetCullMode(CullMode mode);

	/// <summary>
	/// 深度テストの有効/無効と比較関数を設定
	/// </summary>
	PSODescriptor& EnableDepth(bool enable, D3D12_COMPARISON_FUNC func = D3D12_COMPARISON_FUNC_LESS_EQUAL);

	/// <summary>
	/// 深度書き込みの有効/無効を設定
	/// </summary>
	PSODescriptor& EnableDepthWrite(bool enable);

	/// <summary>
	/// プリミティブトポロジータイプを設定
	/// </summary>
	PSODescriptor& SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

	/// <summary>
	/// InputLayout要素を追加
	/// </summary>
	PSODescriptor& AddInputElement(const InputElement& element);

	/// <summary>
	/// InputLayout要素をクリア
	/// </summary>
	PSODescriptor& ClearInputElements();

	/// <summary>
	/// レンダーターゲットのフォーマットを設定
	/// </summary>
	PSODescriptor& SetRenderTargetFormat(DXGI_FORMAT format);

	/// <summary>
	/// 深度ステンシルのフォーマットを設定
	/// </summary>
	PSODescriptor& SetDepthStencilFormat(DXGI_FORMAT format);

	///						設定取得メソッド						///

	const ShaderInfo& GetVertexShader() const { return vertexShader_; }
	const ShaderInfo& GetPixelShader() const { return pixelShader_; }

	/// <summary>
	/// D3D12_BLEND_DESCを生成
	/// </summary>
	D3D12_BLEND_DESC GetBlendDesc() const;

	/// <summary>
	/// D3D12_RASTERIZER_DESCを生成
	/// </summary>
	D3D12_RASTERIZER_DESC GetRasterizerDesc() const;

	/// <summary>
	/// D3D12_DEPTH_STENCIL_DESCを生成
	/// </summary>
	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc() const;

	/// <summary>
	/// D3D12_INPUT_ELEMENT_DESCの配列を生成
	/// </summary>
	std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputElementDescs() const;

	/// <summary>
	/// その他の設定を取得
	/// </summary>
	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType() const { return topologyType_; }
	DXGI_FORMAT GetRenderTargetFormat() const { return renderTargetFormat_; }
	DXGI_FORMAT GetDepthStencilFormat() const { return depthStencilFormat_; }
	size_t GetInputElementCount() const { return inputElements_.size(); }

private:
	// シェーダー情報
	ShaderInfo vertexShader_;
	ShaderInfo pixelShader_;

	// ブレンドとラスタライザ設定
	BlendMode blendMode_ = BlendMode::None;
	CullMode cullMode_ = CullMode::Back;
	D3D12_FILL_MODE fillMode_ = D3D12_FILL_MODE_SOLID;

	// 深度ステンシル設定
	bool depthEnable_ = true;
	bool depthWriteEnable_ = true;
	D3D12_COMPARISON_FUNC depthFunc_ = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// プリミティブトポロジー
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType_ = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// レンダーターゲットフォーマット
	DXGI_FORMAT renderTargetFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	DXGI_FORMAT depthStencilFormat_ = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// InputLayout要素
	std::vector<InputElement> inputElements_;

	// InputElement用の文字列を保持（D3D12_INPUT_ELEMENT_DESCが文字列ポインタを保持するため）
	mutable std::vector<std::string> semanticNames_;
};