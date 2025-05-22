//#include "GameObject.h"
//
//void GameObject::CreateResources(DirectXCommon* directX) {
//	// マテリアルリソース作成
//	materialResource = CreateBufferResource(directX->GetDevice(), sizeof(Material));
//	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
//	*materialData = material;
//
//	// トランスフォームリソース作成
//	transformResource = CreateBufferResource(directX->GetDevice(), sizeof(TransformationMatrix));
//	transformResource->Map(0, nullptr, reinterpret_cast<void**>(&transformData));
//	transformData->WVP = MakeIdentity4x4();
//	transformData->World = MakeIdentity4x4();
//}
//
//void GameObject::UpdateTransform(const Matrix4x4& viewProjection) {
//	UpdateMatrix4x4(transform, viewProjection, transformData);
//}