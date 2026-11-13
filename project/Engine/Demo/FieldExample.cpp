#include "FieldExample.h"
#include "Scene.h"

#include "Editor.h"
#include "HierarchyPanel.h"

#include "GameObject.h"
#include "DemoFieldObjectFactory.h"

#include "SceneComponentLoader.h"

#include "ImGuiManager.h"
#include "UIManager.h"

#include "Timer.h"

#include "Random.h"
#include "DebugLine.h"

#include <utility>
#include <algorithm>

std::unordered_map<uint32_t, DemoFieldObject*> FieldExample::uniqueIdMap_;

/*
	シーンを登録する
*/
REGISTER_SCENE(FieldExample);

void FieldExample::Init()
{
	camera_.Initialize();

	// カメラセット
	Scene::SetSceneCamera(&camera_);

	/* モデル */
	playerModel_ = Model::CreateOBJ("resources/Models/block/cube.obj");

	// 赤
	playerModel_->ModelColor({ 1.0f,0,0,1.0f });


	meteoModel_ = Model::CreateOBJ("resources/Models/block/cube.obj");
	// 赤
	meteoModel_->ModelColor({ 1.0f,0,0,1.0f });


	emitMeteoRangeAABB.offset  = { 2.9f,13.2f,-16.7f };

	emitMeteoRangeAABB.width_  = 63.2f;
	emitMeteoRangeAABB.height_ = 2.0f;
	emitMeteoRangeAABB.depth_  = 40.9f;

	naviText.Init();
	naviText.setFont("default");


	// factoryら 初期化
	DemoFieldObjectFactory::Initialize();
	DemoGimmickFactory::Initialize();
	DemoItemFactory::Initialize();

	/* 増やした / 消されたときのラムダ関数を登録しておく */
	CreateFunctionRegion();
	DeleteFunctionRegion();

	/*
		使う（ヒエラルキーを取得する）レイヤーを宣言
	*/
	Editor::GetInstance()->GetHierarchyPanel().SetUseGameObjectLayer("test");
	Editor::GetInstance()->GetHierarchyPanel().SetUseGameObjectLayer("test2");

	// ロード
	FieldLoad();


	//  パーチクル用 -　データ読込
	if (SceneComponentLoader::GetInstance()->Load("resources/Effect/ParticleProp.json")) {

		// ※注　ポインタ
		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		loder->BeginGroup("particle");

		loder->Property("meteoEmit", meteoEmitParam);
		loder->Property("meteoBreakEmit", meteoBreakEmitParam);

		loder->EndGroup();

		/* 読込 */
		loder->LoadGroup("particle");
	}

}

