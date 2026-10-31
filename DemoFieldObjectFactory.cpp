#include "DemoFieldObjectFactory.h"

#include "DemoPlayer.h"

void DemoFieldObjectFactory::Initialize()
{
	/* 事前に必要な処理を登録 */
	/*Register(DemoFieldObject::ObjectType::kBlock, []() { return std::make_unique<DemoBlock>(); });*/

}
void DemoGimmickFactory::Initialize()
{
	//
	/*Register(DemoGimmick::GimmickType::kSwitch, []() { return std::make_unique<DemoSwitch>(); });
	Register(DemoGimmick::GimmickType::kGoal, []() { return std::make_unique<DemoGoal>(); });*/
}


