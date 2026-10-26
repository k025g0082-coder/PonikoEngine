#pragma once

#include "Texture.h"
#include "Camera.h"

//#include "DirectionalLight.h"
//#include "SpotLight.h"
//#include "PointLight.h"

#include "LightingManager.h"

#include "ShadowManager.h"

#include "Matrix.h"
#include "WorldTransform.h"

#include "Bone.h"
#include "Animation.h"

#include <string>

// モデルの頂点データ。要はVS.hlslに送るレイアウト
struct ModelVertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;

	VertexBoneWeightData boneWeight;
};

struct SkinningData
{
	Matrix4x4 boneMatrices[128];
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3]; // 16バイトアラインメントのためのpadding
	Matrix4x4 uvTransform;
};

struct MaterialData {
	std::string textureFilePath;
};

// アニメ有り無し
struct ModelSkinningParam {

	int32_t isSkinning;// アニメ有り無しフラグ
	float padding[3];
};

struct ModelData {

	std::vector<ModelVertexData> vertices;  // 頂点データ
	std::vector<uint32_t> indices;			// 管理番号(面数削減などのため？)

	MaterialData materials;   // マテリアル

	//std::vector <MaterialData> materials;   // 複数対応マテリアル
	// 
	// ボーン
	std::vector<Bone> bones;

	// 対応表？
	std::unordered_map<std::string, uint32_t> boneMap;

	uint32_t boneCount = 0;

	// アニメーション
	std::vector<Animation> animations;

	// アニメがあるか
	bool isSkinning = false;
};


class Model
{
public:
	
	// クローンを作る
	Model* Clone() const;

	// 静的メンバ関数
	static Model* CreateOBJ(const std::string& modelName);

	void Initialize();
	void Update();

	void Draw(Camera camera);
	void Draw(WorldTransform& world,Camera camera);

	// 指定された光源のおち影の描画をする
	void DrawShadow(Matrix4x4 lightView_, Matrix4x4 lightProjection);

	// アニメ更新
	void UpdateAnimation();

	// キーを探してLerpで補完する
	Vector3 SampleTransition(NodeAnimation& nodeAnim, float currentTick);
	Vector3 SampleScale(NodeAnimation& nodeAnim, float currentTick);
	Vector4 SampleRotation(NodeAnimation& nodeAnim, float currentTick);

	Vector4 Slerp(const Vector4& q0, const Vector4& q1, float t)
	{
		Vector4 q1Copy = q1;

		// 内積
		float dot =
			q0.x * q1.x +
			q0.y * q1.y +
			q0.z * q1.z +
			q0.w * q1.w;

		// 反対向きQuaternionなら短い方を使う
		if (dot < 0.0f)
		{
			q1Copy.x = -q1Copy.x;
			q1Copy.y = -q1Copy.y;
			q1Copy.z = -q1Copy.z;
			q1Copy.w = -q1Copy.w;

			dot = -dot;
		}

		// ほぼ同じならLerpでOK
		if (dot > 0.9995f)
		{
			Vector4 result{
				q0.x + (q1Copy.x - q0.x) * t,
				q0.y + (q1Copy.y - q0.y) * t,
				q0.z + (q1Copy.z - q0.z) * t,
				q0.w + (q1Copy.w - q0.w) * t
			};

			return Normalize(result);
		}

		// 角度
		float theta = acosf(dot);

		float sinTheta = sinf(theta);

		float weight0 =
			sinf((1.0f - t) * theta) / sinTheta;

		float weight1 =
			sinf(t * theta) / sinTheta;


		Vector4 result{
			q0.x * weight0 + q1Copy.x * weight1,
			q0.y * weight0 + q1Copy.y * weight1,
			q0.z * weight0 + q1Copy.z * weight1,
			q0.w * weight0 + q1Copy.w * weight1
		};

		return result;
	}

	// LightingManagerから空間すべてのライトを取得して、設定する
	void SetLightSets() {

		directionalLight_ = LightingManager::GetInstance()->GetLightSets().dLight;
		spotLight_ = LightingManager::GetInstance()->GetLightSets().sLight;
		pointLight_ = LightingManager::GetInstance()->GetLightSets().pLight;

		// ライト設定をONに
		materialData->enableLighting = true;
	}

	//// 光源登録
	//void SetDirectionalLight(DirectionalLight* light) { 

	//	directionalLight_ = light;

	//	// ライト設定をONに
	//	materialData->enableLighting = true;
	//}

	//// 光源登録
	//void SetPointLight(PointLight* light) {

	//	pointLight_ = light;

	//	// ライト設定をONに
	//	materialData->enableLighting = true;
	//}

	//// 光源登録
	//void SetSpotLight(SpotLight* light) {

	//	spotLight_ = light;

	//	// ライト設定をONに
	//	materialData->enableLighting = true;
	//}

	// アニメ番号変更
	void ChangeAnimeIndex(int index,bool isloop = true) {
		
		if (index < modelData.animations.size()) {
			animationIndex_ = index;
			animationTime_ = 0;
			isLoop_ = isloop;
		};
	}

	// アニメ一時停止か
	void AnimationStop() { isAnimation_ = false; }
	void AnimationPlay() { isAnimation_ = true; }

	// 再生中か
	bool IsAnimation() { return isAnimation_; }


	// ループ設定
	void SetAnimationLoop(bool b) { isLoop_ = b; }

	// アニメーションのスピードを設定
	void SetAnimationSpeed(float s) {
		animationSpeed = s;
	}

	void ModelColor(Vector4 rgba) {
		materialData->color = { rgba.x,rgba.y,rgba.z,rgba.w };
	};

	// モデルデータ読み取り関数
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	// 標準のWorldTransform
	WorldTransform& GetWorldTransform() { return worldTransform_; }

	
	// 設定
	void SetWorldTransform(const WorldTransform& w)
	{
		worldTransform_ = w;
	}

	
private:

	// マテリアル読み取り関数
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	// 再帰でグローバル行列を計算する関数
	void CalculateGlobalMatrices(int boneIndex, const aiMatrix4x4& parentGlobal);

	// マテリアル用のリソース
	ComPtr <ID3D12Resource> materialResource;

	// 頂点リソース
	ComPtr <ID3D12Resource> vertexResource;

	// モデルデータ
	ModelData modelData;

	// 時間計測アニメ用
	float animationTime_ = 0.0f;

	// 現在のアニメ番号
	uint32_t animationIndex_ = 0;

	// 再生ちゅうか
	bool isAnimation_ = true;
	
	bool isLoop_ = true;

	std::vector<Matrix4x4> boneMatrices_;

	// スキニングデータ転送用の
	Microsoft::WRL::ComPtr<ID3D12Resource> skinningResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> skinningParamResource_;

	SkinningData* skinningData_ = nullptr;
	ModelSkinningParam* skinningParam = nullptr;

	// WorldTransform,扱いやすくメンバ変数のほう
	WorldTransform worldTransform_;


	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// テクスチャ
	Texture* texture = nullptr;


	Material* materialData = nullptr;

	int lightMax = 10; // 一種の限界ライト数

	// Directional（直線）ライト
	DirectionalLight* directionalLight_;
	SpotLight* spotLight_;
	PointLight* pointLight_;

	ID3D12Resource* indexResource;
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// アニメーションスピード
	float animationSpeed = 1.0f;

	// 陰行列　転送用
	Microsoft::WRL::ComPtr<ID3D12Resource> shadowTransformResource_;
	ShadowTransform* shadowTransformData_ = nullptr;
};

