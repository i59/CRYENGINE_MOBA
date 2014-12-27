#include "StdAfx.h"
#include "GameRules.h"
#include "Game.h"
#include "GameCVars.h"

#include "Actor.h"

#include "HUD/UIManager.h"

#include <ILevelSystem.h>

//------------------------------------------------------------------------
CGameRules::CGameRules()
	:  m_timeOfDayInitialized(false)
{
}

//------------------------------------------------------------------------
CGameRules::~CGameRules()
{
	if (IGameFramework *pGameFramework = gEnv->pGame->GetIGameFramework())
	{
		if (pGameFramework->GetIGameRulesSystem())
			pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(0);
		
	}
}

//------------------------------------------------------------------------
bool CGameRules::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetGameObject()->EnablePostUpdates(this);

	IGameFramework *pGameFramework = gEnv->pGame->GetIGameFramework();
	
	pGameFramework->GetIGameRulesSystem()->SetCurrentGameRules(this);
	
	return true;
}

//------------------------------------------------------------------------
void CGameRules::PostInit( IGameObject * pGameObject )
{
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_WithoutAI);
	pGameObject->EnablePostUpdates(this);
}

//------------------------------------------------------------------------
void CGameRules::Update(SEntityUpdateContext& ctx, int updateSlot)
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	if (updateSlot!=0)
		return;
}

//------------------------------------------------------------------------
void CGameRules::ProcessEvent( SEntityEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	static ICVar* pTOD = gEnv->pConsole->GetCVar("sv_timeofdayenable");

	switch(event.event)
	{
	case ENTITY_EVENT_RESET:
		{
			m_timeOfDayInitialized = false;
		}
		break;

	case ENTITY_EVENT_START_GAME:
		m_timeOfDayInitialized = false;

		if (gEnv->bServer && gEnv->bMultiplayer && pTOD && pTOD->GetIVal() && g_pGame->GetIGameFramework()->IsImmersiveMPEnabled())
		{
			static ICVar* pStart = gEnv->pConsole->GetCVar("sv_timeofdaystart");
			if (pStart)
				gEnv->p3DEngine->GetTimeOfDay()->SetTime(pStart->GetFVal(), true);
		}

		break;
	}
}

//------------------------------------------------------------------------
IActor *CGameRules::GetActorByChannelId(int channelId) const
{
	return gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActorByChannelId(channelId);
}

//------------------------------------------------------------------------
IActor *CGameRules::GetActorByEntityId(EntityId entityId) const
{
	return gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
}

const char *CGameRules::GetActorName(IActor *pActor)
{
	if (pActor == nullptr)
		return "<unknown>";
	
	return pActor->GetEntity()->GetName();
}

//------------------------------------------------------------------------
bool CGameRules::ShouldKeepClient(int channelId, EDisconnectionCause cause, const char *desc) const
{
	return (!strcmp("timeout", desc) || cause==eDC_Timeout);
}

//------------------------------------------------------------------------
bool CGameRules::OnClientConnect(int channelId, bool isReset)
{
	IActorSystem *pActorSystem = gEnv->pGame->GetIGameFramework()->GetIActorSystem();
	if(IActor *pActor = pActorSystem->GetActorByChannelId(channelId))
	{
		gEnv->pEntitySystem->RemoveEntity(pActor->GetEntityId());
		pActorSystem->RemoveActor(pActor->GetEntityId());
	}

	const char *playerName = "Player";
	// See Actor.cpp, defined in Game.cpp (CGame::RegisterEntities)
	const char *playerType = "SampleActor";

	if (IActor *pActor = pActorSystem->CreateActor(channelId, playerName, playerType, ZERO, IDENTITY, Vec3(1, 1, 1)))
	{
		// Hide spawned actors until the client *enters* the game
		pActor->GetEntity()->Hide(true);

		return true;
	}

	return false;
}

//------------------------------------------------------------------------
void CGameRules::OnClientDisconnect(int channelId, EDisconnectionCause cause, const char *desc, bool keepClient)
{
	IActor *pActor=GetActorByChannelId(channelId);
	
	if (!pActor)
		return;

	if (keepClient)
	{
		pActor->GetGameObject()->SetAspectProfile(eEA_Physics, eAP_NotPhysicalized);

		return;
	}

	gEnv->pGame->GetIGameFramework()->GetIActorSystem()->RemoveActor(pActor->GetEntityId());

	return;
}

//------------------------------------------------------------------------
bool CGameRules::OnClientEnteredGame(int channelId, bool isReset)
{ 
	IActor *pActor=GetActorByChannelId(channelId);
	if (!pActor)
		return false;

	// Ensure the actor is visible when entering the game (but not in the editor)
	if (!gEnv->IsEditing())
		pActor->GetEntity()->Hide(false);

	// Need to update the time of day serialization chunk so that the new client can start at the right point
	// Note: Since we don't generally have a dynamic time of day, this will likely only effect clients
	// rejoining after a host migration since they won't be loading the value from the level
	CHANGED_NETWORK_STATE(this, eEA_GameServerDynamic);
	CHANGED_NETWORK_STATE(this, eEA_GameServerStatic);

	// TODO: Notify remote client that they've entered game
	//if(gEnv->bMultiplayer)
		//GetGameObject()->InvokeRMIWithDependentObject(ClEnteredGame(), SChannelParams(channelId), eRMI_ToAllClients, pActor->GetEntityId());

	return true;
}

//------------------------------------------------------------------------
void CGameRules::ShowStatus()
{
	float timeRemaining = GetRemainingGameTime();
	int mins = (int)(timeRemaining / 60.0f);
	int secs = (int)(timeRemaining - mins*60);
	CryLogAlways("time remaining: %d:%02d", mins, secs);
}

//------------------------------------------------------------------------
void CGameRules::GetMemoryUsage(ICrySizer * s) const
{
	s->Add(*this);
}

//------------------------------------------------------------------------
bool CGameRules::NetSerialize( TSerialize ser, EEntityAspects aspect, uint8 profile, int flags )
{
		switch (aspect)
		{
		case eEA_GameServerDynamic:
			{	
				uint32 todFlags = 0;
				if (ser.IsReading())
				{
						todFlags |= ITimeOfDay::NETSER_COMPENSATELAG;
						if (!m_timeOfDayInitialized)
						{
								todFlags |= ITimeOfDay::NETSER_FORCESET;
								m_timeOfDayInitialized = true;
						}
				}
				gEnv->p3DEngine->GetTimeOfDay()->NetSerialize( ser, 0.0f, todFlags );
			}
			break;
		case eEA_GameServerStatic:
			{
				gEnv->p3DEngine->GetTimeOfDay()->NetSerialize( ser, 0.0f, ITimeOfDay::NETSER_STATICPROPS );
			}
			break;
		}

		return true;
}