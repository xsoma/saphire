// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "aim.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\autowall\autowall.h"
#include "..\misc\prediction_system.h"
#include "..\fakewalk\slowwalk.h"
#include "..\lagcompensation\local_animations.h"

void aim::run(CUserCmd* cmd)
{
	backup.clear();
	targets.clear();
	scanned_targets.clear();
	final_target.reset();
	should_stop = false;

	if (!vars.ragebot.enable)
		return;

	automatic_revolver(cmd);
	prepare_targets();

	if (csgo.globals.weapon->is_non_aim())
		return;

	if (csgo.globals.current_weapon == -1)
		return;

	scan_targets();

	if (!should_stop && vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_PREDICTIVE])
	{
		auto max_speed = 260.0f;
		auto weapon_info = csgo.globals.weapon->get_csweapon_info();

		if (weapon_info)
			max_speed = csgo.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

		auto ticks_to_stop = math::clamp(engineprediction::get().backup_data.velocity.Length2D() / max_speed * 3.0f, 0.0f, 4.0f);
		auto predicted_eye_pos = csgo.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * ticks_to_stop;

		for (auto& target : targets)
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data, predicted_eye_pos, true);

			if (!last_data.valid())
				continue;

			should_stop = true;
			break;
		}
	}

	automatic_stop(cmd);


	if (scanned_targets.empty())
		return;

	find_best_target();

	if (!final_target.data.valid())
		return;

	fire(cmd);
}

void aim::automatic_revolver(CUserCmd* cmd)
{
	if (!m_engine()->IsActiveApp())
		return;

	if (csgo.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	if (cmd->m_buttons & IN_ATTACK)
		return;

	cmd->m_buttons &= ~IN_ATTACK2;

	static auto r8cock_time = 0.0f;
	auto server_time = TICKS_TO_TIME(csgo.globals.backup_tickbase);

	if (csgo.globals.weapon->can_fire(false))
	{
		if (r8cock_time <= server_time) //-V807
		{
			if (csgo.globals.weapon->m_flNextSecondaryAttack() <= server_time)
				r8cock_time = server_time + 0.234375f;
			else
				cmd->m_buttons |= IN_ATTACK2;
		}
		else
			cmd->m_buttons |= IN_ATTACK;
	}
	else
	{
		r8cock_time = server_time + 0.234375f;
		cmd->m_buttons &= ~IN_ATTACK;
	}

	csgo.globals.revolver_working = true;
}

void aim::prepare_targets()
{
	for (auto i = 1; i <= 32; i++)
	{
		if (vars.player_list.white_list[i])
			continue;

		auto e = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!e->valid(true, false))
			continue;

		auto records = &player_records[i]; //-V826

		if (records->empty())
			continue;

		targets.emplace_back(target(e, get_record(records, false), get_record(records, true)));
	}

	for (auto& target : targets)
		backup.emplace_back(adjust_data(target.e));
}

static bool compare_records(const optimized_adjust_data& first, const optimized_adjust_data& second)
{
	auto first_pitch = math::normalize_pitch(first.angles.x);
	auto second_pitch = math::normalize_pitch(second.angles.x);

	if (fabs(first_pitch - second_pitch) > 15.0f)
		return fabs(first_pitch) < fabs(second_pitch);
	else if (first.duck_amount != second.duck_amount) //-V550
		return first.duck_amount < second.duck_amount;
	else if (first.origin != second.origin)
		return first.origin.DistTo(csgo.local()->GetAbsOrigin()) < second.origin.DistTo(csgo.local()->GetAbsOrigin());

	return first.simulation_time > second.simulation_time;
}

