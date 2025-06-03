#include "MyFunction.h"

/*-----------------------------------------------------------------------*/
//
//								計算関数
//
/*-----------------------------------------------------------------------*/


Microsoft::WRL::ComPtr <ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr <ID3D12Device> device, size_t sizeInBytes)
{
	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;//リソースサイズ
	//バッファの場合はこれらは1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際にリソースを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)
	);
	assert(SUCCEEDED(hr));


	return Resource;
}

//	正射影ベクトルを求める関数
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	Vector3 project = Vector3Multiply(Vector3Normalize(v2), Vector3Dot(v1, Vector3Normalize(v2)));
	return project;
}
//　最近接点を求める関数
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 project = Project(Vector3Subtract(point, segment.origin), segment.diff);
	Vector3 closestPoint = Vector3Add(segment.origin, project);

	return closestPoint;
}

void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, TransformationMatrix* matrixData) {
	matrixData->World = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix = Matrix4x4Multiply(matrixData->World, viewProjectionMatrix);

	matrixData->WVP = worldViewProjectionMatrix;

}





/*-----------------------------------------------------------------------*/
//
//								描画関数
//
/*-----------------------------------------------------------------------*/

//グリッド線を描画する関数

/// <summary>
/// グリッド線を描画する関数
/// </summary>
/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
/// <param name="viewportMatrix">ビューポート</param>
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;									//Gridの半分の幅
	const uint32_t kSubdivision = 16;									//分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);	//1つ分の長さ

	//奥から手前に線を順々（左→右）に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; xIndex++) {
		//ワールド座標上の始点と終点を求める
		//左から一本ずつ引いていく
		float pos_x = -kGridHalfWidth + (xIndex * kGridEvery);
		Vector3 startPos = { pos_x,0,-kGridHalfWidth };
		Vector3 endPos = { pos_x,0,kGridHalfWidth };
		//スクリーン座標系まで変換する
		Matrix4x4 startWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 startWorldViewProjectionMatrix = Matrix4x4Multiply(startWorldMatrix, viewProjectionMatrix);
		Vector3 startNdcPos = Matrix4x4Transform(startPos, startWorldViewProjectionMatrix);
		Vector3 startScreenPos = Matrix4x4Transform(startNdcPos, viewportMatrix);

		Matrix4x4 endWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 endWorldViewProjectionMatrix = Matrix4x4Multiply(endWorldMatrix, viewProjectionMatrix);
		Vector3 endNdcPos = Matrix4x4Transform(endPos, endWorldViewProjectionMatrix);
		Vector3 endScreenPos = Matrix4x4Transform(endNdcPos, viewportMatrix);

		//変換した座標を使って表示
		//Novice::DrawLine(
		//	static_cast<int>(startScreenPos.x),
		//	static_cast<int>(startScreenPos.y),
		//	static_cast<int>(endScreenPos.x),
		//	static_cast<int>(endScreenPos.y),
		//	(xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);//原点だけ黒、それ以外は灰色
	}


	//左から右に線を順々（奥→手前）に引いていく
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; zIndex++) {
		//ワールド座標上の始点と終点を求める
		//左から一本ずつ引いていく
		float pos_z = -kGridHalfWidth + (zIndex * kGridEvery);
		Vector3 startPos = { -kGridHalfWidth,0,pos_z };
		Vector3 endPos = { kGridHalfWidth,0,pos_z };
		//スクリーン座標系まで変換する
		Matrix4x4 startWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 startWorldViewProjectionMatrix = Matrix4x4Multiply(startWorldMatrix, viewProjectionMatrix);
		Vector3 startNdcPos = Matrix4x4Transform(startPos, startWorldViewProjectionMatrix);
		Vector3 startScreenPos = Matrix4x4Transform(startNdcPos, viewportMatrix);

		Matrix4x4 endWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 endWorldViewProjectionMatrix = Matrix4x4Multiply(endWorldMatrix, viewProjectionMatrix);
		Vector3 endNdcPos = Matrix4x4Transform(endPos, endWorldViewProjectionMatrix);
		Vector3 endScreenPos = Matrix4x4Transform(endNdcPos, viewportMatrix);

		//変換した座標を使って表示
		//Novice::DrawLine(
		//	static_cast<int>(startScreenPos.x),
		//	static_cast<int>(startScreenPos.y),
		//	static_cast<int>(endScreenPos.x),
		//	static_cast<int>(endScreenPos.y),
		//	(zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);//原点だけ黒、それ以外は灰色
	}

}






