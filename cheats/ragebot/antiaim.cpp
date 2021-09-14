// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "antiaim.h"
#include "knifebot.h"
#include "zeusbot.h"
#include "..\misc\fakelag.h"
#include "..\misc\prediction_system.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\local_animations.h"

void antiaim::create_move(CUserCmd* m_pcmd)
{
	auto velocity = csgo.local()->m_vecVelocity().Length(); //-V807

	type = ANTIAIM_STAND;

	if (vars.antiaim.antiaim_type)
		type = ANTIAIM_LEGIT;
	else if (velocity >= 5.0f && csgo.globals.slowwalking)
		type = ANTIAIM_SLOW_WALK;
	else if (velocity >= 5.0f && csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)
		type = ANTIAIM_MOVE;
	else if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		type = ANTIAIM_AIR;

	if (!vars.ragebot.enable && type != ANTIAIM_LEGIT)
		return;

	if (condition(m_pcmd))
		return;

	if ((type == ANTIAIM_LEGIT ? vars.antiaim.desync : vars.antiaim.type[type].desync) && (type == ANTIAIM_LEGIT ? !vars.antiaim.legit_lby_type : !vars.antiaim.lby_type) && !vars.misc.fast_stop && (!csgo.globals.weapon->is_grenade() || vars.esp.on_click && !(m_pcmd->m_buttons & IN_ATTACK) && !(m_pcmd->m_buttons & IN_ATTACK2)) && engineprediction::get().backup_data.velocity.Length2D() <= 20.0f) //-V648
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

	if (type != ANTIAIM_LEGIT)
		m_pcmd->m_viewangles.x = get_pitch(m_pcmd);

	m_pcmd->m_viewangles.y = get_yaw(m_pcmd);
}

float antiaim::get_pitch(CUserCmd* m_pcmd)
{
	static auto invert_jitter = false;
	static auto should_invert = false;

	if (csgo.send_packet)
		should_invert = true;
	else if (!csgo.send_packet && should_invert) //-V560
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}

	auto pitch = m_pcmd->m_viewangles.x;

	switch (vars.antiaim.type[type].pitch)
	{
	case 1:
		pitch = 89.0f;
		break;
	case 2:
		pitch = -89.0f;
		break;
	case 3:
		pitch = 0.0f;
		break;
	case 4:
		pitch = math::random_float(-89.0f, 89.0f);
		break;
	case 5:
		pitch = invert_jitter ? 89.0f : -89.0f;
		break;
	case 6:
		pitch = vars.misc.anti_untrusted ? 89.0f : 540.0f;
		break;
	case 7:
		pitch = vars.misc.anti_untrusted ? -89.0f : -540.0f;
		break;
	case 8:
		pitch = invert_jitter ? (vars.misc.anti_untrusted ? 89.0f : 540.0f) : (vars.misc.anti_untrusted ? -89.0f : -540.0f);
		break;
	}

	return pitch;
}

