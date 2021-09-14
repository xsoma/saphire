// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "../../hooks/Render.h"
#include "..\menu.h"
#include "world_esp.h"
#include "grenade_warning.h"

void worldesp::paint_traverse()
{
	skybox_changer();
	for (int i = 1; i <= m_entitylist()->GetHighestEntityIndex(); i++)  //-V807
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_player())
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CEnvTonemapController:
			world_modulation(e);
			break;
		case CInferno:
			molotov_timer(e, i);
			break;
		case CSmokeGrenadeProjectile:
			smoke_timer(e);
			break;
		case CPlantedC4:
			bomb_timer(e);
			break;
		case CC4:
			if (vars.player.type[ENEMY].flags[FLAGS_C4] || vars.player.type[TEAM].flags[FLAGS_C4] || vars.player.type[LOCAL].flags[FLAGS_C4] || vars.esp.bomb_timer)
			{
				auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(e->m_hOwnerEntity());

				if ((vars.player.type[ENEMY].flags[FLAGS_C4] || vars.player.type[TEAM].flags[FLAGS_C4] || vars.player.type[LOCAL].flags[FLAGS_C4]) && owner->valid(false, false))
					csgo.globals.bomb_carrier = owner->EntIndex();
				else if (vars.esp.bomb_timer && !owner->is_player())
				{
					auto screen = ZERO;

					if (math::world_to_screen(e->GetAbsOrigin(), screen))
						g_Render->DrawString(screen.x, screen.y, Color(215, 20, 20), HFONT_CENTERED_X | HFONT_CENTERED_Y | render2::outline, c_menu::get().font, "BOMB");
				}
			}

			break;
		default:
			grenade_projectiles(e);

			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor) //-V648
				dropped_weapons(e);

			break;
		}
	}
}

void worldesp::skybox_changer()
{
	static auto load_skybox = reinterpret_cast<void(__fastcall*)(const char*)>(util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45")));
	auto skybox_name = backup_skybox;

	switch (vars.esp.skybox)
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	case 21:
		skybox_name = vars.esp.custom_skybox;
		break;
	}

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;

	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;

	if (skybox_number != vars.esp.skybox)
	{
		changed = true;
		skybox_number = vars.esp.skybox;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != vars.esp.skybox_color[0])
	{
		changed = true;
		color_r = vars.esp.skybox_color[0];
	}
	else if (color_g != vars.esp.skybox_color[1])
	{
		changed = true;
		color_g = vars.esp.skybox_color[1];
	}
	else if (color_b != vars.esp.skybox_color[2])
	{
		changed = true;
		color_b = vars.esp.skybox_color[2];
	}

	if (changed)
	{
		changed = false;
		load_skybox(skybox_name.c_str());

		auto materialsystem = m_materialsystem();

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), crypt_str("SkyBox")))
				material->ColorModulate(vars.esp.skybox_color[0] / 255.0f, vars.esp.skybox_color[1] / 255.0f, vars.esp.skybox_color[2] / 255.0f);
		}
	}
}

