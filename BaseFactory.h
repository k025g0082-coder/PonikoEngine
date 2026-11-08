#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

/// <summary>
/// 
/// 特定の型から登録された、特定のラムダ処理を行わせる親クラス。主に継承して使う
/// typename... 可変長テンプレート。Argsにより子クラスの引数に合わせた自由な型にできる
/// 
/// 用途・・ステージの読込後のオブジェクトのIDによる初期化処理などに例）GimmickFactory
///			JSONファイルを各自で展開して情報をやり取りする
/// </summary>

template <typename KeyType, typename CoreType, typename... Args>

class BaseFactory
{
public:

	// 登録されるラムダ式関数
	using CreatorFunc = std::function<std::unique_ptr<CoreType>(Args...)>;

	//
	virtual ~BaseFactory() = default;

	/// <summary>
	/// クラス登録
	/// </summary>
	/// <param name="key">キー</param>
	/// <param name="func">ラムダ式。Ｃｒｅａｔｅ登録時に行わせる処理</param>
	static void Register(const KeyType& key, CreatorFunc func) {
		GetRegistry()[key] = func; //
	}

	// クラス生成
	static std::unique_ptr<CoreType> Create(const KeyType& key, Args... args) {

		// 同じキーを検索
		auto& registry = GetRegistry();
		auto it = registry.find(key);

		if (it != registry.end()) {// あったら

			/*
				登録しているラムダ式(作り方)を実行してからreturnする
			*/

			// 引数に設定された引数を渡してnew
			return it->second(std::forward<Args>(args)...);

		}

		return nullptr;
	}

private:

	// キーの型->登録の型 マップ - 永続的なデータをもっている
	static std::unordered_map<KeyType, CreatorFunc>& GetRegistry() {
		static std::unordered_map<KeyType, CreatorFunc> registry_;// 実質ｼﾝｸﾞﾙﾄﾝ
		return registry_;
	}
};
