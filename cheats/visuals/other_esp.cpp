// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\local_animations.h"
#include "../../hooks/Render.h"
#include "..\menu.h"

bool can_penetrate(weapon_t* weapon)
{
	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	CTraceFilter filter;
	filter.pSkip = csgo.local();

	trace_t trace;
	util::trace_line(csgo.globals.eye_pos, csgo.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	if (trace.fraction == 1.0f) //-V550
		return false;

	auto eye_pos = csgo.globals.eye_pos;
	auto hits = 1;
	auto damage = (float)weapon_info->iDamage;
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

	return autowall::get().handle_bullet_penetration(weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat());
}

float penetrate_damage(weapon_t* weapon)
{
	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	CTraceFilter filter;
	filter.pSkip = csgo.local();

	trace_t trace;
	util::trace_line(csgo.globals.eye_pos, csgo.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	if (trace.fraction == 1.0f) //-V550
		return false;

	auto eye_pos = csgo.globals.eye_pos;
	auto hits = 1;
	auto damage = (float)weapon_info->iDamage;
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));
	return autowall::get().handle_bullet_penetration2(weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat());
	
}

void otheresp::penetration_reticle()
{
	if (!vars.player.enable)
		return;

	if (!vars.esp.penetration_reticle)
		return;

	if (!csgo.local()->is_alive())
		return;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto color = Color::Red;

	if (!weapon->is_non_aim() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && can_penetrate(weapon))
		color = Color::Green;

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	
	trace_t enterTrace;
	CTraceFilter filter;
	Ray_t ray;
	auto weapon_info = weapon->get_csweapon_info();
	if (!weapon_info)
		return;
	Vector viewangles; m_engine()->GetViewAngles(viewangles);
	Vector direction; math::angle_vectors(viewangles, direction);
	Vector start = csgo.globals.eye_pos;
	auto m_flMaxRange = weapon_info->flRange * 2; //
	Vector end = start + (direction * m_flMaxRange);

	filter.pSkip = csgo.local();
	ray.Init(start, end);
	m_trace()->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &enterTrace);

	float anglez = math::dot_product(Vector(0, 0, 1), enterTrace.plane.normal);
	float invanglez = math::dot_product(Vector(0, 0, -1), enterTrace.plane.normal);
	float angley = math::dot_product(Vector(0, 1, 0), enterTrace.plane.normal);
	float invangley = math::dot_product(Vector(0, -1, 0), enterTrace.plane.normal);
	float anglex = math::dot_product(Vector(1, 0, 0), enterTrace.plane.normal);
	float invanglex = math::dot_product(Vector(-1, 0, 0), enterTrace.plane.normal);

	if (anglez > 0.5 || invanglez > 0.5)
		g_Render->filled_rect_world(enterTrace.endpos, Vector2D(3, 3), Color(color.r(), color.g(), color.b(), 100), 0, int(penetrate_damage(weapon)));
	else if (angley > 0.5 || invangley > 0.5)
		g_Render->filled_rect_world(enterTrace.endpos, Vector2D(3, 3), Color(color.r(), color.g(), color.b(), 100), 1, int(penetrate_damage(weapon)));
	else if (anglex > 0.5 || invanglex > 0.5)
		g_Render->filled_rect_world(enterTrace.endpos, Vector2D(3, 3), Color(color.r(), color.g(), color.b(), 100), 2, int(penetrate_damage(weapon)));
	Vector pos2d;
	if (math::world_to_screen(enterTrace.endpos, pos2d) && penetrate_damage(weapon) > 0)
		g_Render->DrawString(pos2d.x, pos2d.y - 10, Color(255, 255, 255), 2 | 4, c_menu::get().font, std::to_string(int(penetrate_damage(weapon))).c_str());
	//g_Render->DrawString(10, 10, Color::White, 2 | 4, c_menu::get().isis, std::to_string(csgo.globals.new_dt.charge_ticks).c_str());
	//g_Render->DrawString(10, 40, Color::White, 2 | 4, c_menu::get().isis, std::to_string(csgo.globals.tickbase_shift).c_str());
	//g_Render->DrawString(10, 70, Color::White, 2 | 4, c_menu::get().isis, csgo.globals.new_dt.shifting ? "true" : "false");

}

