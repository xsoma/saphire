#pragma once
#include "../../includes.hpp"

struct ReturnInfo_t
{
	int m_damage;
	int m_hitgroup;
	int m_hitbox;
	int m_penetration_count;
	bool m_did_penetrate_wall;
	float m_thickness;
	Vector m_end;
	player_t* m_hit_entity;

	ReturnInfo_t(int damage, int hitgroup, int m_hitbox, int penetration_count, bool did_penetrate_wall, float thickness, player_t* hit_entity)
	{
		m_damage = damage;
		m_hitgroup = hitgroup;
		m_penetration_count = penetration_count;
		m_did_penetrate_wall = did_penetrate_wall;
		m_thickness = thickness;
		m_hit_entity = hit_entity;
	}
};

class CAutoWall : public singleton<CAutoWall>
{
private:
	struct FireBulletData_t
	{
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		CTraceFilter* m_filter;
		CGameTrace m_enter_trace;

		float m_current_damage;
		int m_penetration_count;
	};

	void ScaleDamage(player_t* e, weapon_info_t* weapon_info, int hitgroup, float& current_damage);
	bool VectortoVectorVisible(Vector src, Vector point);
	bool HandleBulletPenetration(weapon_info_t* info, FireBulletData_t& data, bool extracheck = false, Vector point = Vector(0, 0, 0));
	bool BreakableEntity(player_t* entity);
	bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction);
	void TraceLine(Vector& start, Vector& end, unsigned int mask, player_t* ignore, CGameTrace* trace);
	void ClipTrace(Vector& start, Vector& end, player_t* e, unsigned int mask, CTraceFilter* filter, CGameTrace* old_trace);

	float HitgroupDamage(int iHitGroup);

	bool IsArmored(player_t* player, int hitgroup);

public:
	std::vector<float> scanned_damage;
	std::vector<Vector> scanned_points;
	void reset() {
		scanned_damage.clear();
		scanned_points.clear();
	}
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);
	ReturnInfo_t Think(Vector pos, player_t* target, int specific_hitgroup = -1, Vector eye_pos = csgo.globals.eye_pos, player_t* start_ent = csgo.local());
};