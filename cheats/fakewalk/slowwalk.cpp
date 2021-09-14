// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "slowwalk.h"
void slowwalk::autostop(CUserCmd* m_pcmd, float custom_speed)
{
	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto weapon_info = csgo.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	auto modifier = custom_speed == -1.0f ? 0.12f : custom_speed; //-V550
	auto max_speed = modifier * (csgo.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);

	if (!csgo.globals.weapon->is_non_aim())
	{
		auto move_length = sqrt(m_pcmd->m_sidemove * m_pcmd->m_sidemove + m_pcmd->m_forwardmove * m_pcmd->m_forwardmove);

		auto forwardmove = m_pcmd->m_forwardmove / move_length;
		auto sidemove = m_pcmd->m_sidemove / move_length;

		if (move_length > max_speed)
		{
			if (max_speed + 1.0f < csgo.local()->m_vecVelocity().Length2D())
			{
				m_pcmd->m_forwardmove = 0.0f;
				m_pcmd->m_sidemove = 0.0f;
			}
			else
			{
				m_pcmd->m_sidemove = max_speed * sidemove;
				m_pcmd->m_forwardmove = max_speed * forwardmove;
			}
		}
	}
	else
	{
		auto forwardmove = m_pcmd->m_forwardmove;
		auto sidemove = m_pcmd->m_sidemove;

		auto move_length = sqrt(sidemove * sidemove + forwardmove * forwardmove);
		auto move_length_backup = move_length;

		if (move_length > 110.0f)
		{
			m_pcmd->m_forwardmove = forwardmove / move_length_backup * 110.0f;
			move_length = sidemove / move_length_backup * 110.0f;
			m_pcmd->m_sidemove = sidemove / move_length_backup * 110.0f;
		}
	}
}
void slowwalk::create_move(CUserCmd* m_pcmd, float custom_speed)
{
	if (!(csgo.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto weapon_info = csgo.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return;

	if (custom_speed == -1.0f) //-V550
		csgo.globals.slowwalking = true;

	auto modifier = custom_speed == -1.0f ? 0.3f : custom_speed; //-V550
	auto max_speed = modifier * (csgo.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);

	if (!csgo.globals.weapon->is_non_aim())
	{
		auto move_length = sqrt(m_pcmd->m_sidemove * m_pcmd->m_sidemove + m_pcmd->m_forwardmove * m_pcmd->m_forwardmove);

		auto forwardmove = m_pcmd->m_forwardmove / move_length;
		auto sidemove = m_pcmd->m_sidemove / move_length;

		if (move_length > max_speed)
		{
			if (max_speed + 1.0f < csgo.local()->m_vecVelocity().Length2D())
			{
				m_pcmd->m_forwardmove = 0.0f;
				m_pcmd->m_sidemove = 0.0f;
			}
			else
			{
				m_pcmd->m_sidemove = max_speed * sidemove;
				m_pcmd->m_forwardmove = max_speed * forwardmove;
			}
		}
	}
	else
	{
		auto forwardmove = m_pcmd->m_forwardmove;
		auto sidemove = m_pcmd->m_sidemove;

		auto move_length = sqrt(sidemove * sidemove + forwardmove * forwardmove);
		auto move_length_backup = move_length;

		if (move_length > 110.0f)
		{
			m_pcmd->m_forwardmove = forwardmove / move_length_backup * 110.0f;
			move_length = sidemove / move_length_backup * 110.0f;
			m_pcmd->m_sidemove = sidemove / move_length_backup * 110.0f;
		}
	}
}