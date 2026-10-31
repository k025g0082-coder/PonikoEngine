#include "SceneComponentLoader.h"

#include "LightingManager.h"
#include "ImGuiManager.h"

SceneComponentLoader* SceneComponentLoader::GetInstance() {
	static SceneComponentLoader instance;
	return &instance;
}

bool SceneComponentLoader::Load(const std::string& filePath)
{
	/* すでに読み込んでいる場合は成功としてReturn */
	if (JsonDatas_.contains(filePath)) {
		currentJsonPath_ = filePath;
		return true;
	}

	std::ifstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	try {

		file >> JsonDatas_[filePath].jsonData_;

		// 現在開いているJsonパス
		currentJsonPath_ = filePath;

	}
	catch (const json::parse_error&) {
		JsonDatas_.erase(filePath); // 読み込み失敗時はマップから削除
		return false;
	}



	return true;
}


bool SceneComponentLoader::Save(const std::string& filePath)
{
	/* もしSwitchJson関数で括ってなかったら、Return */
	if (currentJsonPath_ != filePath)
		return false;

	// 親ディレクトリを自動生成
	std::filesystem::path path(filePath);
	std::filesystem::create_directories(path.parent_path());

	std::ofstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	// 登録したGroupを一括でセーブ
	for (std::string groupName : JsonDatas_[currentJsonPath_].jsonGroupNames_) {
		SaveGroup(groupName);
	}

	file << JsonDatas_[filePath].jsonData_.dump(4);

	return true;
}

// ライトをセーブ
void SceneComponentLoader::LightSave() {
	
	// 今開いてるJsonを参照
	json& jsonData_ = JsonDatas_[currentJsonPath_].jsonData_;
	

	// ===DirectionalLight
	auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();


	/*
		シーンの固定ライト数を数える。
		で、見つかったらライトだけsaveIndexをつかって前詰めで登録している
	*/

	int saveIndex = 0;

	for (int i = 0; i < param.count; ++i) {

		auto& light = param.lights[i];

		// シーンの固定の光源か否か
		bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&light);

		// もしシーン固定の光源じゃないならセーブしない
		if (!isSceneLight) continue;



		jsonData_["DirectionalLights"]["lights"][saveIndex]["color"] = {
		 light.color.x,
		 light.color.y,
		 light.color.z,
		 light.color.w
		};

		jsonData_["DirectionalLights"]["lights"][saveIndex]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["DirectionalLights"]["lights"][saveIndex]["intensity"] =
			light.intensity;

		/* 加算 */
		++saveIndex;
	}

	/* 見つかった固定ライトの分だけ */
	jsonData_["DirectionalLights"]["count"] = saveIndex;
	saveIndex = 0;

	// === PointLight
	auto& pParam = LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();


	for (int i = 0; i < pParam.count; ++i) {

		auto& light = pParam.lights[i];


		// シーンの固定の光源か否か
		bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&light);

		// もしシーン固定の光源じゃないならセーブしない
		if (!isSceneLight) continue;


		jsonData_["PointLights"]["lights"][saveIndex]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["PointLights"]["lights"][saveIndex]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["PointLights"]["lights"][saveIndex]["intensity"] =
			light.intensity;

		jsonData_["PointLights"]["lights"][saveIndex]["radius"] =
			light.radius;

		// 加算
		++saveIndex;
	}


	/* 見つかった固定ライトの分だけ */
	jsonData_["PointLights"]["count"] = saveIndex;
	saveIndex = 0;


	// === SpotLight
	auto& sParam = LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();


	for (int i = 0; i < sParam.count; ++i) {

		// シーンの固定の光源か否か
		bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&sParam.lights[i]);

		// もしシーン固定の光源じゃないならセーブしない
		if (!isSceneLight) continue;


		auto& light = sParam.lights[i];

		jsonData_["SpotLights"]["lights"][saveIndex]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["SpotLights"]["lights"][saveIndex]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["SpotLights"]["lights"][saveIndex]["intensity"] =
			light.intensity;

		jsonData_["SpotLights"]["lights"][saveIndex]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["SpotLights"]["lights"][saveIndex]["innerAngle"] =
			light.innerAngle;

		jsonData_["SpotLights"]["lights"][saveIndex]["outerAngle"] =
			light.outerAngle;

		jsonData_["SpotLights"]["lights"][saveIndex]["distance"] =
			light.distance;

		jsonData_["SpotLights"]["lights"][saveIndex]["falloff"] =
			light.falloff;

		// かさん
		++saveIndex;
	}

	/* 見つかった分だけ */
	jsonData_["SpotLights"]["count"] = saveIndex;
	saveIndex = 0;
}

