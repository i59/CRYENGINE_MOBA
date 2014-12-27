/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: 
  
 -------------------------------------------------------------------------
  History:
  - 3:8:2004   11:23 : Created by Marcio Martins

*************************************************************************/
#ifndef __GAME_H__
#define __GAME_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IGame.h>
#include <IGameFramework.h>

#define GAME_NAME				"CE Minimal"
#define GAME_LONGNAME		"CRYENGINE Minimal Game Sample"

#define GAME_GUID "{ab83ed6c-0056-4ce5-a3f5-4e88b40ccf60}"

#define ACTIONMAP_DEFAULT_PROFILE	"scripts/config/defaultActionmap.xml"

struct ISystem;
struct IConsole;

struct IActionMap;
struct IActionFilter;
class CGameRules;
struct SCVars;

class CGamePhysicsSettings;

// when you add stuff here, also update in CGame::RegisterGameObjectEvents
enum ECryGameEvent
{
	eCGE_ActorRevive = 256,
	eCGE_TurnRagdoll,
	eCGE_EnableFallAndPlay,
	eCGE_DisableFallAndPlay,
	eCGE_TextArea,
	eCGE_InitiateAutoDestruction,
	eCGE_Event_Collapsing,
	eCGE_Event_Collapsed,
	eCGE_MultiplayerChatMessage,
	eCGE_ResetMovementController,
	eCGE_AnimateHands,
	eCGE_Ragdoll,
	eCGE_EnablePhysicalCollider,
	eCGE_DisablePhysicalCollider,
	eCGE_RebindAnimGraphInputs,
	eCGE_OpenParachute,
  eCGE_Turret_LockedTarget,
  eCGE_Turret_LostTarget,
	eCGE_ReactionEnd
};

class CGame 
	: public IGame
{
public:
	CGame();
	VIRTUAL ~CGame();

	// IGame
	VIRTUAL bool  Init(IGameFramework *pFramework);
	VIRTUAL bool  CompleteInit();
	VIRTUAL void  Shutdown();
	VIRTUAL int   Update(bool haveFocus, unsigned int updateFlags);
	VIRTUAL void  ConfigureGameChannel(bool isServer, IProtocolBuilder *pBuilder) {}
	VIRTUAL void  EditorResetGame(bool bStart) {}
	VIRTUAL void  PlayerIdSet(EntityId playerId) {}
	VIRTUAL string  InitMapReloading() { return ""; }
	VIRTUAL bool IsReloading() { return false; }
	VIRTUAL IGameFramework *GetIGameFramework() { return m_pFramework; }

	VIRTUAL const char *GetLongName();
	VIRTUAL const char *GetName();

	VIRTUAL void GetMemoryStatistics(ICrySizer *s);

	VIRTUAL void OnClearPlayerIds() {}
	//auto-generated save game file name
	VIRTUAL IGame::TSaveGameName CreateSaveGameName() { return ""; }
	//level names were renamed without changing the file/directory
	VIRTUAL const char* GetMappedLevelName(const char *levelName) const { return levelName; }
	// 
	VIRTUAL IGameStateRecorder* CreateGameStateRecorder(IGameplayListener* pL) { return nullptr; }

	virtual void FullSerialize( TSerialize ser ) {}
	virtual void PostSerialize() {}

	virtual IGame::ExportFilesInfo ExportLevelData( const char* levelName, const char* missionName ) const { return IGame::ExportFilesInfo("Game", 0); }
	virtual void   LoadExportedLevelData( const char* levelName, const char* missionName ) {}

	VIRTUAL void RegisterGameFlowNodes();

	virtual IGamePhysicsSettings* GetIGamePhysicsSettings();

	VIRTUAL const bool DoInitialSavegame() const { return true; }

	VIRTUAL void CreateLobbySession( const SGameStartParams* pGameStartParams ) {;}
	VIRTUAL void DeleteLobbySession() {;}
	// ~IGame

  void RegisterEntities();

	VIRTUAL uint32 AddGameWarning(const char* stringId, const char* paramMessage, IGameWarningsListener* pListener = NULL);
	VIRTUAL void RenderGameWarnings() {} 
	VIRTUAL void RemoveGameWarning(const char* stringId) {}

	virtual void OnRenderScene(const SRenderingPassInfo &passInfo) {}

	VIRTUAL bool GameEndLevel(const char* stringId) { return false; }
	VIRTUAL void OnRenderScene() {}

	CGameRules *GetGameRules() const;
	
	IPlayerProfileManager* GetPlayerProfileManager() { return m_pPlayerProfileManager; }

	bool IsGameActive() const;

	void ClearGameSessionHandler();

	uint32 GetRandomNumber();
	
  ILINE SCVars *GetCVars() {return m_pCVars;}
	
	VIRTUAL void LoadActionMaps(const char* filename);
	void EnableActionMap(const char *name, bool bEnable);

	string Localize(const char *msg);

	virtual IAntiCheatManager *GetAntiCheatManager() { return nullptr; }
	
	ILINE CGamePhysicsSettings *GetGamePhysicsSettings() { return m_pGamePhysicsSettings; }

	static void VerifyMaxPlayers(ICVar * pVar);

protected:
	// These funcs live in GameCVars.cpp
	VIRTUAL void RegisterConsoleVars();
	VIRTUAL void RegisterConsoleCommands();
	VIRTUAL void UnregisterConsoleCommands();

	VIRTUAL void RegisterGameObjectEvents();

protected:
	CRndGen m_randomGenerator;

	IGameFramework			*m_pFramework;
	IConsole						*m_pConsole;

	IPlayerProfileManager* m_pPlayerProfileManager;

	bool								m_inDevMode;

	SCVars	*m_pCVars;

	CGamePhysicsSettings *m_pGamePhysicsSettings;

	bool m_bInitialized;
};

extern CGame *g_pGame;

#define SAFE_HARDWARE_MOUSE_FUNC(func)\
	if(gEnv->pHardwareMouse)\
		gEnv->pHardwareMouse->func

#endif //__GAME_H__