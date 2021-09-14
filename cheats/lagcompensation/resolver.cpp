
#include "animation_system.h"
#include "..\ragebot\aim.h"
void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
	player = e;
	player_record = record;

	original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
	original_pitch = math::normalize_pitch(pitch);
}

Vector player_t::get_eye_pos() {
	return m_vecOrigin() + m_vecViewOffset();
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	side = false;

	was_first_bruteforce = false;
	was_second_bruteforce = false;

	original_goal_feet_yaw = 0.0f;
	original_pitch = 0.0f;
}

bool resolver::low_delta()
{
	auto record = player_record;
	if (!csgo.local()->is_alive())
		return false;
	float angle_diff = math::angle_diff(player->m_angEyeAngles().y, player->get_animation_state()->m_flGoalFeetYaw);
	Vector first = ZERO, second = ZERO, third = ZERO;
	first = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y + min(angle_diff, 35), player->hitbox_position(HITBOX_HEAD).z);
	second = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y, player->hitbox_position(HITBOX_HEAD).z);
	third = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y - min(angle_diff, 35), player->hitbox_position(HITBOX_HEAD).z);
	Ray_t one, two, three;
	trace_t tone, ttwo, ttree;
	CTraceFilter fl;
	fl.pSkip = player;
	one.Init(csgo.local()->get_shoot_position(), first);
	two.Init(csgo.local()->get_shoot_position(), second);
	three.Init(csgo.local()->get_shoot_position(), third);
	m_trace()->TraceRay(one, MASK_PLAYERSOLID, &fl, &tone);
	m_trace()->TraceRay(two, MASK_PLAYERSOLID, &fl, &ttwo);
	m_trace()->TraceRay(three, MASK_PLAYERSOLID, &fl, &ttree);
	if (!tone.allsolid && !ttwo.allsolid && !ttree.allsolid && tone.fraction < 0.97 && ttwo.fraction < 0.97 && ttree.fraction < 0.97)
		return true;

	float lby = fabs(math::normalize_yaw(player->m_flLowerBodyYawTarget()));
	if (lby < 35 && lby > -35)
		return true;
	return false;
}

bool resolver::low_delta2()
{
	auto record = player_record;
	if (!csgo.local()->is_alive())
		return false;
	float angle_diff = math::angle_diff(player->m_angEyeAngles().y, player->get_animation_state()->m_flGoalFeetYaw);
	Vector first = ZERO, second = ZERO, third = ZERO;
	first = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y + min(angle_diff, 20), player->hitbox_position(HITBOX_HEAD).z);
	second = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y, player->hitbox_position(HITBOX_HEAD).z);
	third = Vector(player->hitbox_position(HITBOX_HEAD).x, player->hitbox_position(HITBOX_HEAD).y - min(angle_diff, 20), player->hitbox_position(HITBOX_HEAD).z);
	Ray_t one, two, three;
	trace_t tone, ttwo, ttree;
	CTraceFilter fl;
	fl.pSkip = player;
	one.Init(csgo.local()->get_shoot_position(), first);
	two.Init(csgo.local()->get_shoot_position(), second);
	three.Init(csgo.local()->get_shoot_position(), third);
	m_trace()->TraceRay(one, MASK_PLAYERSOLID, &fl, &tone);
	m_trace()->TraceRay(two, MASK_PLAYERSOLID, &fl, &ttwo);
	m_trace()->TraceRay(three, MASK_PLAYERSOLID, &fl, &ttree);
	if (!tone.allsolid && !ttwo.allsolid && !ttree.allsolid && tone.fraction < 0.97 && ttwo.fraction < 0.97 && ttree.fraction < 0.97)
		return true;

	float lby = fabs(math::normalize_yaw(player->m_flLowerBodyYawTarget()));
	if (lby < 20 && lby > -20)
		return true;
	return false;
}

float resolver::GetBackwardYaw(player_t* ent) {
	return math::calculate_angle(csgo.local()->GetAbsOrigin(), ent->GetAbsOrigin()).y;
}

float resolver::GetForwardYaw(player_t* ent) {
	return math::normalize_yaw(GetBackwardYaw(ent) - 180.f);
}

void resolver::resolve_yaw()
{
	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info))
		return;

	static int side[63];
	auto animstate  = player->get_animation_state();
	auto speed = csgo.local()->m_vecVelocity().Length2D();
	if (speed <= 0.1f)
	{
		if (player_record->layers[3].m_flWeight == 0.0 && player_record->layers[3].m_flCycle == 0.0)
		{
			side[player->EntIndex()] = 2 * (math::normalize_diff(player->m_angEyeAngles().y, player_record->abs_angles.y) <= 0.0) - 1;
		}
	}
	else
	{
		const float f_delta = abs(player_record->layers[6].m_flPlaybackRate -   player_record->left_layers[6].m_flPlaybackRate);
		const float s_delta = abs(player_record->layers[6].m_flPlaybackRate - player_record->center_layers[6].m_flPlaybackRate);
		const float t_delta = abs(player_record->layers[6].m_flPlaybackRate -  player_record->right_layers[6].m_flPlaybackRate);

		if (f_delta < s_delta || t_delta <= s_delta || (s_delta * 1000.0))
		{
			if (f_delta >= t_delta && s_delta > t_delta && !(t_delta * 1000.0))
			{
				side[player->EntIndex()] = 1;
			}
		}
		else
		{
			side[player->EntIndex()] = -1;
		}
	}

	if (player->GetAbsOrigin().y == GetForwardYaw(player))
		side[player->EntIndex()] *= -1;
	
	if (csgo.globals.missed_shots[player->EntIndex()] == 0) {
		if (side[player->EntIndex()] >= 1) {
			//right
			player_record->side = low_delta() ? RESOLVER_LOW_FIRST : RESOLVER_FIRST;
		}
		else if (side[player->EntIndex()] <= -1) {
			//left
			player_record->side = low_delta() ? RESOLVER_LOW_SECOND : RESOLVER_SECOND;
		}
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] == 1) {
		if (side[player->EntIndex()] >= 1) {
			//right
			player_record->side = low_delta2() ? RESOLVER_LOW_FIRST1 : RESOLVER_FIRST;
		}
		else if (side[player->EntIndex()] <= -1) {
			//left
			player_record->side = low_delta2() ? RESOLVER_LOW_SECOND1 : RESOLVER_SECOND;
		}
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] == 2) {
		player_record->side = RESOLVER_SECOND;
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] == 3) {
		player_record->side = RESOLVER_FIRST;
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] == 4) {
		player_record->side = RESOLVER_LOW_FIRST;
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] == 5) {
		player_record->side = RESOLVER_LOW_SECOND;
	}
	else if (csgo.globals.missed_shots[player->EntIndex()] >= 6) {
		m_cvar()->ConsoleColorPrintf(Color(255, 0, 0), u8"иди нахуй, это сприд, у тебя кфг иссуе\n");
		csgo.globals.missed_shots[player->EntIndex()] = 0;
	}

}

float resolver::resolve_pitch()
{
}
