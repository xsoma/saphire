#pragma once
const char* watermark_adder[] =
{
	"Name",
	"Server",
	"Ping",
	"Time",
	"FPS"
};

const char* AimType[] =
{
	"Hitbox",
	"Nearest hitbox"
};

const char* tracer_mode[] =
{
	"Off",
	"Line",
	"Beam"
};

const char* LegitHitbox[] =
{
	"Head",
	"Neck",
	"Pelvis",
	"Stomach",
	"Lower chest",
	"Chest",
	"Upper chest"
};

const char* LegitSelection[] =
{
	"Near crosshair",
	"Lowest health",
	"Highest damage",
	"Lowest distance"
};

const char* antiaim_type[] =
{
	"Rage",
	"Legit"
};

const char* movement_type[] =
{
	"Stand",
	"Slow walk",
	"Move",
	"Air"
};

const char* LegitFov[] =
{
	"Static",
	"Dynamic"
};

const char* LegitSmooth[] =
{
	"Static",
	"Humanized"
};

const char* RCSType[] =
{
	"Always on",
	"On target"
};

const char* selection[] =
{
	"Cycle",
	"Near crosshair",
	"Lowest distance",
	"Lowest health",
	"Highest damage"
};

const char* bodyaimlevel[] =
{
	"Low",
	"Medium",
	"High"
};

const char* bodyaim[] =
{
	"In air",
	"On high velocity",
	"On lethal",
	"On double tap",
	"If unresolved",
	"Prefer"
};

const char* autostop_modifiers[] =
{
	"Between shots",
	"If lethal",
	"Only Visible",
	"Only Center",
	"Force accuracy",
	"Early"
};

const char* cond[] =
{
	"Lethal",
	"Prefer",
	"Enemy in air",
	"Resolver unsure"
};

const char* hitboxes[] =
{
	"Head",
	"Upper chest",
	"Chest",
	"Lower chest",
	"Stomach",
	"Pelvis",
	"Arms",
	"Legs",
	"Feet"
};

const char* pitch[] =
{
	"None",
	"Down",
	"Up",
	"Zero",
	"Jitter",
	"Fake down",
	"Fake up",
	"Fake jitter",
	"Custom",
};

const char* yaw[] =
{
	"Off",
	"Jitter",
	"Spin",

};
const char* yaw_add[] =
{
	"Off",
	"Forward",
	"Backward",
	"Right",
	"Left",
	"Left/Right Jitter",
	"Zero/Back Jitter",
	"Backward Jitter",
	"Custom"
};


const char* fs_mode[] =
{
	"Yaw",
	"Desync side",
	"Yaw",
	"Desync side",
	"Yaw",
	"Desync side",
	"Yaw",
	"Desync side",
	"Yaw",
	"Desync side",
	"Yaw",
	"Desync side",
};

const char* baseangle[] =
{
	
	"Local view",
	"At targets",
};

const char* desync[] =
{
	"None",
	"Static",
	"Switch"
};

const char* lby_type[] =
{
	"Minimum",
	"Maximum",
	"Sway",
};

const char* timer_type[] =
{
	"Circular",
	"Rectangle"
};

const char* proj_combo[] =
{
	"Icon",
	"Text",
	"Box",
	"Glow"
};

const char* weaponplayer[] =
{
	"Icon",
	"Text"
};

const char* weaponesp[] =
{
	"Icon",
	"Text",
	"Box",
	"Distance",
	"Glow",
	"Ammo"
};
const char* safe_if[] =
{
	"Lethal",
	"Prefer"
};

const char* baim_if[] =
{
	"Lethal",
	"Prefer",
	"Resolver unsure"

};

const char* hitmarkers[] =
{
	"At crosshair",
	"At hitted enemy"
};

const char* glowtarget[] =
{
	"Enemy",
	"Teammate",
	"Local"
};

const char* glowtype[] =
{
	"Standard",
	"Pulse",
	"Inner"
};
const char* beamtype[] =
{
	"Beam",
	"Line",

};
const char* enemy_chams_type[] =
{
	"Visible",
	"Hidden",
	"Backtrack",
	"Ragdoll",
	"Shot",
};

const char* local_chams_type[] =
{
	"Real",
	"Fake",
	"Arms",
	"Weapon"
};

const char* chams_layer[] =
{
	"Main",
	"Second"
};

const char* chamsvisact[] =
{
	"Visible",
	"Hidden"
};

