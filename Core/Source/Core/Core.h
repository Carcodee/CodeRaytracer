#pragma once
#include <iostream>
#include <string>
#include <functional>
namespace Core {

	void RunEngine(int widht, int height, std::string appName);

	void RunEngineEditor(int widht, int height, std::string appName, std::function<void()>&& editorContext);


}