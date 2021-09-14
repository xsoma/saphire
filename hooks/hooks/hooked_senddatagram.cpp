// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"

using PacketStart_t = void(__thiscall*)(void*, int, int);

void __fastcall hooks::hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing)
{
	static auto original_fn = clientstate_hook->get_func_address <PacketStart_t> (5);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

    if (!csgo.available())
        return original_fn(ecx, incoming, outgoing);

	if (!csgo.local()->is_alive())
		return original_fn(ecx, incoming, outgoing);
   
    if (csgo.globals.commands.empty())
        return original_fn(ecx, incoming, outgoing);

    if (m_gamerules()->m_bIsValveDS())
        return original_fn(ecx, incoming, outgoing);

    for (auto it = csgo.globals.commands.rbegin(); it != csgo.globals.commands.rend(); ++it)
    {
        if (!it->is_outgoing)
            continue;

        if (it->command_number == outgoing || outgoing > it->command_number && (!it->is_used || it->previous_command_number == outgoing))
        {
            it->previous_command_number = outgoing;
            it->is_used = true;
            original_fn(ecx, incoming, outgoing);
            break;
        }
    }

    auto result = false;

    for (auto it = csgo.globals.commands.begin(); it != csgo.globals.commands.end();) 
    {
        if (outgoing == it->command_number || outgoing == it->previous_command_number)
            result = true;

        if (outgoing > it->command_number && outgoing > it->previous_command_number) 
            it = csgo.globals.commands.erase(it);
        else 
            ++it;
    }

    if (!result)
        original_fn(ecx, incoming, outgoing);
}

using PacketEnd_t = void(__thiscall*)(void*);

void __fastcall hooks::hooked_packetend(void* ecx, void* edx)
{
	static auto original_fn = clientstate_hook->get_func_address <PacketEnd_t>(6);
	csgo.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!csgo.local()->is_alive())  //-V807
	{
		csgo.globals.data.clear();
		return original_fn(ecx);
	}

	if (*(int*)((uintptr_t)ecx + 0x164) == *(int*)((uintptr_t)ecx + 0x16C))
	{
		auto ack_cmd = *(int*)((uintptr_t)ecx + 0x4D2C);
		auto correct = std::find_if(csgo.globals.data.begin(), csgo.globals.data.end(),
			[&ack_cmd](const correction_data& other_data)
			{
				return other_data.command_number == ack_cmd;
			}
		);

		auto netchannel = m_engine()->GetNetChannelInfo();

		if (netchannel && correct != csgo.globals.data.end())
		{
			if (csgo.globals.last_velocity_modifier > csgo.local()->m_flVelocityModifier() + 0.1f)
			{
				auto weapon = csgo.local()->m_hActiveWeapon().Get();

				if (!weapon || weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_grenade()) //-V648
				{
					for (auto& number : csgo.globals.choked_number)
					{
						auto cmd = &m_input()->m_pCommands[number % MULTIPLAYER_BACKUP];
						auto verified = &m_input()->m_pVerifiedCommands[number % MULTIPLAYER_BACKUP];

						if (cmd->m_buttons & (IN_ATTACK | IN_ATTACK2))
						{
							cmd->m_buttons &= ~IN_ATTACK;

							verified->m_cmd = *cmd;
							verified->m_crc = cmd->GetChecksum();
						}
					}
				}
			}

			csgo.globals.last_velocity_modifier = csgo.local()->m_flVelocityModifier();
		}
	}

	return original_fn(ecx);
}

void __fastcall hooks::hooked_checkfilecrcswithserver(void* ecx, void* edx)
{
    
}

bool __fastcall hooks::hooked_loosefileallowed(void* ecx, void* edx)
{
    return true;
}