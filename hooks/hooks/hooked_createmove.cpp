// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\ragebot\antiaim.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\misc\prediction_system.h"
#include "..\..\cheats\ragebot\aim.h"
#include "..\..\cheats\legitbot\legitbot.h"
#include "..\..\cheats\misc\bunnyhop.h"
#include "..\..\cheats\misc\airstrafe.h"
#include "..\..\cheats\misc\spammers.h"
#include "..\..\cheats\fakewalk\slowwalk.h"
#include "..\..\cheats\misc\misc.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\ragebot\knifebot.h"
#include "..\..\cheats\ragebot\zeusbot.h"
#include "..\..\cheats\lagcompensation\local_animations.h"
#include "..\..\cheats\lagcompensation\animation_system.h"

using CreateMove_t = bool(__thiscall*)(IClientMode*, float, CUserCmd*);

bool __stdcall hooks::hooked_createmove(float smt, CUserCmd* m_pcmd)
{
	static auto original_fn = clientmode_hook->get_func_address <CreateMove_t>(24);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	csgo.globals.in_createmove = false;

	if (!m_pcmd)
		return original_fn(m_clientmode(), smt, m_pcmd);

	if (!m_pcmd->m_command_number)
		return original_fn(m_clientmode(), smt, m_pcmd);

	auto original_result = original_fn(m_clientmode(), smt, m_pcmd);

	if (original_result)
	{
		m_prediction()->SetLocalViewAngles(m_pcmd->m_viewangles);
		m_engine()->SetViewAngles(m_pcmd->m_viewangles);
	}

	if (!csgo.available())
		return original_fn(m_clientmode(), smt, m_pcmd);

	misc::get().rank_reveal();
	spammers::get().clan_tag();

	if (!csgo.local()->is_alive()) //-V807
		return original_fn(m_clientmode(), smt, m_pcmd);

	uintptr_t* frame_ptr;
	__asm mov frame_ptr, ebp;

	csgo.globals.weapon = csgo.local()->m_hActiveWeapon().Get();

	if (!csgo.globals.weapon)
		return original_fn(m_clientmode(), smt, m_pcmd);

	csgo.globals.in_createmove = true;
	csgo.set_command(m_pcmd);

	if (menu_open && csgo.globals.focused_on_input)
	{
		m_pcmd->m_buttons = 0;
		m_pcmd->m_forwardmove = 0.0f;
		m_pcmd->m_sidemove = 0.0f;
		m_pcmd->m_upmove = 0.0f;
	}

	static auto should_recharge = false;

	if (should_recharge)
	{
		csgo.globals.ticks_allowed++;

		m_pcmd->m_tickcount = INT_MAX;
		m_pcmd->m_forwardmove = 0.0f;
		m_pcmd->m_sidemove = 0.0f;
		m_pcmd->m_upmove = 0.0f;
		m_pcmd->m_buttons &= ~IN_ATTACK;
		m_pcmd->m_buttons &= ~IN_ATTACK2;

		if (csgo.globals.ticks_allowed >= 16)
		{
			should_recharge = false;
			*(bool*)(*frame_ptr - 0x1C) = true; 
		}
		else
			*(bool*)(*frame_ptr - 0x1C) = false;

		return false;
	}

	if (csgo.globals.ticks_allowed < 16 && (misc::get().double_tap_enabled && misc::get().double_tap_key || misc::get().hide_shots_enabled && misc::get().hide_shots_key))
		should_recharge = true;

	csgo.globals.backup_tickbase = csgo.local()->m_nTickBase();

	if (csgo.globals.next_tickbase_shift)
		csgo.globals.fixed_tickbase = csgo.local()->m_nTickBase() - csgo.globals.next_tickbase_shift;
	else
		csgo.globals.fixed_tickbase = csgo.globals.backup_tickbase;

	if (menu_open)
	{
		m_pcmd->m_buttons &= ~IN_ATTACK;
		m_pcmd->m_buttons &= ~IN_ATTACK2;
	}

	if (m_pcmd->m_buttons & IN_ATTACK2 && vars.ragebot.enable && csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		m_pcmd->m_buttons &= ~IN_ATTACK2;

	if (vars.ragebot.enable && !csgo.globals.weapon->can_fire(true))
	{
		if (m_pcmd->m_buttons & IN_ATTACK && !csgo.globals.weapon->is_non_aim() && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			m_pcmd->m_buttons &= ~IN_ATTACK;
		else if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && (csgo.globals.weapon->is_knife() || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT)
		{
			if (m_pcmd->m_buttons & IN_ATTACK)
				m_pcmd->m_buttons &= ~IN_ATTACK;

			if (m_pcmd->m_buttons & IN_ATTACK2)
				m_pcmd->m_buttons &= ~IN_ATTACK2;
		}
	}

	if (m_pcmd->m_buttons & IN_FORWARD && m_pcmd->m_buttons & IN_BACK)
	{
		m_pcmd->m_buttons &= ~IN_FORWARD;
		m_pcmd->m_buttons &= ~IN_BACK;
	}

	if (m_pcmd->m_buttons & IN_MOVELEFT && m_pcmd->m_buttons & IN_MOVERIGHT)
	{
		m_pcmd->m_buttons &= ~IN_MOVELEFT;
		m_pcmd->m_buttons &= ~IN_MOVERIGHT;
	}

	csgo.send_packet = true;
	csgo.globals.tickbase_shift = 0;
	csgo.globals.double_tap_fire = false;
	csgo.globals.force_send_packet = false;
	csgo.globals.exploits = misc::get().double_tap_key || misc::get().hide_shots_key;
	csgo.globals.current_weapon = csgo.globals.weapon->get_weapon_group(vars.ragebot.enable);
	csgo.globals.slowwalking = false;
	csgo.globals.original_forwardmove = m_pcmd->m_forwardmove;
	csgo.globals.original_sidemove = m_pcmd->m_sidemove;

	antiaim::get().breaking_lby = false;

	auto wish_angle = m_pcmd->m_viewangles;

	misc::get().fast_stop(m_pcmd);

	if (vars.misc.bunnyhop)
		bunnyhop::get().create_move();

	if (vars.misc.slidewalk == 2 && math::random_int(0, 2) == 1)
	    misc::get().SlideWalk(m_pcmd);
	else 
		misc::get().SlideWalk(m_pcmd);

	misc::get().NoDuck(m_pcmd);

	misc::get().AutoCrouch(m_pcmd);

	GrenadePrediction::get().Tick(m_pcmd->m_buttons);

	if (vars.misc.crouch_in_air && !(csgo.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_DUCK;

	engineprediction::get().prediction_data.reset(); //-V807
	engineprediction::get().setup();

	engineprediction::get().predict(m_pcmd);
	local_animations::get().update_prediction_animations();

	csgo.globals.eye_pos = csgo.local()->get_shoot_position();

	if (vars.misc.airstrafe)
		airstrafe::get().create_move(m_pcmd);

	if (key_binds::get().get_key_bind_state(19) && engineprediction::get().backup_data.flags & FL_ONGROUND && !(csgo.local()->m_fFlags() & FL_ONGROUND)) //-V807
		m_pcmd->m_buttons |= IN_JUMP;

	if (key_binds::get().get_key_bind_state(21))
		slowwalk::get().create_move(m_pcmd);

	if (vars.ragebot.enable && !csgo.globals.weapon->is_non_aim() && engineprediction::get().backup_data.flags & FL_ONGROUND && csgo.local()->m_fFlags() & FL_ONGROUND)
		slowwalk::get().create_move(m_pcmd, 0.95f + 0.003125f * (16 - m_clientstate()->iChokedCommands));

	if (!should_recharge)
		fakelag::get().Createmove();

	csgo.globals.aimbot_working = false;
	csgo.globals.revolver_working = false;

	auto backup_velocity = csgo.local()->m_vecVelocity();
	auto backup_abs_velocity = csgo.local()->m_vecAbsVelocity();

	csgo.local()->m_vecVelocity() = engineprediction::get().backup_data.velocity;
	csgo.local()->m_vecAbsVelocity() = engineprediction::get().backup_data.velocity;

	csgo.globals.weapon->update_accuracy_penality();

	csgo.local()->m_vecVelocity() = backup_velocity;
	csgo.local()->m_vecAbsVelocity() = backup_abs_velocity;

	csgo.globals.inaccuracy = csgo.globals.weapon->get_inaccuracy();
	csgo.globals.spread = csgo.globals.weapon->get_spread();

	aim::get().run(m_pcmd);
	legit_bot::get().createmove(m_pcmd);

	zeusbot::get().run(m_pcmd);
	knifebot::get().run(m_pcmd);

	misc::get().automatic_peek(m_pcmd, wish_angle.y);

	antiaim::get().desync_angle = 0.0f;
	antiaim::get().create_move(m_pcmd);

	if (m_gamerules()->m_bIsValveDS() && m_clientstate()->iChokedCommands >= 6) //-V648
	{
		csgo.send_packet = true;
		fakelag::get().started_peeking = false;
	}
	else if (!m_gamerules()->m_bIsValveDS() && m_clientstate()->iChokedCommands >= 16) //-V648
	{
		csgo.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (csgo.globals.should_send_packet)
	{
		csgo.globals.force_send_packet = true;
		csgo.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (csgo.globals.should_choke_packet)
	{
		csgo.globals.should_choke_packet = false;
		csgo.globals.should_send_packet = true;
		csgo.send_packet = false;
	}

	if (!csgo.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (m_pcmd->m_buttons & IN_ATTACK && csgo.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			csgo.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !csgo.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (m_pcmd->m_buttons & IN_ATTACK && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
		{
			static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));

			if (vars.ragebot.enable)
				m_pcmd->m_viewangles -= csgo.local()->m_aimPunchAngle() * weapon_recoil_scale->GetFloat();

			if (!csgo.globals.fakeducking)
			{
				csgo.globals.force_send_packet = true;
				csgo.globals.should_choke_packet = true;
				csgo.send_packet = true;
				fakelag::get().started_peeking = false;
			}

			aim::get().last_shoot_position = csgo.globals.eye_pos;

			if (!double_tap_aim_check)
				csgo.globals.double_tap_aim = false;
		}
	}
	else if (!csgo.globals.fakeducking && csgo.globals.weapon->is_knife() && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2))
	{
		csgo.globals.force_send_packet = true;
		csgo.globals.should_choke_packet = true;
		csgo.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (csgo.globals.fakeducking)
		csgo.globals.force_send_packet = csgo.send_packet;

	for (auto& backup : aim::get().backup)
		backup.adjust_player();

	auto backup_ticks_allowed = csgo.globals.ticks_allowed;

	if (misc::get().double_tap(m_pcmd))
		misc::get().hide_shots(m_pcmd, false);
	else
	{
		csgo.globals.ticks_allowed = backup_ticks_allowed;
		misc::get().hide_shots(m_pcmd, true);
	}

	if (!csgo.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (m_pcmd->m_buttons & IN_ATTACK && csgo.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			csgo.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !csgo.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (!double_tap_aim_check && m_pcmd->m_buttons & IN_ATTACK && csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
			csgo.globals.double_tap_aim = false;
	}

	if (m_globals()->m_tickcount - csgo.globals.last_aimbot_shot > 16) //-V807
	{
		csgo.globals.double_tap_aim = false;
		csgo.globals.double_tap_aim_check = false;
	}

	engineprediction::get().finish();

	if (csgo.globals.loaded_script)
		for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_createmove")))
			current.func();

	if (vars.misc.anti_untrusted)
		math::normalize_angles(m_pcmd->m_viewangles);
	else
	{
		m_pcmd->m_viewangles.y = math::normalize_yaw(m_pcmd->m_viewangles.y);
		m_pcmd->m_viewangles.z = 0.0f;
	}

	util::movement_fix(wish_angle, m_pcmd);
	
	if (should_recharge)
		csgo.send_packet = true;

	static auto previous_ticks_allowed = csgo.globals.ticks_allowed;

	if (csgo.send_packet && m_clientstate()->pNetChannel)
	{
		auto choked_packets = m_clientstate()->pNetChannel->m_nChokedPackets;

		if (choked_packets >= 0)
		{
			auto ticks_allowed = csgo.globals.ticks_allowed;
			auto command_number = m_pcmd->m_command_number - choked_packets;

			do
			{
				auto command = &m_input()->m_pCommands[m_pcmd->m_command_number - MULTIPLAYER_BACKUP * (command_number / MULTIPLAYER_BACKUP) - choked_packets];

				if (!command || command->m_tickcount > m_globals()->m_tickcount + 72)
				{
					if (--ticks_allowed < 0)
						ticks_allowed = 0;

					csgo.globals.ticks_allowed = ticks_allowed;
				}

				++command_number;
				--choked_packets;
			} 
			while (choked_packets >= 0);
		}
	}

	if (csgo.globals.ticks_allowed > 17)
		csgo.globals.ticks_allowed = math::clamp(csgo.globals.ticks_allowed - 1, 0, 17);

	if (previous_ticks_allowed && !csgo.globals.ticks_allowed)
		csgo.globals.ticks_choke = 16;

	previous_ticks_allowed = csgo.globals.ticks_allowed;

	if (csgo.globals.ticks_choke)
	{
		csgo.send_packet = csgo.globals.force_send_packet;
		--csgo.globals.ticks_choke;
	}

	auto& correct = csgo.globals.data.emplace_front();

	correct.command_number = m_pcmd->m_command_number;
	correct.choked_commands = m_clientstate()->iChokedCommands + 1;
	correct.tickcount = m_globals()->m_tickcount;

	if (csgo.send_packet)
		csgo.globals.choked_number.clear();
	else
		csgo.globals.choked_number.emplace_back(correct.command_number);

	while (csgo.globals.data.size() > (int)(2.0f / m_globals()->m_intervalpertick))
		csgo.globals.data.pop_back();

	auto& out = csgo.globals.commands.emplace_back();

	out.is_outgoing = csgo.send_packet;
	out.is_used = false;
	out.command_number = m_pcmd->m_command_number;
	out.previous_command_number = 0;

	while (csgo.globals.commands.size() > (int)(1.0f / m_globals()->m_intervalpertick))
		csgo.globals.commands.pop_front();

	if (!csgo.send_packet && !m_gamerules()->m_bIsValveDS())
	{
		auto net_channel = m_clientstate()->pNetChannel;

		if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
		{
			auto backup_choke = net_channel->m_nChokedPackets;
			net_channel->m_nChokedPackets = 0;

			net_channel->send_datagram();
			--net_channel->m_nOutSequenceNr;

			net_channel->m_nChokedPackets = backup_choke;
		}
	}

	if (csgo.send_packet && !csgo.globals.should_send_packet && (!csgo.globals.should_choke_packet || (!misc::get().hide_shots_enabled && !csgo.globals.double_tap_fire)))
	{
		local_animations::get().local_data.fake_angles = m_pcmd->m_viewangles; //-V807
		local_animations::get().local_data.real_angles = local_animations::get().local_data.stored_real_angles;
	}

	if (!antiaim::get().breaking_lby)
		local_animations::get().local_data.stored_real_angles = m_pcmd->m_viewangles;

	if (csgo.send_packet && csgo.globals.should_send_packet)
		csgo.globals.should_send_packet = false;

	if (vars.misc.buybot_enable && csgo.globals.should_buy)
	{
		--csgo.globals.should_buy;

		if (!csgo.globals.should_buy)
		{
			std::string buy;

			switch (vars.misc.buybot1)
			{
			case 1:
				buy += crypt_str("buy g3sg1; ");
				break;
			case 2:
				buy += crypt_str("buy awp; ");
				break;
			case 3:
				buy += crypt_str("buy ssg08; ");
				break;
			}

			switch (vars.misc.buybot2)
			{
			case 1:
				buy += crypt_str("buy elite; ");
				break;
			case 2:
				buy += crypt_str("buy deagle; buy revolver; ");
				break;
			}

			if (vars.misc.buybot3[BUY_ARMOR])
				buy += crypt_str("buy vesthelm; buy vest; ");

			if (vars.misc.buybot3[BUY_TASER])
				buy += crypt_str("buy taser; ");

			if (vars.misc.buybot3[BUY_GRENADES])
				buy += crypt_str("buy molotov; buy hegrenade; buy smokegrenade; ");

			if (vars.misc.buybot3[BUY_DEFUSER])
				buy += crypt_str("buy defuser; ");

			m_engine()->ExecuteClientCmd(buy.c_str());
		}
	}

	csgo.globals.in_createmove = false;

	*(bool*)(*frame_ptr - 0x1C) = csgo.send_packet;
	return false;
}