float antiaim::get_yaw(CUserCmd* m_pcmd)
{
	static auto invert_jitter = false;
	static auto should_invert = false;

	if (csgo.send_packet)
		should_invert = true;
	else if (!csgo.send_packet && should_invert) //-V560
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}

	auto max_desync_delta = csgo.local()->get_max_desync_delta(); //-V807

	auto yaw = 0.0f;
	auto lby_type = 0;

	if (type == ANTIAIM_LEGIT)
	{	
		yaw = m_pcmd->m_viewangles.y;

		if (!vars.antiaim.desync)
			return yaw;

		if (vars.antiaim.desync != 2 && (vars.antiaim.flip_desync.key <= KEY_NONE || vars.antiaim.flip_desync.key >= KEY_MAX))
			flip = automatic_direction();
		else if (vars.antiaim.desync == 1)
			flip = key_binds::get().get_key_bind_state(16);
		else if (vars.antiaim.desync == 2)
			flip = invert_jitter;

		desync_angle = max_desync_delta;

		if (vars.antiaim.legit_lby_type && csgo.local()->m_vecVelocity().Length() < 5.0f && csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)
			desync_angle *= 2.0f;

		if (flip)
		{
			desync_angle = -desync_angle;
			max_desync_delta = -max_desync_delta;
		}

		yaw -= desync_angle;
		lby_type = vars.antiaim.legit_lby_type;
	}
	else
	{
		if (manual_side == SIDE_NONE)
			freestanding(m_pcmd);
		else
			final_manual_side = manual_side;

		auto base_angle = m_pcmd->m_viewangles.y + 180.0f;

		if (final_manual_side == SIDE_LEFT)
			base_angle -= 90.0f;
		if (final_manual_side == SIDE_RIGHT)
			base_angle += 90.0f;

		if (vars.antiaim.type[type].base_angle && manual_side == SIDE_NONE)
			base_angle = at_targets();

		if (vars.antiaim.type[type].desync != 2 && (vars.antiaim.flip_desync.key <= KEY_NONE || vars.antiaim.flip_desync.key >= KEY_MAX))
		{
			if (final_manual_side == SIDE_LEFT)
				flip = true;
			else if (final_manual_side == SIDE_RIGHT)
				flip = false;
			else
				flip = automatic_direction();
		}
		else if (vars.antiaim.type[type].desync == 1)
			flip = key_binds::get().get_key_bind_state(16);

		auto yaw_angle = 0.0f;

		switch (vars.antiaim.type[type].yaw)
		{
		case 1:
			yaw_angle = invert_jitter ? (float)vars.antiaim.type[type].range * -0.5f : (float)vars.antiaim.type[type].range * 0.5f;
			break;
		case 2:
		{
			if (flip)
			{
				auto start_angle = (float)vars.antiaim.type[type].range * 0.5f;
				auto end_angle = (float)vars.antiaim.type[type].range * -0.5f;

				static auto angle = start_angle;

				auto angle_add_amount = (float)vars.antiaim.type[type].speed * 0.5f;

				if (angle - angle_add_amount >= end_angle)
					angle -= angle_add_amount;
				else
					angle = start_angle;

				yaw_angle = angle;
			}
			else
			{
				auto start_angle = (float)vars.antiaim.type[type].range * -0.5f;
				auto end_angle = (float)vars.antiaim.type[type].range * 0.5f;

				static auto angle = start_angle;

				auto angle_add_amount = (float)vars.antiaim.type[type].speed * 0.5f;

				if (angle + angle_add_amount <= end_angle)
					angle += angle_add_amount;
				else
					angle = start_angle;

				yaw_angle = angle;
			}
		}
		break;
		}

		desync_angle = 0.0f;

		if (vars.antiaim.type[type].desync)
		{
			if (vars.antiaim.type[type].desync == 2)
				flip = invert_jitter;

			auto desync_delta = max_desync_delta;

			if (type == ANTIAIM_STAND && vars.antiaim.lby_type)
				desync_delta *= 2.0f;
			else
			{
				if (!flip)
					desync_delta = min(desync_delta, (float)vars.antiaim.type[type].desync_range);
				else
					desync_delta = min(desync_delta, (float)vars.antiaim.type[type].inverted_desync_range);
			}

			if (!flip)
			{
				desync_delta = -desync_delta;
				max_desync_delta = -max_desync_delta;
			}

			base_angle -= desync_delta;

			if (type != ANTIAIM_STAND && type != ANTIAIM_LEGIT || !vars.antiaim.lby_type)
			{
				if (!flip)
					base_angle += desync_delta * (float)vars.antiaim.type[type].body_lean * 0.01f;
				else
					base_angle += desync_delta * (float)vars.antiaim.type[type].inverted_body_lean * 0.01f;
			}

			desync_angle = desync_delta;
		}

		yaw = base_angle + yaw_angle;

		if (!desync_angle) //-V550
			return yaw;

		lby_type = vars.antiaim.lby_type;
	}

	static auto sway_counter = 0;
	static auto force_choke = false;

	if (should_break_lby(m_pcmd, lby_type))
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

		if (lby_type != 2 || sway_counter > 3)
		{
			if (desync_angle > 0.0f)
				yaw -= 179.0f;
			else
				yaw += 179.0f;
		}

		if (sway_counter < 8)
			++sway_counter;
		else
			sway_counter = 0;

		breaking_lby = true;
		force_choke = true;
		csgo.send_packet = false;

		return yaw;
	}
	else if (force_choke)
	{
		force_choke = false;
		csgo.send_packet = false;

		return yaw;
	}
	else if (csgo.send_packet)
		yaw += desync_angle;

	return yaw;
}

