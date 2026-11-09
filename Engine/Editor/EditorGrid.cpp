#include "EditorGrid.h"

#include "DebugLine.h"
#include "ImGuiManager.h"

float EditorGrid::gridWidth_ = 1.0f;
float EditorGrid::gridHeight_ = 1.0f;
float EditorGrid::gridDepth_ = 1.0f;

void EditorGrid::Update()
{

	// オイラー角->回転行列->ベクトル変換
	rotationMatrix = MakeRotationMatrix(rotation_);


	// 初期状態の法線（水平な平面の上方向）
	Vector3 normal = { 0.0f, 1.0f, 0.0f };


	// 判定用のplaneもグリッドに同期させる

	plane_.width = gridWidth_ * static_cast<float>(horizontal_);
	plane_.height = gridHeight_ * static_cast<float>(vertical_);


	// グリッドの中心（ローカル座標）
	Vector3 localCenter = {
		gridWidth_ * horizontal_ * 0.5f,
		0.0f,
		gridHeight_ * vertical_ * 0.5f
	};

	Vector3 offset;
	//if (isCenter_) {

	//		offset = {
	//			-gridWidth_ / 2.0f,
	//			-gridHeight_ / 2.0f,
	//			0
	//		};

	//}

	// 回転させてワールド座標へ
	plane_.point =
		TransformNormal(localCenter, rotationMatrix) + (translation_ + offset);

	// 法線を回転させる
	plane_.normal = Normalize(
		TransformNormal(normal, rotationMatrix)
	);


	plane_.width = gridWidth_ * static_cast<float>(horizontal_);
	plane_.height = gridHeight_ * static_cast<float>(vertical_);

}

void EditorGrid::AddLine()
{

	if (!isSnap_) return;

	/* 升目描画 */
	const Vector4 color = { 0.0f, 1.0f, 0.0f, 1.0f };


	Vector3 offset = {
		0.0f,
		0.0f,
		0.0f
	};
	//if (isCenter_) {
	//	offset = {
	//		-gridWidth_ / 2.0f,
	//		
	//		0,-gridHeight_ / 2.0f,
	//	};
	//}

	// ローカル座標からワールド座標へ変換するラムÐ式
	auto TransformGridPoint = [&](const Vector3& point) {

		Vector3 local = point + offset;

		// ローカル座標を回転させる
		return TransformNormal(local, rotationMatrix) + translation_;
		};

	// X方向の線
	for (int x = 0; x <= horizontal_; ++x) {
		float posX = x * gridWidth_;

		Vector3 start = { posX, 0.0f, 0.0f };
		Vector3 end = { posX, 0.0f, vertical_ * gridHeight_ };

		DebugLine::AddLine(
			TransformGridPoint(start),
			TransformGridPoint(end),
			color
		);
	}

	// Z方向の線
	for (int z = 0; z <= vertical_; ++z) {
		float posZ = z * gridHeight_;

		Vector3 start = { 0.0f, 0.0f, posZ };
		Vector3 end = { horizontal_ * gridWidth_, 0.0f, posZ };

		DebugLine::AddLine(
			TransformGridPoint(start),
			TransformGridPoint(end),
			color
		);
	}

	// 判定用-平面描画　傾きはGridの傾きを回転行列として使う
	DebugLine::DrawPlane(plane_, rotation_, { 1.0f,0,0,1.0f });


}

void EditorGrid::ImGuiView()
{
#ifdef USE_IMGUI

	ImGui::DragInt("Grid-Vertical", &vertical_, 1, 1, 500);
	ImGui::DragInt("Grid-Horizontal", &horizontal_, 1, 1, 500);

	ImGui::DragFloat("Grid-width", &gridWidth_, 0.1f, 1.0f, 500.0f);
	ImGui::DragFloat("Grid-height", &gridHeight_, 0.1f, 1.0f, 500.0f);
	ImGui::DragFloat("Grid-Depth", &gridDepth_, 0.1f, 1.0f, 500.0f);

	ImGui::DragFloat3("Grid Translation", &translation_.x, 0.1f);
	ImGui::DragFloat3("plane point", &plane_.point.x, 0.1f);
	ImGui::Separator();

	ImGui::Checkbox("isSnap", &isSnap_);
	ImGui::Checkbox("isCenter", &isCenter_);

	ImGui::SeparatorText("RotateGrid");

	if (ImGui::Button("x")) {
		rotation_.x -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

	// 横に
	if (ImGui::Button("y")) {
		rotation_.y -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

	// 横に
	if (ImGui::Button("z")) {
		rotation_.z -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

#endif
}
Vector3  EditorGrid::translation_ = { 0,0,0 };
bool EditorGrid::isCenter_ = false;

Vector3  EditorGrid::snapGrid(Vector3 v) {

	Vector3 result;
	Vector3 offset;

	if (isCenter_) {
		offset = {
			-gridWidth_ / 2.0f,
			-gridHeight_ / 2.0f,
			-gridHeight_ / 2.0f
		};
	};

	result.x = std::floor(v.x / gridWidth_) * gridWidth_ + gridWidth_ / 2.0f,
		result.y = std::floor(v.y / gridHeight_) * gridHeight_ + gridHeight_ / 2.0f;
	result.z = std::floor(v.z / gridDepth_) * gridDepth_ + gridDepth_ / 2.0f;

	result = result + offset;

	// マスの大きさで割って配置
	return result;

}