void worldesp::fog_changer()
{
	static auto fog_override = m_cvar()->FindVar(crypt_str("fog_override")); //-V807

	if (!vars.esp.fog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(FALSE);

		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(TRUE);

	static auto fog_start = m_cvar()->FindVar(crypt_str("fog_start"));

	if (fog_start->GetInt())
		fog_start->SetValue(0);

	static auto fog_end = m_cvar()->FindVar(crypt_str("fog_end"));

	if (fog_end->GetInt() != vars.esp.fog_distance)
		fog_end->SetValue(vars.esp.fog_distance);

	static auto fog_maxdensity = m_cvar()->FindVar(crypt_str("fog_maxdensity"));

	if (fog_maxdensity->GetFloat() != (float)vars.esp.fog_density * 0.01f) //-V550
		fog_maxdensity->SetValue((float)vars.esp.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", vars.esp.fog_color.r(), vars.esp.fog_color.g(), vars.esp.fog_color.b());

	static auto fog_color = m_cvar()->FindVar(crypt_str("fog_color"));

	if (strcmp(fog_color->GetString(), buffer_color)) //-V526
		fog_color->SetValue(buffer_color);
}

void worldesp::world_modulation(entity_t* entity)
{
	if (!vars.esp.world_modulation)
		return;

	entity->set_m_bUseCustomBloomScale(TRUE);
	entity->set_m_flCustomBloomScale(vars.esp.bloom * 0.01f);

	entity->set_m_bUseCustomAutoExposureMin(TRUE);
	entity->set_m_flCustomAutoExposureMin(vars.esp.exposure * 0.001f);

	entity->set_m_bUseCustomAutoExposureMax(TRUE);
	entity->set_m_flCustomAutoExposureMax(vars.esp.exposure * 0.001f);
}

void convex_hull(std::vector<ImVec2>& points)
{
	if (points.size() <= 3)
		return;

	std::sort(points.begin(), points.end(), [](ImVec2 left, ImVec2 right)
		{
			return left.x < right.x || left.x == right.x && left.y < right.y;
		}
	);

	auto orientation = [&](ImVec2 a, ImVec2 b, ImVec2 c) -> float {
		return a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);
	};

	ImVec2
		front = points.front(),
		back = points.back();

	std::vector<ImVec2> up, down;

	up.push_back(front);
	down.push_back(back);

	for (size_t i = 1; i < points.size(); ++i)
	{
		if (i == points.size() - 1 || orientation(front, points[i], back) > 0)
		{
			while (up.size() >= 2 && orientation(up[up.size() - 2], up[up.size() - 1], points[i]) < 0)
			{
				up.pop_back();
			}
			up.push_back(points[i]);
		}

		if (i == points.size() - 1 || orientation(front, points[i], back) < 0)
		{
			while (down.size() >= 2 && orientation(down[down.size() - 2], down[down.size() - 1], points[i]) > 0)
			{
				down.pop_back();
			}
			down.push_back(points[i]);
		}
	}

	points.clear();

	for (auto& u : up)
	{
		points.emplace_back(u);
	}

	for (size_t i = down.size() - 2; i > 0; --i)
	{
		points.emplace_back(down[i]);
	}
};
void worldesp::molotov_timer(entity_t* entity, int id)
{
	if (!vars.esp.molotov_timer)
		return;

	auto inferno = reinterpret_cast<inferno_t*>(entity);
	auto origin = inferno->GetAbsOrigin();
	auto col = vars.esp.molotov_timer_color;
	Vector screen_origin;
	auto spawn_time = inferno->get_spawn_time();
	auto factor = (spawn_time + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();

	auto dpos = origin;
	Vector mins, maxs;
	inferno->GetClientRenderable()->GetRenderBounds(mins, maxs);
	
	int* m_fireXDelta = reinterpret_cast<int*>((DWORD)inferno + 0x9E4);
	int* m_fireYDelta = reinterpret_cast<int*>((DWORD)inferno + 0xB74);
	int* m_fireZDelta = reinterpret_cast<int*>((DWORD)inferno + 0xD04);


	static const auto flame_polygon = [] {
		std::array<Vector, 3> points;
		for (std::size_t i = 0; i < points.size(); ++i) {
			points[i] = Vector{ 60.0f * std::cos(DEG2RAD(i * (360.0f / points.size()))),
								60.0f * std::sin(DEG2RAD(i * (360.0f / points.size()))),
								0.0f };
		}
		return points;
	}();

	std::vector<Vector> points;

	for (int i = 0; i <= inferno->m_fireCount(); i++)
		points.push_back(entity->m_vecOrigin() + Vector(m_fireXDelta[i], m_fireYDelta[i], m_fireZDelta[i]));

	std::vector<ImVec2> screen_points;

	for (const auto& pos : points) {
		for (const auto& point : flame_polygon) {
			Vector screen;

			if (math::world_to_screen(pos + point, screen))
				screen_points.push_back(ImVec2(screen.x, screen.y));
		}
	}
	std::vector<ImVec2> hull_points; convex_hull(screen_points);
	hull_points = screen_points;
	if (!hull_points.empty())
		g_Render->PolyLine(hull_points.data(), hull_points.size(), Color(col.r(), col.g(), col.b(), 255), true, 2.f, Color(col.r(), col.g(), col.b(), 35));
	else {
		auto ûsize = Vector(maxs - mins).Length2D() * 0.5;
        g_Render->DrawRing3D(dpos.x, dpos.y, dpos.z, ûsize, 360, Color(col.r(), col.g(), col.b(), 255), Color(col.r(), col.g(), col.b(), 35), 2, factor);
	}



	if (!math::world_to_screen(origin, screen_origin))
		return;

	static auto size = Vector2D(35.0f, 5.0f);
	g_Render->CircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 21, Color(25, 25, 25, col.a()), 60);
	g_Render->two_sided_arc(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 20, 1.f - factor, Color(col.r(), col.g(), col.b()), 3);
	g_Render->DrawString(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, Color(col.r(), col.g(), col.b()), render2::centered_x | render2::centered_y | render2::outline, c_menu::get().timersz, "M");
	

}

void worldesp::smoke_timer(entity_t* entity)
{
	if (!vars.esp.smoke_timer)
		return;

	auto smoke = reinterpret_cast<smoke_t*>(entity);

	if (!smoke->m_nSmokeEffectTickBegin() || !smoke->m_bDidSmokeEffect())
		return;
	auto spawn_time = TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin());
	auto factor = (spawn_time + smoke_t::get_expiry_time() - m_globals()->m_curtime) / smoke_t::get_expiry_time();
	auto origin = smoke->GetAbsOrigin();

	auto col = vars.esp.smoke_timer_color;
	g_Render->DrawRing3D(origin.x, origin.y, origin.z, 150, 360, Color(col.r(), col.g(), col.b(), 255), Color(col.r(), col.g(), col.b(), 35), 2, factor, true);
	Vector screen_origin;
	if (!math::world_to_screen(origin, screen_origin))
		return;
	
	static auto size = Vector2D(35.0f, 5.0f);
	g_Render->CircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 19, Color(25, 25, 25, col.a()), 60);
	g_Render->two_sided_arc(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 18, 1.f - factor, Color(col.r(), col.g(), col.b()), 2);
	g_Render->DrawString(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, Color(col.r(), col.g(), col.b()), render2::centered_x | render2::centered_y | render2::outline, c_menu::get().timersz, "S");
}