void otheresp::indicators()
{
	if (!csgo.local()->is_alive()) //-V807
		return;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (vars.esp.indicators[INDICATOR_FAKE] && (antiaim::get().type == ANTIAIM_LEGIT || vars.antiaim.type[antiaim::get().type].desync))
	{
		auto color = Color(130, 20, 20);
		auto animstate = csgo.local()->get_animation_state();

		if (animstate && local_animations::get().local_data.animstate)
		{
			auto delta = fabs(math::normalize_yaw(animstate->m_flGoalFeetYaw - local_animations::get().local_data.animstate->m_flGoalFeetYaw));
			auto desync_delta = max(csgo.local()->get_max_desync_delta(), 58.0f);

			color = Color(130, 20 + (int)(min(delta / desync_delta, 1.0f) * 150.0f), 20);
		}

		m_indicators.push_back(m_indicator("FAKE", color));
	}

	if (vars.esp.indicators[INDICATOR_DESYNC_SIDE] && (antiaim::get().type == ANTIAIM_LEGIT && vars.antiaim.desync == 1 || antiaim::get().type != ANTIAIM_LEGIT && vars.antiaim.type[antiaim::get().type].desync == 1) && !antiaim::get().condition(csgo.get_command()))
	{
		auto side = antiaim::get().desync_angle > 0.0f ? "RIGHT" : "LEFT";

		if (antiaim::get().type == ANTIAIM_LEGIT)
			side = antiaim::get().desync_angle > 0.0f ? "LEFT" : "RIGHT";

		m_indicators.push_back(m_indicator(side, Color(130, 170, 20)));
	}

	auto choke_indicator = false;

	if (vars.esp.indicators[INDICATOR_CHOKE] && !fakelag::get().condition && !misc::get().double_tap_enabled && !misc::get().hide_shots_enabled)
	{
		m_indicators.push_back(m_indicator(("CHOKE: " + std::to_string(fakelag::get().max_choke)), Color(130, 170, 20)));
		choke_indicator = true;
	}

	if (vars.esp.indicators[INDICATOR_DAMAGE] && csgo.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4 + csgo.globals.current_weapon) && !weapon->is_non_aim())
	{
		if (vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage > 100)
			m_indicators.push_back(m_indicator(("DAMAGE: HP + " + std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage - 100)), Color(130, 170, 20)));
		else
			m_indicators.push_back(m_indicator(("DAMAGE: " + std::to_string(vars.ragebot.weapon[csgo.globals.current_weapon].minimum_override_damage)), Color(130, 170, 20)));
	}

	if (vars.esp.indicators[INDICATOR_SAFE_POINTS] && key_binds::get().get_key_bind_state(3) && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator("SAFE POINTS", Color(130, 170, 20)));

	if (vars.esp.indicators[INDICATOR_BODY_AIM] && key_binds::get().get_key_bind_state(22) && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator("BODY AIM", Color(130, 170, 20)));

	if (choke_indicator)
		return;

	if (vars.esp.indicators[INDICATOR_DT] && vars.ragebot.double_tap && vars.ragebot.double_tap_key.key > KEY_NONE && vars.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key)
		m_indicators.push_back(m_indicator(("DT [" + std::to_string(csgo.globals.tickbase_shift) + "]"), !csgo.local()->m_bGunGameImmunity() && !(csgo.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().double_tap_enabled && !weapon->is_grenade() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && weapon->can_fire(false) ? Color(130, 170, 20) : Color(130, 20, 20)));

	if (vars.esp.indicators[INDICATOR_HS] && vars.antiaim.hide_shots && vars.antiaim.hide_shots_key.key > KEY_NONE && vars.antiaim.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key)
		m_indicators.push_back(m_indicator("HS", !csgo.local()->m_bGunGameImmunity() && !(csgo.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().hide_shots_enabled ? Color(130, 170, 20) : Color(130, 20, 20)));
}

void otheresp::draw_indicators()
{
	if (!csgo.local()->is_alive()) //-V807
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto h = height - 325;

	for (auto& indicator : m_indicators)
	{
		//render::get().text(fonts[INDICATORFONT], 27, h, indicator.m_color, HFONT_CENTERED_Y, indicator.m_text.c_str());
		h -= 25;
	}

	m_indicators.clear();
}

void otheresp::hitmarker_paint() {}
void otheresp::damage_marker_paint()
{
	
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);

void otheresp::spread_crosshair(LPDIRECT3DDEVICE9 device)
{
	if (!vars.player.enable)
		return;

	if (!vars.esp.show_spread)
		return;

	if (!csgo.local()->is_alive())
		return;

	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	int w, h;
	m_engine()->GetScreenSize(w, h);

	draw_circe((float)w * 0.5f, (float)h * 0.5f, csgo.globals.inaccuracy * 500.0f, 50, D3DCOLOR_RGBA(vars.esp.show_spread_color.r(), vars.esp.show_spread_color.g(), vars.esp.show_spread_color.b(), vars.esp.show_spread_color.a()), D3DCOLOR_RGBA(0, 0, 0, 0), device);
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle(resolution + 2);

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0.0f;

	circle[0].rhw = 1.0f;
	circle[0].color = color2;

	for (auto i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - radius * sin(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0.0f;

		circle[i].rhw = 1.0f;
		circle[i].color = color;
	}

	device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX2), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr); //-V107

	if (!g_pVB2)
		return;

	void* pVertices;

	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX2), (void**)&pVertices, 0); //-V107
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX2));
	g_pVB2->Unlock();

	device->SetTexture(0, nullptr);
	device->SetPixelShader(nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX2));
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	g_pVB2->Release();
}
void draw_peek(int x, int y, int z)
{
	for (int i = 0; i < 15; i++)
	{
		g_Render->DrawRing3D(x, y, z, 0 + i * 2, 88, Color(0,0,0,0), Color(130, 132, 170, 150 / max(1, i)), 0);
	}
}
void otheresp::automatic_peek_indicator()
{
	auto weapon = csgo.local()->m_hActiveWeapon().Get();

	//sorry, but this code work only from here (c) INSANE
	for (int i = 1; i <= m_entitylist()->GetHighestEntityIndex(); i++) {
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

		if (client_class->m_ClassID != CSmokeGrenadeProjectile)
			continue;

	}

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!csgo.globals.start_position.IsZero())
		position = csgo.globals.start_position;

	if (position.IsZero())
		return;

	static auto alpha = 0.0f;

	if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18) || alpha)
	{
		if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18))
			alpha += 3.0f * m_globals()->m_frametime; //-V807
		else
			alpha -= 3.0f * m_globals()->m_frametime;

		alpha = math::clamp(alpha, 0.0f, 1.0f);
		float rad = max(2, 24 * alpha) - 1.f;
		for (int i = 1; i < max(2,24 * alpha); i++) {
			if (rad > 2.f)
				g_Render->DrawRing3D(position.x, position.y, position.z, round(rad), 180, Color(0,00,0,0), Color(vars.misc.automatic_peek_color.r(), vars.misc.automatic_peek_color.g(), vars.misc.automatic_peek_color.b(), int(i * alpha)), 0, 0.f);

			rad -= 1.f;
		}
	}
	
}