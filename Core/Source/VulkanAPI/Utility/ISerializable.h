#pragma once
#include <string>

#include "../../Libraries/nlohmann/json.hpp"


class ISerializableBase {
public:
    virtual void SaveData() = 0;
};
template <typename T>
class ISerializable : public ISerializableBase
{
public:
	virtual T Deserialize(nlohmann::json& jsonObj) = 0;
	virtual nlohmann::json Serialize() = 0;
	void SaveData() override = 0;
};



