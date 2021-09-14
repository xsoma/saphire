// This is an independent project of an individual developer.Dear PVS - Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "../../hooks/Render.h"
#include "player_esp.h"
#include "..\misc\misc.h"
#include "..\ragebot\aim.h"
#include "..\menu.h"
#include "dormant_esp.h"
#include "Hitmarker.h"


class RadarPlayer_t
{
public:
	Vector pos; //0x0000
	Vector angle; //0x000C
	Vector spotted_map_angle_related; //0x0018
	DWORD tab_related; //0x0024
	char pad_0x0028[0xC]; //0x0028
	float spotted_time; //0x0034
	float spotted_fraction; //0x0038
	float time; //0x003C
	char pad_0x0040[0x4]; //0x0040
	__int32 player_index; //0x0044
	__int32 entity_index; //0x0048
	char pad_0x004C[0x4]; //0x004C
	__int32 health; //0x0050
	char name[32]; //0x785888
	char pad_0x0074[0x75]; //0x0074
	unsigned char spotted; //0x00E9
	char pad_0x00EA[0x8A]; //0x00EA
};

class CCSGO_HudRadar
{
public:
	char pad_0x0000[0x14C];
	RadarPlayer_t radar_info[65];
};

void playeresp::paint_traverse()
{
	static auto alpha = 1.0f;
	c_dormant_esp::get().start();

	if (vars.player.arrows && csgo.local()->is_alive())
	{
		static auto switch_alpha = false;

		if (alpha <= 0.0f || alpha >= 1.0f)
			switch_alpha = !switch_alpha;

		alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
		alpha = math::clamp(alpha, 0.0f, 1.0f);
	}

	static auto FindHudElement = (DWORD(__thiscall*)(void*, const char*))util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	static auto hud_ptr = *(DWORD**)(util::FindSignature(crypt_str("client.dll"), crypt_str("81 25 ? ? ? ? ? ? ? ? 8B 01")) + 0x2);

	auto radar_base = FindHudElement(hud_ptr, "CCSGO_HudRadar");
	auto hud_radar = (CCSGO_HudRadar*)(radar_base - 0x14);

	for (auto i = 1; i < m_globals()->m_maxclients; i++) //-V807
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(false, false))
			continue;

		type = ENEMY;

		if (e == csgo.local())
			type = LOCAL;
		else if (e->m_iTeamNum() == csgo.local()->m_iTeamNum())
			type = TEAM;

		if (type == LOCAL && !m_input()->m_fCameraInThirdPerson)
			continue;

		auto valid_dormant = false;
		auto backup_flags = e->m_fFlags();
		auto backup_origin = e->GetAbsOrigin();

		if (e->IsDormant())
			valid_dormant = c_dormant_esp::get().adjust_sound(e);
		else
		{
			health[i] = e->m_iHealth();
			health2[i] = e->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, e->GetAbsOrigin(), e->m_fFlags());
		}

		if (radar_base && hud_radar && e->IsDormant() && e->m_iTeamNum() != csgo.local()->m_iTeamNum() && e->m_bSpotted())
			health[i] = hud_radar->radar_info[i].health;

		if (!health[i])
		{
			if (e->IsDormant())
			{
				e->m_fFlags() = backup_flags;
				e->set_abs_origin(backup_origin);
			}

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime; //-V807
		auto slow = 0.25f * m_globals()->m_frametime;

		if (e->IsDormant())
		{
			auto origin = e->GetAbsOrigin();

			if (origin.IsZero())
				esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && esp_alpha_fade[i] > 0.0f)
				esp_alpha_fade[i] -= slow;
			else if (valid_dormant && esp_alpha_fade[i] < 1.0f)
				esp_alpha_fade[i] += fast;
		}
		else if (esp_alpha_fade[i] < 1.0f)
			esp_alpha_fade[i] += fast;

		esp_alpha_fade[i] = math::clamp(esp_alpha_fade[i], 0.0f, 1.0f);

		if (vars.player.type[type].skeleton && !e->IsDormant())
		{
			auto color = vars.player.type[type].skeleton_color;
			color.SetAlpha(min(255.0f * esp_alpha_fade[i], color.a()));


			draw_skeleton(e, color, e->m_CachedBoneData().Base());
		}


		Box box;

		if (util::get_bbox(e, box, true))
		{
			if (e->GetRenderOrigin() == Vector(0, 0, 0))
				continue;

			auto& hpbox = hp_info[i];

			if (hpbox.hp == -1)
				hpbox.hp = math::clamp(health2[i], 0, 100);
			else
			{
				auto hp = math::clamp(health2[i], 0, 100);

				if (hp != hpbox.hp)
				{
					if (hpbox.hp > hp)
					{
						if (hpbox.hp_difference_time) //-V550
							hpbox.hp_difference += hpbox.hp - hp;
						else
							hpbox.hp_difference = hpbox.hp - hp;

						hpbox.hp_difference_time = m_globals()->m_curtime;
					}
					else
					{
						hpbox.hp_difference = 0;
						hpbox.hp_difference_time = 0.0f;
					}

					hpbox.hp = hp;
				}

				if (m_globals()->m_curtime - hpbox.hp_difference_time > 0.1f && hpbox.hp_difference)
				{
					auto difference_factor = 2.0f * m_globals()->m_frametime * hpbox.hp_difference;

					hpbox.hp_difference -= difference_factor;
					hpbox.hp_difference = math::clamp(hpbox.hp_difference, 0, 100);

					if (!hpbox.hp_difference)
						hpbox.hp_difference_time = 0.0f;
				}
			}
			draw_box(e, box);
			draw_name(e, box);
			draw_health(e, box, hpbox);
			draw_weapon(e, box, draw_ammobar(e, box));
			draw_ammobar(e, box);
			if (!e->IsDormant())
				draw_flags(e, box);
		}

		if (type == ENEMY || type == TEAM)
		{

			if (type == ENEMY)
			{
				if (vars.player.arrows && csgo.local()->is_alive())
					misc::get().PovArrows(e, Color(vars.player.arrows_color));
			}
		}

		if (e->IsDormant())
		{
			e->m_fFlags() = backup_flags;
			e->set_abs_origin(backup_origin);
		}
	}
}

