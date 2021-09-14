#pragma once
#include <string>
#include <deque>
#include "../../includes.hpp"

namespace notify {
	void render(ImDrawList* draw);
	void add_log(std::string pre, std::string text, Color color_pre, Color color_text = Color(255, 255, 255, 255), int life_time = 4700);
	namespace console {
		void clear();
		void notify(std::string pre, std::string msg, Color clr_pre);
	}
	namespace screen {
		void notify(std::string pre, std::string text, Color color_pre, Color color_text = Color(255, 255, 255, 255), int life_time = 4700);
	}
}