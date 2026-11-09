
#include "Editor.h"


#include "WinApp.h"

#include "Game.h"
#include "Scene.h"


#include "ImGuiManager.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

#include "LightingManager.h"

#include "Input.h"

#include "GameObject.h"

#include "FileDropManager.h"

#include "UIManager.h"

#include "PostEffectManager.h"

#include "UIManager.h"

#include "DebugLine.h"

#include "Vector.h"
#include "Matrix.h"

#include "Timer.h"

#include <algorithm>
#include <numbers>
#include <memory>
#include <format>
#include <optional>

//HierarchyPanel Editor::hierarchyPanel_;
EditorGrid Editor::grid;
std::string Editor::currentSceneJsonPath;
SceneData* Editor::currentSceneData_;

bool Editor::isEditorMode_ = false;

Editor* Editor::GetInstance() {
	static Editor instance;
	return &instance;
}

/* エディター初期化 */
void Editor::Initialize() {

	camera_.Initialize();

	// エディタの設定読込
	LoadIniFile();


	/* エディターモードの場合 */
	if (isEditorMode_) {

		// もしデバッグモードの場合（力技）
#ifdef _DEBUG
		Game::GetInstance()->SetWindowViewMode(Game::WindowViewMode::kEditorMode);
#else
		Game::GetInstance()->SetWindowViewMode(Game::WindowViewMode::kFullScreenMode);
#endif

	}
	else {
		Game::GetInstance()->SetWindowViewMode(Game::WindowViewMode::kFullScreenMode);
	}

	/// .. 各パネルの処理はSceneのStartGameSceneのメソッド内に描きました
	/// 　 理由：main.cppのStartの書く位置がEditorのここの初期化より後のせいで読み込めなかったから

	effectSequencer_.mFrameMin = -100;
	effectSequencer_.mFrameMax = 1000;
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 0, 10, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 1, 20, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 3, 12, 60 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 2, 61, 90 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 4, 90, 99 });

}

void Editor::PanelInit(std::string sceneName)
{
	//
	currentSceneName_ = sceneName;

	/*
		各パネルの初期化
	*/ 

	// モデルなどアセット読み込み
	AssetsPanel::GetInstance()->Initialize();
	AssetsPanel::GetInstance()->SetHierarchyPanel(currentSceneData_->hierarchyPanel_.get());

	// アセットをもとにヒエラルキ生成
	currentSceneData_->hierarchyPanel_.get()->Initialize(sceneName,grid);

	/* UIManagerにセット */
	UIManager::GetInstance()->SetHierarchyPanel(currentSceneData_->hierarchyPanel_.get());

	/* UIManagerの初期化 */
	UIManager::GetInstance()->Initialize(sceneName);

}

static bool showProperty = true; //
static bool showHierarchy = true; // 
static bool showLighting = false;
static bool showPostEffect = false;
static bool showEffectTimeline = false;
static bool showAssets = true; //

/* エディター更新 */
void Editor::Update()
{

	// 新しくドロップされたシーンのJsonパスを取得
	auto paths =
		FileDropManager::TakeFilePaths(FileType::kJson);

	for (const auto& path : paths) {

		// 絶対パス → resources/ 以下の相対パス
		std::string relativePath =
			FileDropManager::GetRelativePath(path);

		// 相対パスが取得できなければスキップ
		if (relativePath.empty()) {
			continue;
		}

		/* 
			ここにエフェクト用のjsonだかを識別する何かをいつかかく
		*/
		//LoadScene(relativePath);
		//currentSceneJsonPath = relativePath;
	}


	// 起動時にGameでSceneカメラ設定終わってからEditorModeのカメラせっていしたかたったのでこんなんなりました
	if (!ini) {

		if (isEditorMode_ && Scene::GetSceneCamera()) {

			//* isEditor切り替えたときと一緒 *///
			WinApp::GetInstance()->SetDragEnabled(true);

			Scene::GetSceneCamera()->SetEditorMode(true);

			ini = true;
		}
		
	};


	/* 
		常に更新するパネルの内容
	*/

	// 現在のシーンのヒエラルキー
	currentSceneData_->hierarchyPanel_->Update();

	AssetsPanel::GetInstance()->Update();


#ifdef USE_IMGUI

	ImGui::Begin("Game View");



	/* 
		タブバーの開始
	*/
	if (ImGui::BeginTabBar("SceneTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {

		// エディターで開いているシーンの数だけタブを作成
		for (int i = 0; i < sceneDatas_.size(); i++) {
			auto& data = sceneDatas_[i];

			// タブが開いているかどうか（xボタン対応）
			bool open = true;

			// タブの描画（選択されたら true）
			if (ImGui::BeginTabItem(data.sceneName_.c_str(), &open)) {

				// ★ タブが切り替わったら、アクティブなシーンを変更する
				if (currentSceneData_ != &data) {
					currentSceneData_ = &data;

					/*
						シーンのインスタンスを置き換える
					*/
					Scene::SetActiveInstanceReference(data.scene_);

					/* データ等も切り替え */
					SwitchActiveScene(currentSceneData_->sceneName_);

				}

				// --- ゲーム画面の描画 ---
				
				DrawMainImageView();


				ImGui::EndTabItem();
			}

			// タブの 'x' ボタンが押されたら閉じる処理
			if (!open) {
				//CloseTab(i); // sceneDatas_ から削除
				break;
			}
		}

		ImGui::EndTabBar();
	}



	/*   AssetsPanelから、Textureがッドラッグされた際の処理　　*/


	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("TEXTURE"))
		{

			// 受け取ったモデルの配列番号
			DragTextureResource resource = *(const DragTextureResource*)payload->Data;

			/*
				UIElementを生成
			*/
			UIManager::GetInstance()->AddUIElement(currentSceneData_->hierarchyPanel_->GetSelectedCurrentLayerName(), resource.pathId);
			
		}

		ImGui::EndDragDropTarget();
	}




	// 通知のビョーガ
	notification_.Draw();


	/// ImGuizmoのフレーム開始
	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);


	// カメラの投影モードを設定（透視投影の場合は false）
	ImGuizmo::SetOrthographic(false);

	ImGuizmo::SetRect(mainViewPos.x, mainViewPos.y, mainViewSize.x, mainViewSize.y);

	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	/*
		便利なキューブ
	*/
	DrawViewManipulate();


	/*
		カメラ操作などによる選択の変形など
	*/
	DrawArrowToSelectedTransform();