void worldesp::grenade_projectiles(entity_t* entity)
{
	if (!vars.esp.grenade_proximity_warning)
		return;

	c_grenade_prediction::get().grenade_warning((projectile_t*)entity);

}

void worldesp::bomb_timer(entity_t* entity)
{
	if (vars.esp.bomb_timer)
		return;

	ImGui::Begin("BOmb timer", NULL);

	ImGui::Text("asdasdadadad");
	if (!csgo.globals.bomb_timer_enable)
		return;

	static auto mp_c4timer = m_cvar()->FindVar(crypt_str("mp_c4timer"));
	auto bomb = (CCSBomb*)entity;

	auto c4timer = mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - m_globals()->m_curtime;

	ImGui::End();
}

void worldesp::dropped_weapons(entity_t* entity)
{
	auto weapon = (weapon_t*)entity; //-V1027
	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(weapon->m_hOwnerEntity());

	if (owner->is_player())
	    return;

	Box box;

	if (util::get_bbox(weapon, box, false))
	{
	    auto offset = 0;

	    if (vars.esp.weapon[WEAPON_BOX])
	    {
			g_Render->Rect(box.x, box.y, box.w, box.h, vars.esp.box_color);

	        if (vars.esp.weapon[WEAPON_ICON])
	        {
	            g_Render->DrawString(box.x + box.w / 2, box.y - 14, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().weapon_icons, weapon->get_icon());
	            offset = 14;
	        }

	        if (vars.esp.weapon[WEAPON_TEXT])
	            g_Render->DrawString( box.x + box.w / 2, box.y + box.h + 2, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().esp_name, weapon->get_name().c_str());

	        if (vars.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
	        {
	            auto inner_back_color = Color::Black;
	            inner_back_color.SetAlpha(153);

				g_Render->FilledRect(box.x - 1, box.y + box.h + 14, box.w + 2, 4, inner_back_color);
				g_Render->FilledRect(box.x, box.y + box.h + 15, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, vars.esp.weapon_ammo_color);
	        }

	        if (vars.esp.weapon[WEAPON_DISTANCE])
	        {
	            auto distance = csgo.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
	            g_Render->DrawString( box.x + box.w / 2, box.y - 13 - offset, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().font, "%i m", (int)distance / 3.14);
	        }
	    }
	    else
	    {
	        if (vars.esp.weapon[WEAPON_ICON])
	            g_Render->DrawString(box.x + box.w / 2, box.y + box.h / 2 - 7, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().weapon_icons, weapon->get_icon());

	        if (vars.esp.weapon[WEAPON_TEXT])
	            g_Render->DrawString( box.x + box.w / 2, box.y + box.h / 2 + 6, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().esp_name, weapon->get_name().c_str());

	        if (vars.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
	        {
	            static auto pos = 0;

	            if (vars.esp.weapon[WEAPON_ICON] && vars.esp.weapon[WEAPON_TEXT])
	                pos = 19;
	            else if (vars.esp.weapon[WEAPON_ICON])
	                pos = 8;
	            else if (vars.esp.weapon[WEAPON_TEXT])
	                pos = 19;

	            auto inner_back_color = Color::Black;
	            inner_back_color.SetAlpha(153);

				g_Render->FilledRect(box.x - 1, box.y + box.h / 2 + pos - 1, box.w + 2, 4, inner_back_color);
				g_Render->FilledRect(box.x, box.y + box.h / 2 + pos, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, vars.esp.weapon_ammo_color);
	        }

	        if (vars.esp.weapon[WEAPON_DISTANCE])
	        {
	            auto distance = csgo.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

	            if (vars.esp.weapon[WEAPON_ICON] && vars.esp.weapon[WEAPON_TEXT])
	                offset = 21;
	            else if (vars.esp.weapon[WEAPON_ICON])
	                offset = 21;
	            else if (vars.esp.weapon[WEAPON_TEXT])
	                offset = 8;

	            g_Render->DrawString( box.x + box.w / 2, box.y + box.h / 2 - offset, vars.esp.weapon_color, HFONT_CENTERED_X | render2::outline, c_menu::get().font, "%i m", (int)distance / 3.14);
	        }
	    }
	}
}