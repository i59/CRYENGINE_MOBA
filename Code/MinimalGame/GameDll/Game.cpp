/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  
 -------------------------------------------------------------------------
  History:
  - 3:8:2004   11:26 : Created by Marcio Martins
  - 17:8:2005        : Modified - NickH: Factory registration moved to GameFactory.cpp

*************************************************************************/
#include "StdAfx.h"
#include "Game.h"
#include "GameCVars.h"

#include "GameStartup.h"
#include "GameRules.h"
#include "GameFactory.h"
#include "GameUtils.h"

#include "GameRules.h"

#include "Nodes/G2FlowBaseNode.h"

#include "HUD/UIManager.h"

#include "GamePhysicsSettings.h"

#include "Actor.h"

#ifdef USE_CRYMONO
#include <IMonoScriptSystem.h>
#endif

#include <ICryPak.h>
#include <CryPath.h>
#include <IActionMapManager.h>
#include <IViewSystem.h>
#include <ILevelSystem.h>
#include <IVehicleSystem.h>
#include <IMovieSystem.h>
#include <IPlayerProfiles.h>
#include <IPlatformOS.h>

#include <ISaveGame.h>
#include <ILoadGame.h>
#include <CryPath.h>
#include <IPathfinder.h>

#include <IMaterialEffects.h>

#include <ICheckPointSystem.h>

#ifdef WIN32
#include <WinSock2.h>

#include <windows.h>
#include <wininet.h>
#include <iostream>

#include <Ws2tcpip.h>

#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif

#undef GetUserName

// Needed for the Game02 specific flow node
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pFirst=0;
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pLast=0;

CGame *g_pGame = 0;
SCVars *g_pGameCVars = 0;

CGame::CGame()
: m_pFramework(0),
	m_pConsole(0),
	m_pPlayerProfileManager(0),
	m_randomGenerator(gEnv->bNoRandomSeed?0:(uint32)gEnv->pTimer->GetAsyncTime().GetValue()),
	m_pGamePhysicsSettings(nullptr),
	m_bInitialized(false)
{
	m_pCVars = new SCVars();
	g_pGameCVars = m_pCVars;
	g_pGame = this;
	m_inDevMode = false;

	GetISystem()->SetIGame(this);
}

CGame::~CGame()
{
	SAFE_DELETE(m_pGamePhysicsSettings);

	m_pFramework->EndGameContext();

	//SAFE_DELETE(m_pCameraManager);
	SAFE_DELETE(m_pCVars);
	ClearGameSessionHandler(); // make sure this is cleared before the gamePointer is gone
	g_pGame = 0;
	g_pGameCVars = 0;

	gEnv->pGame = 0;
}

bool CGame::Init(IGameFramework *pFramework)
{
	LOADING_TIME_PROFILE_SECTION(GetISystem());

	m_pFramework = pFramework;

	assert(m_pFramework);

	m_pConsole = gEnv->pConsole;

	RegisterConsoleVars();
	RegisterConsoleCommands();
	RegisterGameObjectEvents();

	IActionMapManager *pActionMapMan = m_pFramework->GetIActionMapManager();
	pActionMapMan->AddInputDeviceMapping(eAID_KeyboardMouse, "keyboard");
	pActionMapMan->AddInputDeviceMapping(eAID_XboxPad, "xboxpad");
	pActionMapMan->AddInputDeviceMapping(eAID_PS3Pad, "ps3pad");

	m_pGamePhysicsSettings = new CGamePhysicsSettings();

	LoadActionMaps(ACTIONMAP_DEFAULT_PROFILE);

	// Register all the games factory classes e.g. maps "Player" to CPlayer
	RegisterEntities();

	// set game GUID
	m_pFramework->SetGameGUID(GAME_GUID);
	gEnv->pSystem->GetPlatformOS()->UserDoSignIn(0); // sign in the default user

	CUIManager::Init();

	return true;
}

bool CGame::CompleteInit()
{
	CryLogAlways("Game init done");

	m_bInitialized = true;

	return true;
}

void CGame::RegisterEntities()
{
	REGISTER_FACTORY(m_pFramework, "SampleActor", CActor, false);

	//GameRules
	REGISTER_FACTORY(m_pFramework, "GameRules", CGameRules, false);

	if (IGameRulesSystem *pGameRulesSystem = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem())
		pGameRulesSystem->RegisterGameRules("SinglePlayer", "GameRules");
}

void CGame::RegisterGameFlowNodes()
{
	// Initialize Game02 flow nodes
	if (IFlowSystem *pFlow = m_pFramework->GetIFlowSystem())
	{
		CG2AutoRegFlowNodeBase *pFactory = CG2AutoRegFlowNodeBase::m_pFirst;

		while (pFactory)
		{
			pFlow->RegisterType( pFactory->m_sClassName,pFactory );
			pFactory = pFactory->m_pNext;
		}
	}

#ifdef USE_CRYMONO
	IMonoScriptSystem::g_pThis->RegisterFlownodes();
#endif
}

IGamePhysicsSettings* CGame::GetIGamePhysicsSettings()
{
	return m_pGamePhysicsSettings;
}

int CGame::Update(bool haveFocus, unsigned int updateFlags)
{
	if(!m_bInitialized || m_pFramework == nullptr)
		return 1;

	bool bRun = m_pFramework->PreUpdate( true, updateFlags );

	float frameTime = gEnv->pTimer->GetFrameTime();
	
	// Update game systems here

	m_pFramework->PostUpdate( true, updateFlags );

	if(m_inDevMode != gEnv->pSystem->IsDevMode())
	{
		m_inDevMode = gEnv->pSystem->IsDevMode();
		g_pGame->EnableActionMap("debug", m_inDevMode);
	}
	
	return bRun ? 1 : 0;
}