// ライトをロード
void SceneComponentLoader::LightLoad() {

	// 今開いてるJsonを参照
	json& jsonData_ = JsonDatas_[currentJsonPath_].jsonData_;


	// === DirectionalLight
	auto& dParam =
		LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();




	if (jsonData_.contains("DirectionalLights")) {

		auto& data = jsonData_["DirectionalLights"];

		dParam.count = data.value("count", 0);

		for (int i = 0; i < dParam.count; ++i) {

			// 固定の光源として読み込む
			LightingManager::GetInstance()->GetLightSets().dLight->SetIsSceneLight(i, true);


			auto& light = dParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Direction
			auto direction = json["direction"];
			light.direction = {
				direction[0],
				direction[1],
				direction[2]
			};

			light.intensity = json.value("intensity", 1.0f);
		}
	}


	// === PointLight
	auto& pParam =
		LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

	if (jsonData_.contains("PointLights")) {

		auto& data = jsonData_["PointLights"];

		pParam.count = data.value("count", 0);

		for (int i = 0; i < pParam.count; ++i) {

			// 固定の光源として読み込む
			LightingManager::GetInstance()->GetLightSets().pLight->SetIsSceneLight(i, true);


			auto& light = pParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Position
			auto position = json["position"];
			light.position = {
				position[0],
				position[1],
				position[2]
			};

			light.intensity = json.value("intensity", 1.0f);
			light.radius = json.value("radius", 10.0f);
		}
	}


	// === SpotLight
	auto& sParam =
		LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

	if (jsonData_.contains("SpotLights")) {

		auto& data = jsonData_["SpotLights"];

		sParam.count = data.value("count", 0);

		for (int i = 0; i < sParam.count; ++i) {

			// 固定の光源として読み込む
			LightingManager::GetInstance()->GetLightSets().sLight->SetIsSceneLight(i, true);


			auto& light = sParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Position
			auto position = json["position"];
			light.position = {
				position[0],
				position[1],
				position[2]
			};

			// Direction
			auto direction = json["direction"];
			light.direction = {
				direction[0],
				direction[1],
				direction[2]
			};

			light.intensity = json.value("intensity", 1.0f);
			light.innerAngle = json.value("innerAngle", 15.0f);
			light.outerAngle = json.value("outerAngle", 30.0f);
			light.distance = json.value("distance", 50.0f);
			light.falloff = json.value("falloff", 1.0f);
		}
	}

}


void SceneComponentLoader::SaveGroup(const std::string& groupName) {
	
	/* 今開いているJsonのグループ */
	json& group = GetCurrentJson()[groupName];

	// 登録されたsave関数をすべて実行
	for (auto& [name, property] : JsonDatas_[currentJsonPath_].properties_[groupName]) {
		property.save(group);
	}
}

// グループ読み込み
void SceneComponentLoader::LoadGroup(const std::string& groupName) {

	if (!GetCurrentJson().contains(groupName)) {
		return;
	}

	/* 今開いているJsonのグループ */
	const json& group = GetCurrentJson()[groupName];

	// 登録されたload関数をすべて実行
	for (auto& [name, property] : JsonDatas_[currentJsonPath_].properties_[groupName]) {
		property.load(group);
	}
}


void SceneComponentLoader::Property(const std::string& name, Vector3& value) {

	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {
		[this, &value, name](json& group) {
			group[name] = Vector3ToJson(value);
		},
		[&value, name](const json& group) {
			if (group.contains(name)) {
				value = JsonToVector3(group.at(name));
			}
		}
	};
}

void SceneComponentLoader::Property(const std::string& name, WorldTransform& value) {

	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		[this, &value, name](json& group) {
			group[name] = WorldToJson(value);
		},

		[this, &value, name](const json& group) {

	if (group.contains(name)) {
		WorldTransform loaded = JsonToWorld(group.at(name));

		value.translation_ = loaded.translation_;
		value.rotation_ = loaded.rotation_;
		value.scale_ = loaded.scale_;

		value.UpdateMatrix();
	}
}
	};
}

