#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"
#include <cassert>


#include "Mesh.h"
#include "Material.h"






class Model
{
public:
	Model() = default;
	~Model() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon, const std::string& meshType,
		const std::string& directoryPath = "", const std::string& filename = "");

private:


	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;



	///material（bool型でlight用のセットか、defaultかどうかをinitilizeで決める。通常はｄｅｆａｕｌｔ）
	Material material_;
	///mesh("Triangle"とストリングで引数に入れたらinitilizeでcreateTriangleできるようにする（他の図形でも）)
	Mesh mesh_;

	///メッシュの中にあるloadObjをここに移動させる

	//このクラスの中で、ロードしたデータをマテリアルやmeshに渡す



};

