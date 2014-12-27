/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: 

-------------------------------------------------------------------------
History:
- 2:8:2004   11:04 : Created by Marcio Martins

*************************************************************************/
#ifndef __GAMESTARTUP_H__
#define __GAMESTARTUP_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IGameFramework.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// get rid of (really) annoying MS defines
#undef min
#undef max
#endif

#if defined(APPLE)
#define GAME_FRAMEWORK_FILENAME	"libCryAction.dylib"
#elif defined(LINUX)
#define GAME_FRAMEWORK_FILENAME	"libCryAction.so"
#else
#define GAME_FRAMEWORK_FILENAME	"cryaction.dll"
#endif 
#define GAME_WINDOW_CLASSNAME		"CryENGINE"

// implemented in GameDll.cpp
extern HMODULE GetFrameworkDLL(const char* dllLocalDir);

class CGameStartup :
	public IGameStartup
{
public:
	CGameStartup();
	virtual ~CGameStartup();

	VIRTUAL IGameRef Init(SSystemInitParams &startupParams);
	VIRTUAL void Shutdown();
	VIRTUAL int Update(bool haveFocus, unsigned int updateFlags);
	VIRTUAL bool GetRestartLevel(char** levelName);
	VIRTUAL const char* GetPatch() const;
	VIRTUAL bool GetRestartMod(char* pModName, int nameLenMax) { return false; }
	VIRTUAL int Run( const char * autoStartLevelName );

	static void ForceCursorUpdate();

	static void AllowAccessibilityShortcutKeys(bool bAllowKeys);

private:
	static void FullScreenCVarChanged( ICVar *pVar );
	static bool IsModAvailable(const string& modName);
	static string GetModPath(const string modName);

	static bool InitMod(const char* pName);
	static void ShutdownMod();

	static bool InitWindow(SSystemInitParams &startupParams);
	static void ShutdownWindow();

	static bool InitFramework(SSystemInitParams &startupParams);
	static void ShutdownFramework();

	void LoadLocalizationData();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static void SetupModSupport();

	static IGame						*m_pGame;
	static IGameFramework	*m_pFramework;
	static bool						m_initWindow;

	static HMODULE					m_frameworkDll;

#ifdef USE_CRYMONO
	static HMODULE					m_cryMonoDll;
#endif

	static HWND						m_hWnd;

#ifdef WIN32
	static HICON				m_hIcon;
	static HCURSOR				m_hCursor;
#endif

	// store the mouse move coords inhere, when the mousewheel is used send these coords to prevent sending invalid coords
	static int						m_lastMoveX;
	static int						m_lastMoveY;

	static bool						m_fullScreenCVarSetup;
	static bool						m_bWasFullscreenBeforeAltTab;
};

extern CGameStartup *g_pGameStartup;

#endif //__GAMESTARTUP_H__
