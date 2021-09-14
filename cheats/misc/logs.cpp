// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "logs.h"

void eventlogs::paint_traverse()
{

}
#include "misc.h"
#include "../ragebot/aim.h"
#include "../visuals/logger.hpp"
std::map<std::string, std::string> event_to_normal =
{
	//others
	////////////////////////////////////////////////
	{ "weapon_taser", "Zeus" },
	{ "item_kevlar", "Kevlar" },
	{ "item_defuser", "Defuse kit" },
	{ "item_assaultsuit", "Kevlar + Helmet" },
	////////////////////////////////////////////////
	//


	//pistols
	////////////////////////////////////////////////
	{ "weapon_p250", "P250" },
	{ "weapon_tec9", "TEC-9" },
	{ "weapon_cz75a", "CZ75A" },
	{ "weapon_glock", "Glock" },
	{ "weapon_elite", "Double-Berretas" },
	{ "weapon_deagle", "Desert-Eagle" },
	{ "weapon_hkp2000", "P2000" },
	{ "weapon_usp_silencer", "USP-S" },
	{ "weapon_revolver", "R8 Revolver" },
	{ "weapon_fiveseven", "Five-Seven" },
	////////////////////////////////////////////////
	//

	//pp
	////////////////////////////////////////////////
	{ "weapon_mp9", "MP-9" },
	{ "weapon_mac10", "MAC-10" },
	{ "weapon_mp7", "MP-7" },
	{ "weapon_mp5sd", "MP5-SD" },
	{ "weapon_ump45", "UMP-45" },
	{ "weapon_p90", "P90" },
	{ "weapon_bizon", "PP-Bizon" },
	////////////////////////////////////////////////
	//

	//rifles
	////////////////////////////////////////////////
	{ "weapon_famas", "FAMAS" },
	{ "weapon_m4a1_silencer", "M4A1-s" },
	{ "weapon_m4a1", "M4A1" },
	{ "weapon_ssg08", "SSG08 / Scout" },
	{ "weapon_aug", "AUG" },
	{ "weapon_awp", "AWP" },
	{ "weapon_scar20", "SCAR20" },
	{ "weapon_galilar", "AR-Galil" },
	{ "weapon_ak47", "AK-47" },
	{ "weapon_sg556", "SG553" },
	{ "weapon_g3sg1", "G3SG1" },
	////////////////////////////////////////////////
	//

	//have
	////////////////////////////////////////////////
	{ "weapon_nova", "Nova" },
	{ "weapon_xm1014", "XM1014" },
	{ "weapon_sawedoff", "Sawed-Off" },
	{ "weapon_m249", "M249" },
	{ "weapon_negev", "Negev" },
	{ "weapon_mag7", "MAG-7" },
	////////////////////////////////////////////////
	//

	//granades
	////////////////////////////////////////////////
	{ "weapon_flashbang", "Flasg grenade" },
	{ "weapon_smokegrenade", "Smoke grenade" },
	{ "weapon_molotov", "Molotov" },
	{ "weapon_incgrenade", "Incereative grenade" },
	{ "weapon_decoy", "Decoy grenade" },
	{ "weapon_hegrenade", "HE Grenade" },
	////////////////////////////////////////////////
	//
};
std::string bomb_site(int site_id, std::string map_name)
{
	if (map_name == "de_inferno")
	{
		if (site_id == 333)
		{
			return "A";
		}
		else if (site_id == 422)
		{
			return "B";
		}
	}
	else if (map_name == "de_mirage")
	{
		if (site_id == 425)
		{
			return "A";
		}
		else if (site_id == 426)
		{
			return "B";
		}
	}
	else if (map_name == "de_dust2")
	{
		if (site_id == 281)
		{
			return "A";
		}
		else if (site_id == 282)
		{
			return "B";
		}
	}
	else if (map_name == "de_overpass")
	{
		if (site_id == 79)
		{
			return "A";
		}
		else if (site_id == 504)
		{
			return "B";
		}
	}
	else if (map_name == "de_vertigo")
	{
		if (site_id == 262)
		{
			return "A";
		}
		else if (site_id == 314)
		{
			return "B";
		}
	}
	else if (map_name == "de_nuke")
	{
		if (site_id == 154)
		{
			return "A";
		}
		else if (site_id == 167)
		{
			return "B";
		}
	}
	else if (map_name == "de_train")
	{
		if (site_id == 94)
		{
			return "A";
		}
		else if (site_id == 536)
		{
			return "B";
		}
	}
	else if (map_name == "de_cache")
	{
		if (site_id == 317)
		{
			return "A";
		}
		else if (site_id == 318)
		{
			return "B";
		}
	}
	else if (map_name == "de_cbble")
	{
		if (site_id == 216)
		{
			return "A";
		}
		else if (site_id == 107)
		{
			return "B";
		}
	}
	else if (map_name == "de_shortdust")
	{
		if (site_id == 217)
		{
			return "A";
		}
	}
	else if (map_name == "de_rialto")
	{
		if (site_id == 99)
		{
			return "A";
		}
	}
	else if (map_name == "de_lake")
	{
		if (site_id == 209)
		{
			return "A";
		}
	}

	return "unck";

}

