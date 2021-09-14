// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "autowall.h"
using Fna = bool(__fastcall)(IClientEntity*);
bool autowall::is_breakable_entity(IClientEntity* e)
{
	if (!e && !e->EntIndex())
		return false;

	static auto is_breakable = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 85 F6 74 68"));

	int chTakeDamage = *(uintptr_t*)(e + 0x280);
	int chTakeDamageBackup = chTakeDamage;
	auto client_class = e->GetClientClass();

	if ((client_class->m_pNetworkName[1] == 'B' &&
		client_class->m_pNetworkName[9] == 'e' &&
		client_class->m_pNetworkName[10] == 'S' &&
		client_class->m_pNetworkName[16] == 'e') ||
		(client_class->m_pNetworkName[1] != 'B' ||
			client_class->m_pNetworkName[5] != 'D'))
		chTakeDamage = DAMAGE_YES;

	using Fn = bool(__thiscall*)(IClientEntity*);
	auto bBreakable = ((Fn)is_breakable)(e);
	chTakeDamage = chTakeDamageBackup;

	return bBreakable;
}

void autowall::scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage)
{
	const auto is_armored = [&]() -> bool
	{
		if (e->m_ArmorValue() > 0.f)
		{
			switch (enterTrace.hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			case HITGROUP_HEAD:
				return e->m_bHasHelmet();
			default:
				break;
			}
		}

		return false;
	};

	switch (enterTrace.hitgroup)
	{
	case HITGROUP_HEAD:
		if (e->m_bHasHeavyArmor())
			currentDamage = currentDamage * .5f;
		else
			currentDamage *= 4.f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= .75f;
		break;
	default:
		break;
	}

	if (is_armored())
	{
		auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weaponData->flArmorRatio * .5f;

		if (e->m_bHasHeavyArmor())
		{
			armor_bonus_ratio = 0.5f;
			armor_ratio = weaponData->flArmorRatio * 0.5f;
			modifier = 1.f;
		}

		auto new_damage = currentDamage * armor_ratio;

		if (e->m_bHasHeavyArmor())
			new_damage = new_damage * 0.85000002f;

		if ((currentDamage - currentDamage * armor_ratio) * (modifier * armor_bonus_ratio) > e->m_ArmorValue())
			new_damage = currentDamage - e->m_ArmorValue() / armor_bonus_ratio;

		currentDamage = new_damage;
	}

	return;
}
void TraceLine(Vector& absStart, const Vector& absEnd, unsigned int mask, player_t* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	m_trace()->TraceRay(ray, mask, &filter, ptr);
}
bool autowall::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, const Vector& direction)
{
	static ConVar* sv_clip_penetration_traces_to_players = m_cvar()->FindVar(crypt_str("sv_clip_penetration_traces_to_players"));

	Vector          new_end, out;
	float           dist = 0.0f;
	int                iterations = 23;
	int                first_contents = 0;
	int             contents;
	Ray_t r{};

	while (1)
	{
		iterations--;

		if (iterations <= 0 || dist > 90.f)
			break;

		dist += 4.0f;
		out = startPosition + (direction * dist);

		contents = m_trace()->GetPointContents(out, 0x4600400B, nullptr);

		if (first_contents == -1)
			first_contents = contents;

		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
			continue;

		new_end = out - (direction * 4.f);

		TraceLine(out, new_end, 0x4600400B, nullptr, &exitTrace);

		if (exitTrace.startsolid && (exitTrace.surface.flags & SURF_HITBOX) != 0)
		{
			TraceLine(out, startPosition, MASK_SHOT_HULL, (player_t*)exitTrace.hit_entity, &exitTrace);

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{
				out = exitTrace.endpos;
				return true;
			}
			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid)
		{
			if (enterTrace.hit_entity != m_entitylist()->GetClientEntity(0))
			{
				if (exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
				{
					exitTrace.surface.surfaceProps = enterTrace.surface.surfaceProps;
					exitTrace.endpos = startPosition + direction;
					return true;
				}
			}

			continue;
		}

		if ((exitTrace.surface.flags & 0x80u) != 0)
		{
			if (enterTrace.hit_entity && is_breakable_entity(enterTrace.hit_entity) && exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
			{
				out = exitTrace.endpos;
				return true;
			}

			if (!(enterTrace.surface.flags & 0x80u))
				continue;
		}

		if (exitTrace.plane.normal.Dot(direction) <= 1.f)
		{
			out -= direction * (exitTrace.fraction * 4.0f);
			return true;
		}
	}

	return false;
}

bool autowall::handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	if (weaponData->flPenetration <= 0.0f)
		return false;

	if (possibleHitsRemaining <= 0)
		return false;

	auto contents_grate = enterTrace.contents & CONTENTS_GRATE;
	auto surf_nodraw = enterTrace.surface.flags & SURF_NODRAW;

	auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	auto enter_material = enterSurfaceData->game.material;

	auto is_solid_surf = enterTrace.contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enterTrace.surface.flags >> 7 & SURF_LIGHT;

	trace_t exit_trace;

	if (!trace_to_exit(enterTrace, exit_trace, enterTrace.endpos, direction) && !(m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
		return false;

	auto enter_penetration_modifier = enterSurfaceData->game.flPenetrationModifier;
	auto exit_surface_data = m_physsurface()->GetSurfaceData(exit_trace.surface.surfaceProps);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->game.material;
	auto exit_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_penetration_modifier = 3.0f;
		combined_damage_modifier = 0.05f;
	}
	else if (contents_grate || surf_nodraw)
		combined_penetration_modifier = 1.0f;
	else if (enter_material == CHAR_TEX_FLESH && ((player_t*)enterTrace.hit_entity)->m_iTeamNum() == csgo.local()->m_iTeamNum() && !ff_damage_reduction_bullets)
	{
		if (!ff_damage_bullet_penetration) //-V550
			return false;

		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.0f;
	}

	auto penetration_modifier = std::fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	auto penetration_distance = (exit_trace.endpos - enterTrace.endpos).Length();

	penetration_distance = penetration_distance * penetration_distance * penetration_modifier * 0.041666668f;

	auto damage_modifier = max(0.0f, 3.0f / weaponData->flPenetration * 1.25f) * penetration_modifier * 3.0f + currentDamage * combined_damage_modifier + penetration_distance;
	auto damage_lost = max(0.0f, damage_modifier);

	if (damage_lost > currentDamage)
		return false;

	currentDamage -= damage_lost;

	if (currentDamage < 1.0f)
		return false;

	eyePosition = exit_trace.endpos;
	--possibleHitsRemaining;

	return true;
}

float autowall::handle_bullet_penetration2(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	if (weaponData->flPenetration <= 0.0f)
		return false;

	if (possibleHitsRemaining <= 0)
		return false;

	auto contents_grate = enterTrace.contents & CONTENTS_GRATE;
	auto surf_nodraw = enterTrace.surface.flags & SURF_NODRAW;

	auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	auto enter_material = enterSurfaceData->game.material;

	auto is_solid_surf = enterTrace.contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enterTrace.surface.flags >> 7 & SURF_LIGHT;

	trace_t exit_trace;

	if (!trace_to_exit(enterTrace, exit_trace, enterTrace.endpos, direction) && !(m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
		return false;

	auto enter_penetration_modifier = enterSurfaceData->game.flPenetrationModifier;
	auto exit_surface_data = m_physsurface()->GetSurfaceData(exit_trace.surface.surfaceProps);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->game.material;
	auto exit_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_penetration_modifier = 3.0f;
		combined_damage_modifier = 0.05f;
	}
	else if (contents_grate || surf_nodraw)
		combined_penetration_modifier = 1.0f;
	else if (enter_material == CHAR_TEX_FLESH && ((player_t*)enterTrace.hit_entity)->m_iTeamNum() == csgo.local()->m_iTeamNum() && !ff_damage_reduction_bullets)
	{
		if (!ff_damage_bullet_penetration) //-V550
			return false;

		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.0f;
	}

	auto penetration_modifier = std::fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	auto penetration_distance = (exit_trace.endpos - enterTrace.endpos).Length();

	penetration_distance = penetration_distance * penetration_distance * penetration_modifier * 0.041666668f;

	auto damage_modifier = max(0.0f, 3.0f / weaponData->flPenetration * 1.25f) * penetration_modifier * 3.0f + currentDamage * combined_damage_modifier + penetration_distance;
	auto damage_lost = max(0.0f, damage_modifier);

	if (damage_lost > currentDamage)
		return false;

	currentDamage -= damage_lost;

	if (currentDamage < 1.0f)
		return false;

	eyePosition = exit_trace.endpos;
	--possibleHitsRemaining;

	return currentDamage - damage_lost;
}

bool autowall::fire_bullet(weapon_t* pWeapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e, float length, const Vector& pos)
{
	if (!pWeapon)
		return false;

	auto weaponData = pWeapon->get_csweapon_info();

	if (!weaponData)
		return false;

	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = csgo.local();
	currentDamage = weaponData->iDamage;

	auto eyePosition = pos;
	auto currentDistance = 0.0f;
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = 3000.0f;
	auto penetrationPower = weaponData->flPenetration;
	auto possibleHitsRemaining = 4;

	while (currentDamage >= 1.0f)
	{
		maxRange -= currentDistance;
		auto end = eyePosition + direction * maxRange;

		CTraceFilter filter;
		filter.pSkip = csgo.local();

		util::trace_line(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);
		util::clip_trace_to_players(e, eyePosition, end + direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);

		auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
		auto enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
		auto enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.0f)  //-V550
			break;

		currentDistance += enterTrace.fraction * maxRange;
		currentDamage *= pow(weaponData->flRangeModifier, currentDistance / 500.0f);

		if (currentDistance > penetrationDistance && weaponData->flPenetration || enterSurfPenetrationModifier < 0.1f)  //-V1051
			break;

		auto canDoDamage = enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC;
		auto isPlayer = ((player_t*)enterTrace.hit_entity)->is_player();
		auto isEnemy = ((player_t*)enterTrace.hit_entity)->m_iTeamNum() != csgo.local()->m_iTeamNum();

		if (canDoDamage && isPlayer && isEnemy)
		{
			scale_damage((player_t*)enterTrace.hit_entity, enterTrace, weaponData, currentDamage);
			hitbox = enterTrace.hitbox;
			return true;
		}

		if (!possibleHitsRemaining)
			break;

		static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
		static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat(), !e))
			break;

		visible = false;
	}

	return false;
}

autowall::returninfo_t autowall::wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e)
{
	csgo.globals.autowalling = true;
	auto tmp = point - eye_pos;

	auto angles = ZERO;
	math::vector_angles(tmp, angles);

	auto direction = ZERO;
	math::angle_vectors(angles, direction);

	direction.NormalizeInPlace();

	auto visible = true;
	auto damage = -1.0f;
	auto hitbox = -1;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (fire_bullet(weapon, direction, visible, damage, hitbox, e, 0.0f, eye_pos))
	{
		csgo.globals.autowalling = false;
		return returninfo_t(visible, (int)damage, hitbox); //-V2003
	}
	else
	{
		csgo.globals.autowalling = false;
		return returninfo_t(false, -1, -1);
	}
}