////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIManager.h
//  Version:     v1.00
//  Created:     08/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __UIManager_H__
#define __UIManager_H__

#include "IUIGameEventSystem.h"

#include <IGameFramework.h>

class CUIManager
	: public IGameFrameworkListener
{
public:
	static void Init();
	static void Shutdown();
	static void Destroy();

	static bool IsDestroyed() { return m_bDestroyed; }

	// can return NULL if dedicated server!
	static CUIManager* GetInstance();

	void PostInit();

	IUIGameEventSystem *GetUIEventSystem(const char* typeName) const;
	
	void ProcessViewParams(SViewParams &viewParams);
	
	// IGameFrameworkListener
	virtual void OnPostUpdate(float fDeltaTime);
	virtual void OnSaveGame(ISaveGame* pSaveGame) {}
	virtual void OnLoadGame(ILoadGame* pLoadGame) {}
	virtual void OnLevelEnd(const char* nextLevel) {}
	virtual void OnActionEvent(const SActionEvent& event) {}
	virtual void OnPreRender() {};
	// ~IGameFrameworkListener

private:
	CUIManager();
	CUIManager(const CUIManager& ) {}
	CUIManager operator=(const CUIManager& ) { return *this; }
	~CUIManager();

	static CUIManager* m_pInstance;
	static bool m_bDestroyed;

private:
	typedef std::map<string, IUIGameEventSystem*> TUIEventSystems;
	TUIEventSystems m_EventSystems;
};

namespace UIEvents
{
	template <class T>
	T* Get()
	{
		CUIManager* pManager = CUIManager::GetInstance();
		return pManager ? (T*) pManager->GetUIEventSystem(T::GetTypeNameS()) : NULL;
	}
}


#endif