void SceneComponentLoader::Property(const std::string& name, ScreenTransform& value)
{
	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

	[this, &value, name](json& group) {
		group[name] = {
			{ "scale",  Vector3ToJson(value.scale_) },
			{ "rotation", Vector3ToJson(value.rotation_) },
			{ "translation",Vector3ToJson(value.translation_) }
		};
	},

	[this, &value, name](const json& group) {

		if (group.contains(name)) {

			value.scale_ = JsonToVector3(group[name]["scale"]);
			value.rotation_ = JsonToVector3(group[name]["rotation"]);
			value.translation_ = JsonToVector3(group[name]["translation"]);

			value.UpdateMatrix();

		}
	}
	};
}

void SceneComponentLoader::Property(const std::string& name, Text& value)
{
	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

	// セーブ
	[this, &value, name](json& group) {
		group[name] = TextDatasToJson(value.GetTextDatas());
	},

	// 読み取り
	[this, &value, name](const json& group) {

		if (group.contains(name)) {

			value.setFont("default");
			value.SetFontSize(20);
			}

			TextDatas data = JsonToTextDatas(group[name]);

			value.SetTextDatas(data);

			value.setFont(data.fontName);
		}
	};
}


// === テキスト用
void LoadTextData(std::string tag, Text* text) {

	

}


// パーティクル用設定
void SceneComponentLoader::Property(
	const std::string& name,
	ParticleParam& value)
{
	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		// 保存
		[this, &value, name](json& group) {
			json data;

			data["color"] = {
				value.color.x,
				value.color.y,
				value.color.z,
				value.color.w
			};

			data["speed"] = value.speed;
			data["speedAmp"] = value.speedAmp;
			data["gravity"] = value.gravity;
			data["scaleAmpMin"] = value.scaleAmpMin;
			data["scaleAmpMax"] = value.scaleAmpMax;
			data["life"] = value.life;
			data["isBilliboard"] = value.isBilliboard;
			data["isDilute"] = value.isDilute;
			data["blendMode"] = static_cast<int>(value.blendMode_);

			group[name] = data;
		},

		// 読み込み
		[&value, name](const json& group) {
			if (!group.contains(name)) {
				return;
			}

			const json& data = group.at(name);

			if (data.contains("color") &&
				data["color"].is_array() &&
				data["color"].size() >= 4) {
				value.color = {
					data["color"][0].get<float>(),
					data["color"][1].get<float>(),
					data["color"][2].get<float>(),
					data["color"][3].get<float>()
				};
			}

			if (data.contains("speed"))
				value.speed = data["speed"].get<float>();

			if (data.contains("speedAmp"))
				value.speedAmp = data["speedAmp"].get<float>();

			if (data.contains("gravity"))
				value.gravity = data["gravity"].get<float>();

			if (data.contains("scaleAmpMin"))
				value.scaleAmpMin = data["scaleAmpMin"].get<float>();

			if (data.contains("scaleAmpMax"))
				value.scaleAmpMax = data["scaleAmpMax"].get<float>();

			if (data.contains("life"))
				value.life = data["life"].get<int>();

			if (data.contains("isBilliboard"))
				value.isBilliboard = data["isBilliboard"].get<bool>();

			if (data.contains("isDilute"))
				value.isDilute = data["isDilute"].get<bool>();

			if (data.contains("blendMode")) {
				value.blendMode_ =
					static_cast<BlendMode>(
						data["blendMode"].get<int>());
			}
		}
	};
}


/* ライツ */
void SceneComponentLoader::Property(const std::string & name, PointLightParam & value)
{
	// 保存
	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		// 保存
		[this, &value, name](json& group) {

			json data;

			data["color"] = Vector4ToJson(value.color);
			data["position"] = Vector3ToJson(value.position);
			data["itensity"] = value.intensity;
			data["radius"] = value.radius;

			group[name] = data;
		},

		// 読み込み
		[&value, name](const json& group) {

			if (!group.contains(name)) {
				return;
			}
			const json& data = group.at(name);

			if (data.contains("color"))
				value.color = JsonToVector4(data["color"]);

			if (data.contains("position"))
				value.position = JsonToVector3(data["position"]);

			if (data.contains("itensity"))
				value.intensity = data["itensity"].get<float>();

			if (data.contains("radius"))
				value.radius = data["radius"].get<float>();

		}
	};


}

