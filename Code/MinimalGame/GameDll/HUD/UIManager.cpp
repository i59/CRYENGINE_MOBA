////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2011.
// -------------------------------------------------------------------------
//  File name:   UIManager.cpp
//  Version:     v1.00
//  Created:     08/8/2011 by Paul Reindell.
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "UIManager.h"

#include "Game.h"

#include <IGame.h>
#include <IGameFramework.h>
#include <IFlashUI.h>

IUIEventSystemFactory* IUIEventSystemFactory::s_pFirst = NULL;
IUIEventSystemFactory* IUIEventSystemFactory::s_pLast;


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Singleton ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

CUIManager* CUIManager::m_pInstance = NULL;
bool CUIManager::m_bDestroyed = false;

void CUIManager::Init()
{ 
	assert( m_pInstance == NULL );
	if ( !m_pInstance && !gEnv->IsDedicated()  )
	{
		m_pInstance = new CUIManager();
		m_pInstance->PostInit();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::Destroy()
{
	SAFE_DELETE( m_pInstance );
	m_bDestroyed = true;
}

/////////////////////////////////////////////////////////////////////////////////////
CUIManager* CUIManager::GetInstance()
{
	if ( !m_pInstance )
		Init();
	return m_pInstance;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// CTor/DTor ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CUIManager::CUIManager()
{
	IUIEventSystemFactory* pFactory = IUIEventSystemFactory::GetFirst();
	while (pFactory)
	{
		IUIGameEventSystem* pGameEvent = pFactory->Create();
		CRY_ASSERT_MESSAGE(pGameEvent, "Invalid IUIEventSystemFactory!");
		const char* name = pGameEvent->GetTypeName();
		TUIEventSystems::const_iterator it = m_EventSystems.find(name);
		if(it == m_EventSystems.end())
		{
			m_EventSystems[name] = pGameEvent;
		}
		else
		{
			string str;
			str.Format("IUIGameEventSystem \"%s\" already exists!", name);
			CRY_ASSERT_MESSAGE(false, str.c_str());
			SAFE_DELETE(pGameEvent);
		}
		pFactory = pFactory->GetNext();
	}

	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->InitEventSystem();
	}

	g_pGame->GetIGameFramework()->RegisterListener(this, "CUIManager", FRAMEWORKLISTENERPRIORITY_HUD);
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::PostInit()
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->PostInitEventSystems();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
CUIManager::~CUIManager()
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->UnloadEventSystem();
	}

	it = m_EventSystems.begin();
	for (;it != end; ++it)
	{
		delete it->second;
	}

	g_pGame->GetIGameFramework()->UnregisterListener(this);
}

IUIGameEventSystem* CUIManager::GetUIEventSystem(const char* type) const
{
	TUIEventSystems::const_iterator it = m_EventSystems.find(type);
	assert(it != m_EventSystems.end());
	return it != m_EventSystems.end() ? it->second : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::OnPostUpdate(float fDeltaTime)
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->OnUpdate(fDeltaTime);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CUIManager::ProcessViewParams(SViewParams &viewParams)
{
	TUIEventSystems::const_iterator it = m_EventSystems.begin();
	TUIEventSystems::const_iterator end = m_EventSystems.end();
	for (;it != end; ++it)
	{
		it->second->UpdateView(viewParams);
	}
}