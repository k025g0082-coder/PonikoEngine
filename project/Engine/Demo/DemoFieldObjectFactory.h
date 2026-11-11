#pragma once
#include "BaseFactory.h"

#include "DemoFieldObject.h"
#include "DemoGimmick.h"
#include "DemoFieldItem.h"

/*
	フィールド上の登録関数
*/

// ブロック等
class DemoFieldObjectFactory : public BaseFactory < DemoFieldObject::BlockType, DemoFieldObject, const GameObjectData&, nlohmann::json* >
{
public:
	// 初期登録
	static void Initialize();



};

// ギミック
class DemoGimmickFactory : public BaseFactory<DemoGimmick::GimmickType,DemoGimmick, const GameObjectData&, nlohmann::json* >
{
public:
	// 初期登録
	static void Initialize();

};


// アイテム
class DemoItemFactory : public BaseFactory<DemoFieldItem::ItemType, DemoFieldItem, const GameObjectData&, nlohmann::json* >
{
public:
	// 初期登録
	static void Initialize();

};
