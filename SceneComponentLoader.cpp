#include "SceneComponentLoader.h"

#include "LightingManager.h"

SceneComponentLoader* SceneComponentLoader::GetInstance() {
	static SceneComponentLoader instance;
	return &instance;
}

bool SceneComponentLoader::Load(const std::string& filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	try {
		file >> jsonData_;
	}
	catch (const json::parse_error&) {
		return false;
	}

	return true;
}


bool SceneComponentLoader::Save(const std::string& filePath)
{
	// 親ディレクトリを自動生成
	std::filesystem::path path(filePath);
	std::filesystem::create_directories(path.parent_path());

	std::ofstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	file << jsonData_.dump(4);

	return true;
}

// ライトをセーブ
void SceneComponentLoader::LightSave() {
	if (jsonData_.empty()) {
		return;
	}

	// こっちのシングルトンのほうのjsonは消えてもクラスの方のメンバの方のは消えないのでcleanしちゃう
	jsonData_.clear();
	
	// ===DirectionalLight
	auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

	jsonData_["DirectionalLights"]["count"] = param.count;

	for (int i = 0; i < param.count; ++i) {

		auto& light = param.lights[i];

		jsonData_["DirectionalLights"]["lights"][i]["color"] = {
		 light.color.x,
		 light.color.y,
		 light.color.z,
		 light.color.w
		};

		jsonData_["DirectionalLights"]["lights"][i]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["DirectionalLights"]["lights"][i]["intensity"] =
			light.intensity;
	}


	// === PointLight
	auto& pParam = LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

	jsonData_["PointLights"]["count"] = pParam.count;

	for (int i = 0; i < pParam.count; ++i) {

		auto& light = pParam.lights[i];

		jsonData_["PointLights"]["lights"][i]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["PointLights"]["lights"][i]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["PointLights"]["lights"][i]["intensity"] =
			light.intensity;

		jsonData_["PointLights"]["lights"][i]["radius"] =
			light.radius;
	}


	// === SpotLight
	auto& sParam = LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

	jsonData_["SpotLights"]["count"] = sParam.count;

	for (int i = 0; i < sParam.count; ++i) {

		auto& light = sParam.lights[i];

		jsonData_["SpotLights"]["lights"][i]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["SpotLights"]["lights"][i]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["SpotLights"]["lights"][i]["intensity"] =
			light.intensity;

		jsonData_["SpotLights"]["lights"][i]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["SpotLights"]["lights"][i]["innerAngle"] =
			light.innerAngle;

		jsonData_["SpotLights"]["lights"][i]["outerAngle"] =
			light.outerAngle;

		jsonData_["SpotLights"]["lights"][i]["distance"] =
			light.distance;

		jsonData_["SpotLights"]["lights"][i]["falloff"] =
			light.falloff;
	}
}

// ライトをロード
void SceneComponentLoader::LightLoad() {
	if (jsonData_.empty()) {
		return;
	}

	// === DirectionalLight
	auto& dParam =
		LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

	if (jsonData_.contains("DirectionalLights")) {

		auto& data = jsonData_["DirectionalLights"];

		dParam.count = data.value("count", 0);

		for (int i = 0; i < dParam.count; ++i) {

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


void SceneComponentLoader::EffectParticleCompornentSave(std::string tag, ParticleCompornent& param_) {

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
	if (jsonData_.empty()) {
		return;
	}

	if (!jsonData_.contains("EffectParticleCompornents")) {
		return;
	}

	for (auto& [tag, data] : jsonData_["EffectParticleCompornents"].items()) {

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