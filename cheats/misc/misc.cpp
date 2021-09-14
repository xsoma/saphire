// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "misc.h"
#include "fakelag.h"
#include "..\ragebot\aim.h"
#include "..\visuals\world_esp.h"
#include "prediction_system.h"
#include "logs.h"
#include "../menu.h"
#include "../../hooks/Render.h"

#define VERSION crypt_str("limehook | ")

void misc::watermark()
{
	if (!vars.menu.watermark)
		return;

	auto width = 0, height = 0;
	m_engine()->GetScreenSize(width, height); //-V807

	auto watermark = VERSION + csgo.username + crypt_str(" | ") + csgo.globals.time;

	if (m_engine()->IsInGame())
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto server = nci->GetAddress();

			if (!strcmp(server, crypt_str("loopback")))
				server = crypt_str("Local server");
			else if (m_gamerules()->m_bIsValveDS())
				server = crypt_str("Valve server");

			auto tickrate = std::to_string((int)(1.0f / m_globals()->m_intervalpertick));
			watermark = VERSION + csgo.username + crypt_str(" | ") + server + crypt_str(" | ") + std::to_string(csgo.globals.ping) + crypt_str(" ms | ") + tickrate + crypt_str(" tick | ") + csgo.globals.time;
		}
	}

	//auto box_width = render::get().text_width(fonts[NAME], watermark.c_str()) + 10;

	//.gradient(width - 10 - box_width, 10, box_width, 1, Color(vars.misc.watermark_color.r(), vars.misc.watermark_color.g(), vars.misc.watermark_color.b(), 255));

	//render::get().rect_filled(width - 10 - box_width, 11, box_width, 18, Color(10, 10, 10, 150));

	//render::get().text(fonts[NAME], width - 10 - box_width + 5, 20, Color(255, 255, 255, 220), HFONT_CENTERED_Y, watermark.c_str());
}

void misc::NoDuck(CUserCmd* cmd)
{
	if (!vars.misc.noduck)
		return;

	if (m_gamerules()->m_bIsValveDS())
		return;

	cmd->m_buttons |= IN_BULLRUSH;
}

void misc::ChatSpamer()
{
	if (!vars.misc.chat)
		return;

	static std::string chatspam[] = 
	{ 
		crypt_str("limehook - stay with us or lose the game."),
		crypt_str("Get good. Get limehook."),
		crypt_str("limehook - just the best."),
		crypt_str("Go to another level with limehook."),
	};

	static auto lastspammed = 0;

	if (GetTickCount() - lastspammed > 800)
	{
		lastspammed = GetTickCount();

		srand(m_globals()->m_tickcount);
		std::string msg = crypt_str("say ") + chatspam[rand() % 4];

		m_engine()->ExecuteClientCmd(msg.c_str());
	}
}

void misc::AutoCrouch(CUserCmd* cmd)
{
	if (fakelag::get().condition)
	{
		csgo.globals.fakeducking = false;
		return;
	}

	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
	{
		csgo.globals.fakeducking = false;
		return;
	}

	if (m_gamerules()->m_bIsValveDS())
	{
		csgo.globals.fakeducking = false;
		return;
	}

	if (!key_binds::get().get_key_bind_state(20))
	{
		csgo.globals.fakeducking = false;
		return;
	}

	if (!csgo.globals.fakeducking && m_clientstate()->iChokedCommands != 7)
		return;

	if (m_clientstate()->iChokedCommands >= 7)
		cmd->m_buttons |= IN_DUCK;
	else
		cmd->m_buttons &= ~IN_DUCK;

	csgo.globals.fakeducking = true;
}