/* スポットライト */
void SceneComponentLoader::Property(
	const std::string& name,
	SpotLightParam& value)
{
	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		// 保存
		[this, &value, name](json& group) {

			json data;

			data["color"] = Vector4ToJson(value.color);
			data["position"] = Vector3ToJson(value.position);
			data["intensity"] = value.intensity;
			data["direction"] = Vector3ToJson(value.direction);

			data["innerAngle"] = value.innerAngle;
			data["outerAngle"] = value.outerAngle;

			data["distance"] = value.distance;
			data["falloff"] = value.falloff;

			group[name] = data;
		},

		// 読み込み
		[&value, name](const json& group) {

			if (!group.contains(name)) {
				return;
			}

			const json& data = group.at(name);

			if (data.contains("color"))
				value.color = JsonToVector4(data["color"]);

			if (data.contains("position"))
				value.position = JsonToVector3(data["position"]);

			if (data.contains("intensity"))
				value.intensity = data["intensity"].get<float>();

			if (data.contains("direction"))
				value.direction = JsonToVector3(data["direction"]);

			if (data.contains("innerAngle"))
				value.innerAngle = data["innerAngle"].get<float>();

			if (data.contains("outerAngle"))
				value.outerAngle = data["outerAngle"].get<float>();

			if (data.contains("distance"))
				value.distance = data["distance"].get<float>();

			if (data.contains("falloff"))
				value.falloff = data["falloff"].get<float>();
		}
	};
}

//void SceneComponentLoader::Property(const std::string & name, DirectionalLightParam & value)
//{}


void SceneComponentLoader::Property(const std::string& name, std::vector<Hierarchy<Model>>& value) {

	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		// save
		[this, &value, name](json& group) {

			// 空でも必ず配列として保存
			group[name] = json::array();

			for (const auto& hierarchy : value) {

				json item;

				item["type"] = hierarchy.type_;
				item["name"] = hierarchy.name_;

				item["id"] = hierarchy.id_;

				// ヒエラルキのトランスフォーム情報を（強引に）ワールドのにして保存
				item["transform"] = WorldToJson(
					*static_cast<WorldTransform*>(hierarchy.transform_.get())
				);

				// シーン内のカスタム設定。空じゃなけりゃ保存
				if (!hierarchy.json_.empty()) {
					item["propaties"] = hierarchy.json_;
				}


				group[name].push_back(item);

			}
		},

		// load
		[this, &value, name](const json& group) {

			// なかった場合
			if (!group.contains(name)) {
				return;
			}

			for (const auto& item : group.at(name)) {

				Hierarchy<Model> hierarchy;

				hierarchy.name_ = item.at("name").get<std::string>();
				hierarchy.id_ = item.at("id").get<std::string>();

				// ワールドトランスフォーム初期化　makeUniqueで生成！
				auto world = JsonToWorld(item.at("transform"));

				hierarchy.transform_ =
					std::make_unique<WorldTransform>(world);
				
				//
				if (item.contains("propaties")) {
					hierarchy.json_ = item.at("propaties");
				}

				// リストに追加
				value.push_back(std::move(hierarchy));

			}
		}
	};
}

// 
void SceneComponentLoader::Property(const std::string& name, std::vector<ModelResource>& value) {

	JsonDatas_[currentJsonPath_].properties_[currentGroup_][name] = {

		// save
		[this, &value, name](json& group) {

			// 空でも必ず配列として保存
			group[name] = json::array();

			for (const auto& resourceProp : value) {

				json item;

				item["pathId"] = resourceProp.pathId_;
				item["isView"] = resourceProp.isView_;

				group[name].push_back(item);

			}
		},

		// load
		[this, &value, name](const json& group) {

			// なかった場合
			if (!group.contains(name)) {
				return;
			}

			for (const auto& item : group.at(name)) {

				ModelResource modelResource;

				modelResource.pathId_   = item.at("pathId").get<std::string>();

				if (item.contains("isView")) {
					modelResource.isView_ = item.at("isView").get<bool>();
				}
				else {
					modelResource.isView_ = true;
				}

				// 既に同じIDがあるか検索
				auto it = std::find_if(
					value.begin(),
					value.end(),
					[&modelResource](const ModelResource& r) {
						return r.pathId_ == modelResource.pathId_;
					}
				);
				
				// 無ければ追加
				if (it == value.end()) {
					value.push_back(std::move(modelResource));
				}

			}
			
		}
	};
}

