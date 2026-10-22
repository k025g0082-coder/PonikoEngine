
#include "Editor.h"


#include "WinApp.h"

#include "Game.h"
#include "Scene.h"


#include "ImGuiManager.h"
#include <misc/cpp/imgui_stdlib.h>

#include "LightingManager.h"

#include "Input.h"

//#include "Particle.h"
//#include "EffectManager.h"

#include "DebugLine.h"

#include "Vector.h"
#include "Matrix.h"

#include <algorithm>
#include <numbers>
#include <memory>
#include <format>
#include <optional>


Editor* Editor::GetInstance() {
	static Editor instance;
	return &instance;
}

/* エディター初期化 */
void Editor::Initialize() {

	camera_.Initialize();

	currentSceneJsonPath = "resources/Scene/example.json";

	// 各パネルの初期化
	hierarchyPanel_.Initialize(currentSceneJsonPath,grid);

	assetsPanel_.Initialize();
	assetsPanel_.SetHierarchyPanel(hierarchyPanel_);



	effectSequencer_.mFrameMin = -100;
	effectSequencer_.mFrameMax = 1000;
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 0, 10, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 1, 20, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 3, 12, 60 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 2, 61, 90 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 4, 90, 99 });

}

/* エディター更新 */
void Editor::Update()
{

	/* 常に更新するパネルの内容 */
	hierarchyPanel_.Update();
	assetsPanel_.Update();

#ifdef USE_IMGUI

	static bool showProperty = false;
	static bool showHierarchy = false;
	static bool showLighting = false;
	static bool showEffectTimeline = false;
	static bool showAssets = false;

	ImVec2 mouse = ImGui::GetMousePos();

	float width = static_cast<float>(
		WinApp::GetInstance()->GetClientWidth());

	float height = static_cast<float>(
		WinApp::GetInstance()->GetClientHeight());

	bool insideScene =
		mouse.x >= 0.0f &&
		mouse.x <= width &&
		mouse.y >= 0.0f &&
		mouse.y <= height;

	/* ImGuiの枠にかさならないよう左クリックを押された際の処理 */
	if (insideScene &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
		!ImGui::GetIO().WantCaptureMouse &&
		!ImGuizmo::IsUsing()) {

		// スクリーンの大きさとスクリーン座標からから、レイ生成
		Camera::Ray ray = camera_.ScreenPointToRay( Vector2{ mouse.x,mouse.y },width,height );

		float distance = 0.0f;

		hierarchyPanel_.SetSelectedHierarchyModelIndex(std::nullopt);
		selectedModelWorldTransform_ = nullptr;

		//* モード別の処理　*//
		switch (currentModelHierarchyMode_) {


		//* 選択モード
		case ModelHierarchyMode::kSelectMode:

			// すべてのモデル
			for (int i = 0; i < hierarchyPanel_.GetHierarchyModels().size(); i++)
			{

				// ヒエラルキからワールドのポインタ取得！
				WorldTransform* world = static_cast<WorldTransform*>(hierarchyPanel_.GetHierarchyModels().at(i).transform_.get());

				// だいたいの中心
				float size = 1.0f;

				AABB aabb;
				aabb.min = Vector3{ -size, -size, -size } + world->translation_;
				aabb.max = Vector3{ size,  size,  size } + world->translation_;

				// モデルのAABBと判定
				if (camera_.IntersectRayAABB(ray, aabb, distance)) {

					hierarchyPanel_.SetSelectedHierarchyModelIndex(i);
					selectedModelWorldTransform_ = world;

					// ビューポートにモデルを追加
					//effectViewport_.AddModel(hierarchyModels_.at(i).object);

					// 選択中のモデルのview更新
					SelectModelUpdateView();

					break;
				}
			}
			break;

		//* 一気に配置モード
		case ModelHierarchyMode::kCreateMode:

			if (assetsPanel_.selectedHierarchyResource_.pathId_.empty()) break;

			// グリッドのplaneと判定
			std::optional<Vector3> hit = camera_.IntersectRayPlane(ray, grid.plane_);
			
			// 当たったら
			if (hit.has_value()) {

				// マスの大きさで割ったのを設定する
				Vector3 sub =  ImGuizmoGrid::snapGrid(hit.value());

				///* モデルのヒエラルキ追加 */
				Hierarchy<Model> h;

				h.type_ = HierarchyType::kModel; // タイプ
				h.id_ = assetsPanel_.selectedHierarchyResource_.pathId_;
				h.name_ = assetsPanel_.selectedHierarchyResource_.name_;

				// WorldTransform座標決定
				h.transform_ = std::make_unique<WorldTransform>();

				h.transform_->Initialize();
				h.transform_->translation_ = { sub.x,sub.y,sub.z };

				WorldTransform* world = static_cast<WorldTransform*>(h.transform_.get());
				
				// 位置更新
				world->UpdateMatrix();


				/* ヒエラルキ-モデル - 追加 */
				hierarchyPanel_.AddHierarchyModel(h);

			}

			break;

		}
	}

	/* ImGuiの枠にかさならないよう右クリックを押された際の処理 */
	if (insideScene &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
		!ImGui::GetIO().WantCaptureMouse &&
		!ImGuizmo::IsUsing()) {
	
		// スクリーンの大きさとスクリーン座標からから、レイ生成
		Camera::Ray ray = camera_.ScreenPointToRay(Vector2{ mouse.x,mouse.y }, width, height);

		float distance = 0.0f;


		//* モード別の処理　*//
		switch (currentModelHierarchyMode_) {

		//* 配置モード
		case ModelHierarchyMode::kCreateMode:

			if (assetsPanel_.selectedHierarchyResource_.pathId_.empty()) break;

			// グリッドのplaneと判定
			std::optional<Vector3> hit = camera_.IntersectRayPlane(ray, grid.plane_);

			// 当たったら
			if (hit.has_value()) {

				// すべてのモデル
				for (int i = 0; i < hierarchyPanel_.GetHierarchyModels().size(); i++)
				{

					// ヒエラルキからワールドのポインタ取得！
					WorldTransform* world = static_cast<WorldTransform*>(hierarchyPanel_.GetHierarchyModels().at(i).transform_.get());

					// だいたいの中心
					float size = 1.0f;

					AABB aabb;
					aabb.min = Vector3{ -size, -size, -size } + world->translation_;
					aabb.max = Vector3{ size,  size,  size } + world->translation_;

					// モデルのAABBと判定
					if (camera_.IntersectRayAABB(ray, aabb, distance)) {

						/* 当たったヒエラルキモデルを削除 */
						hierarchyPanel_.DeleteHierarchyModel(i);

						break;
					}
				}
			}

			break;

		}


	}


	// ImGuizmoのフレーム開始
	ImGuizmo::BeginFrame();

	

	ImGuizmo::Enable(true);

	// 3. カメラの投影モードを設定（透視投影の場合は false）
	ImGuizmo::SetOrthographic(false);

	float w = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	float h = static_cast<float>(WinApp::GetInstance()->GetClientHeight());

	ImGuizmo::SetRect(0.0f, 0.0f, w, h);

if (isEditorMode_) {

		// グリッド？ヤバ
		ImGuizmo::DrawGridCustom(
			&camera_.GetViewMatrix().m[0][0],
			&camera_.GetProjectionMatrix().m[0][0],
			&grid.worldMatrix.m[0][0],
			grid.gridSize,
			grid.squareSize,
			grid.kSubdivision
		);


		//DebugLine::GetInstance()->Draw(camera_);
	}

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	/*カメラ視点の回転 */

	/* ホイール操作 */
	if (Input::GetInstance()->GetMouseWheel() != 0) {
		if (selectedModelWorldTransform_) {
			
			selectedModelLength_ += static_cast<float>(Input::GetInstance()->GetMouseWheel()) * 0.075f;

			// 近いの遠いの防止
			selectedModelLength_ = std::clamp(selectedModelLength_, 10.0f, 100.0f);
			
			//  また例の処理
			//

			Vector3 targetPos = selectedModelWorldTransform_->translation_;

			// 1) View行列からカメラのワールド行列を復元
			Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

			// 2) ワールド座標を取り出す
			Vector3 camPos = {
				cameraWorld.m[3][0],
				cameraWorld.m[3][1],
				cameraWorld.m[3][2]
			};

			// 4) thetaとphi
			Vector3 offset = camPos - targetPos;
			selectedSphereTheta_ = std::asin(offset.y / selectedModelLength_);
			selectedSpherePhi_ = std::atan2(offset.z, offset.x);

			// 5) view更新！！
			camera_.SphericalCameraUpdate(
				targetPos,
				selectedModelLength_,
				selectedSphereTheta_,
				selectedSpherePhi_
			);


		}
	}

	/* ギズモによるカメラ回転 */
	if (selectedModelWorldTransform_) {

		// 現在のカメラView行列を取得
		Matrix4x4 viewMatrix = camera_.GetViewMatrix();

		ImGuizmo::ViewManipulate(
			&viewMatrix.m[0][0],
			selectedModelLength_,
			ImVec2(ImGui::GetIO().DisplaySize.x - 256.0f, 0.0f),
			ImVec2(256.0f, 256.0f),
			0x10101010
		);

		// 変更されたView行列をそのまま反映
		camera_.SetViewMatrix(viewMatrix);
	}
	else {

		/* 非選択時は固定値８．０ｆ */
		Matrix4x4 viewMatrix = camera_.GetViewMatrix();

		ImGuizmo::ViewManipulate(
			&viewMatrix.m[0][0],
			8.0f,
			ImVec2(ImGui::GetIO().DisplaySize.x - 256.0f, 0.0f),
			ImVec2(256.0f, 256.0f),
			0x10101010
		);

		// 変更されたView行列をそのまま反映
		camera_.SetViewMatrix(viewMatrix);
	}


	if (isEditorMode_) {

		// 選択中のモデルがあるときだけ操作
		if (selectedModelWorldTransform_) {

			// 転置なしでそのまま渡す
			Matrix4x4 viewMat = camera_.GetViewMatrix();
			Matrix4x4 projMat = camera_.GetProjectionMatrix();
			Matrix4x4 worldMat = selectedModelWorldTransform_->matWorld_;

			bool isManipulated = ImGuizmo::Manipulate(
				&viewMat.m[0][0],
				&projMat.m[0][0],
				operation_,
				ImGuizmo::WORLD,
				&worldMat.m[0][0]
			);

			/* もしスナップ中なら */
			if (grid.isSnap_) {

				Vector3 translation = {
					worldMat.m[3][0],
					worldMat.m[3][1],
					worldMat.m[3][2],
				};

				selectedModelWorldTransform_->translation_ = ImGuizmoGrid::snapGrid(translation);

			}

			if (isManipulated || ImGuizmo::IsUsing()) {

				switch (operation_) {
				case ImGuizmo::TRANSLATE:
					selectedModelWorldTransform_->translation_.x = worldMat.m[3][0];
					selectedModelWorldTransform_->translation_.y = worldMat.m[3][1];
					selectedModelWorldTransform_->translation_.z = worldMat.m[3][2];
					break;
				case ImGuizmo::ROTATE:
					selectedModelWorldTransform_->rotation_ = ExtractRotation(worldMat, selectedModelWorldTransform_->scale_);
					break;
				case ImGuizmo::SCALE:
					selectedModelWorldTransform_->scale_ = ExtractScale(worldMat);
					break;

				}


				selectedModelWorldTransform_->UpdateMatrix();
			}
		}
		else {

			// 非選択時はキーで移動
			FreeMoveCamera();
		}
	}

	/* メニュー表示 */
	if (ImGui::BeginMainMenuBar()) {
	
		if (ImGui::BeginMenu("Window")) {

			ImGui::MenuItem("Property", nullptr, &showProperty);
			ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
			ImGui::MenuItem("Assets", nullptr, &showAssets);
			ImGui::MenuItem("EffectTimeline", nullptr, &showEffectTimeline);
			ImGui::MenuItem("LightingManager", nullptr, &showLighting);

			ImGui::EndMenu();
		}

		// Window の横に配置
		ImGui::SameLine(60.0f);

		if (ImGui::Checkbox("Editor ", &isEditorMode_)) {

			// 押された瞬間,メインカメラからviewをコピー
			if (isEditorMode_) {
				camera_.SetCamera(*Scene::GetSceneCamera());
			}

			//	 ON/OFF切り替え
			Scene::GetSceneCamera()->SetEditorMode(isEditorMode_);

		}
		ImGui::SameLine(200.0f);

		if (isEditorMode_) {

			if (ImGui::Button("Translate")) {
				operation_ = ImGuizmo::TRANSLATE;
			}

			ImGui::SameLine();

			if (ImGui::Button("Rotate")) {
				operation_ = ImGuizmo::ROTATE;
			}

			ImGui::SameLine();

			if (ImGui::Button("Scale")) {
				operation_ = ImGuizmo::SCALE;
			}
		}

		ImGui::EndMainMenuBar();
	}

	// プロパティー
	if (showProperty) {

		/* 色 */
		ImGui::PushStyleColor(
			ImGuiCol_WindowBg,
			ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
		);

		ImGui::Begin("Property");

			/* 今開いてるシーン名 */
			ImGui::Text("SceneName: %s", currentSceneName_.c_str());
			ImGui::SameLine();


			ImGui::InputText(" ", &currentSceneJsonPath);


			// ロードボタン
			if (ImGui::Button("Load")) {
				 hierarchyPanel_.HierarchyLoad(currentSceneJsonPath);
			}

			ImGui::SameLine();

			// セーブボタン
			if (ImGui::Button("Save")) {
				hierarchyPanel_.HierarchySave(currentSceneJsonPath);
			}


			// ワールド座標系に
			Matrix4x4 viewMatrix = Inverse(camera_.GetViewMatrix());

			/* カメラから位置と回転を入手 */
			Vector3 cameraPosition;
			cameraPosition.x = viewMatrix.m[3][0];
			cameraPosition.y = viewMatrix.m[3][1];
			cameraPosition.z = viewMatrix.m[3][2];

			Vector3 cameraRotation = ExtractRotation(viewMatrix, { 1.0f,1.0f,1.0f });

			ImGui::Separator();

			ImGui::Text("Camera WorldTransrate x:%3.2f y:%3.2f z:%3.2f ", cameraPosition.x, cameraPosition.y, cameraPosition.z);
			ImGui::Text("Camera WorldRotation x:%3.2f y:%3.2f z:%3.2f ", cameraRotation.x, cameraRotation.y, cameraRotation.z);
			
			// カメラ位置コピー（小癪な・・）
			if (ImGui::Button("Copy")) {

				// 少数第3位までフォーマット
				std::string tpos = "Vector3{"+
					std::format("{:.3f}", cameraPosition.x) + "f," +
					std::format("{:.3f}", cameraPosition.y) + "f," +
					std::format("{:.3f}", cameraPosition.z) + "f}";

				std::string trot = "Vector3{" +
					std::format("{:.3f}", cameraRotation.x) + "f," +
					std::format("{:.3f}", cameraRotation.y) + "f," +
					std::format("{:.3f}", cameraRotation.z) + "f}";

				std::string txt = tpos + "\n" + trot;

				ImGui::SetClipboardText(txt.c_str());
			}


			ImGui::Text("hoi-ru %d", Input::GetInstance()->GetMouseWheel());
			ImGui::Text("targetleng %f", selectedModelLength_);

			ImGui::Separator();

			// === グリッド設定
			ImGui::SeparatorText("Grid");

			// スナップ状態なら
			if (grid.isSnap_) {

				// 選択中は色変える
				if (currentModelHierarchyMode_ == ModelHierarchyMode::kCreateMode)
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));

				bool b = ImGui::Button("MapCreateMode");

				if (currentModelHierarchyMode_ == ModelHierarchyMode::kCreateMode)
					ImGui::PopStyleColor();


				if (b) {
					// 切り替え
					currentModelHierarchyMode_ == ModelHierarchyMode::kSelectMode ?
						currentModelHierarchyMode_ = ModelHierarchyMode::kCreateMode :
						currentModelHierarchyMode_ = ModelHierarchyMode::kSelectMode;

				}
			}

		grid.ImGuiView();


		ImGui::End();

		// 色の
		ImGui::PopStyleColor();
	}

	// ヒエラルキーパネル
	if (showHierarchy) {
		hierarchyPanel_.ViewPanel(showHierarchy);
	}

	/* エフェクトタイムライン */
	if (showEffectTimeline) {
		EffectEditorTimeLine();
	}

	// ライティングパネル
	if (showLighting) {
		LightingManagerTab(showLighting);
	}

	// アセットパネル
	if (showAssets) {
		assetsPanel_.PanelView(showAssets);
	}

	//ImGui::End();