void misc::SlideWalk(CUserCmd* cmd)
{
	if (!csgo.local()->is_alive()) //-V807
		return;

	if (csgo.local()->get_move_type() == MOVETYPE_LADDER)
		return;

	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	if (antiaim::get().condition(cmd, true) && (vars.misc.slidewalk == 2 && math::random_int(0, 2) == 0|| vars.misc.slidewalk == 1))
	{
		if (cmd->m_forwardmove > 0.0f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}
		else if (cmd->m_forwardmove < 0.0f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}

		if (cmd->m_sidemove > 0.0f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}
		else if (cmd->m_sidemove < 0.0f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		auto buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

		if (vars.misc.slidewalk == 2 && math::random_int(0, 1) || vars.misc.slidewalk == 1)
		{
			if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
				return;

			if (cmd->m_forwardmove <= 0.0f)
				buttons |= IN_BACK;
			else
				buttons |= IN_FORWARD;

			if (cmd->m_sidemove > 0.0f)
				goto LABEL_15;
			else if (cmd->m_sidemove >= 0.0f)
				goto LABEL_18;

			goto LABEL_17;
		}
		else
			goto LABEL_18;

		if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
			return;

		if (cmd->m_forwardmove <= 0.0f) //-V779
			buttons |= IN_FORWARD;
		else
			buttons |= IN_BACK;

		if (cmd->m_sidemove > 0.0f)
		{
		LABEL_17:
			buttons |= IN_MOVELEFT;
			goto LABEL_18;
		}

		if (cmd->m_sidemove < 0.0f)
			LABEL_15:

		buttons |= IN_MOVERIGHT;

	LABEL_18:
		cmd->m_buttons = buttons;
	}
}

void misc::automatic_peek(CUserCmd* cmd, float wish_yaw)
{
	if (!csgo.globals.weapon->is_non_aim() && key_binds::get().get_key_bind_state(18))
	{
		if (csgo.globals.start_position.IsZero())
		{
			csgo.globals.start_position = csgo.local()->GetAbsOrigin();

			if (!(engineprediction::get().backup_data.flags & FL_ONGROUND))
			{
				Ray_t ray;
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				ray.Init(csgo.globals.start_position, csgo.globals.start_position - Vector(0.0f, 0.0f, 1000.0f));
				m_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);
				
				if (trace.fraction < 1.0f)
					csgo.globals.start_position = trace.endpos + Vector(0.0f, 0.0f, 2.0f);
			}
		}
		else
		{
			auto revolver_shoot = csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !csgo.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

			if (cmd->m_buttons & IN_ATTACK && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
				csgo.globals.fired_shot = true;

			if (csgo.globals.fired_shot)
			{
				auto current_position = csgo.local()->GetAbsOrigin();
				auto difference = current_position - csgo.globals.start_position;

				if (difference.Length2D() > 5.0f)
				{
					auto velocity = Vector(difference.x * cos(wish_yaw / 180.0f * M_PI) + difference.y * sin(wish_yaw / 180.0f * M_PI), difference.y * cos(wish_yaw / 180.0f * M_PI) - difference.x * sin(wish_yaw / 180.0f * M_PI), difference.z);

					cmd->m_forwardmove = -velocity.x * 20.0f;
					cmd->m_sidemove = velocity.y * 20.0f;
				}
				else
				{
					csgo.globals.fired_shot = false;
					csgo.globals.start_position.Zero();
				}
			}
		}
	}
	else
	{
		csgo.globals.fired_shot = false;
		csgo.globals.start_position.Zero();
	}
}

void misc::ViewModel()
{
	if (vars.esp.viewmodel_fov)
	{
		auto viewFOV = (float)vars.esp.viewmodel_fov + 68.0f;
		static auto viewFOVcvar = m_cvar()->FindVar(crypt_str("viewmodel_fov"));

		if (viewFOVcvar->GetFloat() != viewFOV) //-V550
		{
			*(float*)((DWORD)&viewFOVcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewFOVcvar->SetValue(viewFOV);
		}
	}
	
	if (vars.esp.viewmodel_x)
	{
		auto viewX = (float)vars.esp.viewmodel_x / 2.0f;
		static auto viewXcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_x")); //-V807

		if (viewXcvar->GetFloat() != viewX) //-V550
		{
			*(float*)((DWORD)&viewXcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewXcvar->SetValue(viewX);
		}
	}

	if (vars.esp.viewmodel_y)
	{
		auto viewY = (float)vars.esp.viewmodel_y / 2.0f;
		static auto viewYcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_y"));

		if (viewYcvar->GetFloat() != viewY) //-V550
		{
			*(float*)((DWORD)&viewYcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewYcvar->SetValue(viewY);
		}
	}

	if (vars.esp.viewmodel_z)
	{
		auto viewZ = (float)vars.esp.viewmodel_z / 2.0f;
		static auto viewZcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_z"));

		if (viewZcvar->GetFloat() != viewZ) //-V550
		{
			*(float*)((DWORD)&viewZcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewZcvar->SetValue(viewZ);
		}
	}
}

void misc::FullBright()
{		
	if (!vars.player.enable)
		return;

	static auto mat_fullbright = m_cvar()->FindVar(crypt_str("mat_fullbright"));

	if (mat_fullbright->GetBool() != vars.esp.bright)
		mat_fullbright->SetValue(vars.esp.bright);
}
#include "../../hooks/Render.h"
void misc::PovArrows(player_t* e, Color color)
{
	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int iScreenWidth, iScreenHeight;
		m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

		auto xOk = iScreenWidth > screen.x; 
		auto yOk = iScreenHeight > screen.y;

		return xOk && yOk;
	};

	Vector screenPos;

	if (isOnScreen(e->GetAbsOrigin(), screenPos))
		return;

	Vector viewAngles;
	m_engine()->GetViewAngles(viewAngles);

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto screenCenter = Vector2D(width * 0.5f, height * 0.5f);
	auto angleYawRad = DEG2RAD(viewAngles.y - math::calculate_angle(csgo.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);

	auto radius = vars.player.distance;
	auto size = vars.player.size;

	auto newPointX = screenCenter.x + ((((width - (size * 3)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
	auto newPointY = screenCenter.y + ((((height - (size * 3)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));

	std::array <Vector2D, 3> points
	{
		Vector2D(newPointX - size, newPointY - size),
		Vector2D(newPointX + size, newPointY),
		Vector2D(newPointX - size, newPointY + size)
	};

	math::rotate_triangle(points, viewAngles.y - math::calculate_angle(csgo.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);
	g_Render->TriangleFilled(points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, Color(color.r(), color.g(), color.b(), math::clamp(color.a(), 0, 125)));
	g_Render->Triangle(points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, Color(color.r(), color.g(), color.b(), color.a()));

	auto weapon = e->m_hActiveWeapon();
	if (!weapon)
		return;
	if (!weapon->get_csweapon_info())
		return;
	g_Render->DrawString(newPointX, newPointY, weapon->m_iItemDefinitionIndex() == WEAPON_TASER || weapon->is_grenade() ? Color(255, 50, 50) :  Color(255,255,255), 2 | 4, c_menu::get().esp_weapon, weapon->get_icon());
}

void misc::zeus_range()
{
	if (!vars.player.enable)
		return;

	if (!vars.esp.taser_range)
		return;

	if (!m_input()->m_fCameraInThirdPerson)
		return;

	if (!csgo.local()->is_alive())  //-V807
		return;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER)
		return;

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	//render::get().Draw3DRainbowCircle(csgo.local()->get_shoot_position(), weapon_info->flRange);
}

void misc::NightmodeFix()
{
	static auto in_game = false;

	if (m_engine()->IsInGame() && !in_game)
	{
		in_game = true;

		csgo.globals.change_materials = true;
		worldesp::get().changed = true;

		static auto skybox = m_cvar()->FindVar(crypt_str("sv_skyname"));
		worldesp::get().backup_skybox = skybox->GetString();
		return;
	}
	else if (!m_engine()->IsInGame() && in_game)
		in_game = false;

	static auto player_enable = vars.player.enable;

	if (player_enable != vars.player.enable)
	{
		player_enable = vars.player.enable;
		csgo.globals.change_materials = true;
		return;
	}

	static auto setting = vars.esp.nightmode;

	if (setting != vars.esp.nightmode)
	{
		setting = vars.esp.nightmode;
		csgo.globals.change_materials = true;
		return;
	}

	static auto setting_world = vars.esp.world_color;

	if (setting_world != vars.esp.world_color)
	{
		setting_world = vars.esp.world_color;
		csgo.globals.change_materials = true;
		return;
	}

	static auto setting_props = vars.esp.props_color;

	if (setting_props != vars.esp.props_color)
	{
		setting_props = vars.esp.props_color;
		csgo.globals.change_materials = true;
	}
}

void misc::desync_arrows()
{
	if (!csgo.local()->is_alive())
		return;

	if (!vars.ragebot.enable)
		return;

	if (!vars.antiaim.enable)
		return;

	if ((vars.antiaim.manual_back.key <= KEY_NONE || vars.antiaim.manual_back.key >= KEY_MAX) && (vars.antiaim.manual_left.key <= KEY_NONE || vars.antiaim.manual_left.key >= KEY_MAX) && (vars.antiaim.manual_right.key <= KEY_NONE || vars.antiaim.manual_right.key >= KEY_MAX))
		antiaim::get().manual_side = SIDE_NONE;

	if (!vars.antiaim.flip_indicator)
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	static auto alpha = 1.0f;
	static auto switch_alpha = false;

	if (alpha <= 0.0f || alpha >= 1.0f)
		switch_alpha = !switch_alpha;

	alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
	alpha = math::clamp(alpha, 0.0f, 1.0f);

	auto color = vars.antiaim.flip_indicator_color;
	color.SetAlpha((int)(min(255.0f * alpha, color.a())));

	//if (antiaim::get().manual_side == SIDE_BACK)
		//render::get().triangle(Vector2D(width / 2, height / 2 + 80), Vector2D(width / 2 - 10, height / 2 + 60), Vector2D(width / 2 + 10, height / 2 + 60), color);
//	else if (antiaim::get().manual_side == SIDE_LEFT)
		//render::get().triangle(Vector2D(width / 2 - 55, height / 2 + 10), Vector2D(width / 2 - 75, height / 2), Vector2D(width / 2 - 55, height / 2 - 10), color);
	//else if (antiaim::get().manual_side == SIDE_RIGHT)
	//	render::get().triangle(Vector2D(width / 2 + 55, height / 2 - 10), Vector2D(width / 2 + 75, height / 2), Vector2D(width / 2 + 55, height / 2 + 10), color);
}

void misc::aimbot_hitboxes()
{
	if (!vars.player.enable)
		return;

	if (!vars.player.lag_hitbox)
		return;

	auto player = (player_t*)m_entitylist()->GetClientEntity(aim::get().last_target_index);

	if (!player)
		return;

	auto model = player->GetModel(); //-V807

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;
	
	auto hitbox_set = studio_model->pHitboxSet(player->m_nHitboxSet());

	if (!hitbox_set)
		return;

	for (auto i = 0; i < hitbox_set->numhitboxes; i++)
	{
		auto hitbox = hitbox_set->pHitbox(i);

		if (!hitbox)
			continue;

		if (hitbox->radius == -1.0f) //-V550
			continue;

		auto min = ZERO;
		auto max = ZERO;

		math::vector_transform(hitbox->bbmin, aim::get().last_target[aim::get().last_target_index].record.matrixes_data.main[hitbox->bone], min);
		math::vector_transform(hitbox->bbmax, aim::get().last_target[aim::get().last_target_index].record.matrixes_data.main[hitbox->bone], max);

		m_debugoverlay()->AddCapsuleOverlay(min, max, hitbox->radius, vars.player.lag_hitbox_color.r(), vars.player.lag_hitbox_color.g(), vars.player.lag_hitbox_color.b(), vars.player.lag_hitbox_color.a(), 4.0f, 0, 1);
	}
}

void misc::ragdolls()
{
	if (!vars.misc.ragdolls)
		return;

	for (auto i = 1; i <= m_entitylist()->GetHighestEntityIndex(); ++i)
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != CCSRagdoll)
			continue;

		auto ragdoll = (ragdoll_t*)e;
		ragdoll->m_vecForce().z = 800000.0f;
	}
}

void misc::rank_reveal()
{
	if (!vars.misc.rank_reveal)
		return;

	using RankReveal_t = bool(__cdecl*)(int*);
	static auto Fn = (RankReveal_t)(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 A1 ? ? ? ? 85 C0 75 37")));

	int array[3] = 
	{
		0,
		0,
		0
	};

	Fn(array);
}

void misc::fast_stop(CUserCmd* m_pcmd)
{
	if (!vars.misc.fast_stop)
		return;

	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto pressed_move_key = m_pcmd->m_buttons & IN_FORWARD || m_pcmd->m_buttons & IN_MOVELEFT || m_pcmd->m_buttons & IN_BACK || m_pcmd->m_buttons & IN_MOVERIGHT || m_pcmd->m_buttons & IN_JUMP;

	if (pressed_move_key)
		return;

	if (!((antiaim::get().type == ANTIAIM_LEGIT ? vars.antiaim.desync : vars.antiaim.type[antiaim::get().type].desync) && (antiaim::get().type == ANTIAIM_LEGIT ? !vars.antiaim.legit_lby_type : !vars.antiaim.lby_type) && (!csgo.globals.weapon->is_grenade() || vars.esp.on_click & !(m_pcmd->m_buttons & IN_ATTACK) && !(m_pcmd->m_buttons & IN_ATTACK2))) || antiaim::get().condition(m_pcmd)) //-V648
	{
		auto velocity = csgo.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;
		}
	}
	else
	{
		auto velocity = csgo.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;
		}
		else
		{
			auto speed = 1.01f;

			if (m_pcmd->m_buttons & IN_DUCK || csgo.globals.fakeducking)
				speed *= 2.94117647f;

			static auto switch_move = false;

			if (switch_move)
				m_pcmd->m_sidemove += speed;
			else
				m_pcmd->m_sidemove -= speed;

			switch_move = !switch_move;
		}
	}
}

void misc::spectators_list()
{
	
}

bool misc::double_tap(CUserCmd* m_pcmd)
{
	double_tap_enabled = true;

	static auto recharge_double_tap = false;
	static auto last_double_tap = 0;

	if (recharge_double_tap)
	{
		recharge_double_tap = false;
		recharging_double_tap = true;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (recharging_double_tap)
	{
		auto recharge_time = csgo.globals.weapon->can_double_tap() ? TIME_TO_TICKS(0.50f) : TIME_TO_TICKS(0.9f);

		if (!aim::get().should_stop && fabs(csgo.globals.fixed_tickbase - last_double_tap) > recharge_time)
		{
			last_double_tap = 0;

			recharging_double_tap = false;
			double_tap_key = true;
			hide_shots_key = false;
		}
		else if (m_pcmd->m_buttons & IN_ATTACK)
			last_double_tap = csgo.globals.fixed_tickbase;
	}
	
	if (!vars.ragebot.enable)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (!vars.ragebot.double_tap)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (vars.ragebot.double_tap_key.key <= KEY_NONE || vars.ragebot.double_tap_key.key >= KEY_MAX)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (double_tap_key && vars.ragebot.double_tap_key.key != vars.antiaim.hide_shots_key.key)
		hide_shots_key = false;

	if (!double_tap_key)
	{
		double_tap_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (csgo.local()->m_bGunGameImmunity() || csgo.local()->m_fFlags() & FL_FROZEN) //-V807
	{
		double_tap_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (m_gamerules()->m_bIsValveDS())
	{
		double_tap_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (csgo.globals.fakeducking)
	{
		double_tap_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.new_dt.charge_ticks = 0;
		csgo.globals.next_tickbase_shift = 0;
		return false;
	}

	if (antiaim::get().freeze_check)
		return true;

	auto max_tickbase_shift = csgo.globals.weapon->get_max_tickbase_shift();
	if (!csgo.globals.weapon->is_grenade() && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_C4 && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !csgo.globals.weapon->is_knife() && csgo.send_packet && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2 && csgo.globals.weapon->is_knife())) //-V648
	{
		auto next_command_number = m_pcmd->m_command_number + 1;
		auto user_cmd = m_input()->GetUserCmd(next_command_number);

		memcpy(user_cmd, m_pcmd, sizeof(CUserCmd)); //-V598
		user_cmd->m_command_number = next_command_number;

		util::copy_command(user_cmd, max_tickbase_shift);

		if (csgo.globals.aimbot_working)
		{
			csgo.globals.double_tap_aim = true;
			csgo.globals.double_tap_aim_check = true;
		}

		recharge_double_tap = true;
		double_tap_enabled = false;
		double_tap_key = false;
		last_double_tap = csgo.globals.fixed_tickbase;
		csgo.globals.tickbase_shift = max_tickbase_shift;
	}
	else if (!csgo.globals.weapon->is_knife() && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_C4 && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT && !csgo.globals.weapon->is_grenade() && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		csgo.globals.tickbase_shift = max_tickbase_shift;

	return true;
}

void misc::hide_shots(CUserCmd* m_pcmd, bool should_work)
{
	hide_shots_enabled = true;

	if (!vars.ragebot.enable)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			csgo.globals.ticks_allowed = 0;
			csgo.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (!vars.antiaim.hide_shots)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			csgo.globals.ticks_allowed = 0;
			csgo.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (vars.antiaim.hide_shots_key.key <= KEY_NONE || vars.antiaim.hide_shots_key.key >= KEY_MAX)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			csgo.globals.ticks_allowed = 0;
			csgo.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (!should_work && double_tap_key)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		return;
	}

	if (!hide_shots_key)
	{
		hide_shots_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.next_tickbase_shift = 0;
		return;
	}

	double_tap_key = false;

	if (csgo.local()->m_bGunGameImmunity() || csgo.local()->m_fFlags() & FL_FROZEN)
	{
		hide_shots_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.next_tickbase_shift = 0;
		return;
	}

	if (csgo.globals.fakeducking)
	{
		hide_shots_enabled = false;
		csgo.globals.ticks_allowed = 0;
		csgo.globals.next_tickbase_shift = 0;
		return;
	}

	if (antiaim::get().freeze_check)
		return;

	csgo.globals.next_tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 9;

	auto revolver_shoot = csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !csgo.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);
	auto weapon_shoot = m_pcmd->m_buttons & IN_ATTACK && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || m_pcmd->m_buttons & IN_ATTACK2 && csgo.globals.weapon->is_knife() || revolver_shoot;

	if (csgo.send_packet && !csgo.globals.weapon->is_grenade() && weapon_shoot)
		csgo.globals.tickbase_shift = csgo.globals.next_tickbase_shift;
}