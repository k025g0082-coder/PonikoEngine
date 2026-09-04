#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

#include "Vector.h"
#include "WorldTransform.h"

using json = nlohmann::json;


// Vector3->Jsonの形式に変換
json Vector3ToJson(const Vector3& value);

// Json->Vector3の形式に変換
Vector3 JsonToVector3(const json& value);

// WorldTransform->Jsonの形式に変換
json WorldToJson(const WorldTransform& world);

// Json->Vector3の形式に変換
WorldTransform JsonToWorld(const json& value);


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

	// LightingManagerからライトをロード
	void LightLoad();
	
	// LightingManagerからライトをセーブ
	void LightSave();

private:

	json jsonData_;
};