#endif

	// もしデバッグモードであれば、デバッグカメラのviewをメインカメラにコピー
	if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {
		Scene::GetSceneCamera()->SetCamera(camera_);
	}
}

// エディター描画
void Editor::Draw() {

	if (isEditorMode_) {

		grid.MatrixUpdate();

		assetsPanel_.Draw();

		// 平面描画　傾きはGridの傾きを回転行列として使う
		DebugLine::DrawPlane(grid.plane_, grid.rotation_, { 1.0f,0,0,1.0f });

		if (selectedModelWorldTransform_) {

			// 選択のAABB
			WorldTransform* world = selectedModelWorldTransform_;

			// だいたいの中心サイズ
			float size = 1.0f;

			AABB aabb;
			aabb.min = Vector3{ -size, -size, -size } + world->translation_;
			aabb.max = Vector3{ size,  size,  size } + world->translation_;

			DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });


			DebugLine::GetInstance()->Draw(camera_);
		}
	}


	// シーンカメラが設定されてれば
	if (Scene::GetSceneCamera() != nullptr) {

		/* ヒエラルキーモデルの描画 */
		for (const Hierarchy<Model>& hierarchy : hierarchyPanel_.GetHierarchyModels()) {

			WorldTransform* world = static_cast<WorldTransform*>(hierarchy.transform_.get());

			// マップで登録されたidと連携したモデルで描画
			if (isEditorMode_) {
				hierarchyPanel_.GethierarchyModelResourceList()[hierarchy.id_]->Draw(*world, camera_);
			}
			// エディターモードじゃなければ、シーンのカメラで描画する
			else {
				hierarchyPanel_.GethierarchyModelResourceList()[hierarchy.id_]->Draw(*world, *Scene::GetSceneCamera());
			}

		}
		

	}



	//if (isEditorMode_) {

	//	if (electedModelWorldTransform_) {

	//		AABB aabb;
	//		aabb.min = Vector3{ -0.5f, -0.5f, -0.5f } + selectedModel_->GetWorldTransform().translation_;
	//		aabb.max = Vector3{ 0.5f,  0.5f,  0.5f } + selectedModel_->GetWorldTransform().translation_;

	//		// 四角描画
	//		DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });

	//	}
	//}


}

