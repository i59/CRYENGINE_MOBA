#include "StdAfx.h"

#include "Game.h"
#include "GameCVars.h"

// game related cvars must start with an g_
// game server related cvars must start with sv_
// game client related cvars must start with cl_
// no other types of cvars are allowed to be defined here!
void SCVars::InitCVars(IConsole *pConsole)
{
	uint32 cheatFlag = VF_CHEAT | VF_NET_SYNCED;

	//REGISTER_CVAR(g_minFPS, 30, VF_INVISIBLE | cheatFlag, "");
}

//------------------------------------------------------------------------
void CGame::RegisterConsoleVars()
{
	assert(m_pConsole);

	if (m_pCVars)
		m_pCVars->InitCVars(m_pConsole);
}

//------------------------------------------------------------------------
void CGame::RegisterConsoleCommands()
{
	assert(m_pConsole);

	//REGISTER_COMMAND("SetSunPos", CmdSetSunPos, VF_NULL, "Sets the position of the sun, expects 2 parameters (longitude and latitude).");
}

//------------------------------------------------------------------------
void CGame::UnregisterConsoleCommands()
{
	assert(m_pConsole);
}