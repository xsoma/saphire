// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"

std::deque <adjust_data> player_records[65];


void lagcompensation::fsn(ClientFrameStage_t stage)
{
	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!vars.ragebot.enable)
		return;
	for (auto i = 1; i < m_globals()->m_maxclients; i++) //-V807
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (e == csgo.local())
			continue;

		if (!valid(i, e))
			continue;

		if (!e->is_alive())
			csgo.globals.missed_shots[e->EntIndex()] = 0;

		auto update = player_records[i].empty() || e->m_flSimulationTime() != e->m_flOldSimulationTime(); //-V550
		
		if (update) //-V550
		{
			if (!player_records[i].empty() && (e->m_vecOrigin() - player_records[i].front().origin).LengthSqr() > 4096.0f)
				for (auto& record : player_records[i])
					record.invalid = true;

			player_records[i].emplace_front(adjust_data());
			update_player_animations(e);

			while (player_records[i].size() > 32)
				player_records[i].pop_back();
		}
	}
}

void lagcompensation::extrapolate(player_t* player, Vector& origin, Vector& velocity, int& flags, bool wasonground)
{
	static auto sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));
	static auto sv_jump_impulse = m_cvar()->FindVar(crypt_str("sv_jump_impulse"));

	if (!(flags & FL_ONGROUND))
		velocity.z -= (m_globals()->m_frametime * sv_gravity->GetFloat());
	else if (wasonground)
		velocity.z = sv_jump_impulse->GetFloat();

	const Vector mins = player->GetCollideable()->OBBMins();
	const Vector max = player->GetCollideable()->OBBMaxs();

	const Vector src = origin;
	Vector end = src + (velocity * m_globals()->m_frametime);

	Ray_t ray;
	ray.Init(src, end, mins, max);

	trace_t trace;
	CTraceFilter filter;
	filter.pSkip = (void*)(player);

	m_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	if (trace.fraction != 1.f)
	{
		for (int i = 0; i < 2; i++)
		{
			velocity -= trace.plane.normal * velocity.Dot(trace.plane.normal);

			const float dot = velocity.Dot(trace.plane.normal);
			if (dot < 0.f)
			{
				velocity.x -= dot * trace.plane.normal.x;
				velocity.y -= dot * trace.plane.normal.y;
				velocity.z -= dot * trace.plane.normal.z;
			}

			end = trace.endpos + (velocity * (m_globals()->m_intervalpertick * (1.f - trace.fraction)));

			ray.Init(trace.endpos, end, mins, max);
			m_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.fraction == 1.f)
				break;
		}
	}

	origin = trace.endpos;
	end = trace.endpos;
	end.z -= 2.f;

	ray.Init(origin, end, mins, max);
	m_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	flags &= ~(1 << 0);

	if (trace.DidHit() && trace.plane.normal.z > 0.7f)
		flags |= (1 << 0);
}

bool lagcompensation::valid(int i, player_t* e)
{
	if (!vars.ragebot.enable || !e->valid(false))
	{
		if (!e->is_alive())
		{
			is_dormant[i] = false;
			player_resolver[i].reset();

			csgo.globals.fired_shots[i] = 0;
		}
		else if (e->IsDormant())
			is_dormant[i] = true;

		player_records[i].clear();
		return false;
	}

	return true;
}