ImGui::End();


	/*
		上部バー
	*/
	TopMenuVar();


	/*
		プロパティ
	*/
	PropartyTab();


	/* 各パネルはEditormodeがONの場合表示させる */
	if (isEditorMode_) {

		// ヒエラルキーパネル
		if (showHierarchy) {
			currentSceneData_->hierarchyPanel_->ViewPanel(showHierarchy);
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
			AssetsPanel::GetInstance()->PanelView(showAssets);
		}

		// ポストエフェクトパネル
		if (showPostEffect) {
			PostEffectManager::GetInstance()->ImGuiView(showPostEffect);
		}

	}

	//*
	if (!isEditorMode_) return;

	/*
		キーボード操作
	*/
	OperationKeyBoard();


	mouse = ImGui::GetMousePos();

	// ウインドウの｛０，０｝から見たローカル座標
	maiViewLocalMouseX = mouse.x - mainViewPos.x;
	mainViewLocalMouseY = mouse.y - mainViewPos.y;

	mainViewWidth = mainViewSize.x;
	mainViewHeight = mainViewSize.y;

	/* ビュー上にあるか */
	bool insideScene =
		mouse.x >= mainViewPos.x &&
		mouse.x <= (mainViewPos.x + mainViewSize.x) &&
		mouse.y >= mainViewPos.y &&
		mouse.y <= (mainViewPos.y + mainViewSize.y);


	// ViewManipulateの領域
	ImVec2 manipulateSize = ImVec2(128.0f, 128.0f);

	ImVec2 manipulatePos = ImVec2(
		mainViewPos.x + mainViewSize.x - manipulateSize.x - 10.0f,
		mainViewPos.y + 10.0f
	);

	/* マウスがキューブ上にあるか */
	bool insideViewCube =
		mouse.x >= manipulatePos.x &&
		mouse.x <= manipulatePos.x + manipulateSize.x &&
		mouse.y >= manipulatePos.y &&
		mouse.y <= manipulatePos.y + manipulateSize.y;

	// ImGuiの枠にかさならない
	if (insideScene &&
		!insideViewCube &&
		!ImGuizmo::IsUsing()) {

		/*
			左、右クリ操作
		*/
		OperationLeftClick();
		OperationRightClick();

	}


	if (isEditorMode_) {
		if (selectedModelWorldTransform_ == nullptr) {
			// 非選択時はキーで移動
			FreeMoveCamera();
		}
	}

#endif

	// もしデバッグモードであれば、デバッグカメラのviewをメインカメラにコピー
	if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {
		Scene::GetSceneCamera()->SetCamera(camera_);
	}
}

