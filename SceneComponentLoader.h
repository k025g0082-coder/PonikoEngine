#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

#include <functional>
#include <unordered_map>
#include <string>

#include "Effect.h"

#include "Vector.h"

#include "WorldTransform.h"
#include "ScreenTransform.h"

#include "HierarchyPanel.h"

#include <vector>
#include "Model.h"
#include "Text.h"

#include "Particle.h"
#include "LightingManager.h"


using json = nlohmann::json;


// Vector3->Jsonの形式に変換
json Vector3ToJson(const Vector3& value);
json Vector4ToJson(const Vector4& value);

// Json->Vector3の形式に変換
Vector3 JsonToVector3(const json& value);
Vector4 JsonToVector4(const json& value);


class SceneComponentLoader
{
public:
	// しんぐるとｎ
	static SceneComponentLoader* GetInstance();

	// JSON読み込み
	bool Load(const std::string& filePath);

	// JSONセーブ
	bool Save(const std::string& filePath);

	// JSON取得 直接
	json& GetJson(){
		return jsonData_;
	}

	// ワールドトランスフォーム読み取り
	WorldTransform GetWorldTransformData(std::string tag) {

		WorldTransform world;
		world.Initialize();
		world.UpdateMatrix();

		// もし存在しなかったら
		if (!jsonData_.contains(tag)) {
			return world;
		}

		return JsonToWorld(jsonData_[tag]);
	}

// スクリーントランスフォーム読み取り
	ScreenTransform GetScreenTransformData(std::string tag) {

		ScreenTransform screen;
		screen.Initialize();
		screen.UpdateMatrix();

		// もし存在しなかったら
		if (!jsonData_.contains(tag)) {
			return screen;
		}

		return JsonToScreen(jsonData_[tag]);
	}

	// ワールドトランスフォーム書き込み
	void SetWorldTransformData(std::string tag, WorldTransform world) {

			jsonData_[tag] = WorldToJson(world);

	};
	// スクリーントランスフォーム書き込み
	void SetScreenTransformData(std::string tag, ScreenTransform screen) {

			jsonData_[tag] = ScreenToJson(screen);

	};

	// === テキスト用
	void LoadTextData(std::string tag, Text* text) {

		text->Init();
		text->setFont("default");
		text->SetFontSize(20);

		// もし存在しなかったら
		if (!jsonData_.contains(tag)) {
			return;
		}
		TextDatas data = JsonToTextDatas(jsonData_[tag]);

		text->SetTextDatas(data);

		text->setFont(data.fontName);

	}

	void SaveTextData(std::string tag, Text* text) {
		jsonData_[tag] = TextDatasToJson(text->GetTextDatas());
	}


	// LightingManagerからライトをロード
	void LightLoad();
	
	// LightingManagerからライトをセーブ
	void LightSave();

	//こっちのシングルトンのほうのjsonDataは消えても、クラスの方のメンバの方のは消えないので中身をcleanしたいときに使う関数
	void ClearJson() { 
		jsonData_.clear();
		jsonGroupNames_.clear();
	}

	// Effect内のParticleCompornentのセーブ
	void EffectParticleCompornentSave(std::string tag, ParticleCompornent& param_);
	void EffectParticleCompornentLoad(std::map<std::string, ParticleCompornent>& particleEmitParams_);


	// グループ開始
	void BeginGroup(const std::string& groupName) {

		currentGroup_ = groupName;

		// グループ名登録
		jsonGroupNames_.push_back(groupName);
	
	}

	// グループ終了
	void EndGroup() {
		currentGroup_.clear();
	}

	// グループ保存
	void SaveGroup(const std::string& groupName);

	// グループ読み込み
	void LoadGroup(const std::string& groupName);

	// プロパティ登録。int,float,bool,stringなど対応できて凄い
	template<typename T>
	void Property(const std::string& name, T& value) {

		/*  　　　現在のグループ名　指定のタグ名　 */
		properties_[currentGroup_][name] = {
			[&value, name](json& group) {
				group[name] = value;
			},
			[&value, name](const json& group) {
				if (group.contains(name)) {
					value = group.at(name).get<T>();
				}
			}
		};
	}

	// 特殊な型なので、これらはそれぞれオーバーライド
	void Property(const std::string& name, Vector3& value);
	void Property(const std::string& name, WorldTransform& value);
	void Property(const std::string& name, ScreenTransform& value);
	void Property(const std::string& name, Text& value);


	// パーティクル
	void Property(const std::string& name, ParticleParam& value);

	// ライト
	void Property(const std::string& name, PointLightParam& value);
	void Property(const std::string& name, SpotLightParam& value);
	//void Property(const std::string& name, DirectionalLightParam& value);


	// モデルの登録
	void Property(const std::string& name, std::vector<Hierarchy< Model >> &value);

	//* モデルのIDやパスの登録
	void Property(const std::string& name, std::vector< ModelResource >& value);



	// Property内で設定したもののImGUIセーブボタン配置
	void SetImGuiSaveButton(std::string jsonPath);


private:

	json jsonData_;

	// グループの名前を記録
	std::vector<std::string> jsonGroupNames_;


	// WorldTransform->Jsonの形式に変換
	json WorldToJson(const WorldTransform& world);
	// WorldTransform->Jsonの形式に変換
	json ScreenToJson(const ScreenTransform& screen);

	// Json->Vector3の形式に変換
	WorldTransform JsonToWorld(const json& value);
	// Json->Vector2の形式に変換
	ScreenTransform JsonToScreen(const json& value);

	// Json <-> TextDatas
	TextDatas JsonToTextDatas(const json& value);
	json TextDatasToJson(const TextDatas& data);


	/* ラムダ式。 */

	/*
	
		properties_[現在のグループ名][指定のタグ名]　で登録されたProperty構造体内のラムダ式の関数を管理している

		要は楽になる便利な仕組みってこと 2026.10.21感想

	*/

	struct PropertyData {

		// JSONに値を書き込む処理を保存
		std::function<void(json&)> save;

		// JSONから値を読み込む処理を保存
		std::function<void(const json&)> load;

	};

	/*　Jsonのデータと各グループに紐づけられた名前のロード・セーブ関数　*/
	struct JsonData
	{
		json jsonData_;

		// タグー＞登録されてるタグのラムダ式の関数　というマップ
		std::unordered_map<
			std::string,
			std::unordered_map<std::string, PropertyData>
		> properties_;

	};

	// .Jsonのファイルパス名から上の情報を得る
	std::unordered_map<std::string, JsonData> JsonDatas_;

	
	/* 今開いているJsonのパス */
	std::string currentJsonPath_;


	//
	std::string currentGroup_;
	
};

