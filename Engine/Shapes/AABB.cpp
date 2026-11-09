#include "AABB.h"
#include "ImGuiManager.h"

void AABB::UpdatePosition(const Vector3& position)
{

	min = Vector3{ -width_ * 0.5f, -height_ * 0.5f, -depth_ * 0.5f } + position;
	max = Vector3{ width_ * 0.5f,  height_ * 0.5f,  depth_ * 0.5f } + position;

}

void AABB::ImGuiView(bool viewMinMax) {

#ifdef USE_IMGUI

	ImGui::SeparatorText("AABB Properties");

	if (!viewMinMax) {
		ImGui::DragFloat3("min   : ", &min.x, 0.1f);
		ImGui::DragFloat3("max   : ", &max.x, 0.1f);
	}

	ImGui::DragFloat("width ", &width_, 0.1f,0);
	ImGui::DragFloat("height", &height_, 0.1f,0);
	ImGui::DragFloat("depth ", &depth_, 0.1f,0);

	ImGui::Separator();

#endif

}