void playeresp::draw_skeleton(player_t* e, Color color, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	auto model = e->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto upper_direction = get_bone_position(7) - get_bone_position(6);
	auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (auto i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->pBone(i);

		if (!bone)
			continue;

		if (bone->parent == -1)
			continue;

		if (!(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		auto child = get_bone_position(i);
		auto parent = get_bone_position(bone->parent);

		auto delta_child = child - breast_bone;
		auto delta_parent = parent - breast_bone;

		if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			parent = breast_bone;

		if (i == 5)
			child = breast_bone;

		if (fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f || i == 6)
			continue;

		auto schild = ZERO;
		auto sparent = ZERO;

		if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
			g_Render->DrawLine(schild.x, schild.y, sparent.x, sparent.y, color);
	}
}

void DrawLine(float x1, float y1, float x2, float y2, Color color, float size = 1.f) {


	if (size == 1.f)
	{

		g_Render->DrawLine(x1, y1, x2, y2, Color(color.r(), color.g(), color.b(), color.a()));
	}
	else
	{
		g_Render->FilledRect(x1 - (size / 2.f), y1 - (size / 2.f), x2 + (size / 2.f), y2 + (size / 2.f), Color(color.r(), color.g(), color.b(), color.a()));
	}

}

void playeresp::draw_box(player_t* m_entity, const Box& box)
{
	if (!vars.player.type[type].box)
		return;

	auto alpha = 255.0f * esp_alpha_fade[m_entity->EntIndex()];

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : vars.player.type[type].box_bg_color;
	color.SetAlpha(min(alpha, color.a()));

	g_Render->Rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color::Black);
	g_Render->Rect(box.x, box.y, box.w, box.h, color);
	g_Render->Rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color::Black);
}

