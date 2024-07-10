#pragma once
#include <string>

#include "../../Libraries/nlohmann/json.hpp"


template <typename T>
class ISerializable
{
public:
	virtual T Deserialize(nlohmann::json& jsonObj) = 0;
	virtual nlohmann::json Serialize() = 0;
	virtual void SaveData() = 0;
};