const char* chamsvis[] =
{
	"Visible"
};
const char* cl_tag[] =
{
	"Off",
	"twotick",
	"Custom"
};
const char* dm_type[] =
{
	"Contour",
	"Wide contour"
};
const char* mm_funk[] =
{
	"Unlock inventory access",
	"Spoof sv_pure" ,
	"Rank reveal",
	"Auto accept"
};
const char* chamstype[] =
{
	"Default",
	"Metal",
	"Flat",
	"Pulse",
	"Crystal",
	"Glass",
	"Contour",
	"Wide contour",
	"Animated",
	"Velvet",
	"Glow Wireframe"
};
const char* flags[] =
{
	"Money",
	"Armor",
	"Have kits",
	"In scope",
	"Fakeduck",
	"Vulnerable",
	"Flashed",
	"Exploit",
	"Delay",
	"Bomb carrier"
};

const char* removals[] =
{
	"Scope",
	"Zoom",
	"Smoke",
	"Flash",
	"Recoil",
	"Landing bob",
	"Postprocessing",
	"Fog"
};

const char* indicators[] =
{
	"Fake",
	"Desync side",
	"Choke",
	"Damage override",
	"Safe points",
	"Body aim",
	"Double tap",
	"Hide shots"
};

const char* skybox[] =
{
	"None",
	"Tibet",
	"Baggage",
	"Italy",
	"Aztec",
	"Vertigo",
	"Daylight",
	"Daylight 2",
	"Clouds",
	"Clouds 2",
	"Gray",
	"Clear",
	"Canals",
	"Cobblestone",
	"Assault",
	"Clouds dark",
	"Night",
	"Night 2",
	"Night flat",
	"Dusty",
	"Rainy",
	"Custom"
};

const char* mainwep[] =
{
	"None",
	"SCAR20/G3SG1",
	"AWP",
	"SSG 08"
};
const char* dt_mode[] =
{
	"Static",
	"Teleport"
};
const char* p_angs_v[] =
{
	"Down",
	"Up",
	"Zero"
};
const char* p_angs_r[] =
{
	"Up(if zero)",
	"Down(if zero)",
	"Down(if up)",
	"Up(if down)",
};
const char* secwep[] =
{
	"None",
	"Dual Berettas",
	"Deagle/Revolver"
};

const char* bbhh[] =
{
	"None",
	"Default",
	"Hitchanced"

};

const char* strafes[] =
{
	"None",
	"Legit",
	"Directional",
	"Rage"
};

const char* events_output[] =
{
	"Cheat events",
	"Game events"
};

const char* events[] =
{
	"Player hits",
	"Items",
	"Bomb",
	"Spread misses",
	"Occlusion misses",
	"Misses by unknown reanons"
};

const char* weather[] =
{
	"Off",
	"Rain"

};

const char* grenades[] =
{
	"Grenades",
	"Armor",
	"Taser",
	"Defuser"
};

const char* fakelags[] =
{
	"Static",
	"Fluctuate"
};

const char* lagstrigger[] =
{
	"Slow walk",
	"Move",
	"Air",
	"Peek"
};

const char* sounds[] =
{
	"None",
	"Cod",
	"Bubble",
	"Flick"
};

const char* player_model_t[] =
{
	"None",
	"Enforcer",
	"Soldier",
	"Ground Rebel",
	"Maximus",
	"Osiris",
	"Slingshot",
	"Dragomir",
	"Blackwolf",
	"Prof. Shahmat",
	"Rezan The Ready",
	"Doctor Romanov",
	"Mr. Muhlik",
	"Darryl Miami",
	"Darryl Silent",
	"Skullhead",
	"Royale",
	"Loudmouth",
	"Safecracker Voltzmann",
	"Little Kev",
	"Number K",
	"Getaway Sally",
	"Rezan the redshirt",
	"Dragomir",
	"Seal Team 6",
	"3rd Commando",
	"Operator FBI",
	"Squadron Officer",
	"Markus Delrow",
	"Buckshot",
	"McCoy",
	"Commander Ricksaw",
	"Agent Ava",
	"Cmdr. Mae 'Dead Cold' Jamison",
	"1st lieutenant farlow",
	"John 'Van Healen' Kask",
	"Bio-Haz Specialist",
	"Sergant Bombson",
	"Chem-Haz Specialist"
	"Buckshot",
	"Mccoy"
};

