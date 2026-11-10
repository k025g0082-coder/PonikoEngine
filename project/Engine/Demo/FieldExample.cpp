#include "FieldExample.h"
#include "Scene.h"

#include "Editor.h"
#include "HierarchyPanel.h"

#include "GameObject.h"
#include "DemoFieldObjectFactory.h"
#include "PostEffectManager.h"

#include "ImGuiManager.h"
#include "UIManager.h"

#include <utility>

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

	/* モデル（仮） */
	playerModel_ = Model::CreateOBJ("resources/Models/cube/cube.obj");
	blockModel_  = Model::CreateOBJ("resources/Models/cube/cube.obj");
	goalModel_   = Model::CreateOBJ("resources/Models/cube/cube.obj");

	// factoryら 初期化
	DemoFieldObjectFactory::Initialize();
	DemoGimmickFactory::Initialize();

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


}

void FieldExample::Update(Input * input)
{
	input;
	
	if (player_) {

		// カメラ
		Vector3 targetPosition = player_->GetWorldTransform().translation_;
		Vector3 targetRotation = player_->GetWorldTransform().rotation_;

		Vector3 targetOffsetPos = { -6.487f,9.148f,-24.729f };
		Vector3 targetOffsetRot = { 0.300f,0.171f,-0.000f };

		// プレイヤーに追跡
		camera_.GetWorldTransform().translation_ = targetPosition + targetOffsetPos;
		camera_.GetWorldTransform().rotation_ = targetRotation + targetOffsetRot;

		/* 
			プレイヤー更新
		*/
		player_->Update();
	}

	camera_.Update();


	/* 更新 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {
		obj->Update();
	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {

		gmk->Update();

		// プレイヤーとギミック判定
		if (player_)
			DemoFieldObject::Collision(player_.get(), gmk.get());
	}

#ifdef USE_IMGUI
	ImGui::Begin("FieldExample");

	//if (ImGui::DragFloat3("playerVelocity", &player_->velocity_.x, 0.1f)) {
	//	// なんかあれば
	//}]


	//
	if (ImGui::Button("vfx")) {
		s
	}


	/*
		てすとUI あにめ
	*/
	if (ImGui::Button("アニメ")) {

		UIManager::PlayAnimationElement(
			"Canvas",
			"element_1",
			"NewAnimetest"
		);
	};


	if (ImGui::Button("Reload")) {

		blocks_.clear();
		gimmicks_.clear();

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

		player_ = std::make_unique<DemoPlayer>(playerModel_, &camera_, data_,&hierarchy.json_);

		/* bloomテスト */
		PostEffectManager::GetInstance()->AddBloomModel(playerModel_, &player_->GetWorldTransform());

		// ヒエラルキ側にも登録
		hierarchy.gameObject_ = player_.get();
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
		object = gimmicks_.back().get();

		break;

	}

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

	};

}

void FieldExample::RecordUniqueId(DemoFieldObject* object)
{
	// UniqueIDを取り出して記録
	uniqueIdMap_[object->GetUniqueID()] = object;

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