void eventlogs::events(IGameEvent* event)
{
	int event_type = 0;
	static auto get_hitgroup_name = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return crypt_str("head");
		case HITGROUP_CHEST:
			return crypt_str("chest");
		case HITGROUP_STOMACH:
			return crypt_str("stomach");
		case HITGROUP_LEFTARM:
			return crypt_str("left arm");
		case HITGROUP_RIGHTARM:
			return crypt_str("right arm");
		case HITGROUP_LEFTLEG:
			return crypt_str("left leg");
		case HITGROUP_RIGHTLEG:
			return crypt_str("right leg");
		default:
			return crypt_str("generic");
		}
	};

	if (!strcmp(event->GetName(), crypt_str("player_hurt")))
	{
		auto userid = event->GetInt(crypt_str("userid")), attacker = event->GetInt(crypt_str("attacker"));

		if (!userid || !attacker)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker); //-V807

		player_info_t userid_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		if (!m_engine()->GetPlayerInfo(attacker_id, &attacker_info))
			return;

		auto m_victim = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		std::stringstream ss;

		if (attacker_id == m_engine()->GetLocalPlayer() && userid_id != m_engine()->GetLocalPlayer())
		{
			event_type = 0;

			ss << crypt_str("Hurt to ") << userid_info.szName << "'s " << get_hitgroup_name(event->GetInt(crypt_str("hitgroup"))) << " for " << event->GetInt(crypt_str("dmg_health")) << " damage";
			notify::add_log("Hurt", ss.str().c_str(), Color(130, 235, 25));
		}
		else if (userid_id == m_engine()->GetLocalPlayer() && attacker_id != m_engine()->GetLocalPlayer())
		{
			event_type = 2;
			ss << crypt_str("Hurm by ") << attacker_info.szName << " in " << get_hitgroup_name(event->GetInt(crypt_str("hitgroup"))) << " for " << event->GetInt(crypt_str("dmg_health")) << " damage";
			notify::add_log("Hurm", ss.str().c_str(), Color(235, 140, 5));
		}
	}

	if (vars.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] && !strcmp(event->GetName(), crypt_str("item_purchase")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!csgo.local() || !m_player)
			return;

		if (csgo.local() == m_player)
			csgo.globals.should_buy = 0;

		if (m_player->m_iTeamNum() == csgo.local()->m_iTeamNum())
			return;

		std::string weapon = event->GetString(crypt_str("weapon"));
		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" bought ") << event_to_normal[weapon.c_str()] << " remaining money: " << m_player->m_iAccount() << "$";
		notify::add_log("Buy", ss.str().c_str(), Color(0, 150, 240));
	}

	if (vars.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_beginplant")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;
		int site_id = event->GetInt("site");
		std::string namemap = m_engine()->GetLevelNameShort();
		std::string bomb_site_s = bomb_site(site_id, namemap);
		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" planting the bomb" << crypt_str(" at site ") << bomb_site_s);
		notify::add_log("Plant", ss.str().c_str(), Color(255, 66, 66));
	}

	if (vars.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_begindefuse")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;
		event_type = 5;
		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" defusing the bomb ") << (event->GetBool(crypt_str("haskit")) ? crypt_str("with defuse kit") : crypt_str("without defuse kit"));

		notify::add_log("Defusing", ss.str().c_str(), Color(255, 66, 66));
	}
}

void eventlogs::add(std::string text, bool full_display)
{

}