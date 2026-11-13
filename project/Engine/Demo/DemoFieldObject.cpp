#include "DemoFieldObject.h"
#include "DemoGimmick.h"

#include "ImGuiManager.h"

#include "DemoPlayer.h"
#include "DemoGimmick.h"
#include "DemoFieldItem.h"

#include "Collision.h"


// フィールドの種類名
const char* DemoFieldObject::fieldObjectTypeName[(int)Type::count] = {
	"null",

	"Player",

	"Block",
	"Gimmick",
	"Item",

};


//
void DemoFieldObject::Initialize()
{

	// なんかあれば

}
void DemoFieldObject::Update()
{

	GameObject::Update();

}
void DemoFieldObject::Draw()
{

	GameObject::Draw();

}

void DemoFieldObject::Collision(DemoFieldObject* a, DemoFieldObject* b)
{
	// どっちもTrueなら押し戻す(ブロック判定)
	bool push = a->isCollisionBlock_ && b->isCollisionBlock_;

	CollisionResult result = ResolveAABBCollision( a->GetAABB(), a->GetWorldTransform().translation_, b->GetAABB(),push);

	// 接触していれば
	if (result.isCollision) {
		a->onCollision(b, result);
		b->onCollision(a, result);
	}

}

void DemoFieldObject::ImGuiView()
{
	BeginImGui();
	EndImGui();
}

void DemoFieldObject::BeginImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("FieldObject", &showImGuiView);

	if (ImGui::ColorPicker4("modelColor", &model_->GetModelColor().x)) {
		hierarchy->modelColor_ = model_->GetModelColor();
	}

	/* 種類名など */
	int v = static_cast<int>(type_);

	if (ImGui::DragInt("type_", &v, 0, static_cast<int>(Type::count)-1)) {

		type_ = static_cast<Type>(v);

		UpdateObjectTypeJson();
	}

	// サブ
	int v2 = static_cast<int>(subType_);

	if (ImGui::DragInt("subType_", &v2, 0,0)) {

		subType_ = static_cast<uint32_t>(v2);

		UpdateObjectTypeJson();
	}


	// タイプ名
	ImGui::Text("Type_:    %s", DemoFieldObject::GetFieldObjectTypeName(type_));

	// 種類名
	switch (type_) {
	case Type::kBlock:
		ImGui::Text("subType_: %s", DemoFieldObject::GetFieldObjectTypeName(static_cast<DemoFieldObject::Type>(subType_)));
		break;
	case Type::kGimmick:
		ImGui::Text("subType_: %s", DemoGimmick::GetGimmickTypeName(static_cast<DemoGimmick::GimmickType>(subType_)));
		break;
	case Type::kItem:
		ImGui::Text("subType_: %s", DemoFieldItem::GetItemTypeName(static_cast<DemoFieldItem::ItemType>(subType_)));
		break;
	}


	ImGui::Text("UniqueID_: %d", uniqueID_);

	ImGui::Separator();

#endif
}
void DemoFieldObject::EndImGui()
{
#ifdef USE_IMGUI
	ImGui::End();
#endif
}

void DemoFieldObject::UpdateObjectTypeJson()
{

	if (json_ == nullptr) return;
	(*json_)["objectType"] = static_cast<uint32_t>(type_);
	(*json_)["subObjectType"] = subType_;

}



void DemoBlock::Initialize()
{}
void DemoBlock::Update()
{
	DemoFieldObject::Update();
}
void DemoBlock::Draw()
{
	DemoFieldObject::Draw();
}

void DemoBlock::ImGuiView()
{
#ifdef USE_IMGUI
	DemoFieldObject::BeginImGui();

	ImGui::Text("Block");

	DemoFieldObject::EndImGui();
#endif
}