/// <summary>
/// 球体を描画する関数
/// </summary>
/// <param name="sphere">球体</param>
/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
/// <param name="viewportMatrix">ビューポート</param>
/// <param name="color">色</param>
void DrawSphere(const SphereMath& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 10;//分割数
	const float kLonEvery = (2 * std::numbers::pi_v<float>) / kSubdivision;		//経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / kSubdivision;				//緯度分割1つ分の角度

	//緯度の方向に分割　-π/2　~π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = (-std::numbers::pi_v<float> / 2.0f) + kLatEvery * latIndex;	//現在の緯度(θ)
		//経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;						//現在の経度（φ）
			//world座標系でのa,b,cを求める
			Vector3 a = {
			cos(lat) * cos(lon),
			sin(lat),
			cos(lat) * sin(lon)
			};
			Vector3 b = {
			cos(lat + kLatEvery) * cos(lon),
			sin(lat + kLatEvery),
			cos(lat + kLatEvery) * sin(lon)
			};
			Vector3 c = {
			cos(lat) * cos(lon + kLonEvery),
			sin(lat),
			cos(lat) * sin(lon + kLonEvery)
			};

			//球体の中心座標と半径を加味して、a,b,cをワールド座標に変換
			a = Vector3Add(sphere.center, Vector3Multiply(a, sphere.radius));
			b = Vector3Add(sphere.center, Vector3Multiply(b, sphere.radius));
			c = Vector3Add(sphere.center, Vector3Multiply(c, sphere.radius));

			//a,b,cをスクリーン座標に変換
			Vector3 aNdcPos = Matrix4x4Transform(a, viewProjectionMatrix);
			Vector3 aScreenPos = Matrix4x4Transform(aNdcPos, viewportMatrix);

			Vector3 bNdcPos = Matrix4x4Transform(b, viewProjectionMatrix);
			Vector3 bScreenPos = Matrix4x4Transform(bNdcPos, viewportMatrix);

			Vector3 cNdcPos = Matrix4x4Transform(c, viewProjectionMatrix);
			Vector3 cScreenPos = Matrix4x4Transform(cNdcPos, viewportMatrix);

			//ab,bcで線を引く
			//Novice::DrawLine(
			//	static_cast<int>(aScreenPos.x),
			//	static_cast<int>(aScreenPos.y),
			//	static_cast<int>(bScreenPos.x),
			//	static_cast<int>(bScreenPos.y),
			//	color);
			//Novice::DrawLine(
			//	static_cast<int>(aScreenPos.x),
			//	static_cast<int>(aScreenPos.y),
			//	static_cast<int>(cScreenPos.x),
			//	static_cast<int>(cScreenPos.y),
			//	color);

		}

	}

}


/// <summary>
/// 線分を描画する関数
/// </summary>
/// <param name="segment">線分</param>
/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
/// <param name="viewportMatrix">ビューポート</param>
/// <param name="color">色</param>
void DrawLine(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	///線分の両端を求める
	Vector3 start = segment.origin;
	Vector3 end = Vector3Add(segment.origin, segment.diff);
	///スクリーン座標に変換
	Vector3 screenStart = Matrix4x4Transform(Matrix4x4Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Matrix4x4Transform(Matrix4x4Transform(end, viewProjectionMatrix), viewportMatrix);
	///描画
	/*Novice::DrawLine(
		static_cast<int>(screenStart.x),
		static_cast<int>(screenStart.y),
		static_cast<int>(screenEnd.x),
		static_cast<int>(screenEnd.y),
		color);*/


}