void Editor::SelectModelUpdateView()
{

	if (selectedModelWorldTransform_ == nullptr){
		return;
	}

	// 1) カメラを、選択したモデルのワールド座標、注視点とする
	// 2) カメラのviewMatrixからワールド座標を摘出する
	// 3) 選択モデルとカメラの距離を計算（ImViewGizmoの引数に必要）
	// 4) 距離から球面座標系に必要なthita横回転とphi縦を摘出
	// 5) 球面座標としてカメラのviewにセット、thitaとｐひもImViewGizmoにあげる

	Vector3 targetPos = selectedModelWorldTransform_->translation_;

	// 1) View行列からカメラのワールド行列を復元
	Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

	// 2) ワールド座標を取り出す
	Vector3 camPos = {
		cameraWorld.m[3][0],
		cameraWorld.m[3][1],
		cameraWorld.m[3][2]
	};

	// 3)距離
	selectedModelLength_ = Distance(camPos, targetPos);

	// 4) thetaとphi
	Vector3 offset = camPos - targetPos;
	selectedSphereTheta_ = std::asin(offset.y / selectedModelLength_);
	selectedSpherePhi_ = std::atan2(offset.z, offset.x);

	// 5) view更新！！
	camera_.SphericalCameraUpdate(
		targetPos,
		selectedModelLength_,
		selectedSphereTheta_,
		selectedSpherePhi_
	);
}