void CGame::Shutdown()
{
	if (m_pPlayerProfileManager)
		m_pPlayerProfileManager->LogoutUser(m_pPlayerProfileManager->GetCurrentUser());

	CUIManager::Destroy();

	this->~CGame();
}

const char *CGame::GetLongName()
{
	return GAME_LONGNAME;
}

const char *CGame::GetName()
{
	return GAME_NAME;
}

uint32 CGame::AddGameWarning(const char *stringId, const char *paramMessage, IGameWarningsListener *pListener)
{
	gEnv->pSystem->Warning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, 0, NULL, "[%s] %s", stringId, paramMessage);

	return 0;
}

CGameRules *CGame::GetGameRules() const
{
	return static_cast<CGameRules *>(m_pFramework->GetIGameRulesSystem()->GetCurrentGameRules());
}

void CGame::LoadActionMaps(const char *filename)
{
	// make sure that they are also added to the GameActions.actions file!
	XmlNodeRef rootNode = m_pFramework->GetISystem()->LoadXmlFromFile(filename);
	if(rootNode)
	{
		IActionMapManager *pActionMapManager = GetIGameFramework()->GetIActionMapManager();

		pActionMapManager->Clear();
		pActionMapManager->LoadFromXML(rootNode);

		pActionMapManager->Enable(true);
	}
	else
	{
		GameWarning("Could not open configuration file %s", filename);
		GameWarning("this will probably cause an infinite loop later while loading a map");
	}
}

void CGame::EnableActionMap(const char *name, bool bEnable)
{
	IActionMapManager *pActionMapManager = GetIGameFramework()->GetIActionMapManager();
	pActionMapManager->EnableActionMap(name, bEnable);

	if(IActionMap *pActionMap = pActionMapManager->GetActionMap(name))
		pActionMap->SetActionListener(GetIGameFramework()->GetClientActorId());
}

string CGame::Localize(const char *uiString)
{
	wstring localizedString;
	gEnv->pSystem->GetLocalizationManager()->LocalizeString(uiString, localizedString);

	const wchar_t *source = localizedString.c_str();

	size_t length = wcslen(source) + 1;
	char *buffer = new char[length];

	size_t convertedChars = 0;
#ifdef LINUX
	convertedChars = wcstombs(buffer, source, length);
#else
	wcstombs_s(&convertedChars, buffer, length, source, _TRUNCATE);
#endif
	string sBuffer = string(buffer);
	delete[] buffer;

	return sBuffer;
}

void CGame::RegisterGameObjectEvents()
{
	IGameObjectSystem* pGOS = m_pFramework->GetIGameObjectSystem();

	pGOS->RegisterEvent(eCGE_ActorRevive,"ActorRevive");
	pGOS->RegisterEvent(eCGE_TurnRagdoll,"TurnRagdoll");
	pGOS->RegisterEvent(eCGE_EnableFallAndPlay,"EnableFallAndPlay");
	pGOS->RegisterEvent(eCGE_DisableFallAndPlay,"DisableFallAndPlay");
	pGOS->RegisterEvent(eCGE_TextArea,"TextArea");
	pGOS->RegisterEvent(eCGE_InitiateAutoDestruction,"InitiateAutoDestruction");
	pGOS->RegisterEvent(eCGE_Event_Collapsing,"Event_Collapsing");
	pGOS->RegisterEvent(eCGE_Event_Collapsed,"Event_Collapsed");
	pGOS->RegisterEvent(eCGE_MultiplayerChatMessage,"MultiplayerChatMessage");
	pGOS->RegisterEvent(eCGE_ResetMovementController,"ResetMovementController");
	pGOS->RegisterEvent(eCGE_AnimateHands,"AnimateHands");
	pGOS->RegisterEvent(eCGE_Ragdoll,"Ragdoll");
	pGOS->RegisterEvent(eCGE_EnablePhysicalCollider,"EnablePhysicalCollider");
	pGOS->RegisterEvent(eCGE_DisablePhysicalCollider,"DisablePhysicalCollider");
	pGOS->RegisterEvent(eCGE_RebindAnimGraphInputs,"RebindAnimGraphInputs");
	pGOS->RegisterEvent(eCGE_OpenParachute, "OpenParachute");
	pGOS->RegisterEvent(eCGE_ReactionEnd, "ReactionEnd");
}

void CGame::GetMemoryStatistics(ICrySizer * s)
{
	s->Add(*this);

	s->Add(*m_pCVars);

	if (m_pPlayerProfileManager)
		m_pPlayerProfileManager->GetMemoryStatistics(s);
}

bool CGame::IsGameActive() const
{
	assert(g_pGame);
	IGameFramework* pGameFramework = g_pGame->GetIGameFramework();
	assert(pGameFramework);
	return (pGameFramework->StartingGameContext() || pGameFramework->StartedGameContext()) && pGameFramework->GetClientChannel();
}

void CGame::ClearGameSessionHandler()
{
	GetIGameFramework()->SetGameSessionHandler(NULL);
}

uint32 CGame::GetRandomNumber()
{
	return m_randomGenerator.GenerateUint32();
}

void CGame::VerifyMaxPlayers(ICVar *pVar)
{
	// Perform any MP max player count limitations here
}

#include UNIQUE_VIRTUAL_WRAPPER(IGame)
