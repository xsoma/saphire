#include "..\..\includes.hpp"
#include "hitmarker.h"
#include "other_esp.h"

player_t* sget_entity(const int index) { return reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(index)); }

void hitmarker::listener( IGameEvent * game_event )
{
	if ( !vars.esp.hitmarker[1])
		return;

	const auto attacker = m_engine()->GetPlayerForUserID( game_event->GetInt( "attacker" ) );

	const auto victim = m_engine()->GetPlayerForUserID( game_event->GetInt( "userid" ) );

	if ( attacker != m_engine()->GetLocalPlayer() )
		return;

	if ( victim == m_engine()->GetLocalPlayer() )
		return;

	const auto player = sget_entity( victim );
	if ( !player || !(player->m_iTeamNum() != csgo.local()->m_iTeamNum()) )
		return;

}
int bones(int event_bone)
{
	switch (event_bone)
	{
	case 1:
		return HITBOX_HEAD;
	case 2:
		return HITBOX_CHEST;
	case 3:
		return HITBOX_STOMACH;
	case 4:
		return HITBOX_LEFT_HAND;
	case 5:
		return HITBOX_RIGHT_HAND;
	case 6:
		return HITBOX_RIGHT_CALF;
	case 7:
		return HITBOX_LEFT_CALF;
	default:
		return HITBOX_PELVIS;
	}
}
void hitmarker::DrawDamageIndicator(ImDrawList* draw)
{
	float CurrentTime = m_globals()->m_curtime;

	for (int i = 0; i < d_indicators.size(); i++) {

		if (d_indicators[i].erase_time < CurrentTime) {
			d_indicators.erase(d_indicators.begin() + i);
			continue;
		}

		if (d_indicators[i].erase_time - 1.f < CurrentTime) {
			d_indicators[i].col._CColor[3] = math::lerp(d_indicators[i].col.a(), 0, 0.1f);
		}

		if (!d_indicators[i].initialized) {
			d_indicators[i].position = d_indicators[i].player->hitbox_position(bones(d_indicators[i].hit_box));

			if (!d_indicators[i].player->is_alive()) {
				d_indicators[i].position.z -= 39.f; //лично у меня из-за того что игрок умирает, весь его хитбокс прыгает вверх на 39.f

				d_indicators[i].position.z += 24;
			}
			else {
				d_indicators[i].position.z += 12;
			}

			d_indicators[i].end_position = d_indicators[i].position + Vector(math::random_float(-24, 24), math::random_float(-84, 84), 80);

			d_indicators[i].initialized = true;
		}

		if (d_indicators[i].initialized) {
			d_indicators[i].position.z = math::lerp(d_indicators[i].position.z, d_indicators[i].end_position.z, 9 / 1.5f * m_globals()->m_frametime);
			d_indicators[i].position.x = math::lerp(d_indicators[i].position.x, d_indicators[i].end_position.x, 9 / 1.5f * m_globals()->m_frametime);
			d_indicators[i].position.y = math::lerp(d_indicators[i].position.y, d_indicators[i].end_position.y, 9 / 1.5f * m_globals()->m_frametime);

			d_indicators[i].last_update = CurrentTime;
		}

		Vector ScreenPosition;
		std::string damage = "-";
		damage += std::to_string(d_indicators[i].damage);
		if (math::world_to_screen(d_indicators[i].position, ScreenPosition)) {
			draw->AddText(ImVec2(ScreenPosition.x, ScreenPosition.y), ImColor(d_indicators[i].col.r(), d_indicators[i].col.g(), d_indicators[i].col.b(), d_indicators[i].col.a()), damage.c_str());
			//render::get().text(fonts[DAMAGE_MARKER], ScreenPosition.x, ScreenPosition.y, d_indicators[i].col, true, damage.c_str());
		}
	}
}
void hitmarker::draw_hits(ImDrawList* draw)
{
	DrawDamageIndicator(draw);
	for ( auto i = 0; i < hits.size(); i++ )
	{
		auto& hit = hits[ i ];

		if ( hit.time + 2.1f < m_globals()->m_curtime )
		{
			hits.erase( hits.begin() + i );
			i--;
		}

		Vector screen_pos;

		if ( math::world_to_screen( hit.pos, screen_pos ) )
		{
			render_hitmarker( hit, screen_pos , draw);
		}
	}
}

void hitmarker::add_hit( const hitmarker_t hit )
{
	hits.push_back( hit );
}


#include "../../hooks/Render.h"	
void hitmarker::render_hitmarker( hitmarker_t& hit, const Vector& screen_pos, ImDrawList* draw)
{
	static auto line_size = 6;

	const auto step = 255.f / 1.0f * m_globals()->m_frametime;
	const auto step_move = 30.f / 1.5f * m_globals()->m_frametime;
	const auto multiplicator = 0.3f;

	hit.moved -= step_move;
	
	if ( hit.time + 1.8f <= m_globals()->m_curtime )
		hit.alpha -= step;

	const auto int_alpha = static_cast< int >( hit.alpha );

	if ( int_alpha > 0 )
	{
		auto col = Color( 255, 255, 255, int_alpha );
		const int size = 4;
		const int gap = 3;
		const float thickness = 1.f;
		const int padding = 2;
		const float expire_time = 0.75f;
		g_Render->DrawLine(screen_pos.x - padding, screen_pos.y - padding, screen_pos.x - padding - size, screen_pos.y - padding - size, Color(255, 255, 255, int_alpha), thickness);
		g_Render->DrawLine(screen_pos.x + padding, screen_pos.y + padding, screen_pos.x + padding + size, screen_pos.y + padding + size, Color(255, 255, 255, int_alpha), thickness);
		g_Render->DrawLine(screen_pos.x + padding, screen_pos.y - padding, screen_pos.x + padding + size, screen_pos.y - padding - size, Color(255, 255, 255, int_alpha), thickness);
		g_Render->DrawLine(screen_pos.x - padding, screen_pos.y + padding, screen_pos.x - padding - size, screen_pos.y + padding + size, Color(255, 255, 255, int_alpha), thickness);
		
	}
}

void hitmarker::render_damage(hitmarker_t& hit, const Vector& screen_pos, Color col)
{
	
}