void playeresp::draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox)
{
	if (!vars.player.type[type].health)
		return;

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	const int red = min((510 * (100 - m_entity->m_iHealth())) / 100, 255);
	const int green = min((510 * m_entity->m_iHealth()) / 100, 255);
	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto back_color = Color(0, 0, 0, (int)(alpha * 0.6f));
	auto color = m_entity->IsDormant() ? Color(130, 130, 130) : Color(red, green, 0);
	auto color2 = m_entity->IsDormant() ? Color(130, 130, 130) : Color(red, green, 0);
	auto hp_effect_color = Color(215, 20, 20, alpha);

	if (vars.player.type[type].custom_health_color) {
		color = m_entity->IsDormant() ? Color(130, 130, 130) : vars.player.type[type].health_color;
		color2 = m_entity->IsDormant() ? Color(130, 130, 130) : vars.player.type[type].health_color2;
	}

	color.SetAlpha(alpha);
	g_Render->Rect(box.x - 6, box.y, 4, box.h, back_color);
	Box n_box =
	{
		box.x - 5,
		box.y,
		2,
		box.h
	};
	auto bar_width = (int)((float)hpbox.hp * (float)n_box.w / 100.0f);
	auto bar_height = (int)((float)hpbox.hp * (float)n_box.h / 100.0f);
	auto offset = n_box.h - bar_height;
	g_Render->FilledRect(n_box.x, n_box.y - 1, 2, n_box.h + 2, back_color);
	
	auto height = n_box.h - hpbox.hp * n_box.h / 100;
	g_Render->DrawLineGradient(n_box.x, n_box.y + offset, 2, bar_height, color, color2, 1, true);
	g_Render->DrawLineGradient(n_box.x, n_box.y + height - hpbox.hp_difference * n_box.h / 100, 2, hpbox.hp_difference * n_box.h / 100, color, color2, 1, true);
	if (m_entity->m_iHealth() < 100)
		g_Render->DrawString(n_box.x, n_box.y + height - hpbox.hp_difference * n_box.h / 100, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, c_menu::get().esp_name, std::to_string(hpbox.hp).c_str());
}

bool playeresp::draw_ammobar(player_t* m_entity, const Box& box)
{

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon->is_non_aim())
		return false;

	if (weapon->is_non_aim())
		return false;

	



	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	auto outline_alpha = (int)(alpha * 0.7f);
	auto inner_back_alpha = (int)(alpha * 0.6f);

	Color outline_color =
	{
		0,
		0,
		0,
		outline_alpha
	};

	Color inner_back_color =
	{
		0,
		0,
		0,
		inner_back_alpha
	};

	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : vars.player.type[type].ammobar_color;

	color.SetAlpha(min(alpha, color.a()));

	Box n_box =
	{
		box.x,
		box.y + box.h + 3,
		box.w + 2,
		2
	};

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	auto ammo = weapon->m_iClip1();
	auto bar_width = ammo * box.w / weapon_info->iMaxClip1;
	auto reloading = false;

	auto animlayer = m_entity->get_animlayers()[1];

	if (animlayer.m_nSequence)
	{
		auto activity = m_entity->sequence_activity(animlayer.m_nSequence);

		reloading = activity == ACT_CSGO_RELOAD && animlayer.m_flWeight;

		if (reloading && animlayer.m_flCycle < 1.0f)
			bar_width = animlayer.m_flCycle * box.w;
	}

	g_Render->FilledRect(n_box.x - 1, n_box.y - 1, n_box.w, 4, inner_back_color);
	g_Render->FilledRect(n_box.x, n_box.y, bar_width, 2, color);

	if (weapon->m_iClip1() != weapon_info->iMaxClip1 && !reloading)
		g_Render->DrawString(n_box.x + bar_width, n_box.y + 1, text_color, HFONT_CENTERED_X | HFONT_CENTERED_Y | render2::outline, c_menu::get().font, std::to_string(ammo).c_str());

	return true;
}

void playeresp::draw_name(player_t* m_entity, const Box& box)
{
	if (!vars.player.type[type].name)
		return;

	static auto sanitize = [](char* name) -> std::string
	{
		name[127] = '\0';

		std::string tmp(name) ;
		if (tmp.length() > 20)
		{
			tmp.erase(20, tmp.length() - 20);
			tmp.append("...");
		}

		return tmp;
	};

	player_info_t player_info;

	if (m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info))
	{
		auto name = sanitize(player_info.szName);

		auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : vars.player.type[type].name_color;
		color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

		g_Render->DrawString(box.x + box.w / 2, box.y - 15, color, render2::centered_x | render2::outline, c_menu::get().esp_name, (name + " ").c_str());
	}
}

void playeresp::draw_weapon(player_t* m_entity, const Box& box, bool space)
{
	if (!vars.player.type[type].weapon[WEAPON_ICON] && !vars.player.type[type].weapon[WEAPON_TEXT])
		return;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto pos = box.y + box.h + 2;

	if (space)
		pos += 5;

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : vars.player.type[type].weapon_color;
	color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

	if (vars.player.type[type].weapon[WEAPON_TEXT])
	{
		g_Render->DrawString(box.x + box.w / 2, pos, color, render2::centered_x | render2::outline, c_menu::get().esp_name, weapon->get_name().c_str());
		pos += 11;
	}

	if (vars.player.type[type].weapon[WEAPON_ICON])
	{
		g_Render->DrawString(box.x + box.w / 2, pos, color, render2::centered_x | render2::outline, c_menu::get().esp_weapon, weapon->get_icon());
	}
}

