#pragma once
#include "../misc/Color.hpp"
#include "../math/Vector.hpp"

enum dlight_flags {
	dlight_no_world_illumination = 0x1,
	dlight_no_model_illumination = 0x2,
	dlight_add_displacement_alpha = 0x4,
	dlight_subtract_displacement_alpha = 0x8,
	dlight_displacement_mask = (dlight_add_displacement_alpha | dlight_subtract_displacement_alpha),
};

struct dlight_t {
	int flags;
	Vector origin;
	float radius;
	Color color;
	float die_time;
	float decay;
	float min_light;
	int	key;
	int	style;
	Vector direction;
	float inner_angle;
	float outer_angle;
};


class iv_effects
{
public:
	dlight_t* cl_alloc_dlight(int key) {
		using original_fn = dlight_t * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[4](this, key);
	}
	dlight_t* cl_alloc_elight(int key) {
		using original_fn = dlight_t * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[5](this, key);
	}
	dlight_t* get_elight_by_key(int key) {
		using original_fn = dlight_t * (__thiscall*)(void*, int);
		return (*(original_fn**)this)[8](this, key);
	}
}; 

class IEffects
{
public:
	virtual ~IEffects() {};

	virtual void Beam(const Vector& Start, const Vector& End, int nModelIndex,
		int nHaloIndex, unsigned char frameStart, unsigned char frameRate,
		float flLife, unsigned char width, unsigned char endWidth, unsigned char fadeLength,
		unsigned char noise, unsigned char red, unsigned char green,
		unsigned char blue, unsigned char brightness, unsigned char speed) = 0;

	//-----------------------------------------------------------------------------
	// Purpose: Emits smoke sprites.
	// Input  : origin - Where to emit the sprites.
	//			scale - Sprite scale * 10.
	//			framerate - Framerate at which to animate the smoke sprites.
	//-----------------------------------------------------------------------------
	virtual void Smoke(const Vector& origin, int modelIndex, float scale, float framerate) = 0;

	virtual void Sparks(const Vector& position, int nMagnitude = 1, int nTrailLength = 1, const Vector* pvecDir = NULL) = 0;

	virtual void Dust(const Vector& pos, const Vector& dir, float size, float speed) = 0;

	virtual void MuzzleFlash(const Vector& vecOrigin, const Vector& vecAngles, float flScale, int iType) = 0;

	// like ricochet, but no sound
	virtual void MetalSparks(const Vector& position, const Vector& direction) = 0;

	virtual void EnergySplash(const Vector& position, const Vector& direction, bool bExplosive = false) = 0;

	virtual void Ricochet(const Vector& position, const Vector& direction) = 0;

	// FIXME: Should these methods remain in this interface? Or go in some 
	// other client-server neutral interface?
	virtual float Time() = 0;
	virtual bool IsServer() = 0;

	// Used by the playback system to suppress sounds
	virtual void SuppressEffectsSounds(bool bSuppress) = 0;
};
