#include "MyMath/MyFunction.h"

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
	Vector3 project = Multiply(Normalize(v2), Dot(v1, Normalize(v2)));
	return project;
}

//　最近接点を求める関数
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 project = Project(Subtract(point, segment.origin), segment.diff);
	Vector3 closestPoint = Add(segment.origin, project);

	return closestPoint;
}

void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, TransformationMatrix* matrixData) {
	matrixData->World = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix = Matrix4x4Multiply(matrixData->World, viewProjectionMatrix);

	matrixData->WVP = worldViewProjectionMatrix;

}

/*-----------------------------------------------------------------------*/
//
//								当たり判定
//
/*-----------------------------------------------------------------------*/

//　球と球の衝突判定
bool IsCollision(const SphereMath& SphereMath1, const SphereMath& SphereMath2) {
	bool isCollision = false;

	//2つの球の中心転換の距離を求める
	float distance = Length(Subtract(SphereMath1.center, SphereMath2.center));

	//半径の合計より短ければ衝突
	if (distance <= SphereMath1.radius + SphereMath2.radius) {
		isCollision = true;
	}

	return isCollision;
}

//球と平面の衝突判定
bool IsCollision(const SphereMath& SphereMath, const Plane& plane) {

	//1.点と平面との距離
	//そのままだと符号付き距離になってしまうので、絶対値(abs)を取る
	float distance = std::abs(Dot(plane.normal, SphereMath.center) - plane.distance);

	//2.1の距離<=球の半径なら衝突
	if (distance <= SphereMath.radius) {
		return true;
	}

	return false;

}


//線分と平面の衝突判定
bool IsCollision(const Segment& segment, const Plane& plane) {
	//衝突しているかどうか
	bool isCollision = false;

	//垂直判定を行うため、法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);
	//垂直=平行であるので、衝突していない
	if (dot == 0.0f) {
		return false;
	}

	//tを求める
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;

	//tの値と線の種類によって衝突しているかを判定する
	//線分なので0~1
	if (t >= 0 && t <= 1) {
		isCollision = true;
	}

	return isCollision;
}

// 線分と平面の衝突点の座標を求める
Vector3 MakeCollisionPoint(const Segment& segment, const Plane& plane) {
	///衝突点
	Vector3 CollsionPoint;

	//平面と線の衝突判定と同様
	float dot = Dot(segment.diff, plane.normal);
	assert(dot != 0.0f);
	float t = (plane.distance - (Dot(segment.origin, plane.normal))) / dot;

	//衝突点を求める
	//p=origin+tb
	CollsionPoint = Add(segment.origin, Multiply(segment.diff,t));
	return CollsionPoint;
}


// 三角形と線分の衝突判定
bool IsCollision(const MultiplyMath& MultiplyMath, const Segment& segment) {
	//衝突しているかどうか
	bool isCollision = false;

	///1.線と三角形の存在する平面との衝突判定を行う
	//三角形の中心座標求める
	Vector3 MultiplyMathCenter = {
		(MultiplyMath.vertices[0].x + MultiplyMath.vertices[1].x + MultiplyMath.vertices[2].x) / 3,
		(MultiplyMath.vertices[0].y + MultiplyMath.vertices[1].y + MultiplyMath.vertices[2].y) / 3,
		(MultiplyMath.vertices[0].z + MultiplyMath.vertices[1].z + MultiplyMath.vertices[2].z) / 3
	};

	//平面を作成する
	Plane plane;
	//距離
	plane.distance = Distance(MultiplyMathCenter, { 0,0,0 });
	//法線
	plane.normal = Cross(Subtract(MultiplyMath.vertices[1], MultiplyMath.vertices[0]), Subtract(MultiplyMath.vertices[2], MultiplyMath.vertices[1]));


	if (IsCollision(segment, plane)) {


		///2.衝突していたら、衝突点が三角形の内側にあるのかを調べる
		//衝突点pを作成
		Vector3 p = MakeCollisionPoint(segment, plane);
		//衝突点と、三角形それぞれの辺で新たな三角形を作成する。(衝突点が[2]になるように)
		//a.各辺を結んだベクトル
		Vector3 v01 = Subtract(MultiplyMath.vertices[1], MultiplyMath.vertices[0]);
		Vector3 v12 = Subtract(MultiplyMath.vertices[2], MultiplyMath.vertices[1]);
		Vector3 v20 = Subtract(MultiplyMath.vertices[0], MultiplyMath.vertices[2]);
		//b.頂点と衝突点pを結んだベクトル
		Vector3 v1p = Subtract(p, MultiplyMath.vertices[1]);
		Vector3 v2p = Subtract(p, MultiplyMath.vertices[2]);
		Vector3 v0p = Subtract(p, MultiplyMath.vertices[0]);

		///法線ベクトルと同じ方向を向いているか見るため、aとbで外積を行う
		Vector3 cross01 = Cross(v01, v1p);
		Vector3 cross12 = Cross(v12, v2p);
		Vector3 cross20 = Cross(v20, v0p);

		///全ての小さな三角形の外積と法線が同じ方向を向いていたら、衝突している
			//全ての小さい三角形のクロス積と法線が同じ方法を向いていたら衝突
		if (
			Dot(cross01, plane.normal) >= 0.0f &&
			Dot(cross12, plane.normal) >= 0.0f &&
			Dot(cross20, plane.normal) >= 0.0f
			) {
			isCollision = true;

		}
	}

	return isCollision;

}