adjust_data* aim::get_record(std::deque <adjust_data>* records, bool history)
{
	if (history)
	{
		std::deque <optimized_adjust_data> optimized_records; //-V826

		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);
			optimized_adjust_data optimized_record;

			optimized_record.i = i;
			optimized_record.player = record->player;
			optimized_record.simulation_time = record->simulation_time;
			optimized_record.duck_amount = record->duck_amount;
			optimized_record.angles = record->angles;
			optimized_record.origin = record->origin;

			optimized_records.emplace_back(optimized_record);
		}

		if (optimized_records.size() < 2)
			return nullptr;

		std::sort(optimized_records.begin(), optimized_records.end(), compare_records);

		for (auto& optimized_record : optimized_records)
		{
			auto record = &records->at(optimized_record.i);

			if (!record->valid())
				continue;

			return record;
		}
	}
	else
	{
		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);

			if (!record->valid())
				continue;

			return record;
		}
	}

	return nullptr;
}
#include "../autowall/Autowall2.h"
int aim::get_minimum_damage(bool visible, int health)
{
	auto minimum_damage = 1;

	if (visible)
	{
		if (vars.ragebot.weapon[csgo.globals.current_weapon].minimum_visible_damage > 100)
			minimum_damage = health + vars.ragebot.weapon[csgo.globals.current_weapon].minimum_visible_damage - 100;
		else
			minimum_damage = math::clamp(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_visible_damage, 1, health);
	}
	else
	{
		if (vars.ragebot.weapon[csgo.globals.current_weapon].minimum_damage > 100)
			minimum_damage = health + vars.ragebot.weapon[csgo.globals.current_weapon].minimum_damage - 100;
		else
			minimum_damage = math::clamp(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_damage, 1, health);
	}

	if (key_binds::get().get_key_bind_state(4))
	{
		if (vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage > 100)
			minimum_damage = health + vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage - 100;
		else
			minimum_damage = math::clamp(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage, 1, health);
	}

	return minimum_damage;
}

void aim::scan_targets()
{
	if (targets.empty())
		return;

	for (auto& target : targets)
	{
		if (target.history_record->valid())
		{
			scan_data last_data;

			if (target.last_record->valid())
			{
				target.last_record->adjust_player();
				scan(target.last_record, last_data);
			}

			scan_data history_data;

			target.history_record->adjust_player();
			scan(target.history_record, history_data);

			if (last_data.valid() && last_data.damage > history_data.damage)
				scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
			else if (history_data.valid())
				scanned_targets.emplace_back(scanned_target(target.history_record, history_data));
		}
		else
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data);

			if (!last_data.valid())
				continue;

			scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
		}
	}
}

bool aim::automatic_stop(CUserCmd* cmd)
{
	if (!should_stop)
		return true;

	if (!vars.ragebot.weapon[csgo.globals.current_weapon].autostop)
		return true;

	if (csgo.globals.slowwalking)
		return true;

	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)) //-V807
		return true;

	if (csgo.globals.weapon->is_empty())
		return true;

	if (!vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_BETWEEN_SHOTS] && !csgo.globals.weapon->can_fire(false))
		return true;

	auto animlayer = csgo.local()->get_animlayers()[1];

	if (animlayer.m_nSequence)
	{
		auto activity = csgo.local()->sequence_activity(animlayer.m_nSequence);

		if (activity == ACT_CSGO_RELOAD && animlayer.m_flWeight > 0.0f)
			return true;
	}

	auto weapon_info = csgo.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return true;
	Vector angle;
	Vector directional;

	math::vector_angles(directional, angle);
	auto get_standing_accuracy = [&]() -> const float
	{
		const auto max_speed = csgo.globals.weapon->m_zoomLevel() > 0 ? weapon_info->flMaxPlayerSpeed : weapon_info->flMaxPlayerSpeedAlt;
		return max_speed / 3.f;
	};
	auto velocity = csgo.local()->m_vecVelocity();
	float speed = velocity.Length2D();
	float max_speed = (csgo.globals.weapon->m_zoomLevel() == 0 ? weapon_info->flMaxPlayerSpeed : weapon_info->flMaxPlayerSpeedAlt) * 0.1f;

	if (speed <= get_standing_accuracy() / 2)
		slowwalk::get().autostop(csgo.get_command());
	else
	{
		Vector direction;
		Vector original = csgo.get_command()->m_viewangles;
		math::vector_angles(velocity, direction);
		direction.y = original.y - direction.y;
		Vector forward;
		math::angle_vectors(direction, forward);
		Vector negated_direction = forward * -speed;
		csgo.get_command()->m_forwardmove = negated_direction.x;
		csgo.get_command()->m_sidemove = negated_direction.y;
		if (vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_FORCE_ACCURACY])
			return false;
	}
	return true;
}

