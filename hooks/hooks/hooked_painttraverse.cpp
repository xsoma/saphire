#include "..\hooks.hpp"
#include "..\..\cheats\menu.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\visuals\player_esp.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\visuals\grenade_warning.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\visuals\Hitmarker.h"
#include "..\..\cheats\visuals\world_esp.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\visuals\bullet_tracers.h"
#include "..\..\cheats\visuals\dormant_esp.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

using PaintTraverse_t = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);

bool reload_fonts()
{
	static int old_width, old_height;
	static int width, height;

	m_engine()->GetScreenSize(width, height);

	if (width != old_width || height != old_height)
	{
		old_width = width;
		old_height = height;

		return true;
	}

	return false;
}



#include "../../utils/recv.h"
namespace fnfv
{
	constexpr uint32_t offsetBasis = 0x811c9dc5;
	constexpr uint32_t prime = 0x1000193;

	constexpr uint32_t hash(const char* str, const uint32_t value = offsetBasis) noexcept
	{
		return *str ? hash(str + 1, (value ^ *str) * static_cast<unsigned long long>(prime)) : value;
	}

	constexpr uint32_t hashRuntime(const char* str) noexcept
	{
		auto value = offsetBasis;

		while (*str)
		{
			value ^= *str++;
			value *= prime;
		}

		return value;
	}
}


typedef void(__fastcall* CParticleCollection_t)(void*);
void __fastcall hooks::hooked_particlecollection_simulate(CParticleCollection* thisPtr, void* edx) {
	
	(CParticleCollection_t(hooks::original_particlecollection))(thisPtr);
	{
		CParticleCollection* root_colection = thisPtr;
		while (root_colection->m_pParent)
			root_colection = root_colection->m_pParent;

		const char* root_name = root_colection->m_pDef.m_pObject->m_Name.buffer;

		switch (fnfv::hashRuntime(root_name))
		{
		case fnfv::hash("molotov_groundfire"):
		case fnfv::hash("molotov_groundfire_00MEDIUM"):
		case fnfv::hash("molotov_groundfire_00HIGH"):
		case fnfv::hash("molotov_groundfire_fallback"):
		case fnfv::hash("molotov_groundfire_fallback2"):
		case fnfv::hash("molotov_explosion"):
		case fnfv::hash("explosion_molotov_air"):
		case fnfv::hash("extinguish_fire"):
		case fnfv::hash("weapon_molotov_held"):
		case fnfv::hash("weapon_molotov_fp"):
		case fnfv::hash("weapon_molotov_thrown"):
		case fnfv::hash("incgrenade_thrown_trail"):
			switch (fnfv::hashRuntime(thisPtr->m_pDef.m_pObject->m_Name.buffer))
			{
			case fnfv::hash("explosion_molotov_air_smoke"):
			case fnfv::hash("molotov_smoking_ground_child01"):
			case fnfv::hash("molotov_smoking_ground_child02"):
			case fnfv::hash("molotov_smoking_ground_child02_cheapo"):
			case fnfv::hash("molotov_smoking_ground_child03"):
			case fnfv::hash("molotov_smoking_ground_child03_cheapo"):
			case fnfv::hash("molotov_smoke_screen"):
				{
					for (int i = 0; i < thisPtr->m_nActiveParticles; i++) {
						float* pColor = thisPtr->m_ParticleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
						*pColor = 0.f;
					}
				}
				break;
			default:
				for (int i = 0; i < thisPtr->m_nActiveParticles; i++) {
					float* pColor = thisPtr->m_ParticleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
					ImGui::ColorConvertHSVtoRGB(fmodf((float)ImGui::GetTime() / 2, 1), 1, 1, pColor[0], pColor[4], pColor[8]);
				}
				break;
			}
			break;

		}
	}

}