void Editor::FreeMoveCamera()
{
	/* 移動 */
	Vector3 move{ 0,0,0 };
	float speed = 0.2f;

	if (Input::GetInstance()->PushKey(DIK_W)) {
		move.z += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		move.z += -speed;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		move.x += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		move.x += -speed;
	}
	if (Input::GetInstance()->PushKey(DIK_E)) {
		move.y += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_Q)) {
		move.y -= speed;
	}

	// View行列からカメラのワールド行列を取得
	Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

	// カメラの位置を増減
	cameraWorld.m[3][0] += move.x;
	cameraWorld.m[3][1] += move.y;
	cameraWorld.m[3][2] += move.z;

	// ワールド行列からView行列に戻す
	camera_.SetViewMatrix(Inverse(cameraWorld));

}

void Editor::LightingManagerTab(bool& show) {

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("Lighting-Manager", &show);

	if (ImGui::Button("Add_DirectionalLight")) {
		DirectionalLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}
	if (ImGui::Button("Add_SpotLight")) {
		SpotLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}
	if (ImGui::Button("Add_PointLight")) {
		PointLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}


	// === DirectionalLight
	if (ImGui::TreeNode("DirectionalLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::DragFloat3(
				"Direction",
				&param.lights[i].direction.x,
				0.1f
			);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat(
				"itensity",
				&param.lights[i].intensity,
				0.1f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().dLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// === SpotlLight
	if (ImGui::TreeNode("SpotLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat3(
				"Position",
				&param.lights[i].position.x,
				0.1f
			);

			ImGui::DragFloat(
				"Intensity",
				&param.lights[i].intensity,
				0.1f
			);

			ImGui::DragFloat3(
				"Direction",
				&param.lights[i].direction.x,
				0.1f
			);

			ImGui::DragFloat(
				"Inner Angle",
				&param.lights[i].innerAngle,
				0.1f
			);

			ImGui::DragFloat(
				"Outer Angle",
				&param.lights[i].outerAngle,
				0.1f
			);

			ImGui::DragFloat(
				"Distance",
				&param.lights[i].distance,
				0.1f
			);

			ImGui::DragFloat(
				"Falloff",
				&param.lights[i].falloff,
				0.1f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().sLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// === PointlLight
	if (ImGui::TreeNode("PointLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat3(
				"Position",
				&param.lights[i].position.x,
				0.1f
			);

			ImGui::DragFloat(
				"Intensity",
				&param.lights[i].intensity,
				0.1f,
				0.0f
			);

			ImGui::DragFloat(
				"Radius",
				&param.lights[i].radius,
				0.1f,
				0.0f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().pLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}


	ImGui::End();

	ImGui::PopStyleColor();
#endif
}

void Editor::AssetsTab(bool& show)
{

}



void Editor::EffectEditorTimeLine()
{

#ifdef USE_IMGUI




	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("EffectView");

	// let's create the sequencer
	static int selectedEntry = -1;
	static int firstFrame = 0;
	static bool expanded = true;
	static int currentFrame = 100;

	ImGui::PushItemWidth(130);
	ImGui::InputInt("Frame Min", &effectSequencer_.mFrameMin);
	ImGui::SameLine();
	ImGui::InputInt("Frame ", &currentFrame);
	ImGui::SameLine();
	ImGui::InputInt("Frame Max", &effectSequencer_.mFrameMax);
	ImGui::PopItemWidth();
	Sequencer(&effectSequencer_, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);

	// add a UI to edit that particular item
	if (selectedEntry != -1)
	{
		const EffectSequencer::SequenceItem& item = effectSequencer_.items_[selectedEntry];
		ImGui::Text("I am a %s, please edit me", effectSequencer_.GetSequencerItemTypeNames(item.mType));
		// switch (type) ....
	}

	ImGui::End();

	// iro
	ImGui::PopStyleColor();
#endif
}


Vector3 Editor::ExtractScale(const Matrix4x4& m)
{
	return {
		std::sqrt(
			m.m[0][0] * m.m[0][0] +
			m.m[0][1] * m.m[0][1] +
			m.m[0][2] * m.m[0][2]
		),
		std::sqrt(
			m.m[1][0] * m.m[1][0] +
			m.m[1][1] * m.m[1][1] +
			m.m[1][2] * m.m[1][2]
		),
		std::sqrt(
			m.m[2][0] * m.m[2][0] +
			m.m[2][1] * m.m[2][1] +
			m.m[2][2] * m.m[2][2]
		)
	};
}
Vector3 Editor::ExtractRotation(const Matrix4x4& m, const Vector3& scale)
{
	// スケールを除去
	float r00 = m.m[0][0] / scale.x;
	float r01 = m.m[0][1] / scale.x;
	float r02 = m.m[0][2] / scale.x;

	float r10 = m.m[1][0] / scale.y;
	float r11 = m.m[1][1] / scale.y;
	float r12 = m.m[1][2] / scale.y;

	float r20 = m.m[2][0] / scale.z;
	float r21 = m.m[2][1] / scale.z;
	float r22 = m.m[2][2] / scale.z;

	// MakeAffineMatrix の Rx * Ry * Rz に対応
	float ry = std::asin((std::clamp)(-r02, -1.0f, 1.0f));

	float cy = std::cos(ry);

	float rx;
	float rz;

	if (std::abs(cy) > 1.0e-6f) {
		rx = std::atan2(r12, r22);
		rz = std::atan2(r01, r00);
	}
	else {
		// ジンバルロック付近
		rx = std::atan2(-r21, r11);
		rz = 0.0f;
	}

	return { rx, ry, rz };
}


float ImGuizmoGrid::squareSize = 1.0f;


void ImGuizmoGrid::MatrixUpdate()
{
	// 初期状態の法線（水平な平面の上方向）
	Vector3 normal = { 0.0f, 1.0f, 0.0f };

	// オイラー角->回転行列->ベクトル変換
	Matrix4x4 rotationMatrix = MakeRotationMatrix(rotation_);

	// 法線を回転させる
	plane_.normal = Normalize(
		TransformNormal(normal, rotationMatrix)
	);

	//* 判定用の平面の法線はグリッドと同じにする
	plane_.distance = gridSize;
	plane_.point = translation_;

	// SRT合成
	worldMatrix = MakeAffineMatrix({1.0f,1.0f,1.0f}, rotation_, translation_);
}

void ImGuizmoGrid::ImGuiView()
{
#ifdef USE_IMGUI

	ImGui::DragFloat("Grid Size", &gridSize, 0.1f, 1.0f, 5000.0f);
	ImGui::DragFloat("Grid SquareSize", &squareSize, 1.0f, 1.0f,100.0f);
	ImGui::DragFloat3("Grid Translation", &translation_.x, 0.1f);
	ImGui::Separator();

	//ImGui::DragFloat3("Plane pos", &plane_.point.x, 0.1f, 1.0f, 5000.0f);
	//ImGui::DragFloat3("Plane normal", &plane_.normal.x, 0.1f, 1.0f, 5000.0f);

	
	ImGui::Checkbox("isSnap", &isSnap_);

	ImGui::SeparatorText("RotateGrid");

	if (ImGui::Button("x")) {
		rotation_.x += std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

	// 横に
	if (ImGui::Button("y")) {
		rotation_.y += std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

#endif
}

Vector3  ImGuizmoGrid::snapGrid(Vector3 v){

	Vector3 result;

	result.x = std::floor(v.x / squareSize) * squareSize + squareSize / 2.0f;
	result.y = std::floor(v.y / squareSize) * squareSize + squareSize / 2.0f;
	result.z = std::floor(v.z / squareSize) * squareSize + squareSize / 2.0f;

	// マスの大きさで割って配置
	return result;

}
