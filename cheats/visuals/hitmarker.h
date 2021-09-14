#pragma once
#include "..\..\includes.hpp"

struct hitmarker_t
{
	hitmarker_t( const float& time, const int& index, const int& damage, const int& hitgroup, const Vector& pos )
	{
		this->time = time;
		this->index = index;
		this->damage = damage;
		this->hitgroup = hitgroup;
		this->pos = pos;
		moved = 0.f;
		alpha = 255.f;
	}
	float time;
	int index;
	int damage;
	int hitgroup;
	float moved;
	float alpha;
	Color col;
	Vector pos;
};

class hitmarker : public singleton<hitmarker>
{
public:
	void DrawDamageIndicator(ImDrawList* draw);
	void listener( IGameEvent * game_event );
	void draw_hits(ImDrawList* draw);
	void render_damage(hitmarker_t& hit, const Vector& screen_pos, Color col);
	void add_hit( hitmarker_t hit );
	static void render_hitmarker( hitmarker_t& hit, const Vector & screen_pos, ImDrawList* draw);
	std::deque<hitmarker_t> hits;

	struct d_indicator_t {
		int damage;
		bool initialized;
		float erase_time;
		float last_update;
		player_t* player;
		int hit_box;
		Color col;
		Vector position;
		Vector end_position;
	};

	std::vector<d_indicator_t> d_indicators;
private:
	//std::deque<shot_t> shots;
};