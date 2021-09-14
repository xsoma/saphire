// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\lagcompensation\animation_system.h"
#include "..\..\cheats\visuals\nightmode.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\nSkinz\SkinChanger.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\visuals\world_esp.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

using FrameStageNotify_t = void(__stdcall*)(ClientFrameStage_t);

Vector flb_aim_punch;
Vector flb_view_punch;

Vector* aim_punch;
Vector* view_punch;

void weather()
{
	static ClientClass* client_class = nullptr;

	if (!client_class)
		client_class = m_client()->GetAllClasses();

	while (client_class)
	{
		if (client_class->m_ClassID == CPrecipitation)
			break;

		client_class = client_class->m_pNext;
	}

	if (!client_class)
		return;

	auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
	auto serial = math::random_int(0, 4095);

	csgo.globals.m_networkable = client_class->m_pCreateFn(entry, serial);

	if (!csgo.globals.m_networkable)
		return;

	auto m_precipitation = csgo.globals.m_networkable->GetIClientUnknown()->GetBaseEntity();

	if (!m_precipitation)
		return;

	csgo.globals.m_networkable->PreDataUpdate(0);
	csgo.globals.m_networkable->OnPreDataChanged(0);

	static auto m_nPrecipType = netvars::get().get_offset(crypt_str("CPrecipitation"), crypt_str("m_nPrecipType"));
	static auto m_vecMins = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMins"));
	static auto m_vecMaxs = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMaxs"));

	*(int*)(uintptr_t(m_precipitation) + m_nPrecipType) = 0;
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMaxs) = Vector(32768.0f, 32768.0f, 32768.0f);
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMins) = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->GetCollideable()->OBBMaxs() = Vector(32768.0f, 32768.0f, 32768.0f);
	m_precipitation->GetCollideable()->OBBMins() = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->set_abs_origin((m_precipitation->GetCollideable()->OBBMins() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f);
	m_precipitation->m_vecOrigin() = (m_precipitation->GetCollideable()->OBBMaxs() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f;

	m_precipitation->OnDataChanged(0);
	m_precipitation->PostDataUpdate(0);
}

void remove_smoke()
{
	if (vars.player.enable && vars.esp.removals[REMOVALS_SMOKE])
	{
		static auto smoke_count = *reinterpret_cast<uint32_t**>(util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 57 8B CB")) + 0x1);
		*(int*)smoke_count = 0;
	}

	if (csgo.globals.should_remove_smoke == vars.player.enable && vars.esp.removals[REMOVALS_SMOKE])
		return;

	csgo.globals.should_remove_smoke = vars.player.enable && vars.esp.removals[REMOVALS_SMOKE];

	static std::vector <const char*> smoke_materials =
	{
		"effects/overlaysmoke",
		"particle/beam_smoke_01",
		"particle/particle_smokegrenade",
		"particle/particle_smokegrenade1",
		"particle/particle_smokegrenade2",
		"particle/particle_smokegrenade3",
		"particle/particle_smokegrenade_sc",
		"particle/smoke1/smoke1",
		"particle/smoke1/smoke1_ash",
		"particle/smoke1/smoke1_nearcull",
		"particle/smoke1/smoke1_nearcull2",
		"particle/smoke1/smoke1_snow",
		"particle/smokesprites_0001",
		"particle/smokestack",
		"particle/vistasmokev1/vistasmokev1",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_nearcull",
		"particle/vistasmokev1/vistasmokev1_nearcull_fog",
		"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev4_emods_nocull",
		"particle/vistasmokev1/vistasmokev4_nearcull",
		"particle/vistasmokev1/vistasmokev4_nocull"
	};

	for (auto material_name : smoke_materials)
	{
		auto material = m_materialsystem()->FindMaterial(material_name, nullptr);

		if (!material)
			continue;

		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, csgo.globals.should_remove_smoke);
	}
	
}
#include "../../cheats/visuals/logger.hpp"
void __stdcall hooks::hooked_fsn(ClientFrameStage_t stage)
{
	static auto original_fn = client_hook->get_func_address <FrameStageNotify_t>(37);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!csgo.available())
	{
		nightmode::get().clear_stored_materials();
		return original_fn(stage);
	}

	if (stage == FRAME_START)
		key_binds::get().update_key_binds();

	aim_punch = nullptr;
	view_punch = nullptr;

	flb_aim_punch.Zero();
	flb_view_punch.Zero();

	if (csgo.globals.updating_skins && m_clientstate()->iDeltaTick > 0) //-V807
		csgo.globals.updating_skins = false;

	SkinChanger::run(stage);
	local_animations::get().run(stage);

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && csgo.local()->is_alive()) //-V522 //-V807
	{
		auto viewmodel = csgo.local()->m_hViewModel().Get();

		if (viewmodel && engineprediction::get().viewmodel_data.weapon == viewmodel->m_hWeapon().Get() && engineprediction::get().viewmodel_data.sequence == viewmodel->m_nSequence() && engineprediction::get().viewmodel_data.animation_parity == viewmodel->m_nAnimationParity()) //-V807
		{
			viewmodel->m_flCycle() = engineprediction::get().viewmodel_data.cycle;
			viewmodel->m_flAnimTime() = engineprediction::get().viewmodel_data.animation_time;
		}
	}

	if (stage == FRAME_RENDER_START)
	{
		if (vars.esp.client_bullet_impacts)
		{
			static auto last_count = 0;
			auto& client_impact_list = *(CUtlVector <client_hit_verify_t>*)((uintptr_t)csgo.local() + 0xBC00);

			for (auto i = client_impact_list.Count(); i > last_count; --i)
				m_debugoverlay()->BoxOverlay(client_impact_list[i - 1].position, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f), QAngle(0.0f, 0.0f, 0.0f), vars.esp.client_bullet_impacts_color.r(), vars.esp.client_bullet_impacts_color.g(), vars.esp.client_bullet_impacts_color.b(), vars.esp.client_bullet_impacts_color.a(), 4.0f);

			if (client_impact_list.Count() != last_count)
				last_count = client_impact_list.Count();
		}

		remove_smoke();
		misc::get().ragdolls();

		if (vars.esp.removals[REMOVALS_FLASH] && csgo.local()->m_flFlashDuration() && vars.player.enable) //-V807
			csgo.local()->m_flFlashDuration() = 0.0f;

		if (*(bool*)m_postprocessing() != (vars.player.enable && vars.esp.removals[REMOVALS_POSTPROCESSING] && (!vars.esp.world_modulation || !vars.esp.exposure)))
			*(bool*)m_postprocessing() = vars.player.enable && vars.esp.removals[REMOVALS_POSTPROCESSING] && (!vars.esp.world_modulation || !vars.esp.exposure);

		if (vars.esp.removals[REMOVALS_RECOIL] && vars.player.enable)
		{
			aim_punch = &csgo.local()->m_aimPunchAngle();
			view_punch = &csgo.local()->m_viewPunchAngle();

			flb_aim_punch = *aim_punch;
			flb_view_punch = *view_punch;

			(*aim_punch).Zero();
			(*view_punch).Zero(); //-V656
		}

		auto get_original_scope = false;

		if (csgo.local()->is_alive())
		{
			csgo.globals.in_thirdperson = key_binds::get().get_key_bind_state(17);

			if (vars.player.enable && vars.esp.removals[REMOVALS_SCOPE])
			{
				auto weapon = csgo.local()->m_hActiveWeapon().Get();

				if (weapon)
				{
					get_original_scope = true;

					csgo.globals.scoped = csgo.local()->m_bIsScoped() && weapon->m_zoomLevel();
					csgo.local()->m_bIsScoped() = weapon->m_zoomLevel();
				}
			}
		}

		if (!get_original_scope)
			csgo.globals.scoped = csgo.local()->m_bIsScoped();
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		static auto rain = false;

		if (rain != vars.esp.rain || csgo.globals.should_update_weather)
		{
			rain = vars.esp.rain;

			if (csgo.globals.m_networkable)
			{
				csgo.globals.m_networkable->Release();
				csgo.globals.m_networkable = nullptr;
			}

			if (rain)
				weather();

			csgo.globals.should_update_weather = false;
		}

		vars.player_list.refreshing = true;
		vars.player_list.players.clear();

		for (auto i = 1; i < m_globals()->m_maxclients; i++)
		{
			auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

			if (!e)
			{
				vars.player_list.white_list[i] = false;
				vars.player_list.high_priority[i] = false;
				vars.player_list.force_body_aim[i] = false;

				continue;
			}

			if (e->m_iTeamNum() == csgo.local()->m_iTeamNum())
			{
				vars.player_list.white_list[i] = false;
				vars.player_list.high_priority[i] = false;
				vars.player_list.force_body_aim[i] = false;

				continue;
			}

			player_info_t player_info;
			m_engine()->GetPlayerInfo(i, &player_info);

			vars.player_list.players.emplace_back(Player_list_data(i, player_info.szName));
		}

		vars.player_list.refreshing = false;
	}

	if (stage == FRAME_RENDER_END)
	{
		static auto r_drawspecificstaticprop = m_cvar()->FindVar(crypt_str("r_drawspecificstaticprop")); //-V807

		if (r_drawspecificstaticprop->GetBool())
			r_drawspecificstaticprop->SetValue(FALSE);

		if (csgo.globals.change_materials)
		{
			if (vars.esp.nightmode && vars.player.enable)
				nightmode::get().apply();
			else
				nightmode::get().remove();

			csgo.globals.change_materials = false;
		}

		worldesp::get().skybox_changer();
		worldesp::get().fog_changer();

		misc::get().FullBright();
		misc::get().ViewModel();

		static auto cl_foot_contact_shadows = m_cvar()->FindVar(crypt_str("cl_foot_contact_shadows")); //-V807

		if (cl_foot_contact_shadows->GetBool())
			cl_foot_contact_shadows->SetValue(FALSE);

		static auto zoom_sensitivity_ratio_mouse = m_cvar()->FindVar(crypt_str("zoom_sensitivity_ratio_mouse"));

		if (vars.player.enable && vars.esp.removals[REMOVALS_ZOOM] && vars.esp.fix_zoom_sensivity && zoom_sensitivity_ratio_mouse->GetFloat() == 1.0f) //-V550
			zoom_sensitivity_ratio_mouse->SetValue(0.0f);
		else if ((!vars.player.enable || !vars.esp.removals[REMOVALS_ZOOM] || !vars.esp.fix_zoom_sensivity) && !zoom_sensitivity_ratio_mouse->GetFloat())
			zoom_sensitivity_ratio_mouse->SetValue(1.0f);

		static auto r_modelAmbientMin = m_cvar()->FindVar(crypt_str("r_modelAmbientMin"));

		if (vars.esp.world_modulation && vars.esp.ambient && r_modelAmbientMin->GetFloat() != vars.esp.ambient * 0.05f) //-V550
			r_modelAmbientMin->SetValue(vars.esp.ambient * 0.05f);
		else if ((!vars.esp.world_modulation || !vars.esp.ambient) && r_modelAmbientMin->GetFloat())
			r_modelAmbientMin->SetValue(0.0f);
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		auto current_shot = csgo.shots.end();

		auto net_channel = m_engine()->GetNetChannelInfo();
		auto latency = net_channel ? net_channel->GetLatency(FLOW_OUTGOING) + net_channel->GetLatency(FLOW_INCOMING) + 1.0f : 0.0f; //-V807

		for (auto& shot = csgo.shots.begin(); shot != csgo.shots.end(); ++shot)
		{
			if (shot->end)
			{
				current_shot = shot;
				break;
			}
			else if (shot->impacts && m_globals()->m_tickcount - 1 > shot->event_fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				break;
			}
			else if (csgo.globals.backup_tickbase - TIME_TO_TICKS(latency) > shot->fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				current_shot->latency = true;
				break;
			}
		}

		if (current_shot != csgo.shots.end())
		{
			if (!current_shot->latency)
			{
				current_shot->shot_info.should_log = true; //-V807

				if (!current_shot->hurt_player)
				{
					auto reason = "";
					if (current_shot->impact_hit_player)
					{
						{
							reason = "resolver";
							current_shot->shot_info.result = crypt_str("R_MISS");
							if (vars.misc.events_to_log[EVENTLOG_OTH] && current_shot->shot_info.hitchance > 80) {
								notify::add_log("Miss", crypt_str("Missed shot due to bad resolve"), Color(255, 10, 0));
							}
							else if (vars.misc.events_to_log[EVENTLOG_OTH] && current_shot->shot_info.hitchance < 80) {
								notify::add_log("Miss", crypt_str("Missed shot due to desync animation"), Color(255, 10, 0));
							}
							else
								notify::add_log("Miss", "Missed shot due to resolver innacuracy", Color(255, 10, 0));
							csgo.globals.missed_shots[current_shot->last_target]++;
						}
					}
					else if (vars.misc.events_to_log[EVENTLOG_SPREAD] && !current_shot->occlusion && current_shot->shot_info.hitchance < 100)
					{
						reason = "spread";
						current_shot->shot_info.result = crypt_str("SP_MISS");
						std::string m = "Missed shot due to ";
						notify::add_log("Miss", crypt_str(m + "spread"), Color(255, 10, 0));

					}
					else if (vars.misc.events_to_log[EVENTLOG_OTH] && !current_shot->occlusion && current_shot->shot_info.hitchance > 100)
					{
						reason = "prediction error";
						current_shot->shot_info.result = crypt_str("PR_MISS");
						std::string m = "Missed shot due to ";
						if (current_shot->shot_info.client_damage > 80 && current_shot->client_hitbox_int <= HITBOX_UPPER_CHEST)
							notify::add_log("Miss", crypt_str(m + "prediction error(velocity error)"), Color(255, 10, 0));
						else if (current_shot->shot_info.client_damage > 80 && current_shot->client_hitbox_int > HITBOX_UPPER_CHEST)
							notify::add_log("Miss", crypt_str(m + "prediction error(leg movement error)"), Color(255, 10, 0));
						else if (current_shot->shot_info.client_damage < 80 && current_shot->client_hitbox_int <= HITBOX_UPPER_CHEST)
							notify::add_log("Miss", crypt_str(m + "prediction error(not refined hitbox resolve error)"), Color(255, 10, 0));
						else if (current_shot->shot_info.client_damage < 80 && current_shot->client_hitbox_int > HITBOX_UPPER_CHEST)
							notify::add_log("Miss", crypt_str(m + "prediction error(not refined leg resolve error)"), Color(255, 10, 0));
						else if (current_shot->shot_info.client_damage > 1 && misc::get().recharging_double_tap)
							notify::add_log("Miss", crypt_str(m + "prediction error(exploit rechange)"), Color(255, 10, 0));
						else
							notify::add_log("Miss", "Shot unregisted by server", Color(255, 10, 0));

					}
					else if (vars.misc.events_to_log[EVENTLOG_OCC] && current_shot->occlusion)
					{
						reason = "occlusion";
						current_shot->shot_info.result = crypt_str("OC_MISS");
						std::string m = "Missed shot due to ";
						notify::add_log("Miss", crypt_str(m + "occlusion " + "| damage loss:" + (aim::get().last_target[current_shot->last_target].data.visible ? std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_visible_damage - current_shot->shot_info.client_damage).c_str() : std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_damage - current_shot->shot_info.client_damage).c_str())), Color(255, 10, 0));

					}
					else if (vars.misc.events_to_log[EVENTLOG_OCC] && current_shot->occlusion && current_shot->shot_info.hitchance >= 100)
					{
						reason = "occlusion";
						current_shot->shot_info.result = crypt_str("OC_PR_MISS");
						std::string m = "Missed shot due to ";
						notify::add_log("Miss", crypt_str(m + "prediction error(incorrectly calculated damage)"), Color(255, 10, 0));

					}

					std::stringstream log;
					log << "Fired shot to " << current_shot->shot_info.target_name << "'s "
						<< current_shot->shot_info.client_hitbox << " for " << current_shot->shot_info.client_damage << " missed shot reason "
						<< reason << " backtracked for " << current_shot->shot_info.backtrack_ticks << "hitchance " << current_shot->shot_info.hitchance
						<< " [not successful]";
					notify::add_log("Debug", crypt_str(log.str().c_str()), Color(35, 255, 25));

				}
				else
				{
					std::stringstream log;
					log << "Fired shot to " << current_shot->shot_info.target_name << "'s "
						<< current_shot->shot_info.client_hitbox << " for " << current_shot->shot_info.client_damage << " registed shot in "
						<< current_shot->shot_info.server_hitbox << " for " << current_shot->shot_info.server_damage << " backtracked for " << current_shot->shot_info.backtrack_ticks
						<< "hitchance " << current_shot->shot_info.hitchance << " [successful]";
					notify::add_log("Debug", crypt_str(log.str().c_str()), Color(35, 255, 25));
				}
			}

			if (csgo.globals.loaded_script && current_shot->shot_info.should_log)
			{
				current_shot->shot_info.should_log = false;

				for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_shot")))
					current.func(current_shot->shot_info);
			}

			csgo.shots.erase(current_shot);
		}
	}

	lagcompensation::get().fsn(stage);
	
	original_fn(stage);

	if (vars.player.enable && vars.esp.removals[REMOVALS_RECOIL] && csgo.local()->is_alive() && aim_punch && view_punch)
	{
		*aim_punch = flb_aim_punch;
		*view_punch = flb_view_punch;
	}

	static DWORD* death_notice = nullptr;

	if (csgo.local()->is_alive())
	{
		if (!death_notice)
			death_notice = util::FindHudElement <DWORD>(crypt_str("CCSGO_HudDeathNotice"));

		if (death_notice)
		{
			auto local_death_notice = (float*)((uintptr_t)death_notice + 0x50);

			if (local_death_notice)
				*local_death_notice = vars.esp.preserve_killfeed ? FLT_MAX : 1.5f;

			if (csgo.globals.should_clear_death_notices)
			{
				csgo.globals.should_clear_death_notices = false;

				using Fn = void(__thiscall*)(uintptr_t);
				static auto clear_notices = (Fn)util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 0C 53 56 8B 71 58"));

				clear_notices((uintptr_t)death_notice - 0x14);
			}
		}
	}
	else
		death_notice = 0;
}