void FieldExample::Update(Input * input)
{
	input;
	
	if (player_) {

		// カメラ
		Vector3 targetPosition = player_->GetWorldTransform().translation_;
		Vector3 targetRotation = player_->GetWorldTransform().rotation_;

		Vector3 targetOffsetPos = Vector3{ -1.262f,18.762f,-20.052f };
			
		Vector3 targetOffsetRot = Vector3{ 0.708f,0.058f,-0.000f };

		// プレイヤーに追跡
		camera_.GetWorldTransform().translation_ = targetPosition + targetOffsetPos;
		camera_.GetWorldTransform().rotation_ = targetRotation + targetOffsetRot;

		/*
			プレイヤー更新
		*/
		player_->Update();

		// もしプレイヤーがフィールドの下に落ちたら
		if (player_.get()->GetWorldTransform().translation_.y <= -60.0f) {

			player_->Respown();

		}


		camera_.Update();

	}

	/*
		定期的に隕石落下
	*/
	float emitDistance = 0.3f;

	if (emitMeteoElapsedTimer >= emitDistance) {
		
		EmitMeteo();

		emitMeteoElapsedTimer -= emitDistance;
	}
	emitMeteoElapsedTimer += Timer::GetDeltaTime();
	


	/* 更新 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {


		obj->Update();


		// 隕石、ブロックと判定
		for (DemoFieldMeteo& meteo : meteos_) {

			if (IsCollision(obj->GetAABB(), meteo.GetAABB())) {

				if (!meteo.IsBreak())
					meteo.Break();// 破壊

			}
		}

	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {

		gmk->Update();

		// プレイヤーとギミック判定
		if (player_)
			DemoFieldObject::Collision(player_.get(), gmk.get());
	}

	for (const std::unique_ptr<DemoFieldItem>& obj : items_) {

		obj->Update();

		// プレイヤーと当たり判定判定
		if (player_)
			DemoFieldObject::Collision(player_.get(), obj.get());
	}

	/*
		隕石更新
	*/
	for (DemoFieldMeteo& meteo : meteos_) {
		meteo.Update();
	}


	//
	DeleteJudgeFieldObject(blocks_);
	DeleteJudgeFieldObject(gimmicks_);
	DeleteJudgeFieldObject(items_);

	/*
		隕石削除
	*/
	std::erase_if(
		meteos_,
		[](const DemoFieldMeteo& meteo) {
			return meteo.IsDelete();
		}
	);

	static Vector3 emitMeteoPosition;
	emitMeteoRangeAABB.UpdatePosition(emitMeteoPosition);

#ifdef USE_IMGUI
	ImGui::Begin("FieldExample");

	static Vector3 emitPos;

	if (ImGui::Button("隕石")) {
		auto& meteo = meteos_.emplace_back();
		meteo.Initialize(meteoModel_, emitPos,meteoEmitParam,meteoBreakEmitParam);
	};
	ImGui::DragFloat3("隕石Ｐｏｓ", &emitPos.x, 0.1f);
	ImGui::DragFloat3("隕石Velocity", &DemoFieldMeteo::velocity.x, 0.1f);

	emitMeteoRangeAABB.ImGuiView();


	/*
		パーティクルIMGUI
	*/
	ImGui::SeparatorText("EmitPropaties");

	if (ImGui::TreeNode("meteoEmitParam")) {
		meteoEmitParam.ImGuiView();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("meteoBreakEmitParam")) {
		meteoBreakEmitParam.ImGuiView();
		ImGui::TreePop();
	}

	// セーブ
	if (ImGui::Button("save")) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// jsonに
		if (loder->SaveSwitchJson("resources/Effect/ParticleProp.json")) {

			/* 登録 */
			loder->SaveGroup("particle");

			// JSONファイルに保存する
			loder->Save("resources/Effect/ParticleProp.json");
		}
	}


	//naviText.ImGuiView();
	//ImGui::Text("mousePosX:%f mousePosY:%f", Input::GetInstance()->GetMousePosition().x, Input::GetInstance()->GetMousePosition().y);
	//camera_.SphericalImGuiView();
	//player_->ImGuiView();



	/*
		てすとUI あにめ
	*/
	//if (ImGui::Button("アニメ")) {

	//	UIManager::PlayAnimationElement(
	//		"Canvas",
	//		"element_1",
	//		"NewAnimetest"
	//	);
	//};

	ImGui::Separator();

	if (ImGui::Button("Reload")) {

		blocks_.clear();
		gimmicks_.clear();
		items_.clear();

		FieldLoad();

	}

	ImGui::End();
#endif

}

void FieldExample::Draw()
{
	if (player_)
		player_->Draw();


	/* 描画 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {
		obj->Draw();
	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {
		gmk->Draw();
	}

	for (const std::unique_ptr<DemoFieldItem>& itm : items_) {
		itm->Draw();
	}

	/*
		隕石描画
	*/
	for (DemoFieldMeteo& meteo : meteos_) {
		meteo.Draw();
	}

	//
	naviText.SetFontOutlineSize(8);
	naviText.SetFontSize(15);

	naviText.drawString("移動/WASD", { 0,0 }, TextAnchor::LeftTop);


	// 隕石範囲の描画
	DebugLine::GetInstance()->DrawAABB(emitMeteoRangeAABB, { 1.0f,0,0,1.0f });

	
}
void FieldExample::FieldLoad() {

	/* 
	
		宣言したレイヤー名のリスト内に配置されたオブジェクト全て読込
		
	*/

	for (const std::string& s : *Editor::GetAlignHierarchyLayerNames()) {
		FieldLayerLoad(s);
	}

}