bool SceneComponentLoader::SaveSwitchJson(std::string jsonPath)
{

	if (!JsonDatas_.contains(jsonPath)) return false;

	// 切り替え
	currentJsonPath_ = jsonPath;

	return true;
}

void SceneComponentLoader::SetImGuiSaveButton(std::string jsonPath)
{
	// もしそのファイルパス名なかったら
	if (!JsonDatas_.contains(jsonPath)) return;

#ifdef USE_IMGUI
	if (ImGui::Button("Save")) {

		/* json切り替え */
		if (SaveSwitchJson(jsonPath)) {

			// 登録したGroupを一括でセーブ
			for (std::string groupName : JsonDatas_[jsonPath].jsonGroupNames_) {
				SaveGroup(groupName);
			}
		}
		ImGui::SameLine();
		ImGui::Text("saved!");
	}
	
#endif
}



void SceneComponentLoader::EffectParticleCompornentSave(std::string tag, ParticleCompornent& param_) {

	// 今開いてるJsonを参照
	json& jsonData_ = GetCurrentJson();


	auto& data = jsonData_["EffectParticleCompornents"][tag];

	data["particleTag"] = param_.tag;
	data["textureTag"] = param_.textureTag;

//	data["particlePath"] = param_.path;
		
	data["position"] = Vector3ToJson(param_.position);

	data["num"] = param_.num;


	data["param"]["color"] = Vector4ToJson(param_.param_.color);

	data["param"]["speed"] = param_.param_.speed;
	data["param"]["speedAmp"] = param_.param_.speedAmp;
	data["param"]["gravity"] = param_.param_.gravity;

	data["param"]["scaleAmpMin"] = param_.param_.scaleAmpMin;
	data["param"]["scaleAmpMax"] = param_.param_.scaleAmpMax;

	data["param"]["life"] = param_.param_.life;

	data["param"]["isBilliboard"] = param_.param_.isBilliboard;
	data["param"]["isDilute"] = param_.param_.isDilute;

}


void SceneComponentLoader::EffectParticleCompornentLoad(

	std::map<std::string, ParticleCompornent>& particleEmitParams_)
{
	// もし存在しないならreturn
	if (!GetCurrentJson().contains("EffectParticleCompornents")) {
		return;
	}

	for (auto& [tag, data] : GetCurrentJson()["EffectParticleCompornents"].items()) {

		ParticleCompornent& param = particleEmitParams_[tag];

		param.tag = data["particleTag"];
		param.textureTag = data["textureTag"];

//		param.path = data["particlePath"];

		// 使うモデルを登録（既にあるのは弾くのでそれは大丈夫）
		//Particle::GetInstance()->RegionParticleModel(param.tag, param.path);


		param.position = JsonToVector3(data["position"]);


		param.num = data["num"];


		param.param_.color = JsonToVector4(data["param"]["color"]);

		param.param_.speed = data["param"]["speed"];
		param.param_.speedAmp = data["param"]["speedAmp"];
		param.param_.gravity = data["param"]["gravity"];

		param.param_.scaleAmpMin = data["param"]["scaleAmpMin"];
		param.param_.scaleAmpMax = data["param"]["scaleAmpMax"];

		param.param_.life = data["param"]["life"];

		param.param_.isBilliboard = data["param"]["isBilliboard"];
		param.param_.isDilute = data["param"]["isDilute"];
	}
}

void SceneComponentLoader::BeginGroup(const std::string& groupName)
{
	currentGroup_ = groupName;

	if (currentJsonPath_.empty())return;

	/* 既に同じグループ名がないか検索 */
	auto it = std::find_if(
		JsonDatas_[currentJsonPath_].jsonGroupNames_.begin(),
		JsonDatas_[currentJsonPath_].jsonGroupNames_.end(),
		[&groupName](const std::string& s) {
			// find_ifの条件。trueにあったイテレータを返す
			return groupName == s;
		}
	);

	// 見つかった
	if (it != JsonDatas_[currentJsonPath_].jsonGroupNames_.end()) return;


	// グループ名登録
	JsonDatas_[currentJsonPath_].jsonGroupNames_.push_back(groupName);
	
}



