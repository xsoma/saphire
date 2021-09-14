#include "logger.hpp"
#include "../render.hpp"

struct notify_t
{
	float life_ime_local;
	float life_ime;
	std::string type;
	std::string message;
	Color c_type;
	Color c_message;
	float x;
	float y = -15;
	float max_x;
};

std::deque<notify_t> notifications;

notify_t find_notify(std::string pre_text, std::string body)
{
	for (size_t i = 0; i < notifications.size(); i++)
		if (notifications[i].type == pre_text && notifications[i].message == body)
			return notifications[i];
	return notify_t();
}
#include "../menu.h"
#include "../postprocessing/PostProccessing.h"
namespace notify
{
	void render(ImDrawList* draw)
	{
		if (notifications.empty())
			return;

		float last_y = 0;
		int x, y;
		m_engine()->GetScreenSize(x, y);
		for (size_t i = 0; i < notifications.size(); i++)
		{
			auto& notify = notifications.at(i);

			const auto pre = notify.type.c_str();
			const auto text = notify.message.c_str();
			ImVec2 textSize = c_menu::get().font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, notify.type.c_str());

			std::string all_text;
			all_text += pre;
			all_text += "";
			all_text += text;

			ImVec2 all_textSize = c_menu::get().font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, all_text.c_str());

			notify.y = math::lerp(notify.y, (i * 17.f), 0.05f);

			if (notify.y > y + 17) {
				continue;
			}

			if (util::epoch_time() - notify.life_ime_local > notify.life_ime)
			{
				if ((notify.x + all_textSize.x + 16) < 0) {
					notifications.erase(notifications.begin() + i);
					continue;
				}

				notify.max_x = all_textSize.x + 16;

				notify.x = math::lerp(notify.x, (notify.max_x * -1) - 10, 0.05f);

				int procent_x = (100 * (notify.max_x + notify.x)) / notify.max_x;

				auto opacity = int((255 / 100) * procent_x);

				if (procent_x >= 0 && procent_x <= 100)
				{
					notify.c_message = Color(notify.c_message);
					notify.c_message.SetAlpha(opacity);
					notify.c_type = Color(notify.c_type);
					notify.c_type.SetAlpha(opacity);
				}
				else
				{
					notify.c_message = Color(notify.c_message);
					notify.c_message.SetAlpha(255);
					notify.c_type = Color(notify.c_type);
					notify.c_type.SetAlpha(255);
				}
			}

			float box_w = (float)fabs(0 - (all_textSize.x + 16));
			auto main_colf = ImColor(39, 39, 39, 240);
			auto main_coll = ImColor(39, 39, 39, 0);
			draw->AddRectFilledMultiColor(ImVec2(0.f, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2), main_colf, main_coll, main_coll, main_colf);
			auto main_colf2 = ImColor(39, 39, 39, 100);
			draw->AddRectFilledMultiColor(ImVec2(0.f, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2), main_colf2, main_coll, main_coll, main_colf2);
			draw->AddRectFilled(ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 18, last_y + notify.y + all_textSize.y + 2), ImColor(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 150));
			draw->AddText(ImVec2(notify.x + 3, last_y + notify.y), ImColor(notify.c_type.r(), notify.c_type.g(), notify.c_type.b()), pre);
			draw->AddText( ImVec2(notify.x + 13 + textSize.x, last_y + notify.y), ImColor(notify.c_message.r(), notify.c_message.g(), notify.c_message.b()), text);
			
		}
	}

	void add_log(std::string pre, std::string text, Color color_pre, Color color_text, int life_time)
	{
		screen::notify(pre, text, color_pre, color_text, life_time);
		console::notify(pre, text, color_pre);
	}

	namespace console {
		void clear() {
			m_engine()->ExecuteClientCmd("clear");
		}

		void notify(std::string pre, std::string msg, Color clr_pre) {
			m_cvar()->ConsoleColorPrintf(vars.misc.menu_color, "[saphire] ");
			m_cvar()->ConsoleColorPrintf(clr_pre, "[%s]", pre.data());
			m_cvar()->ConsolePrintf(" %s\n", msg.data());
		}
	}

	namespace screen
	{
		void notify(std::string pre, std::string text, Color color_pre, Color color_text, int life_time) {
			std::string type_buf;
			type_buf += "[";
			type_buf += pre;
			type_buf += "]";

			notifications.push_front(notify_t{ static_cast<float>(util::epoch_time()), (float)life_time, type_buf, text, color_pre, color_text });
		}
	}
}