void lagcompensation::update_player_animations(player_t* e)
{
	auto animstate = e->get_animation_state();

	if (!animstate)
		return;

	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(e->EntIndex(), &player_info))
		return;

	auto records = &player_records[e->EntIndex()]; //-V826

	if (records->empty())
		return;

	adjust_data* previous_record = nullptr;

	auto record = &records->front();

	AnimationLayer animlayers[13];
	float pose_parametrs[24];

	memcpy(pose_parametrs, &e->m_flPoseParameter(), 24 * sizeof(float));
	memcpy(animlayers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->left_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->right_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->center_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));

	auto backup_lower_body_yaw_target = e->m_flLowerBodyYawTarget();
	auto backup_duck_amount = e->m_flDuckAmount();
	auto backup_flags = e->m_fFlags();
	auto backup_eflags = e->m_iEFlags();

	auto backup_curtime = m_globals()->m_curtime; //-V807
	auto backup_frametime = m_globals()->m_frametime;
	auto backup_realtime = m_globals()->m_realtime;
	auto backup_framecount = m_globals()->m_framecount;
	auto backup_tickcount = m_globals()->m_tickcount;
	auto backup_interpolation_amount = m_globals()->m_interpolation_amount;

	m_globals()->m_curtime = e->m_flSimulationTime();
	m_globals()->m_frametime = m_globals()->m_intervalpertick;

	if (previous_record)
	{
		auto velocity = e->m_vecVelocity();
		auto was_in_air = e->m_fFlags() & FL_ONGROUND && previous_record->flags & FL_ONGROUND;

		auto time_difference = max(m_globals()->m_intervalpertick, e->m_flSimulationTime() - previous_record->simulation_time);
		auto origin_delta = e->m_vecOrigin() - previous_record->origin;

		auto animation_speed = 0.0f;

		if (!origin_delta.IsZero() && TIME_TO_TICKS(time_difference) > 0)
		{
			e->m_vecVelocity() = origin_delta * (1.0f / time_difference);

			if (e->m_fFlags() & FL_ONGROUND && animlayers[11].m_flWeight > 0.0f && animlayers[11].m_flWeight < 1.0f && animlayers[11].m_flCycle > previous_record->layers[11].m_flCycle)
			{
				auto weapon = e->m_hActiveWeapon().Get();

				if (weapon)
				{
					auto max_speed = 260.0f;
					auto weapon_info = e->m_hActiveWeapon().Get()->get_csweapon_info();

					if (weapon_info)
						max_speed = e->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

					auto modifier = 0.35f * (1.0f - animlayers[11].m_flWeight);

					if (modifier > 0.0f && modifier < 1.0f)
						animation_speed = max_speed * (modifier + 0.55f);
				}
			}

			if (animation_speed > 0.0f)
			{
				animation_speed /= e->m_vecVelocity().Length2D();

				e->m_vecVelocity().x *= animation_speed;
				e->m_vecVelocity().y *= animation_speed;
			}

			if (records->size() >= 3 && time_difference > m_globals()->m_intervalpertick)
			{
				auto previous_velocity = (previous_record->origin - records->at(2).origin) * (1.0f / time_difference);

				if (!previous_velocity.IsZero() && !was_in_air)
				{
					auto current_direction = math::normalize_yaw(RAD2DEG(atan2(e->m_vecVelocity().y, e->m_vecVelocity().x)));
					auto previous_direction = math::normalize_yaw(RAD2DEG(atan2(previous_velocity.y, previous_velocity.x)));

					auto average_direction = current_direction - previous_direction;
					average_direction = DEG2RAD(math::normalize_yaw(current_direction + average_direction * 0.5f));

					auto direction_cos = cos(average_direction);
					auto dirrection_sin = sin(average_direction);

					auto velocity_speed = e->m_vecVelocity().Length2D();

					e->m_vecVelocity().x = direction_cos * velocity_speed;
					e->m_vecVelocity().y = dirrection_sin * velocity_speed;
				}
			}

			if (!(e->m_fFlags() & FL_ONGROUND))
			{
				static auto sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));

				auto fixed_timing = math::clamp(time_difference, m_globals()->m_intervalpertick, 1.0f);
				e->m_vecVelocity().z -= sv_gravity->GetFloat() * fixed_timing * 0.5f;
			}
			else
				e->m_vecVelocity().z = 0.0f;
		}
	}

	e->m_iEFlags() &= ~0x1000;

	if (e->m_fFlags() & FL_ONGROUND && e->m_vecVelocity().Length() > 0.0f && animlayers[6].m_flWeight <= 0.0f)
		e->m_vecVelocity().Zero();

	e->m_vecAbsVelocity() = e->m_vecVelocity();
	e->m_bClientSideAnimation() = true;

	if (is_dormant[e->EntIndex()])
	{
		is_dormant[e->EntIndex()] = false;

		if (e->m_fFlags() & FL_ONGROUND)
		{
			animstate->m_bOnGround = true;
			animstate->m_bInHitGroundAnimation = false;
		}

		animstate->time_since_in_air() = 0.0f;
		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);
	}

	auto updated_animations = false;

	c_baseplayeranimationstate state;
	memcpy(&state, animstate, sizeof(c_baseplayeranimationstate));
	auto ticks_chocked = 1;
	if (previous_record)
	{
		memcpy(e->get_animlayers(), previous_record->layers, e->animlayer_count() * sizeof(AnimationLayer));
		memcpy(&e->m_flPoseParameter(), pose_parametrs, 24 * sizeof(float));
		
		auto simulation_ticks = TIME_TO_TICKS(e->m_flSimulationTime() - previous_record->simulation_time);

		if (simulation_ticks > 0 && simulation_ticks < 17)
			ticks_chocked = simulation_ticks;

		if (ticks_chocked > 1)
		{
			auto land_time = 0.0f;
			auto land_in_cycle = false;
			auto is_landed = false;
			auto on_ground = false;

			if (animlayers[4].m_flCycle < 0.5f && (!(e->m_fFlags() & FL_ONGROUND) || !(previous_record->flags & FL_ONGROUND)))
			{
				land_time = e->m_flSimulationTime() - animlayers[4].m_flPlaybackRate * animlayers[4].m_flCycle;
				land_in_cycle = land_time >= previous_record->simulation_time;
			}

			auto duck_amount_per_tick = (e->m_flDuckAmount() - previous_record->duck_amount) / ticks_chocked;

			for (auto i = 0; i < ticks_chocked; ++i)
			{
				auto simulated_time = previous_record->simulation_time + TICKS_TO_TIME(i);

				if (duck_amount_per_tick) //-V550
					e->m_flDuckAmount() = previous_record->duck_amount + duck_amount_per_tick * (float)i;

				on_ground = e->m_fFlags() & FL_ONGROUND;

				if (land_in_cycle && !is_landed)
				{
					if (land_time <= simulated_time)
					{
						is_landed = true;
						on_ground = true;
					}
					else
						on_ground = previous_record->flags & FL_ONGROUND;
				}

				if (on_ground)
					e->m_fFlags() |= FL_ONGROUND;
				else
					e->m_fFlags() &= ~FL_ONGROUND;

				auto simulated_ticks = TIME_TO_TICKS(simulated_time);

				m_globals()->m_realtime = simulated_time;
				m_globals()->m_curtime = simulated_time;
				m_globals()->m_framecount = simulated_ticks;
				m_globals()->m_tickcount = simulated_ticks;
				m_globals()->m_interpolation_amount = 0.0f;

				csgo.globals.updating_animation = true;
				e->update_clientside_animation();
				csgo.globals.updating_animation = false;

				m_globals()->m_realtime = backup_realtime;
				m_globals()->m_curtime = backup_curtime;
				m_globals()->m_framecount = backup_framecount;
				m_globals()->m_tickcount = backup_tickcount;
				m_globals()->m_interpolation_amount = backup_interpolation_amount;

				updated_animations = true;
			}
		}
	}

	if (!updated_animations)
	{
		csgo.globals.updating_animation = true;
		e->update_clientside_animation();
		csgo.globals.updating_animation = false;
	}

	memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

	auto setup_matrix = [&](player_t* e, AnimationLayer* layers, const int& matrix) -> void
	{
		e->invalidate_physics_recursive(8);

		AnimationLayer backup_layers[13];
		memcpy(backup_layers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		memcpy(e->get_animlayers(), layers, e->animlayer_count() * sizeof(AnimationLayer));

		switch (matrix)
		{
		case MAIN:
			e->setup_bones_rebuilt(record->matrixes_data.main, BONE_USED_BY_ANYTHING);
			break;
		case NONE:
			e->setup_bones_rebuilt(record->matrixes_data.zero, BONE_USED_BY_ANYTHING);
			break;
		case FIRST:
			e->setup_bones_rebuilt(record->matrixes_data.first, BONE_USED_BY_ANYTHING);
			break;
		case SECOND:
			e->setup_bones_rebuilt(record->matrixes_data.second, BONE_USED_BY_ANYTHING);
			break;
		}

		memcpy(e->get_animlayers(), backup_layers, e->animlayer_count() * sizeof(AnimationLayer));
	};

	if (!player_info.fakeplayer && csgo.local()->is_alive() && e->m_iTeamNum() != csgo.local()->m_iTeamNum() && ticks_chocked >= 1)
	{
		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()]; //-V807

		csgo.globals.updating_animation = true;
		e->update_clientside_animation();
		csgo.globals.updating_animation = false;

		previous_goal_feet_yaw[e->EntIndex()] = animstate->m_flGoalFeetYaw;
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y); //-V807

		csgo.globals.updating_animation = true;
		e->update_clientside_animation();
		csgo.globals.updating_animation = false;

		setup_matrix(e, record->center_layers, NONE);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + 60.0f);

		csgo.globals.updating_animation = true;
		e->update_clientside_animation();
		csgo.globals.updating_animation = false;

		setup_matrix(e, record->left_layers, FIRST);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - 60.0f);

		csgo.globals.updating_animation = true;
		e->update_clientside_animation();
		csgo.globals.updating_animation = false;

		setup_matrix(e, record->right_layers, SECOND);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		
		memcpy(&e->m_flPoseParameter(), pose_parametrs, 24 * sizeof(float));
		player_resolver[e->EntIndex()].initialize(e, record, previous_goal_feet_yaw[e->EntIndex()], e->m_angEyeAngles().x);
		player_resolver[e->EntIndex()].resolve_yaw();

		switch (record->side)
		{
		case RESOLVER_ORIGINAL:
			animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()];
			break;
		case RESOLVER_ZERO:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);
			break;
		case RESOLVER_FIRST:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + e->get_max_desync_delta());
			break;
		case RESOLVER_SECOND:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - e->get_max_desync_delta());
			break;
		case RESOLVER_LOW_FIRST:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + 35.f);
			break;
		case RESOLVER_LOW_SECOND:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - 35.f);
			break;
		case RESOLVER_LOW_FIRST1:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + 20.f);
			break;
		case RESOLVER_LOW_SECOND1:
			animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - 20.f);
			break;
		}
	}

	csgo.globals.updating_animation = true;
	e->update_clientside_animation();
	csgo.globals.updating_animation = false;

	setup_matrix(e, animlayers, MAIN);
	memcpy(e->m_CachedBoneData().Base(), record->matrixes_data.main, e->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	m_globals()->m_curtime = backup_curtime;
	m_globals()->m_frametime = backup_frametime;

	e->m_flLowerBodyYawTarget() = backup_lower_body_yaw_target;
	e->m_flDuckAmount() = backup_duck_amount;
	e->m_fFlags() = backup_flags;
	e->m_iEFlags() = backup_eflags;

	memcpy(e->get_animlayers(), animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	record->store_data(e, false);

	if (e->m_flSimulationTime() < e->m_flOldSimulationTime())
		record->invalid = true;
}