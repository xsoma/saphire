#pragma once

#include "..\..\includes.hpp"
#include "..\misc\prediction_system.h"

class slowwalk : public singleton <slowwalk> 
{
public:
	void autostop(CUserCmd* m_pcmd, float custom_speed = -1.0f);
	void create_move(CUserCmd* m_pcmd, float custom_speed = -1.0f);
};