void FieldExample::FieldLayerLoad(std::string layerName)
{

	// Editorからレイヤーでしわけられた（名前で指定）ヒエラルキーリストをもらう
	auto& hierarchys = Editor::GetHierarchyModelLayer(layerName);

	// レイヤーなかったら
	if (hierarchys.empty()) return;

	/* エディター側ー＞ゲーム側に　データを受け渡す！！！！ */
	for (HierarchyNode*h : hierarchys) {

		if (h->type_ != HierarchyType::kModel) return;

		auto model = dynamic_cast<ModelNode*>(h);

		/* ヒエラルキーから,転送用データに変換する */
		GameObjectData data_ = GameObject::ConvertGameData(*model);

	
		uint32_t value;

		uint32_t defaultType = static_cast<uint32_t>(DemoFieldObject::Type::kBlock);

		/* ファクトリーに登録された、クラス生成の型である識別idを、ヒエラルキーJSONから取得 */
		if (!model->json_.contains("objectType")) {
			
			/*** 無い場合デフォルトでブロック ***/
			value = defaultType;

			model->json_["objectType"] = defaultType;
	
		}
		else {

			// タイプ名取得
			value = model->json_["objectType"].get<uint32_t>();

			// nullなら（しつこい）
			if (value == (int)DemoFieldObject::Type::Null)
				value = defaultType;
		}

		// タイプを記録しとく
		data_.objectID = value;

		value = 0;

		// ==読込２
		if (!model->json_.contains("subObjectType")) {

			// なければ０番
			model->json_["subObjectType"] = 0;

		}
		else {
			// タイプ名取得
			value = model->json_["subObjectType"].get<uint32_t>();
		}

		// サブタイプを記録しとく
		data_.subObjectID = value;


		/* 生成！！！！ */
		CreateGameObject(data_.objectID,data_.subObjectID, *model);

		
	}

}

/*                            */
/*   ゲームオブジェクト生成   */
/*                            */

void FieldExample::CreateGameObject(const uint32_t& type, const uint32_t& subType, ModelNode& hierarchy)
{
	//
	if (type == (uint32_t)DemoFieldObject::Type::Null)
		return;


	/* ヒエラルキーから,転送用データに変換する */
	GameObjectData data_ =
		GameObject::ConvertGameData(hierarchy);

	/* クローンを生成 */
	data_.model = data_.model->Clone();
	data_.model->GetModelColor() = hierarchy.modelColor_;// モデルのクローン後、個別にModelNodeで保存された色を設定

	data_.objectID = type;
	data_.subObjectID = subType;

	//
	// シーン側に,それぞれのタイプに登録された、SubIDで振り分けられたラムダ式をまとめて実行して生成！！！	
	//

	// キー
	DemoFieldObject::Type type_ = static_cast<DemoFieldObject::Type>(type);

	DemoFieldObject* object = nullptr;

	switch (type_) {

		//
		//  * プレイヤー *
		//
	case DemoFieldObject::Type::kPlayer:

		/* もうすでにいる場合 */
		if (player_ != nullptr) {
			player_.release();
		}

		player_ = std::make_unique<DemoPlayer>(playerModel_, &camera_, data_, &hierarchy.json_);
		player_->Initialize();


		/* bloomテスト */
		///PostEffectManager::GetInstance()->AddBloomModel(playerModel_, &player_->GetWorldTransform());

		// ヒエラルキ側にも登録
		hierarchy.gameObject_ = player_.get();

		// GameObject側
		player_.get()->SetHierarchy(&hierarchy);


		object = player_.get();

		break;

		//
		//    ブロック
		//
	case DemoFieldObject::Type::kBlock:

		blocks_.push_back(
			std::move(
				DemoFieldObjectFactory::Create(
					static_cast<DemoFieldObject::BlockType>(data_.subObjectID), data_, &hierarchy.json_)));

		// ヒエラルキ側にも登録
		hierarchy.gameObject_ = blocks_.back().get();

		// GameObject側
		blocks_.back().get()->SetHierarchy(&hierarchy);

		object = blocks_.back().get();

		break;

		//
		//    ギミック
		//
	case DemoFieldObject::Type::kGimmick:

		gimmicks_.push_back(
			std::move(
				DemoGimmickFactory::Create(
					static_cast<DemoGimmick::GimmickType>(data_.subObjectID), data_, &hierarchy.json_)));

		// ヒエラルキ側にも登録
		hierarchy.gameObject_ = gimmicks_.back().get();

		// GameObject側
		gimmicks_.back().get()->SetHierarchy(&hierarchy);



		object = gimmicks_.back().get();

		break;

		//
		//    アイテム
		//
	case DemoFieldObject::Type::kItem:

		items_.push_back(
			std::move(
				DemoItemFactory::Create(
					static_cast<DemoFieldItem::ItemType>(data_.subObjectID), data_, &hierarchy.json_)));

		// ヒエラルキ側にも登録
		hierarchy.gameObject_ = items_.back().get();

		// GameObject側
		items_.back().get()->SetHierarchy(&hierarchy);


		object = items_.back().get();

		break;
	}

	//
	object->Initialize();


	// 各FieldObjectの、uniqueIDを記録
	RecordUniqueId(object);

}