// エディター描画
void Editor::Draw() {

	if (isEditorMode_) {

		grid.Update();

		AssetsPanel::GetInstance()->Draw();

		if (selectedModelWorldTransform_) {

			// 選択のAABB
			WorldTransform* world = selectedModelWorldTransform_;

			// だいたいの中心サイズ
			float size = 2.0f;

			AABB aabb;
			aabb.min = Vector3{ -size, -size, -size } + world->translation_;
			aabb.max = Vector3{ size,  size,  size } + world->translation_;


			DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });

		}

		// グリッドの線追加
		grid.AddLine();

		//DebugLine::GetInstance()->Draw(camera_);
	}


	// シーンカメラが設定されてれば
	if (Scene::GetSceneCamera() != nullptr) {

		// 選択中のヒエラルキのAABB描画
		if (isDrawAABB_ && selectedModelWorldTransform_ != nullptr) {

			auto* model = dynamic_cast<ModelNode*>(currentSceneData_->hierarchyPanel_->GetSelectedHierarchy());

			DebugLine::DrawAABB(model->aabb_, { 0.0f,1,1,0.5f });
		}

		/* ヒエラルキーモデルの描画 */
		for (const std::unique_ptr<HierarchyNode>& hierarchy : currentSceneData_->hierarchyPanel_->GetHierarchys()) {

			// 非表示なら描画スキップ
			if (!hierarchy->isView_) continue;

			/* 各ノードの描画 */
			hierarchy->Draw(camera_);
			
			

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

void Editor::TopMenuVar()
{

	/* メニュー表示 */
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Window")) {

			ImGui::MenuItem("Property", nullptr, &showProperty);
			ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
			ImGui::MenuItem("Assets", nullptr, &showAssets);
			ImGui::MenuItem("EffectTimeline", nullptr, &showEffectTimeline);
			ImGui::MenuItem("LightingManager", nullptr, &showLighting);
			ImGui::MenuItem("PostEffectManager", nullptr, &showPostEffect);

			ImGui::EndMenu();
		}

		// Window の横に配置
		ImGui::SameLine(60.0f);
		ImGui::Text("DebugMode:");
		ImGui::SameLine();

		/* エディターモード起動 */
		if (ToggleSwitch("Editor ", &isEditorMode_)) {

			// 有効化
			if (isEditorMode_) {

				////* ドロップ有効に
				WinApp::GetInstance()->SetDragEnabled(true);

				// 押された瞬間,メインカメラからviewをコピー
				camera_.SetCamera(*Scene::GetSceneCamera());

				DebugLine::SetCamera(&camera_);//

				/* ゲームのビューサイズをエディター用に設定 */
				Game::GetInstance()->SetWindowViewMode(Game::WindowViewMode::kEditorMode);

			}

			// 無効化
			else {
				////* ドロップ無効に
				WinApp::GetInstance()->SetDragEnabled(false);

				DebugLine::SetCamera(Scene::GetSceneCamera());//

				/* ゲームのビューサイズをRelease用に設定 */
				Game::GetInstance()->SetWindowViewMode(Game::WindowViewMode::kFullScreenMode);
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

			if (selectedModelWorldTransform_) {

				// もしヒエラルキ移動ボタン押されたら
				if (operation_ == ImGuizmo::TRANSLATE) {
					ImGui::SameLine();
					ImGui::Text("Translate:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::DragFloat3(" ", &selectedModelWorldTransform_->translation_.x, 0.1f);
				}
				// もしヒエラルキ回転ボタン押されたら
				if (operation_ == ImGuizmo::ROTATE) {
					ImGui::SameLine();
					ImGui::Text("Rotate:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::DragFloat3(" ", &selectedModelWorldTransform_->rotation_.x, 0.1f);

					ImGui::SameLine();
					if (ImGui::Button("x")) {
						selectedModelWorldTransform_->rotation_.x -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
					ImGui::SameLine();
					if (ImGui::Button("y")) {
						selectedModelWorldTransform_->rotation_.y -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
					ImGui::SameLine();
					if (ImGui::Button("z")) {
						selectedModelWorldTransform_->rotation_.z -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
				}
				// もしヒエラルキ拡大ボタン押されたら
				if (operation_ == ImGuizmo::SCALE) {
					ImGui::SameLine();
					ImGui::Text("Scale:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::DragFloat3(" ", &selectedModelWorldTransform_->scale_.x, 0.1f);
				}



			}

			/* ずいぶん右に配置 */
			ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);

			if (ImGui::Button("ResetCamera")) {
				camera_.GetWorldTransform().translation_ = tmpCameraPosition_;
				camera_.GetWorldTransform().rotation_ = tmpCameraRotation_;
				camera_.UpdateMatrix();
			}

			/* ずいぶん右に配置 */
			ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);

			if (ImGui::Button("AABB")) {
				isDrawAABB_ = !isDrawAABB_;
			}
			ImGui::Text(isDrawAABB_ ? " true" : " false");
		}


		ImGui::EndMainMenuBar();
	}

}

void Editor::DrawMainImageView()
{
#ifdef USE_IMGUI

	mainViewPos = ImGui::GetCursorScreenPos();   // ウィンドウ内の描画開始位置（左上のスクリーン座標）
	mainViewSize = ImGui::GetContentRegionAvail(); // 利用可能な幅と高さ

	/*
		縦横比固定
	*/

	// 元のアスペクト比
	constexpr float aspectRatio = 1280.0f / 720.0f;

	// 利用可能な領域
	float width = mainViewSize.x;
	float height = width / aspectRatio;

	// 高さが足りなければ高さ基準にする
	if (height > mainViewSize.y) {
		height = mainViewSize.y;
		width = height * aspectRatio;
	}

	mainViewSize = ImVec2(width, height);

	// オフスクリーンのレンダーターゲットテクスチャをImGui上で表示する
	ImGui::Image(
		(ImTextureID)PostEffectManager::GetInstance()->GetOffscreenFinalSRVHandleGPU().ptr,
		mainViewSize
	);

#endif
}

void Editor::DrawViewManipulate()
{

	/*カメラ視点の回転 */
	ImVec2 manipulateSize = ImVec2(128.0f, 128.0f);
	ImVec2 manipulatePos = ImVec2(mainViewPos.x + mainViewSize.x - manipulateSize.x - 10.0f, mainViewPos.y + 10.0f);


	/* ギズモによるカメラ回転 */
	if (selectedModelWorldTransform_) {

		// 現在のカメラView行列を取得
		Matrix4x4 viewMatrix = camera_.GetViewMatrix();


		ImGuizmo::ViewManipulate(
			&viewMatrix.m[0][0],
			selectedModelLength_,
			manipulatePos,
			manipulateSize,
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
			manipulatePos,
			manipulateSize,
			0x10101010
		);

		// 変更されたView行列をそのまま反映
		camera_.SetViewMatrix(viewMatrix);
	}

}

void Editor::UpdateHierarchySelection()
{


}

void Editor::OperationKeyBoard()
{
	if (selectedModelWorldTransform_) {

		/* 回転キーなどショトカ */
		if (Input::GetInstance()->TriggerKey(DIK_T)) {
			operation_ = ImGuizmo::TRANSLATE;
		}
		if (Input::GetInstance()->TriggerKey(DIK_R)) {
			operation_ = ImGuizmo::ROTATE;
		}
		if (Input::GetInstance()->TriggerKey(DIK_S)) {
			operation_ = ImGuizmo::SCALE;
		}

		// ペースト
		if (Input::GetInstance()->TriggerKey(DIK_V)) {

			selectedModelWorldTransform_ = currentSceneData_->hierarchyPanel_->PasteHierarchyModel();

			// コピー処理が成功したタイミングで呼ぶ
			notification_.PopText("Model Pasted!!", ImVec2(170.0f, 30.0f));
		}

	}
}

void Editor::OperationLeftClick()
{
	/*
		現在開かれているシーンのヒエラルキー
	*/
	auto hierarchyPanel_ = currentSceneData_->hierarchyPanel_.get();


	/* 左クリックを押された際の処理 */
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		// スクリーンの大きさとスクリーン座標からから、レイ生成
		Camera::Ray ray = camera_.ScreenPointToRay(Vector2{ maiViewLocalMouseX,mainViewLocalMouseY }, mainViewWidth, mainViewHeight);

		float distance = 0.0f;

		hierarchyPanel_->SetSelectedHierarchyModelIndex(std::nullopt);
		selectedModelWorldTransform_ = nullptr;

		//* モード別の処理　*//
		switch (currentModelHierarchyMode_) {


		//* 選択モード
		case ModelHierarchyMode::kSelectMode:

			// すべてのモデル
			for (int i = 0; i < hierarchyPanel_->GetHierarchys().size(); i++)
			{
				if (hierarchyPanel_->GetHierarchys().at(i)->type_ != HierarchyType::kModel)
					continue;

				auto* model = dynamic_cast<ModelNode*>(hierarchyPanel_->GetHierarchys().at(i).get());


				// もし選択中のモデルアセットがあった場合
				if (!AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_.empty()) {

					/* 選択中以外の、アセット以外のIDのヒエラルキーはとばす */
					if (model->modelId_ != AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_)
						continue;
				}

				// 非表示なら選択もスキップ
				if (!model->isView_)
					continue;

				// ヒエラルキからワールドのポインタ取得！
				WorldTransform* world = static_cast<WorldTransform*>(model->transform_.get());


				// そのヒエラルキが持つAABB？
				AABB* aabb = &model->aabb_;

				// モデルのAABBと判定
				if (camera_.IntersectRayAABB(ray, *aabb, distance)) {

					hierarchyPanel_->SetSelectedHierarchyModelIndex(model);
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

			if (AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_.empty()) break;

			// グリッドのplaneと判定
			std::optional<Vector3> hit = camera_.IntersectRayPlaneArea(ray, grid.plane_);

			// 当たったら
			if (hit.has_value()) {

				// マスの大きさで割ったのを設定する
				Vector3 subPosition = EditorGrid::snapGrid(hit.value());

				bool break_ = false;

				//* もしすでに同じ座標にあるヒエラルキーがある場合、配置せず引き返す（重複する配置の対策）
				for (const std::unique_ptr<HierarchyNode>& h : hierarchyPanel_->GetHierarchys()) {

					auto* model = dynamic_cast<ModelNode*>(h.get());

					/* 選択中以外の、アセット以外のIDのヒエラルキーはとばす */
					if (model->modelId_ != AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_)
						continue;

					if (
						subPosition.x == model->transform_.get()->translation_.x &&
						subPosition.y == model->transform_.get()->translation_.y &&
						subPosition.z == model->transform_.get()->translation_.z) {
						break_ = true;
						break;
					}
				}

				if (break_) break;// breakの場合ここで

				///* モデルのヒエラルキ追加 */
				auto model = std::make_unique <ModelNode>();

				model->type_ = HierarchyType::kModel; // タイプ
				model->modelId_ = AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_;
				model->name_ = AssetsPanel::GetInstance()->selectedHierarchyResource_.name_;

				model->modelResource_ = AssetsPanel::GetInstance()->selectedHierarchyResource_.model_;

				// WorldTransform座標決定
				model->transform_ = std::make_unique<WorldTransform>();

				model->transform_->Initialize();
				model->transform_->translation_ = { subPosition.x,subPosition.y,subPosition.z };

				WorldTransform* world = static_cast<WorldTransform*>(model->transform_.get());
					
				model->isView_ = true;

				// 位置更新
				world->UpdateMatrix();

				/* ヒエラルキ-モデル - 追加 */
				hierarchyPanel_->AddHierarchy(hierarchyPanel_->GetSelectedCurrentLayerName(), std::move(model));


				/* シーン側のGameObjectに干渉してたら一緒に増やす */
				hierarchyPanel_->GameObjectAddCheck();
			}

			break;

		}
	}
}

void Editor::OperationRightClick()
{
	/*
		現在開かれているシーンのヒエラルキー
	*/
	auto hierarchyPanel_ = currentSceneData_->hierarchyPanel_.get();


	/* ImGuiの枠にかさならないよう右クリックを押された際の処理 */
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {


		// スクリーンの大きさとスクリーン座標からから、レイ生成
		Camera::Ray ray = camera_.ScreenPointToRay(Vector2{ mouse.x,mouse.y }, mainViewWidth, mainViewHeight);

		float distance = 0.0f;

		HierarchyNode* selectedNode = nullptr;

		//* モード別の処理　*//
		switch (currentModelHierarchyMode_) {

		//* 選択モード
		case ModelHierarchyMode::kSelectMode:

			if (!selectedModelWorldTransform_) break;


			// なんか気分で 消しとく

			selectedNode = hierarchyPanel_->GetSelectedHierarchy();

			if (selectedNode) {

				// 選択中のノードを削除
				hierarchyPanel_->DeleteHierarchyNode(selectedNode);
				hierarchyPanel_->SetSelectedHierarchyModelIndex(std::nullopt);
				selectedModelWorldTransform_ = nullptr;

			}

			break;
		//* 配置モード
		case ModelHierarchyMode::kCreateMode:

			if (AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_.empty()) break;

			// グリッドのplaneと判定
			std::optional<Vector3> hit = camera_.IntersectRayPlaneArea(ray, grid.plane_);

			// 当たったら
			if (hit.has_value()) {

				// すべてのモデル
				for (int i = 0; i < hierarchyPanel_->GetHierarchys().size(); i++)
				{
					if (hierarchyPanel_->GetHierarchys().at(i).get()->type_ != HierarchyType::kModel)continue;

					auto* model = dynamic_cast<ModelNode*>(hierarchyPanel_->GetHierarchys().at(i).get());


					/* 選択中以外の、アセット以外のIDのヒエラルキーは選択しない */
					if (model->modelId_ != AssetsPanel::GetInstance()->selectedHierarchyResource_.pathId_)
						continue;

					// ヒエラルキからワールドのポインタ取得！
					WorldTransform* world = static_cast<WorldTransform*>(model->transform_.get());

					// だいたいの中心
					//float size = 3.0f;

					//AABB aabb;
					//aabb.min = Vector3{ -size, -size, -size } + world->translation_;
					//aabb.max = Vector3{ size,  size,  size } + world->translation_;

					// そのヒエラルキが持つAABB？
					AABB* aabb = &model->aabb_;

					// モデルのAABBと判定
					if (camera_.IntersectRayAABB(ray, *aabb, distance)) {

						/* 当たったヒエラルキモデルを削除 */
						hierarchyPanel_->DeleteHierarchyNode(model);

						break;
					}
				}
			}

			break;

		}

	}
}

void Editor::OperationWheel()
{
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
}

void Editor::DrawArrowToSelectedTransform()
{
	if (isEditorMode_) {

		/*
			現在開かれているシーンのヒエラルキー
		*/
		auto hierarchyPanel_ = currentSceneData_->hierarchyPanel_.get();


		// 選択中のモデルがあるときだけ操作
		if (selectedModelWorldTransform_) {

			// 転置なしでそのまま渡す
			Matrix4x4 viewMat = camera_.GetViewMatrix();
			Matrix4x4 projMat = camera_.GetProjectionMatrix();
			Matrix4x4 worldMat = selectedModelWorldTransform_->matWorld_;

			/*
				ImGuizmoの矢印を描画
			*/
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

				selectedModelWorldTransform_->translation_ = EditorGrid::snapGrid(translation);

			}

			/*
				変形処理
			*/
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

				if (hierarchyPanel_->GetSelectedHierarchy()->type_ == HierarchyType::kModel) {

					auto* model = dynamic_cast<ModelNode*>(hierarchyPanel_->GetSelectedHierarchy());

					/* 同期させる */
					if (model->gameObject_ != nullptr) {

						model->gameObject_->GetWorldTransform().translation_ = selectedModelWorldTransform_->translation_;
						model->gameObject_->GetWorldTransform().rotation_ = selectedModelWorldTransform_->rotation_;
						model->gameObject_->GetWorldTransform().scale_ = selectedModelWorldTransform_->scale_;

					}
				}
				selectedModelWorldTransform_->UpdateMatrix();
			}
		}
		
	}
}




void Editor::LoadScene(std::string sceneName)
{

	//currentSceneJsonPath = path;

	/*
		現在開かれているシーンのヒエラルキー
	*/

	currentSceneData_->hierarchyPanel_.get()->Initialize(sceneName, grid);
}

/*

	シーン切り替え

*/
void Editor::SwitchActiveScene(std::string sceneName,bool firstInit)
{
	//
	// *設定
	selectedModelWorldTransform_ = nullptr;

	// 同一シーン名のシーンがあるか検索
	auto it = std::find_if(
		sceneDatas_.begin(),
		sceneDatas_.end(),
		[sceneName](const SceneData& s) {
			return s.sceneName_ == sceneName;
		}
	);

	
	if (it != sceneDatas_.end()) {

		currentSceneName_ = sceneName;

		// 見つかった場合、現在のシーンデータに
		currentSceneData_ = &(*it);

		// 現在のシーンのデータをセット・読み込み
		AssetsPanel::GetInstance()->SetHierarchyPanel(currentSceneData_->hierarchyPanel_.get());

		UIManager::GetInstance()->SetHierarchyPanel(currentSceneData_->hierarchyPanel_.get());
		UIManager::GetInstance()->SwitchSceneUIData(sceneName);


	}
	else {

		/* 
			無かった場合、新たにシーンを生成し、タブを（あれば）切り替える　つまりなかったら新しく作る
		*/
		
		// 新規シーン生成。初期化まで行う
		auto scene = SceneFactory::Create(sceneName);

		/* 読み込めた */
		if (scene) {

			// 追加
			auto& sceneData = sceneDatas_.emplace_back();

			sceneData.scene_ = scene.release();// 移動
	
			// 名前登録
			sceneData.sceneName_ = sceneName;

			// ヒエラルキパネル生成
			sceneData.hierarchyPanel_ = std::make_unique<HierarchyPanel>();

			if (!firstInit) {

				sceneData.hierarchyPanel_.get()->Initialize(sceneName, grid);

				AssetsPanel::GetInstance()->SetHierarchyPanel(sceneData.hierarchyPanel_.get());

				UIManager::GetInstance()->SetHierarchyPanel(sceneData.hierarchyPanel_.get());
				UIManager::GetInstance()->SwitchSceneUIData(sceneName);

			}
			// 現在のシーンに登録
			currentSceneData_ = &sceneDatas_.back();

			// 名前設定
			currentSceneName_ = sceneName;
		}
	}
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
	float speed = 0.4f;

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

void Editor::PropartyTab()
{
	/*
		現在開かれているシーンのヒエラルキー
	*/
	auto hierarchyPanel_ = currentSceneData_->hierarchyPanel_.get();

	// プロパティー
	if (showProperty) {

		/* 色 */
		ImGui::PushStyleColor(
			ImGuiCol_WindowBg,
			ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
		);

		ImGui::Begin("Property", &showProperty);

		/* 今開いてるシーン名 */
		ImGui::Text("SceneName: %s", currentSceneName_.c_str());
		ImGui::SameLine();


		//ImGui::InputText(" ", &currentSceneJsonPath);


		// ロードボタン
		//if (ImGui::Button("Load(バグ)")) {
		//	//hierarchyPanel_->HierarchyLoad(currentSceneJsonPath);
		//}

		ImGui::SameLine();

		/*
			一斉セーブボタン
		*/
		if (ImGui::Button("Save")) {

			hierarchyPanel_->HierarchySave();
			AssetsPanel::GetInstance()->AssetsSave();

			UIManager::GetInstance()->SaveElementData();

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
			std::string tpos = "Vector3{" +
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


		ImGui::Separator();
		ImGui::SeparatorText("SceneSize");

		static float size = 1.0f; // 初期値を設定
		static int off[2] = { 1, 1 };
		static float w = 0.0f;
		static float h = 0.0f;

		// 1. ImGui によるサイズ変更処理
		Vector2 of = Game::GetInstance()->GetGameScreenOffset();

		if (ImGui::SliderFloat("Size", &size, 0.1f, 1.0f)) {
			// Lerp で新しい幅と高さを計算
			w = Lerp(
				1.0f,
				static_cast<float>(WinApp::GetInstance()->GetClientWidth()),
				size
			);

			h = Lerp(
				1.0f,
				static_cast<float>(WinApp::GetInstance()->GetClientHeight()),
				size
			);

			// 画面サイズを変更
			Game::GetInstance()->ResizeGameScreen(static_cast<int32_t>(w), static_cast<int32_t>(h), of);
		}

		ImGui::Text("Width: %d", (int)w);
		ImGui::Text("Height: %d", (int)h);

		if (ImGui::SliderFloat2("Offset", &of.x, 1, 1000)) {

			Game::GetInstance()->SetGameScreenOffset(of);

		}

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

		ImGui::ColorEdit4("tuuti backcol", &notification_.backCol_.x);
		ImGui::ColorEdit4("tuuti textcol", &notification_.textCol_.x);

		ImGui::End();

		// 色の
		ImGui::PopStyleColor();
	}
}

void Editor::LightingManagerTab(bool& show) {

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("Lighting-Manager", &show);


	/*
		シーン固定の光源を登録する
	*/

	if (ImGui::Button("Add_DirectionalLight")) {

		DirectionalLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}
	if (ImGui::Button("Add_SpotLight")) {

		SpotLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}
	if (ImGui::Button("Add_PointLight")) {

		PointLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}


	// === DirectionalLight
	if (ImGui::TreeNode("DirectionalLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

		

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;


			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;

			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;
			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

bool Editor::ToggleSwitch(const char* label, bool* value)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight() - 0.2f;
	float width = height * 2.0f;
	float radius = height * 0.5f;

	ImGui::InvisibleButton(label, ImVec2(width, height));

	// クリックした瞬間に状態を切り替える
	bool clicked = ImGui::IsItemClicked();

	if (clicked)
	{
		*value = !*value;
	}

	///* アニメーションの進行度を保存しとく　これによりうんたら
	ImGuiStorage* storage = ImGui::GetStateStorage();
	ImGuiID id = ImGui::GetID(label);

	float t = storage->GetFloat(id, *value ? 1.0f : 0.0f);
	float target = *value ? 1.0f : 0.0f;

	// 徐々に目標値へ近づける
	float speed = 10.0f;
	t = ImLerp(t, target, ImGui::GetIO().DeltaTime * speed);

	// 目標値に十分近づいたら固定
	if (std::abs(t - target) < 0.001f)
	{
		t = target;
	}

	storage->SetFloat(id, t);

	// 背景色
	ImVec4 offColor = ImGui::IsItemHovered()
		? ImVec4(0.78f, 0.78f, 0.78f, 1.0f)
		: ImVec4(0.85f, 0.85f, 0.85f, 1.0f);

	ImVec4 onColor = ImGui::IsItemHovered()
		? ImVec4(0.64f, 0.83f, 0.34f, 1.0f)
		: ImVec4(0.56f, 0.83f, 0.26f, 1.0f);

	ImU32 bgColor = ImGui::GetColorU32(
		ImLerp(offColor, onColor, t)
	);

	drawList->AddRectFilled(
		p,
		ImVec2(p.x + width, p.y + height),
		bgColor,
		height * 0.5f
	);

	// つまみ
	drawList->AddCircleFilled(
		ImVec2(
			p.x + radius + t * (width - radius * 2.0f),
			p.y + radius
		),
		radius - 1.5f,
		IM_COL32(255, 255, 255, 255)
	);

	/* 押した瞬間を返す */
	return clicked;
}


void Editor::SaveIniFile()
{
	/* iniファイルの書き込み */

	// isEditorMode
	WritePrivateProfileStringA(
		"Editor",
		"isEditorMode",
		isEditorMode_ ? "true" : "false",
		".\\PonikoEngine.ini"
	);

	// 最後に触ったJsonパス
	WritePrivateProfileStringA(
		"Editor",
		"LastJsonPath",
		currentSceneJsonPath.c_str(),
		".\\PonikoEngine.ini"
	);

	// isCenter(Grid)
	WritePrivateProfileStringA(
		"Editor",
		"isGridCenter",
		grid.isCenter_ ? "true" : "false",
		".\\PonikoEngine.ini"
	);

	//* View行列からカメラのワールド行列を復元
	Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

	// 位置設定
	camera_.GetWorldTransform().translation_ = {
		cameraWorld.m[3][0],
		cameraWorld.m[3][1],
		cameraWorld.m[3][2],
	};
	camera_.GetWorldTransform().rotation_ = ExtractRotation(cameraWorld,{1.0f,1.0f,1.0f});

	// カメラの位置情報
	SaveIniFileToVector3(camera_.GetWorldTransform().translation_, "DebugCamera", "translation");
	SaveIniFileToVector3(camera_.GetWorldTransform().rotation_, "DebugCamera", "rotation");

}
void Editor::LoadIniFile()
{

	/* iniファイルの読込 */

	char buffer[MAX_PATH]{};

	// true / false読込
	GetPrivateProfileStringA(
		"Editor",
		"IsEditorMode",
		"false",
		buffer,
		sizeof(buffer),
		".\\PonikoEngine.ini"
	);

	isEditorMode_ = (strcmp(buffer, "true") == 0);


	GetPrivateProfileStringA(
		"Editor",
		"LastJsonPath",
		"",
		buffer,
		MAX_PATH,
		".\\PonikoEngine.ini"
	);

	// 現在のシーンのパスに設定
	currentSceneJsonPath = buffer;


	// true / false読込
	GetPrivateProfileStringA(
		"Editor",
		"isGridCenter",
		"false",
		buffer,
		sizeof(buffer),
		".\\PonikoEngine.ini"
	);

	grid.isCenter_ = (strcmp(buffer, "true") == 0);


	Vector3 translation;
	Vector3 rotation;

	// カメラの位置情報
	LoadIniFileToVector3(translation, "DebugCamera", "translation");
	LoadIniFileToVector3(rotation, "DebugCamera", "rotation");

	// viewにして再設定
	Matrix4x4 world = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation, translation);
	camera_.SetViewMatrix(Inverse(world));
}

// Vector3書き込み
void Editor::SaveIniFileToVector3(Vector3 v, const char* janre, const char* tag) {

	// 文字列に変換
	char buffer[256];

	// 位置
	sprintf_s(
		buffer, sizeof(buffer),
		"%f,%f,%f",
		v.x, v.y, v.z
	);
	WritePrivateProfileStringA(
		janre, tag, buffer, ".\\PonikoEngine.ini"
	);
}

// Vector3読込み
void Editor::LoadIniFileToVector3(Vector3& v, const char* janre, const char* tag){

	char buffer[256]{};

GetPrivateProfileStringA(
	janre,
	tag,
	"0,0,0",
	buffer,
	sizeof(buffer),
	".\\PonikoEngine.ini"
);

sscanf_s(
	buffer,
	"%f,%f,%f",
	&v.x,
	&v.y,
	&v.z
);

}

void Editor::DrawNotification(const char* text)
{


}

void Editor::Notification::PopText(const char* t, ImVec2 s)
{

	text = t;
	size = s;
	endSize_.x = s.x;

	isPop_ = true;

	ease.Reset();
	ease.Start(EasingVec::EASE_InOutExpo, easeTime * 60.0f, 0, endSize_.y);
}

// ここら辺ゴミコードすぎワロタ
void Editor::Notification::Draw()
{
	if (!isPop_) return;

	ease.Step();
	size.y = ease.getEasingNum();

	timer_ += Timer::GetDeltaTime() * 60.0f;

	// フェードアウト開始
	if (timer_ >= easeEndTime * 60.0f - easeTime * 60.0f) {
		isClose = true;
		ease.Reset();
		ease.Start(EasingVec::EASE_OutBack, easeTime* 60.0f, endSize_.y, 0);
	}
	
	// 終了
	if (timer_ >= easeEndTime* 60.0f) {

		isPop_ = false;
		timer_ = 0;
		size.y = 0;
		ease.Reset();

	}

	/*
		↑ｇｍ
					( ᓀ‸ᓂ)
		↓ネ申
	*/

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	// 画面サイズ
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	// 通知サイズ・位置
	ImVec2 min = {
		(displaySize.x - size.x) * 0.5f,
		(displaySize.y - size.y) * 0.2f
	};
	ImVec2 max = {
		min.x + size.x,
		min.y + size.y
	};

	// 角丸の背景
	drawList->AddRectFilled(
		min,
		max,
		ImGui::GetColorU32(backCol_),
		12.0f
	);

	// 文字
	ImVec2 textSize = ImGui::CalcTextSize(text);

	ImVec2 textPos = {
		min.x + (size.x - textSize.x) * 0.5f,
		min.y + (size.y - textSize.y) * 0.2f + 6.1f
	};

	drawList->AddText(
		textPos,
		ImGui::GetColorU32(textCol_),
		text
	);
}