static bool compare_points(const scan_point& first, const scan_point& second)
{
	return !first.center && first.hitbox == second.hitbox;
}

void aim::scan(adjust_data* record, scan_data& data, const Vector& shoot_position, bool optimized)
{
	auto weapon = optimized ? csgo.local()->m_hActiveWeapon().Get() : csgo.globals.weapon;

	if (!weapon)
		return;

	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	auto hitboxes = get_hitboxes(record, optimized);

	if (hitboxes.empty())
		return;

	auto force_safe_points = record->player->m_iHealth() <= weapon_info->iDamage || key_binds::get().get_key_bind_state(3) || vars.player_list.force_safe_points[record->i] || vars.ragebot.weapon[csgo.globals.current_weapon].max_misses && csgo.globals.missed_shots[record->i] >= vars.ragebot.weapon[csgo.globals.current_weapon].max_misses_amount; //-V648
	auto best_damage = 0;

	auto minimum_damage = get_minimum_damage(false, record->player->m_iHealth());
	auto minimum_visible_damage = get_minimum_damage(true, record->player->m_iHealth());

	auto get_hitgroup = [](const int& hitbox)
	{
		if (hitbox == HITBOX_HEAD)
			return 0;
		else if (hitbox == HITBOX_PELVIS)
			return 1;
		else if (hitbox == HITBOX_STOMACH)
			return 2;
		else if (hitbox >= HITBOX_LOWER_CHEST && hitbox <= HITBOX_UPPER_CHEST)
			return 3;
		else if (hitbox >= HITBOX_RIGHT_THIGH && hitbox <= HITBOX_LEFT_FOOT)
			return 4;
		else if (hitbox >= HITBOX_RIGHT_HAND && hitbox <= HITBOX_LEFT_FOREARM)
			return 5;

		return -1;
	};

	std::vector <scan_point> points; //-V826

	for (auto& hitbox : hitboxes)
	{
		if (optimized)
		{
			points.emplace_back(scan_point(record->player->hitbox_position_matrix(hitbox, record->matrixes_data.main), hitbox, true));
			continue;
		}

		auto current_points = get_points(record, hitbox);

		for (auto& point : current_points)
		{
			if (!record->bot)
			{
				auto safe = 0.0f;

				if (record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.first[0].GetOrigin() || record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin() || record->matrixes_data.first[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin())
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.first, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.second, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;

				point.safe = safe;
			}
			else
				point.safe = 1.0f;

			if (!force_safe_points || point.safe)
				points.emplace_back(point);
		}
	}

	if (points.empty())
		return;

	if (!optimized)
		std::sort(points.begin(), points.end(), compare_points);

	auto body_hitboxes = true;

	for (auto& point : points)
	{
		if (!optimized && body_hitboxes && (point.hitbox < HITBOX_PELVIS || point.hitbox > HITBOX_UPPER_CHEST))
		{
			body_hitboxes = false;

			if (vars.player_list.force_body_aim[record->i])
				break;

			if (key_binds::get().get_key_bind_state(22))
				break;

			if (best_damage >= record->player->m_iHealth())
				break;

			if (vars.ragebot.weapon[csgo.globals.current_weapon].prefer_body_aim && best_damage >= 1)
				break;
		}

		if (!optimized && (vars.ragebot.weapon[csgo.globals.current_weapon].prefer_safe_points || force_safe_points) && data.point.safe && data.point.safe < point.safe)
			continue;

		auto fire_data = autowall::get().wall_penetration(shoot_position, point.point, record->player);

		if (!fire_data.valid)
			continue;

		if (fire_data.damage < 1)
			continue;

		if (!fire_data.visible && !vars.ragebot.autowall)
			continue;

		auto current_minimum_damage = fire_data.visible ? minimum_visible_damage : minimum_damage;

		if (fire_data.damage >= current_minimum_damage && fire_data.damage >= best_damage)
		{
			if (!optimized && !should_stop)
			{
				should_stop = true;

				if (vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_LETHAL] && fire_data.damage < record->player->m_iHealth())
					should_stop = false;
				else if (vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_VISIBLE] && !fire_data.visible)
					should_stop = false;
				else if (vars.ragebot.weapon[csgo.globals.current_weapon].autostop_modifiers[AUTOSTOP_CENTER] && !point.center)
					should_stop = false;
			}

			if (!optimized && force_safe_points && !point.safe)
				continue;

			best_damage = fire_data.damage;

			data.point = point;
			data.visible = fire_data.visible;
			data.damage = best_damage;
			data.hitbox = fire_data.hitbox;

			if (optimized)
				return;
		}
	}
}

