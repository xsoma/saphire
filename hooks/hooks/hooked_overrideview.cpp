// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\cheats\visuals\GrenadePrediction.h"
#include "..\..\cheats\misc\fakelag.h"
#include "..\..\cheats\lagcompensation\local_animations.h"

using OverrideView_t = void(__stdcall*)(CViewSetup*);

void thirdperson(bool fakeducking);

void __stdcall hooks::hooked_overrideview(CViewSetup* viewsetup)
{
	static auto original_fn = clientmode_hook->get_func_address <OverrideView_t> (18);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!viewsetup)
		return original_fn(viewsetup);

	if (csgo.local())
	{
		static auto fakeducking = false;

		if (!fakeducking && csgo.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !csgo.globals.fakeducking && (!csgo.local()->get_animation_state()->m_fDuckAmount || csgo.local()->get_animation_state()->m_fDuckAmount == 1.0f)) //-V550
			fakeducking = false;

		if (!csgo.local()->is_alive()) //-V807
			fakeducking = false;

		auto weapon = csgo.local()->m_hActiveWeapon().Get();

		if (weapon)
		{
			if (!csgo.local()->m_bIsScoped() && vars.player.enable)
				viewsetup->fov += vars.esp.fov;
			else if (vars.esp.removals[REMOVALS_ZOOM] && vars.player.enable)
			{
				if (weapon->m_zoomLevel() == 1)
					viewsetup->fov = 90.0f + (float)vars.esp.fov;
				else
					viewsetup->fov += (float)vars.esp.fov;
			}
		}
		else if (vars.player.enable)
			viewsetup->fov += vars.esp.fov;

		if (weapon)
		{
			auto viewmodel = (entity_t*)m_entitylist()->GetClientEntityFromHandle(csgo.local()->m_hViewModel());

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)vars.esp.viewmodel_roll;

				viewmodel->set_abs_angles(eyeAng);
			}

			if (weapon->is_grenade() && vars.esp.grenade_prediction && vars.player.enable)
				GrenadePrediction::get().View(viewsetup, weapon);
		}

		if (vars.player.enable && (vars.misc.thirdperson_toggle.key > KEY_NONE && vars.misc.thirdperson_toggle.key < KEY_MAX || vars.misc.thirdperson_when_spectating))
			thirdperson(fakeducking);
		else
		{
			csgo.globals.in_thirdperson = false;
			m_input()->m_fCameraInThirdPerson = false;
		}

		original_fn(viewsetup);

		if (fakeducking)
		{
			viewsetup->origin = csgo.local()->GetAbsOrigin() + Vector(0.0f, 0.0f, m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f);

			if (m_input()->m_fCameraInThirdPerson)
			{
				auto camera_angles = Vector(m_input()->m_vecCameraOffset.x, m_input()->m_vecCameraOffset.y, 0.0f); //-V807
				auto camera_forward = ZERO;

				math::angle_vectors(camera_angles, camera_forward);
				math::VectorMA(viewsetup->origin, -m_input()->m_vecCameraOffset.z, camera_forward, viewsetup->origin);
			}
		}
	}
	else
		return original_fn(viewsetup);
}

void thirdperson(bool fakeducking)
{
	static auto current_fraction = 0.0f;
	static auto in_thirdperson = false;

	if (!in_thirdperson && csgo.globals.in_thirdperson)
	{
		current_fraction = 0.0f;
		in_thirdperson = true;
	}
	else if (in_thirdperson && !csgo.globals.in_thirdperson)
		in_thirdperson = false;

	if (csgo.local()->is_alive() && in_thirdperson) //-V807
	{
		auto distance = (float)vars.misc.thirdperson_distance;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		Vector inverse_angles;
		m_engine()->GetViewAngles(inverse_angles);

		inverse_angles.z = distance;

		Vector forward, right, up;
		math::angle_vectors(inverse_angles, &forward, &right, &up);

		Ray_t ray;
		CTraceFilterWorldAndPropsOnly filter;
		trace_t trace;

		auto eye_pos = fakeducking ? csgo.local()->GetAbsOrigin() + m_gamemovement()->GetPlayerViewOffset(false) : csgo.local()->GetAbsOrigin() + csgo.local()->m_vecViewOffset();
		auto offset = eye_pos + forward * -distance + right + up;

		ray.Init(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f));
		m_trace()->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

		if (current_fraction > trace.fraction)
			current_fraction = trace.fraction;
		else if (current_fraction > 0.9999f)
			current_fraction = 1.0f;

		current_fraction = math::interpolate(current_fraction, trace.fraction, m_globals()->m_frametime * 10.0f);
		angles.z = distance * current_fraction;

		m_input()->m_fCameraInThirdPerson = current_fraction > 0.1f;
		m_input()->m_vecCameraOffset = angles;
	}
	else if (m_input()->m_fCameraInThirdPerson)
	{
		csgo.globals.in_thirdperson = false;
		m_input()->m_fCameraInThirdPerson = false;
	}

	static auto require_reset = false;

	if (csgo.local()->is_alive())
	{
		require_reset = false;
		return;
	}

	if (vars.misc.thirdperson_when_spectating)
	{
		if (require_reset)
			csgo.local()->m_iObserverMode() = OBS_MODE_CHASE;

		if (csgo.local()->m_iObserverMode() == OBS_MODE_IN_EYE)
			require_reset = true;
	}
}