void __fastcall hooks::hooked_painttraverse(void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force)
{
	static auto original_fn = panel_hook->get_func_address <PaintTraverse_t>(41);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true); //-V807

	static auto set_console = true;

	if (set_console)
	{
		set_console = false;

		m_cvar()->FindVar(crypt_str("developer"))->SetValue(FALSE); //-V807
		m_cvar()->FindVar(crypt_str("con_filter_enable"))->SetValue(TRUE);
		m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
		m_engine()->ExecuteClientCmd(crypt_str("clear"));
		m_cvar()->ConsoleColorPrintf(Color(100, 200, 50), "Cheat injection success! \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "\n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "Hello dear, user! \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "\n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "\n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|------ Thank you ------| \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|                       | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|         insane        | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|           qk          | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|          TLGD         | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|        flowxrc        | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|                       | \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "|-----------------------| \n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "\n");
		m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), "\n");
	}

	static auto log_value = true;

	if (log_value != vars.misc.show_default_log)
	{
		log_value = vars.misc.show_default_log;

		if (log_value)
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
		else
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str("IrWL5106TZZKNFPz4P4Gl3pSN?J370f5hi373ZjPg%VOVh6lN"));
	}

	static vgui::VPANEL panel_id = 0;
	static auto in_game = false;

	if (!in_game && m_engine()->IsInGame()) //-V807
	{
		in_game = true;

		for (auto i = 1; i < 65; i++)
		{
			csgo.globals.fired_shots[i] = 0;
			player_records[i].clear();
			lagcompensation::get().is_dormant[i] = false;
			playeresp::get().esp_alpha_fade[i] = 0.0f;
			playeresp::get().health[i] = 100;
			c_dormant_esp::get().m_cSoundPlayers[i].reset();
		}

		antiaim::get().freeze_check = false;
		csgo.globals.next_lby_update = FLT_MIN;
		csgo.globals.last_lby_move = FLT_MIN;
		csgo.globals.last_aimbot_shot = 0;
		csgo.globals.bomb_timer_enable = true;
		csgo.globals.backup_model = false;
		csgo.globals.should_remove_smoke = false;
		csgo.globals.should_update_beam_index = true;
		csgo.globals.should_update_playerresource = true;
		csgo.globals.should_update_gamerules = true;
		csgo.globals.should_update_radar = true;
		csgo.globals.kills = 0;
		csgo.shots.clear();

		csgo.globals.commands.clear();
		SkinChanger::model_indexes.clear();
		SkinChanger::player_model_indexes.clear();
	}
	else if (in_game && !m_engine()->IsInGame())
	{
		in_game = false;

		csgo.globals.should_update_weather = true;
		csgo.globals.m_networkable = nullptr;

		vars.player_list.players.clear();

		misc::get().double_tap_enabled = false;
		misc::get().double_tap_key = false;

		misc::get().hide_shots_enabled = false;
		misc::get().hide_shots_key = false;
	}

	if (m_engine()->IsTakingScreenshot() && vars.misc.anti_screenshot)
		return;

	static uint32_t HudZoomPanel = 0;

	if (!HudZoomPanel)
		if (!strcmp(crypt_str("HudZoom"), m_panel()->GetName(panel)))
			HudZoomPanel = panel;

	if (HudZoomPanel == panel && vars.player.enable && vars.esp.removals[REMOVALS_SCOPE])
		return;

	original_fn(ecx, panel, force_repaint, allow_force);

	if (!panel_id)
	{
		auto panelName = m_panel()->GetName(panel);

		if (!strcmp(panelName, crypt_str("MatSystemTopPanel")))
			panel_id = panel;
	}

	if (panel_id == panel)
	{
		if (csgo.available())
		{
			static auto alive = false;

			if (!alive && csgo.local()->is_alive())
			{
				alive = true;
				csgo.globals.should_clear_death_notices = true;
			}
			else if (alive && !csgo.local()->is_alive())
			{
				alive = false;

				for (auto i = 1; i < m_globals()->m_maxclients; i++)
				{
					csgo.globals.fired_shots[i] = 0;
				}

				local_animations::get().local_data.prediction_animstate = nullptr;
				local_animations::get().local_data.animstate = nullptr;

				csgo.globals.weapon = nullptr;
				csgo.globals.should_choke_packet = false;
				csgo.globals.should_send_packet = false;
				csgo.globals.kills = 0;
				csgo.globals.should_buy = 3;
			}

			csgo.globals.bomb_carrier = -1;

			

			auto weapon = csgo.local()->m_hActiveWeapon().Get();

			auto is_scoped = csgo.globals.scoped && weapon->is_sniper() && weapon->m_zoomLevel();

			if (vars.player.enable && vars.esp.removals[REMOVALS_SCOPE] && is_scoped)
			{
				static int w, h;
				m_engine()->GetScreenSize(w, h);

				render::get().line(w / 2, 0, w / 2, h, Color::Black);
				render::get().line(0, h / 2, w, h / 2, Color::Black);
			}

			bullettracers::get().draw_beams();
		}

		static auto framerate = 0.0f;
		framerate = 0.9f * framerate + 0.1f * m_globals()->m_absoluteframetime;

		if (framerate <= 0.0f)
			framerate = 1.0f;

		csgo.globals.framerate = (int)(1.0f / framerate);
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto latency = m_engine()->IsPlayingDemo() ? 0.0f : nci->GetAvgLatency(FLOW_OUTGOING);

			if (latency) //-V550
			{
				static auto cl_updaterate = m_cvar()->FindVar(crypt_str("cl_updaterate"));
				latency -= 0.5f / cl_updaterate->GetFloat();
			}

			csgo.globals.ping = (int)(max(0.0f, latency) * 1000.0f);
		}

		time_t lt;
		struct tm* t_m;

		lt = time(nullptr);
		t_m = localtime(&lt);

		auto time_h = t_m->tm_hour;
		auto time_m = t_m->tm_min;
		auto time_s = t_m->tm_sec;

		std::string time;

		if (time_h < 10)
			time += "0";

		time += std::to_string(time_h) + ":";

		if (time_m < 10)
			time += "0";

		time += std::to_string(time_m) + ":";

		if (time_s < 10)
			time += "0";

		time += std::to_string(time_s);
		csgo.globals.time = std::move(time);

		static int w, h;
		m_engine()->GetScreenSize(w, h);

		static auto alpha = 0;
		auto speed = 800.0f * m_globals()->m_frametime;
	
		//eventlogs::get().paint_traverse();
		misc::get().NightmodeFix();
		if (csgo.globals.loaded_script)
			for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_paint")))
				current.func();
	}
}