std::vector <int> aim::get_hitboxes(adjust_data* record, bool optimized)
{
	std::vector <int> hitboxes; //-V827

	if (optimized)
	{
		hitboxes.emplace_back(HITBOX_HEAD);
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_STOMACH);
		hitboxes.emplace_back(HITBOX_PELVIS);

		return hitboxes;
	}

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(1))
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(2))
		hitboxes.emplace_back(HITBOX_CHEST);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(3))
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(4))
		hitboxes.emplace_back(HITBOX_STOMACH);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(5))
		hitboxes.emplace_back(HITBOX_PELVIS);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(0))
		hitboxes.emplace_back(HITBOX_HEAD);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(6))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);
	}

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(7))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_THIGH);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);

		hitboxes.emplace_back(HITBOX_RIGHT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
	}

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitboxes.at(8))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
	}

	return hitboxes;
}

std::vector <scan_point> aim::get_points(adjust_data* record, int hitbox, bool from_aim)
{
	std::vector <scan_point> points; //-V827
	auto model = record->player->GetModel();

	if (!model)
		return points;

	auto hdr = m_modelinfo()->GetStudioModel(model);

	if (!hdr)
		return points;

	auto set = hdr->pHitboxSet(record->player->m_nHitboxSet());

	if (!set)
		return points;

	auto bbox = set->pHitbox(hitbox);

	if (!bbox)
		return points;

	auto center = (bbox->bbmin + bbox->bbmax) * 0.5f;

	if (bbox->radius <= 0.0f)
	{
		auto rotation_matrix = math::angle_matrix(bbox->rotation);

		matrix3x4_t matrix;
		math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);

		auto origin = matrix.GetOrigin();

		if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
		{
			auto side = (bbox->bbmin.z - center.z) * 0.875f;

			if (hitbox == HITBOX_LEFT_FOOT)
				side = -side;

			points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), hitbox, true));

			auto min = (bbox->bbmin.x - center.x) * 0.875f;
			auto max = (bbox->bbmax.x - center.x) * 0.875f;

			points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), hitbox, false));
			points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), hitbox, false));
		}
	}
	else
	{
		auto scale = 0.0f;

		if (vars.ragebot.weapon[csgo.globals.current_weapon].static_point_scale)
		{
			if (hitbox == HITBOX_HEAD)
				scale = vars.ragebot.weapon[csgo.globals.current_weapon].head_scale;
			else
				scale = vars.ragebot.weapon[csgo.globals.current_weapon].body_scale;
		}
		else
		{
			auto transformed_center = center;
			math::vector_transform(transformed_center, record->matrixes_data.main[bbox->bone], transformed_center);

			auto spread = csgo.globals.spread + csgo.globals.inaccuracy;
			auto distance = transformed_center.DistTo(csgo.globals.eye_pos);

			distance /= math::fast_sin(DEG2RAD(90.0f - RAD2DEG(spread)));
			spread = math::fast_sin(spread);

			auto radius = max(bbox->radius - distance * spread, 0.0f);
			scale = math::clamp(radius / bbox->radius, 0.0f, 1.0f);
		}

		if (scale <= 0.0f) //-V648
		{
			math::vector_transform(center, record->matrixes_data.main[bbox->bone], center);
			points.emplace_back(scan_point(center, hitbox, true));

			return points;
		}

		auto final_radius = bbox->radius * scale;

		if (hitbox == HITBOX_HEAD)
		{
			auto pitch_down = math::normalize_pitch(record->angles.x) > 85.0f;
			auto backward = fabs(math::normalize_yaw(record->angles.y - math::calculate_angle(record->player->get_shoot_position(), csgo.local()->GetAbsOrigin()).y)) > 120.0f;

			points.emplace_back(scan_point(center, hitbox, !pitch_down || !backward));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x + 0.70710678f * final_radius, bbox->bbmax.y - 0.70710678f * final_radius, bbox->bbmax.z), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - final_radius, bbox->bbmax.z), hitbox, false));

			if (pitch_down && backward)
				points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
		}
		else if (hitbox >= HITBOX_PELVIS && hitbox <= HITBOX_UPPER_CHEST)
		{
			points.emplace_back(scan_point(center, hitbox, true));

			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

			points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - final_radius, center.z), hitbox, true));
		}
		else if (hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF)
		{
			points.emplace_back(scan_point(center, hitbox, true));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
		}
		else if (hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH)
			points.emplace_back(scan_point(center, hitbox, true));
		else if (hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_UPPER_ARM)
		{
			points.emplace_back(scan_point(center, hitbox, true));
			points.emplace_back(scan_point(Vector(bbox->bbmax.x + final_radius, center.y, center.z), hitbox, false));
		}
	}

	for (auto& point : points)
		math::vector_transform(point.point, record->matrixes_data.main[bbox->bone], point.point);

	return points;
}