json Vector3ToJson(const Vector3& value)
{
	return {
		value.x,
		value.y,
		value.z
	};
}
Vector3 JsonToVector3(const json& value)
{
	return Vector3{
		value[0].get<float>(),
		value[1].get<float>(),
		value[2].get<float>()
	};
}

json Vector4ToJson(const Vector4& value)
{
	return {
		value.x,
		value.y,
		value.z,
		value.w
	};
}
Vector4 JsonToVector4(const json& value)
{
	return Vector4{
		value[0].get<float>(),
		value[1].get<float>(),
		value[2].get<float>(),
		value[3].get<float>()
	};
}


// WorldTransform->Jsonの形式に変換
json  SceneComponentLoader::WorldToJson(const WorldTransform& world){
	return {
			{"scale",  Vector3ToJson(world.scale_)},
			{"rotation", Vector3ToJson(world.rotation_)},
			{"translation",Vector3ToJson(world.translation_)}
	};
}

// ScreenTransform->Jsonの形式に変換
json  SceneComponentLoader::ScreenToJson(const ScreenTransform& screen) {
	return {
			{"scale",  Vector3ToJson(screen.scale_)},
			{"rotation", Vector3ToJson(screen.rotation_)},
			{"translation",Vector3ToJson(screen.translation_)}
	};
}

// Json->WorldTransformの形式に変換
WorldTransform  SceneComponentLoader::JsonToWorld(const json& value) {

	WorldTransform world;
	world.Initialize();

	world.scale_ = JsonToVector3(value["scale"]);
	world.rotation_ = JsonToVector3(value["rotation"]);
	world.translation_ = JsonToVector3(value["translation"]);

	world.UpdateMatrix();

	return world;
}


// Json->ScreenTransformの形式に変換
ScreenTransform  SceneComponentLoader::JsonToScreen(const json& value) {

	ScreenTransform screen;
	screen.Initialize();

	screen.scale_ = JsonToVector3(value["scale"]);
	screen.rotation_ = JsonToVector3(value["rotation"]);
	screen.translation_ = JsonToVector3(value["translation"]);

	screen.UpdateMatrix();

	return screen;
}


TextDatas SceneComponentLoader::JsonToTextDatas(const json& value)
{
	TextDatas data;

	data.text = value["text"].get<std::string>();
	data.fontName = value["fontName"].get<std::string>();

	// textLayout
	const auto& layout = value["textLayout"];

	data.textLayout.textSize =
		layout["textSize"].get<int>();

	data.textLayout.textColor = Vector3{
		layout["textColor"][0].get<float>(),
		layout["textColor"][1].get<float>(),
		layout["textColor"][2].get<float>()
	};

	data.textLayout.outlineSize =
		layout["outlineSize"].get<int>();

	data.textLayout.outlineColor = Vector3{
		layout["outlineColor"][0].get<float>(),
		layout["outlineColor"][1].get<float>(),
		layout["outlineColor"][2].get<float>()
	};

	data.textLayout.isOutlineBlur =
		layout["isOutlineBlur"].get<bool>();

	// position
	data.position_ = Vector2{
		value["position"][0].get<float>(),
		value["position"][1].get<float>()
	};

	// anchor
	data.anchor_ =
		static_cast<TextAnchor>(value["anchor"].get<int>());

	return data;
}

json  SceneComponentLoader::TextDatasToJson(const TextDatas& data)
{
	json value;

	value["text"] = data.text;
	value["fontName"] = data.fontName;

	value["textLayout"] = {
		{"textSize", data.textLayout.textSize},

		{"textColor", {
			data.textLayout.textColor.x,
			data.textLayout.textColor.y,
			data.textLayout.textColor.z
		}},

		{"outlineSize", data.textLayout.outlineSize},

		{"outlineColor", {
			data.textLayout.outlineColor.x,
			data.textLayout.outlineColor.y,
			data.textLayout.outlineColor.z
		}},

		{"isOutlineBlur", data.textLayout.isOutlineBlur}
	};

	value["position"] = {
		data.position_.x,
		data.position_.y
	};

	value["anchor"] =
		static_cast<int>(data.anchor_);

	return value;
}