bool antiaim::condition(CUserCmd* m_pcmd, bool dynamic_check)
{
	if (!m_pcmd)
		return true;

	if (!csgo.available())
		return true;

	if (!vars.antiaim.enable)
		return true;

	if (!csgo.local()->is_alive()) //-V807
		return true;

	if (csgo.local()->m_bGunGameImmunity() || csgo.local()->m_fFlags() & FL_FROZEN)
		return true;

	if (csgo.local()->get_move_type() == MOVETYPE_NOCLIP || csgo.local()->get_move_type() == MOVETYPE_LADDER)
		return true;

	if (csgo.globals.aimbot_working)
		return true;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return true;

	if (m_pcmd->m_buttons & IN_ATTACK && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_non_aim())
		return true;

	auto revolver_shoot = weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !csgo.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

	if (revolver_shoot)
		return true;

	if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && weapon->is_knife())
		return true;

	if (dynamic_check && freeze_check)
		return true;

	if (dynamic_check && m_pcmd->m_buttons & IN_USE && !vars.antiaim.antiaim_type)
		return true;

	if (dynamic_check && weapon->is_grenade() && weapon->m_fThrowTime())
		return true;

	return false;
}

bool antiaim::should_break_lby(CUserCmd* m_pcmd, int lby_type)
{	
	if (!lby_type)
		return false;

	if (csgo.globals.fakeducking && m_clientstate()->iChokedCommands > 12)
		return false;

	if (!csgo.globals.fakeducking && m_clientstate()->iChokedCommands > 14)
	{
		csgo.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	auto animstate = csgo.local()->get_animation_state(); //-V807

	if (!animstate)
		return false;

	if (animstate->m_velocity > 0.1f || fabs(animstate->flUpVelocity) > 100.0f)
		csgo.globals.next_lby_update = TICKS_TO_TIME(csgo.globals.fixed_tickbase + 14);
	else
	{
		if (TICKS_TO_TIME(csgo.globals.fixed_tickbase) > csgo.globals.next_lby_update)
		{
			csgo.globals.next_lby_update = 0.0f;
			return true;
		}
	}

	return false;
}

float antiaim::at_targets()
{
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t *>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		auto weapon = e->m_hActiveWeapon().Get();

		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		auto fov = math::get_fov(angles, math::calculate_angle(csgo.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}

	auto angle = 180.0f;

	if (manual_side == SIDE_LEFT)
		angle = 90.0f;
	else if (manual_side == SIDE_RIGHT)
		angle = -90.0f;

	if (!target)
		return csgo.get_command()->m_viewangles.y + angle;

	return math::calculate_angle(csgo.globals.eye_pos, target->GetAbsOrigin()).y + angle;
}

bool antiaim::automatic_direction()
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = csgo.local();
	src3D = csgo.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	csgo.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	csgo.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	csgo.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	csgo.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;

	if (Left - Right > 10.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Right - Left > 10.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (right_ticks > 10)
		return true;
	else if (left_ticks > 10)
		return false;

	return flip;
}

void antiaim::freestanding(CUserCmd* m_pcmd)
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = csgo.local();
	src3D = csgo.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	csgo.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	csgo.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	csgo.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	csgo.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;
	static auto back_ticks = 0;

	if (Right - Left > 20.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Left - Right > 20.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (fabs(Right - Left) <= 20.0f)
		back_ticks++;
	else
		back_ticks = 0;

	if (right_ticks > 10)
		final_manual_side = SIDE_RIGHT;
	else if (left_ticks > 10)
		final_manual_side = SIDE_LEFT;
	else if (back_ticks > 10)
		final_manual_side = SIDE_BACK;
}