static bool compare_targets(const scanned_target& first, const scanned_target& second)
{
	if (vars.player_list.high_priority[first.record->i] != vars.player_list.high_priority[second.record->i])
		return vars.player_list.high_priority[first.record->i];

	return first.data.damage > second.data.damage;

}

void aim::find_best_target()
{
	if (vars.ragebot.weapon[csgo.globals.current_weapon].selection_type)
		std::sort(scanned_targets.begin(), scanned_targets.end(), compare_targets);

	for (auto& target : scanned_targets)
	{
		if (target.fov > (float)vars.ragebot.field_of_view)
			continue;

		final_target = target;
		final_target.record->adjust_player();
		break;
	}
}

bool IsTickValid(float simTime)
{
	static auto cl_interp_ratio = m_cvar()->FindVar(crypt_str("cl_interp_ratio"));
	static auto sv_client_min_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_min_interp_ratio"));
	static auto sv_client_max_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_max_interp_ratio"));
	auto lerp_ratio = math::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
	INetChannelInfo* nci = m_engine()->GetNetChannelInfo();

	if (!nci)
		return false;

	auto LerpTicks = TIME_TO_TICKS(lerp_ratio);

	int predCmdArrivTick = m_globals()->m_tickcount + 1 + TIME_TO_TICKS(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING));

	float flCorrect = math::clamp(lerp_ratio + nci->GetLatency(FLOW_OUTGOING), 0.f, 1.f) - TICKS_TO_TIME(predCmdArrivTick + LerpTicks - (TIME_TO_TICKS(simTime) + TIME_TO_TICKS(lerp_ratio)));

	return abs(flCorrect) < 0.2f;
}

