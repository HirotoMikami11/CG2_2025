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

void CreateSpriteVertexData(VertexData vertexDataSprite[], Vector2 center, Vector2 radius)
{
	//Spriteを表示するための四頂点
	//左下
	vertexDataSprite[0].position = { center.x - radius.x,center.y + radius.y,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	//左上
	vertexDataSprite[1].position = { center.x - radius.x,center.y - radius.y,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	//右下
	vertexDataSprite[2].position = { center.x + radius.x,center.y + radius.y,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };

	//右上
	vertexDataSprite[3].position = { center.x + radius.x,center.y - radius.y,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };


	for (int i = 0; i < 4; i++)
	{
		vertexDataSprite[i].normal = { 0.0f,0.0f,-1.0f };
	}
}



/// <summary>
/// 三角形リスト方式のVetrtexDataを作成する関数
/// </summary>
//void CreateSphereVertexData(VertexData vertexData[]) {
//	const uint32_t kSubdivision = 16;							//分割数
//	const float kLonEvery = (2 * float(M_PI)) / kSubdivision;	//経度分割1つ分の角度
//	const float kLatEvery = float(M_PI) / kSubdivision;			//緯度分割1つ分の角度
//
//	//緯度の方向に分割　-π/2　~π/2
//	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
//		float lat = (-float(M_PI) / 2.0f) + kLatEvery * latIndex;	//現在の緯度(θ)
//
//		//経度の方向に分割 0 ~ 2π
//		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
//
//			float lon = lonIndex * kLonEvery;
//			///データを書き込む最初の場所
//			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
//
//			//頂点にデータを入力する。基準点a
//			vertexData[start].position.x = cos(lat) * cos(lon);
//			vertexData[start].position.y = sin(lat);
//			vertexData[start].position.z = cos(lat) * sin(lon);
//			vertexData[start].position.w = 1.0f;
//			vertexData[start].texcoord.x = float(lonIndex) / float(kSubdivision);
//			vertexData[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
//			vertexData[start].normal = {
//			vertexData[start].position.x ,
//			vertexData[start].position.y,
//			vertexData[start].position.z
//			};
//
//			//頂点にデータを入力する。基準点b
//			vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
//			vertexData[start + 1].position.y = sin(lat + kLatEvery);
//			vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
//			vertexData[start + 1].position.w = 1.0f;
//			vertexData[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
//			vertexData[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
//			vertexData[start + 1].normal = {
//			vertexData[start + 1].position.x ,
//			vertexData[start + 1].position.y,
//			vertexData[start + 1].position.z
//			};
//
//			//頂点にデータを入力する。基準点c
//			vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
//			vertexData[start + 2].position.y = sin(lat);
//			vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
//			vertexData[start + 2].position.w = 1.0f;
//			vertexData[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
//			vertexData[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
//			vertexData[start + 2].normal = {
//			vertexData[start + 2].position.x ,
//			vertexData[start + 2].position.y,
//			vertexData[start + 2].position.z
//			};
//			//頂点にデータを入力する。基準点b
//			vertexData[start + 3].position.x = vertexData[start + 1].position.x;
//			vertexData[start + 3].position.y = vertexData[start + 1].position.y;
//			vertexData[start + 3].position.z = vertexData[start + 1].position.z;
//			vertexData[start + 3].position.w = 1.0f;
//			vertexData[start + 3].texcoord.x = vertexData[start + 1].texcoord.x;
//			vertexData[start + 3].texcoord.y = vertexData[start + 1].texcoord.y;
//			vertexData[start + 3].normal = {
//			vertexData[start + 3].position.x ,
//			vertexData[start + 3].position.y,
//			vertexData[start + 3].position.z
//			};
//			//頂点にデータを入力する。基準点d
//			vertexData[start + 4].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
//			vertexData[start + 4].position.y = sin(lat + kLatEvery);
//			vertexData[start + 4].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
//			vertexData[start + 4].position.w = 1.0f;
//			vertexData[start + 4].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
//			vertexData[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
//			vertexData[start + 4].normal = {
//			vertexData[start + 4].position.x ,
//			vertexData[start + 4].position.y,
//			vertexData[start + 4].position.z
//			};
//			//頂点にデータを入力する。基準点c
//			vertexData[start + 5].position.x = vertexData[start + 2].position.x;
//			vertexData[start + 5].position.y = vertexData[start + 2].position.y;
//			vertexData[start + 5].position.z = vertexData[start + 2].position.z;
//			vertexData[start + 5].position.w = 1.0f;
//			vertexData[start + 5].texcoord.x = vertexData[start + 2].texcoord.x;
//			vertexData[start + 5].texcoord.y = vertexData[start + 2].texcoord.y;
//			vertexData[start + 5].normal = {
//			vertexData[start + 5].position.x ,
//			vertexData[start + 5].position.y,
//			vertexData[start + 5].position.z
//			};
//		}
//	}
//}

void CreateSphereVertexData(VertexData vertexData[])
{
	const uint32_t kSubdivision = 16; // 分割数
	const float kLonEvery = (2 * float(M_PI)) / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision; // 緯度分割1つ分の角度
	uint32_t vertexCount = 0;


	// 頂点データの作成
	// (kSubdivision+1) × (kSubdivision+1) の格子状の頂点を作成
	for (uint32_t latIndex = 0; latIndex <= kSubdivision; ++latIndex) {
		float lat = (-float(M_PI) / 2.0f) + kLatEvery * latIndex; // 現在の緯度(θ)
		for (uint32_t lonIndex = 0; lonIndex <= kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度(φ)

			// 頂点位置の計算
			vertexData[vertexCount].position.x = cos(lat) * cos(lon);
			vertexData[vertexCount].position.y = sin(lat);
			vertexData[vertexCount].position.z = cos(lat) * sin(lon);
			vertexData[vertexCount].position.w = 1.0f;

			// テクスチャ座標の計算
			vertexData[vertexCount].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[vertexCount].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);

			// 法線は頂点位置と同じ（正規化された位置ベクトル）
			vertexData[vertexCount].normal.x = vertexData[vertexCount].position.x;
			vertexData[vertexCount].normal.y = vertexData[vertexCount].position.y;
			vertexData[vertexCount].normal.z = vertexData[vertexCount].position.z;

			vertexCount++;
		}
	}
}

void CreateSphereIndexData(uint32_t indexData[])
{
	const uint32_t kSubdivision = 16; // 分割数
	const float kLonEvery = (2 * float(M_PI)) / kSubdivision; // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision; // 緯度分割1つ分の角度
	uint32_t indexCount = 0;
	uint32_t maxIndex = kSubdivision * kSubdivision * 6;//三角リスト方式の頂点数と同じ数

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			// 各格子点のインデックスを計算
			uint32_t currentRow = latIndex * (kSubdivision + 1);
			uint32_t nextRow = (latIndex + 1) * (kSubdivision + 1);

			uint32_t currentIndex = currentRow + lonIndex;
			uint32_t rightIndex = currentRow + lonIndex + 1;
			uint32_t bottomIndex = nextRow + lonIndex;
			uint32_t bottomRightIndex = nextRow + lonIndex + 1;

			// 1つ目の三角形（左上、左下、右上）
			indexData[indexCount++] = currentIndex;
			indexData[indexCount++] = bottomIndex;
			indexData[indexCount++] = rightIndex;

			// 2つ目の三角形（右上、左下、右下）
			indexData[indexCount++] = rightIndex;
			indexData[indexCount++] = bottomIndex;
			indexData[indexCount++] = bottomRightIndex;
		}
	}

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