/// <summary>
/// AABBとAABBの衝突判定
/// </summary>
/// <param name="aabb1"></param>
/// <param name="aabb2"></param>
/// <returns></returns>
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {

	///衝突判定
	if (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&	//x軸の衝突
		aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&	//y軸の衝突
		aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z	//z軸の衝突
		) {
		return true;
	}
	return false;
}


bool IsCollision(const AABB& aabb, SphereMath& SphereMath) {
	bool isCollision = false;

	//球の中心座標がAABBの[min,max]内にclampすれば、それが最近接点になる
	Vector3 closestPoint{
		std::clamp(SphereMath.center.x,aabb.min.x,aabb.max.x),
		std::clamp(SphereMath.center.y,aabb.min.y,aabb.max.y),
		std::clamp(SphereMath.center.z,aabb.min.z,aabb.max.z),
	};

	//最近接点と球の中心との距離を求める
	float distance = Length(Subtract(closestPoint, SphereMath.center));
	//距離が半径より小さければ衝突
	if (distance <= SphereMath.radius) {
		isCollision = true;
	}
	return isCollision;

}

/// <summary>
/// 最大最小を正しくする関数
/// </summary>
/// <param name="aabb"></param>
void FixAABBMinMax(AABB& aabb) {

	aabb.min.x = (std::min)(aabb.min.x, aabb.max.x);
	aabb.max.x = (std::max)(aabb.min.x, aabb.max.x);

	aabb.min.y = (std::min)(aabb.min.y, aabb.max.y);
	aabb.max.y = (std::max)(aabb.min.y, aabb.max.y);

	aabb.min.z = (std::min)(aabb.min.z, aabb.max.z);
	aabb.max.z = (std::max)(aabb.min.z, aabb.max.z);
}


/// <summary>
/// AABBと線分の衝突判定
/// </summary>
/// <param name="aabb"></param>
/// <param name="segment"></param>
/// <returns></returns>
bool IsCollision(const AABB& aabb, const Segment& segment) {


	// 0除算にならないように、diffが0の時はそれが中にあるかどうか確認する
	if (segment.diff.x == 0.0f && segment.diff.y == 0.0f && segment.diff.z == 0.0f) {
		// 点の場合：始点がAABB内にあれば衝突、なければ衝突してない
		return (segment.origin.x >= aabb.min.x && segment.origin.x <= aabb.max.x &&
			segment.origin.y >= aabb.min.y && segment.origin.y <= aabb.max.y &&
			segment.origin.z >= aabb.min.z && segment.origin.z <= aabb.max.z);
	}


	//線分なので、0~1の範囲でtを求める
	float tmin = 0.0f;  // 線分の開始
	float tmax = 1.0f;  // 線分の終了



	//それぞれの軸の判定を行う

	/// X軸の処理

	// diffが0でない場合、AABBとの交差を計算
	if (segment.diff.x != 0.0f) {
		float txmin = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float txmax = (aabb.max.x - segment.origin.x) / segment.diff.x;

		// Near/Farを正しい順序にする
		float tNearX = min(txmin, txmax);
		float tFarX = max(txmin, txmax);

		// 全体の範囲を更新
		tmin = max(tmin, tNearX);
		tmax = min(tmax, tFarX);

		if (tmin > tmax) {
			return false;// tminがtmaxより大きい場合、衝突していない
		}
	} else {
		//diffが0の場合、線分がX軸に平行
		// 始点のX座標がAABBの範囲内なら衝突
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) {
			return false;
		}
	}


	/// Y軸の処理

	// diffが0でない場合、AABBとの交差を計算
	if (segment.diff.y != 0.0f) {
		float tymin = (aabb.min.y - segment.origin.y) / segment.diff.y;
		float tymax = (aabb.max.y - segment.origin.y) / segment.diff.y;

		// Near/Farを正しい順序にする
		float tNearY = min(tymin, tymax);
		float tFarY = max(tymin, tymax);
		// 全体の範囲を更新
		tmin = max(tmin, tNearY);
		tmax = min(tmax, tFarY);

		if (tmin > tmax) {
			return false;// tminがtmaxより大きい場合、衝突していない
		}
	} else {
		//diffが0の場合、線分がY軸に平行
		// 始点のY座標がAABBの範囲内なら衝突
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y) {
			return false;
		}
	}


	/// Z軸の処理

	// diffが0でない場合、AABBとの交差を計算
	if (segment.diff.z != 0.0f) {
		float tzmin = (aabb.min.z - segment.origin.z) / segment.diff.z;
		float tzmax = (aabb.max.z - segment.origin.z) / segment.diff.z;

		// Near/Farを正しい順序にする
		float tNearZ = min(tzmin, tzmax);
		float tFarZ = max(tzmin, tzmax);

		// 全体の範囲を更新
		tmin = max(tmin, tNearZ);
		tmax = min(tmax, tFarZ);

		if (tmin > tmax) {
			return false;	// tminがtmaxより大きい場合、衝突していない
		}
	} else {
		//diffが0の場合、線分がZ軸に平行
		// 始点のZ座標がAABBの範囲内なら衝突
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z) {
			return false;
		}
	}

	// tmin <= tmax かつ 0 <= tmax かつ tmin <= 1 なら衝突
	if (tmin <= tmax && tmax >= 0.0f && tmin <= 1.0f) {
		return true;
	}

	return false;
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
			a = Add(sphere.center, Multiply(a, sphere.radius));
			b = Add(sphere.center, Multiply(b, sphere.radius));
			c = Add(sphere.center, Multiply(c, sphere.radius));

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
	Vector3 end = Add(segment.origin, segment.diff);
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