void FieldExample::CreateFunctionRegion()
{
	//
	//     エディタ側で実行される、生成関数の登録
	//
	
	auto& createFunc = Editor::GetInstance()->GetHierarchyPanel().gameObjectAdd_;

	//
	createFunc = [this](const uint32_t& type, const uint32_t& subType,ModelNode& hierarchy) {

		CreateGameObject(type,subType, hierarchy);

	};

}
void FieldExample::DeleteFunctionRegion()
{
	//
	//     エディタ側で実行される、削除関数の登録
	//

	auto& deleteFunc = Editor::GetInstance()->GetHierarchyPanel().gameObjectDelete_;

	//
	deleteFunc = [this](ModelNode& hierarchy) {

		// テンプレート削除関数をつかって検索
		if (DeleteObject(blocks_, hierarchy))return;
		if (DeleteObject(gimmicks_, hierarchy))return;
		if (DeleteObject(items_, hierarchy))return;

	};

}

void FieldExample::RecordUniqueId(DemoFieldObject* object)
{
	// UniqueIDを取り出して記録
	uniqueIdMap_[object->GetUniqueID()] = object;

}

void FieldExample::EmitMeteo()
{
	/*
		フィールド上の指定された範囲のランダムな場所を決定してエミットする
	*/
	Vector3 emitPos;

	emitPos.x = Random(emitMeteoRangeAABB.min.x, emitMeteoRangeAABB.max.x);
	emitPos.y = Random(emitMeteoRangeAABB.min.y, emitMeteoRangeAABB.max.y);
	emitPos.z = Random(emitMeteoRangeAABB.min.z, emitMeteoRangeAABB.max.z);

	auto& meteo = meteos_.emplace_back();
	meteo.Initialize(meteoModel_->Clone(), emitPos, meteoEmitParam, meteoBreakEmitParam);

}

//
//std::unique_ptr<GameObject> FieldExample::ConvertHierarchyToGameObject(std::string hierarchyLayerName,Hierarchy<Model> hierarchy)
//{
//	/* ヒエラルキーのレイヤー名と、一致しなかったら */
//	if (hierarchy.name_ != hierarchyLayerName) 
//		return nullptr;
//
//
//	// キャスト
//	auto* world = dynamic_cast<WorldTransform*>(hierarchy.transform_.get());
//
//	GameObjectData data_;
//
//	/* ヒエラルキーから情報取り出す */
//	data_.position = world->translation_;
//	data_.rotation = world->rotation_;
//	data_.scale = world->scale_;
//
//	//
//	/* モデルそのまんま渡してるから、多分いつかCloneとかするかな..でも個別に色変えたい場合とかはだけどポインタだから全部変わる・・うーｎ */
//	//
//
//	std::unique_ptr<GameObject> result;
//
//
//}
//