void aim::fire(CUserCmd* cmd)
{
	if (!csgo.globals.weapon->can_fire(true))
		return;

	auto aim_angle = math::calculate_angle(csgo.globals.eye_pos, final_target.data.point.point).Clamp();

	if (!vars.ragebot.silent_aim)
		m_engine()->SetViewAngles(aim_angle);

	if (!vars.ragebot.autoshoot && !(cmd->m_buttons & IN_ATTACK))
		return;


	auto hitchance_amount = vars.ragebot.weapon[csgo.globals.current_weapon].hitchance_amount;

	auto is_valid_hitchance = hitchance(aim_angle, final_target.record->player, hitchance_amount);

	if (vars.ragebot.weapon[csgo.globals.current_weapon].hitchance)
	{
		if (!is_valid_hitchance)
		{
			auto is_zoomable_weapon = csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1 || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08 || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AUG || csgo.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SG553;

			if (vars.ragebot.autoscope && is_zoomable_weapon && !csgo.globals.weapon->m_zoomLevel())
				cmd->m_buttons |= IN_ATTACK2;

			return;
		}
	}

	if (!is_valid_hitchance)
		return;

	auto backtrack_ticks = 0;
	auto net_channel_info = m_engine()->GetNetChannelInfo();

	if (net_channel_info)
	{
		auto original_tickbase = csgo.globals.backup_tickbase;

		if (misc::get().double_tap_enabled && misc::get().double_tap_key)
			original_tickbase = csgo.globals.backup_tickbase + csgo.globals.weapon->get_max_tickbase_shift();

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto correct = math::clamp(net_channel_info->GetLatency(FLOW_OUTGOING) + net_channel_info->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());
		auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - final_target.record->simulation_time);

		backtrack_ticks = TIME_TO_TICKS(fabs(delta_time));
	}


	static auto get_hitbox_name = [](int hitbox, bool shot_info = false) -> std::string
	{
		switch (hitbox)
		{
		case HITBOX_HEAD:
			return shot_info ? crypt_str("Head") : crypt_str("head");
		case HITBOX_LOWER_CHEST:
			return shot_info ? crypt_str("Lower chest") : crypt_str("lower chest");
		case HITBOX_CHEST:
			return shot_info ? crypt_str("Chest") : crypt_str("chest");
		case HITBOX_UPPER_CHEST:
			return shot_info ? crypt_str("Upper chest") : crypt_str("upper chest");
		case HITBOX_STOMACH:
			return shot_info ? crypt_str("Stomach") : crypt_str("stomach");
		case HITBOX_PELVIS:
			return shot_info ? crypt_str("Pelvis") : crypt_str("pelvis");
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
			return shot_info ? crypt_str("Left arm") : crypt_str("left arm");
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
			return shot_info ? crypt_str("Right arm") : crypt_str("right arm");
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
			return shot_info ? crypt_str("Left leg") : crypt_str("left leg");
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
			return shot_info ? crypt_str("Right leg") : crypt_str("right leg");
		case HITBOX_RIGHT_FOOT:
			return shot_info ? crypt_str("Left foot") : crypt_str("left foot");
		case HITBOX_LEFT_FOOT:
			return shot_info ? crypt_str("Right foot") : crypt_str("right foot");
		}
	};

	player_info_t player_info;
	m_engine()->GetPlayerInfo(final_target.record->i, &player_info);

	cmd->m_viewangles = aim_angle;
	cmd->m_buttons |= IN_ATTACK;
	cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());

	last_target_index = final_target.record->i;
	last_shoot_position = csgo.globals.eye_pos;
	last_target[last_target_index] = Last_target
	{
		*final_target.record, final_target.data, final_target.distance
	};

	auto shot = &csgo.shots.emplace_back();
	shot->last_target = last_target_index;
	shot->side = final_target.record->side;
	shot->fire_tick = m_globals()->m_tickcount;
	shot->shot_info.target_name = player_info.szName;
	shot->shot_info.client_hitbox = get_hitbox_name(final_target.data.hitbox, true);
	shot->shot_info.client_damage = final_target.data.damage;
	shot->shot_info.hitchance = final_hitchance * 100;
	shot->shot_info.backtrack_ticks = IsTickValid(final_target.record->simulation_time) ? 22 : TIME_TO_TICKS(fabsf(final_target.record->player->m_flSimulationTime() - final_target.record->simulation_time));
	shot->shot_info.aim_point = final_target.data.point.point;

	csgo.globals.aimbot_working = true;
	csgo.globals.revolver_working = false;
	csgo.globals.last_aimbot_shot = m_globals()->m_tickcount;
}

static const int iTotalSeeds = 255;
static std::vector<std::tuple<float, float, float>> PreComputedSeeds = {};
void BulidSeedTable() {

	if (!PreComputedSeeds.empty()) return;

	for (auto i = 0; i < iTotalSeeds; i++) {
		math::random_seed(i + 1);

		const auto pi_seed = math::random_float(0.f, M_PI * 2);

		PreComputedSeeds.emplace_back(math::random_float(0.f, 1.f), sin(pi_seed), cos(pi_seed));
	}
}