void playeresp::draw_flags(player_t* e, const Box& box)
{
	auto weapon = e->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto _x = box.x + box.w + 3, _y = box.y - 3;

	if (vars.player.type[type].flags[FLAGS_MONEY])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(170, 190, 80);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "%i$", e->m_iAccount());
		_y += 10;
	}

	if (vars.player.type[type].flags[FLAGS_ARMOR])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		auto kevlar = e->m_ArmorValue() > 0;
		auto helmet = e->m_bHasHelmet();

		std::string text;

		if (helmet && kevlar)
			text = "HK";
		else if (kevlar)
			text = "K";

		if (kevlar)
		{
			g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, text.c_str());
			_y += 10;
		}
	}

	if (vars.player.type[type].flags[FLAGS_KIT] && e->m_bHasDefuser())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "KIT");
		_y += 10;
	}

	if (vars.player.type[type].flags[FLAGS_SCOPED])
	{
		auto scoped = e->m_bIsScoped();

		if (e == csgo.local())
			scoped = csgo.globals.scoped;

		if (scoped)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(30, 120, 235);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "SCOPED");
			_y += 10;
		}
	}

	if (vars.player.type[type].flags[FLAGS_FAKEDUCKING])
	{
		auto animstate = e->get_animation_state();

		if (animstate)
		{
			auto fakeducking = [&]() -> bool
			{
				static auto stored_tick = 0;
				static int crouched_ticks[65];

				if (animstate->m_fDuckAmount) //-V550
				{
					if (animstate->m_fDuckAmount < 0.9f && animstate->m_fDuckAmount > 0.5f) //-V550
					{
						if (stored_tick != m_globals()->m_tickcount)
						{
							crouched_ticks[e->EntIndex()]++;
							stored_tick = m_globals()->m_tickcount;
						}

						return crouched_ticks[e->EntIndex()] > 16;
					}
					else
						crouched_ticks[e->EntIndex()] = 0;
				}

				return false;
			};

			if (fakeducking() && e->m_fFlags() & FL_ONGROUND && !animstate->m_bInHitGroundAnimation)
			{
				auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(215, 20, 20);
				color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

				g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "FD");
				_y += 10;
			}
		}
	}

	if (vars.player.type[type].flags[FLAGS_VULNERABLE] && e->m_hActiveWeapon()->is_non_aim())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(120, 240, 0);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "VULNERABLE");
		_y += 10;
	}

	if (vars.player.type[type].flags[FLAGS_FLASHED] && e->m_flFlashDuration() > 0)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(209, 240, 0);
		color.SetAlpha(float(255 * e->m_flFlashDuration()) * esp_alpha_fade[e->EntIndex()]);
		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "FLASHED");
		_y += 10;
	}

	if (vars.player.type[type].flags[FLAGS_EXPLOIT] && (e->m_flOldSimulationTime() > e->m_flSimulationTime()))
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(209, 23, 0);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, std::string("EXPLOIT").c_str());
		_y += 10;
	}

	if (vars.player.type[type].flags[FLAGS_PING])
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(e->EntIndex(), &player_info);

		if (player_info.fakeplayer)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "BOT");
			_y += 10;
		}
		else
		{
			auto latency = math::clamp(m_playerresource()->GetPing(e->EntIndex()), 0, 999);
			std::string delay = std::to_string(latency) + "MS";

			auto green_factor = (int)math::clamp(255.0f - (float)latency * 225.0f / 200.0f, 0.0f, 255.0f);

			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(150, green_factor, 0);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, delay.c_str());
			_y += 10;
		}
	}

	if (vars.player.type[type].flags[FLAGS_C4] && e->EntIndex() == csgo.globals.bomb_carrier)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(163, 49, 93);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		g_Render->DrawString(_x, _y, color, render2::outline, c_menu::get().smallest_pixel, "C4");
		_y += 10;
	}
}

void playeresp::draw_lines(player_t* e)
{

}

void playeresp::draw_multi_points(player_t* e)
{

}