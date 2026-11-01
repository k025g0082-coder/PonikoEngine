#include "DemoFieldObject.h"
#include "DemoGimmick.h"

#include "ImGuiManager.h"

// ギミックの種類名
const char* DemoFieldObject::fieldObjectTypeName[(int)Type::count] = {
	"Block",
	"Gimmick"
};


//
void DemoFieldObject::Initialize()
{

	// なんかあれば

}
void DemoFieldObject::Update()
{

	model_->Update();

	aabb_.UpdatePosition(worldTransform.translation_);

	// SRTからviewMatrix生成して更新
	worldTransform.UpdateMatrix();

}
void DemoFieldObject::Draw()
{
	/* worldTransform基に描画 */
	model_->Draw(worldTransform, *camera_);
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

	/* 種類名など */
	int v = static_cast<int>(type_);

	if (ImGui::DragInt("type_", &v, 0, static_cast<int>(Type::count)-1)) {

		type_ = static_cast<Type>(v);

		UpdateObjectTypeJson();
	}

	// タイプ名
	ImGui::Text("Type_: %s", DemoFieldObject::GetFieldObjectTypeName(type_));

	// 種類名
	switch (type_) {
	case Type::kBlock:
		ImGui::Text("subType_: %s", DemoFieldObject::GetFieldObjectTypeName(static_cast<DemoFieldObject::Type>(subType_)));
		break;
	case Type::kGimmick:
		ImGui::Text("subType_: %s", DemoGimmick::GetGimmickTypeName(static_cast<DemoGimmick::GimmickType>(subType_)));
		break;
	}

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

	ImGui::Text("test");

	DemoFieldObject::EndImGui();
#endif
}
