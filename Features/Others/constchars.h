#pragma once

const char* AimType[] =
{
	"Hitbox",
	"Nearest hitbox"
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

const char* auto_scope_mode[] =
{
	"Hitchance Fail",
	"Always",
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
	"Lowest HP",
	"Highest HP",
	"Closest To FOV",
	"Highest Damage"

};

const char* bodyaim[] =
{
	"If Enemy Shot",
	"If Safe"
};

const char* autostop_modifiers[] =
{
	"Between shots",
	"Force accuracy",
	"Predictive"
};

const char* autostop_conditions[] =
{
	"Only When Lethal",
	"Visible",
	"Center"
};

const char* doubletap_modifiers[] =
{
	"Slow teleport",
	"Break lagcomp",
	"Lag peek"
};

const char* exploit_modifiers[] =
{
	"Don't recharge on packet loss",
	"Don't recharge on low fps"
};

const char* hitboxes[] =
{
	"Head",
	"Chest",
	"Pelvis",
	"Arms",
	"Legs",
	"Feet"
};

const char* extra_points[] =
{
	"Arms",
	"Legs"
};

const char* pitch[] =
{
	"None",
	"Down",
	"Up",
	"Custom"
};

const char* yaw_direction[] =
{
	"Backwards",
	"Forwards"
};

const char* yaw[] =
{
	"Static",
	"Jitter",
	"Center Jitter"
};

const char* baseangle[] =
{
	"Local view",
	"At targets"
};

const char* desync[] =
{
	"None",
	"Static",
	"Jitter"
};

const char* lby_type[] =
{
	"Normal",
	"Opposite",
	"Sway"
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

const char* hitmarkers[] =
{
	"Crosshair",
	"World"
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

const char* local_chams_type[] =
{
	"Real",
	"Desync"
};

const char* chamsvisact[] =
{
	"Visible",
	"Invisible"
};

const char* chamsvis[] =
{
	"Visible"
};

const char* tracers_type[] = {
	"Laser",
	"Beam",
	"Bubble",
	"Glow",
	"2D"
};
const char* chamstype[] =
{
	"Default",
	"Flat",
	"Glass",
	"Velvet",
	"Circuit",
	"Glow"
};

const char* flags[] =
{
	"Money",
	"Armor",
	"Fake",
	"Hit",
	"Defuse kit",
	"Scoped",
	"Fakeducking",
	"Delay",
	"Bomb carrier"
};

const char* slowwalk_type[] =
{
	"Prefer Accuracy",
	"Custom"
};

const char* legitbot_disablers[] =
{
	"If Flashed",
	"If In Air",
	"If In Smoke",
	"If Not Scoped",
	"If Through Wall"
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
	"Fog",
	"Shadows",
	"Decals",
	"Sleeves"
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

const char* weather[] = {
	"Rain",
	"Ash",
	"Rain storm",
	"Snow"
};

const char* mainwep[] =
{
	"None",
	"AWP",
	"SCAR20/G3SG1",
	"SSG-08"
};

const char* secwep[] =
{
	"None",
	"Deagle / Revolver",
	"Tec-9 / Five-Seven",
	"Dualies"
};

const char* strafes[] =
{
	"None",
	"Legit",
	"Rage"
};

const char* events_output[] =
{
	"Console",
	"Chat"
};

const char* events[] =
{
	"Aimbot hits",
	"Item purchases",
	"Bomb information"
};

const char* grenades[] =
{
	"Armor",
	"Grenades",
	"Defuser",
	"Taser"
};

const char* fakelags[] =
{
	"Static",
	"Random",
	"Dynamic",
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
	"Mettalic",
	"Flick",
	"Ding",
	"Primordial",
	"Magic",
	"Bell"
};

const char* player_models[] =
{
	"None",
	"T Model",
	"CT Model",
	"Silent | Sir Bloody Darryl",
	"Vypa Sista of the Revolution | Guerrilla Warfare",
	"Medium Rare' Crasswater | Guerrilla Warfare",
	"Crasswater The Forgotten | Guerrilla Warfare",
	"Skullhead | Sir Bloody Darryl",
	"Chef d'Escadron Rouchard | Gendarmerie Nationale",
	"Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman",
	"Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman",
	"Royale | Sir Bloody Darryl",
	"Loudmouth | Sir Bloody Darryl",
	"Miami | Sir Bloody Darryl",
	"Getaway Sally | Professional",
	"Elite Trapper Solman | Guerrilla Warfare",
	"Bloody Darryl The Strapped | The Professionals",
	"Chem-Haz Capitaine | Gendarmerie Nationale",
	"Lieutenant Rex Krikey | SEAL Frogman",
	"Arno The Overgrown | Guerrilla Warfare",
	"Col. Mangos Dabisi | Guerrilla Warfare",
	"Officer Jacques Beltram | Gendarmerie Nationale",
	"Trapper | Guerrilla Warfare",
	"Lieutenant 'Tree Hugger' Farlow | SWAT",
	"Sous-Lieutenant Medic | Gendarmerie Nationale",
	"Primeiro Tenente | Brazilian 1st Battalion",
	"D Squadron Officer | NZSAS",
	"Trapper Aggressor | Guerrilla Warfare",
	"Aspirant | Gendarmerie Nationale",
	"AGENT Gandon | Professional",
	"Safecracker Voltzmann | Professional",
	"Little Kev | Professional",
	"Blackwolf | Sabre",
	"Rezan the Redshirt | Sabre",
	"Rezan The Ready | Sabre",
	"Maximus | Sabre",
	"Dragomir | Sabre",
	"Dragomir | Sabre Footsoldier",
	"Lt. Commander Ricksaw | NSWC SEAL",
	"Two Times' McCoy | USAF TACP",
	"Two Times' McCoy | USAF Cavalry",
	"Buckshot | NSWC SEAL",
	"Blueberries Buckshot | NSWC SEAL",
	"Seal Team 6 Soldier | NSWC SEAL",
	"3rd Commando Company | KSK",
	"The Doctor' Romanov | Sabre",
	"Michael Syfers| FBI Sniper",
	"Markus Delrow | FBI HRT",
	"Cmdr. Mae | SWAT",
	"1st Lieutenant Farlow | SWAT",
	"John Van Healen Kask | SWAT",
	"Bio-Haz Specialist | SWAT",
	"Chem-Haz Specialist | SWAT",
	"Sergeant Bombson | SWAT",
	"Operator | FBI SWAT",
	"Street Soldier | Phoenix",
	"Slingshot | Phoenix",
	"Enforcer | Phoenix",
	"Soldier | Phoenix",
	"The Elite Mr. Muhlik | Elite Crew",
	"Prof. Shahmat | Elite Crew",
	"Osiris | Elite Crew",
	"Ground Rebel| Elite Crew",
	"Special Agent Ava | FBI",
	"B Squadron Officer | SAS",
	"Jumpsuit A",
	"Jumpsuit B",
	"Jumpsuit C",
	"Anarchist A",
	"Anarchist B",
	"Anarchist C",
	"Separatist A",
	"Separatist B",
	"Separatist C",
	"Separatist D",
	"CTM. FBI",
	"CTM. FBI A",
	"CTM. FBI B",
	"CTM. FBI C",
	"CTM. FBI E",
	"Gign Model A",
	"Gign Model B",
	"Gign Model C",
	"CTM. ST6",
	"CTM. ST6 A",
	"CTM. ST6 B",
	"CTM. ST6 C",
	"CTM. ST6 D",
	"CTM. IDF B",
	"CTM. IDF C",
	"CTM. IDF D",
	"CTM. IDF E",
	"CTM. IDF F",
	"CTM. Swat",
	"TM. Swat A",
	"CTM. Swat B",
	"CTM. Swat C",
	"CTM. Swat D",
	"CTM. Sas",
	"CTM. Gsg9",
	"CTM. Gsg9 A",
	"CTM. Gsg9 B",
	"CTM. Gsg9 C",
	"CTM. Gsg9 D",
	"Professional A",
	"Professional B",
	"Professional C",
	"Professional D",
	"Leet A",
	"Leet B",
	"Leet C",
	"Leet D",
	"Balkan A",
	"Balkan B",
	"Balkan C",
	"Balkan D",
	"Pirate A",
	"Pirate B",
	"Pirate C",

};

const char* skinchanger_weapons[] = {
	"Knife",
	"Gloves",
	"AK47",
	"AUG",
	"AWP",
	"CZ75A",
	"Deagle",
	"Dual Berretas",
	"Famas",
	"Five Seven",
	"G3SG1",
	"Gail AR",
	"Glock",
	"M249",
	"M4A1-S",
	"M4A4",
	"MAC10",
	"MAG7",
	"MP5SD",
	"MP7",
	"MP9",
	"Negav",
	"Nova",
	"P2000",
	"P250",
	"P90",
	"Biden",
	"Revolver",
	"Shorty",
	"SCAR20",
	"SSG08",
	"SG556",
	"TEC9",
	"UMP45",
	"USPS",
	"XM1014",
};

const char* prefer_body_mode[] = {
	"Low",
	"Medium",
	"High"
};