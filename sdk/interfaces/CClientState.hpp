#pragma once

#include <cstdint>

#include "..\math\Vector.hpp"
#include "..\misc\bf_write.h"

#define GenDefineVFunc(...) ( this, __VA_ARGS__ ); }
#define VFUNC( index, func, sig ) auto func { return call_virtual< sig >( this, index ) GenDefineVFunc

class CClockDriftMgr {
public:
    float m_ClockOffsets[17];   //0x0000
    uint32_t m_iCurClockOffset; //0x0044
    uint32_t m_nServerTick;     //0x0048
    uint32_t m_nClientTick;     //0x004C
};

class INetChannel
{
public:
    char pad_0x0000[0x18]; //0x0000
    __int32 m_nOutSequenceNr; //0x0018 
    __int32 m_nInSequenceNr; //0x001C 
    __int32 m_nOutSequenceNrAck; //0x0020 
    __int32 m_nOutReliableState; //0x0024 
    __int32 m_nInReliableState; //0x0028 
    __int32 m_nChokedPackets; //0x002C

    void Transmit(bool onlyreliable)
    {
        using Fn = bool(__thiscall*)(void*, bool);
        call_virtual<Fn>(this, 49)(this, onlyreliable);
    }

    void send_datagram()
    {
        using Fn = int(__thiscall*)(void*, void*);
        call_virtual<Fn>(this, 46)(this, 0);
    }

    void SetTimeOut(float seconds)
    {
        using Fn = void(__thiscall*)(void*, float);
        return call_virtual<Fn>(this, 4)(this, seconds);
    }

    int RequestFile(const char* filename)
    {
        using Fn = int(__thiscall*)(void*, const char*);
        return call_virtual<Fn>(this, 62)(this, filename);
    }
};

class INetMessage
{
public:
    virtual    ~INetMessage() {};

    // Use these to setup who can hear whose voice.
    // Pass in client indices (which are their ent indices - 1).

    virtual void    SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
    virtual void    SetReliable(bool state) = 0;    // set to true if it's a reliable message

    virtual bool    Process(void) = 0; // calles the recently set handler to process this message

    virtual    bool    ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
    virtual    bool    WriteToBuffer(bf_write& buffer) = 0;    // returns true if writing was OK

    virtual bool    IsReliable(void) const = 0;  // true, if message needs reliable handling

    virtual int                GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
    virtual int                GetGroup(void) const = 0;    // returns net message group of this message
    virtual const char* GetName(void) const = 0;    // returns network message name, eg "svc_serverinfo"
    virtual INetChannel* GetNetChannel(void) const = 0;
    virtual const char* ToString(void) const = 0; // returns a human readable string about message content
};

struct CEventInfo {
    int                m_class_id;
    float            m_delay;
    char            pad1[4];
    ClientClass* m_client_class;
    char            pad2[40];
    CEventInfo* m_next;
};

class CClientState
{
public:
    char            pad0[156];
    INetChannel* pNetChannel;
    int                m_challenge_number;
    char            pad1[4];
    double            m_connect_time;
    int                m_retry_number;
    char            pad2[84];
    int                m_signon_state;
    char            pad3[4];
    double            m_next_cmd_time;
    int                m_server_count;
    int                m_cur_sequence;
    char            pad4[8];

    struct {
        float        m_clock_offsets[16];
        int            m_cur_clock_offset;
        int            m_server_tick;
        int            m_client_tick;
    } m_clock_drift_mgr;

    int                iDeltaTick;
    char            pad5[4];
    int                m_view_entity;
    int                m_player_slot;
    bool            m_paused;
    char            pad6[3];
    char            m_level_name[260];
    char            m_level_name_short[40];
    char            pad7[212];
    int                m_max_clients;
    char            pad8[18836];
    int                m_old_tick_count;
    float            m_tick_remainder;
    float            m_frame_time;
    int                nLastOutgoingCommand;
    int                iChokedCommands;
    int                nLastCommandAck;
    int                m_last_server_tick;
    int                iCommandAck;
    int                m_sound_sequence;
    int                m_last_progress_percent;
    bool            m_is_hltv;
    char            pad9[75];
    Vector        m_view_angles;
    char            pad10[204];
    CEventInfo* m_events;
};