void sin_cos(float radian, float* sin, float* cos) {
	*sin = std::sin(radian);
	*cos = std::cos(radian);
}

bool aim::hitchance(Vector angles, player_t* ent, float chance)
{
	BulidSeedTable();

	auto weapon = csgo.local()->m_hActiveWeapon();
	if (!weapon)
		return false;

	float flChance = vars.ragebot.weapon[csgo.globals.current_weapon].hitchance ? vars.ragebot.weapon[csgo.globals.current_weapon].hitchance_amount : 0;

	Vector fw, rw, uw;
	math::aim::AngleVectors(angles, fw, rw, uw);

	int hits = 0;
	int needed_hits = static_cast<int>(iTotalSeeds * (flChance / 100.f));

	float weapon_spread = weapon->get_spread();
	float weapon_innacuracy = weapon->get_inaccuracy();

	Vector src = csgo.local()->get_eye_pos();

	for (int i = 0; i < iTotalSeeds; i++) {

		float a = math::random_float(0.f, 1.f);
		float b = math::random_float(0.f, M_PI * 2.f);
		float c = math::random_float(0.f, 1.f);
		float d = math::random_float(0.f, M_PI * 2.f);
		float inaccuracy = a * weapon_innacuracy;
		float spread = c * weapon_spread;

		if (weapon->m_iItemDefinitionIndex() == 64/*WEAPON_REVOLVER*/) {
			if (csgo.get_command()->m_buttons & IN_ATTACK2) {
				a = 1.f - a * a;
				c = 1.f - c * c;
			}
		}

		Vector spread_view((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0);
		Vector direction;

		direction.x = fw.x + (spread_view.x * rw.x) + (spread_view.y * uw.x);
		direction.y = fw.y + (spread_view.x * rw.y) + (spread_view.y * uw.y);
		direction.z = fw.z + (spread_view.x * rw.z) + (spread_view.y * uw.z);
		direction.Normalized();

		Vector viewangles_spread;
		Vector view_forward;

		math::aim::Vector_Angles(direction, uw, viewangles_spread);
		viewangles_spread.Normalize();
		math::aim::AngleVectors2(viewangles_spread, view_forward);

		view_forward.NormalizeInPlace();
		view_forward = src + (view_forward * weapon->get_csweapon_info()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(src, view_forward);
		m_trace()->ClipRayToEntity(ray, MASK_SHOT, ent, &tr);
		//m_debugoverlay()->AddLineOverlay(src, tr.endpos, 255, 255, 255, false, 0.01f);

		if (tr.hit_entity == ent)
			hits++;

		if (static_cast<int>((static_cast<float>(hits) / iTotalSeeds) * 100.f) >= flChance)
			return true;

		if ((iTotalSeeds - i + hits) < needed_hits)
			return false;
	}
	final_hitchance = ((float)hits / (float)iTotalSeeds);
	return false;
}

static int clip_ray_to_hitbox(const Ray_t& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, trace_t& trace)
{
	static auto fn = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 F3 0F 10 42"));

	trace.fraction = 1.0f;
	trace.startsolid = false;

	return reinterpret_cast <int(__fastcall*)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, trace_t&)> (fn)(ray, hitbox, matrix, trace);
}

bool aim::hitbox_intersection(player_t* e, matrix3x4_t* matrix, int hitbox, const Vector& start, const Vector& end, float* safe)
{
	auto model = e->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->pHitboxSet(e->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto studio_hitbox = studio_set->pHitbox(hitbox);

	if (!studio_hitbox)
		return false;

	trace_t trace;

	Ray_t ray;
	ray.Init(start, end);

	auto intersected = clip_ray_to_hitbox(ray, studio_hitbox, matrix[studio_hitbox->bone], trace) >= 0;

	if (!safe)
		return intersected;

	Vector min, max;

	math::vector_transform(studio_hitbox->bbmin, matrix[studio_hitbox->bone], min);
	math::vector_transform(studio_hitbox->bbmax, matrix[studio_hitbox->bone], max);

	auto center = (min + max) * 0.5f;
	auto distance = center.DistTo(end);

	if (distance > *safe)
		*safe = distance;

	return intersected;
}