#include "EventCommand.h"

/*
    EventCommand
*/

// 保存
void to_json(nlohmann::json& j, const EventCommand& v)
{
    j = nlohmann::json{
        { "type", (int)v.type_ },
        { "elementRootName", v.elementRootName_ },
        { "elementAnimationName", v.elementAnimationName_ },
        { "elementTextName", v.elementTextName_ },
        { "elementTextChangeTexts", v.elementTextChangeTexts_ },
        { "TriggerName", v.TriggerName_ }
    };
}

// 読み込み
void from_json(const nlohmann::json& j, EventCommand& v)
{
    j.at("type").get_to(v.type_);
    j.at("elementRootName").get_to(v.elementRootName_);
    j.at("elementAnimationName").get_to(v.elementAnimationName_);
    j.at("elementTextName").get_to(v.elementTextName_);
    j.at("elementTextChangeTexts").get_to(v.elementTextChangeTexts_);
    j.at("TriggerName").get_to(v.TriggerName_);
}


/*
    EventCommandAsset
*/

// 保存
void to_json(nlohmann::json& j, const EventCommandAsset& v)
{
    j["eventName"] = v.eventName_;


    j["commands"] = nlohmann::json::array();

    for (const auto& command : v.commands_)
    {
        j["commands"].push_back(command);
    }

}

// 読み込み
void from_json(const nlohmann::json& j, EventCommandAsset& v)
{
    j.at("eventName").get_to(v.eventName_);


    v.commands_.clear();

    for (const auto& command : j.at("commands"))
    {
        v.commands_.push_back(command.get<EventCommand>());
    }
}