void UpdateUVTransform(const Vector3Transform uvtransform, Material* materialData)
{
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvtransform.scale);
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvtransform.rotate.z));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix(uvtransform.translate));
	materialData->uvTransform = uvTransformMatrix;
}

float RandomFloat(float min, float max) {
	static std::random_device rd;
	static std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(mt);
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
		Vector3 startNdcPos = Transform(startPos, startWorldViewProjectionMatrix);
		Vector3 startScreenPos = Transform(startNdcPos, viewportMatrix);

		Matrix4x4 endWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 endWorldViewProjectionMatrix = Matrix4x4Multiply(endWorldMatrix, viewProjectionMatrix);
		Vector3 endNdcPos = Transform(endPos, endWorldViewProjectionMatrix);
		Vector3 endScreenPos = Transform(endNdcPos, viewportMatrix);

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
		Vector3 startNdcPos = Transform(startPos, startWorldViewProjectionMatrix);
		Vector3 startScreenPos = Transform(startNdcPos, viewportMatrix);

		Matrix4x4 endWorldMatrix = MakeAffineMatrix({ 1,1,1 }, { 0,0,0 }, { 0,0,0 });
		Matrix4x4 endWorldViewProjectionMatrix = Matrix4x4Multiply(endWorldMatrix, viewProjectionMatrix);
		Vector3 endNdcPos = Transform(endPos, endWorldViewProjectionMatrix);
		Vector3 endScreenPos = Transform(endNdcPos, viewportMatrix);

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
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 10;//分割数
	const float kLonEvery = (2 * float(M_PI)) / kSubdivision;		//経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivision;				//緯度分割1つ分の角度

	//緯度の方向に分割　-π/2　~π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = (-float(M_PI) / 2.0f) + kLatEvery * latIndex;	//現在の緯度(θ)
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
			Vector3 aNdcPos = Transform(a, viewProjectionMatrix);
			Vector3 aScreenPos = Transform(aNdcPos, viewportMatrix);

			Vector3 bNdcPos = Transform(b, viewProjectionMatrix);
			Vector3 bScreenPos = Transform(bNdcPos, viewportMatrix);

			Vector3 cNdcPos = Transform(c, viewProjectionMatrix);
			Vector3 cScreenPos = Transform(cNdcPos, viewportMatrix);

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
	Vector3 screenStart = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
	///描画
	/*Novice::DrawLine(
		static_cast<int>(screenStart.x),
		static_cast<int>(screenStart.y),
		static_cast<int>(screenEnd.x),
		static_cast<int>(screenEnd.y),
		color);*/


}