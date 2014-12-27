#include "StdAfx.h"
#include <StringUtils.h>
#include "Game.h"
#include "GameCVars.h"

#include "Actor.h"
#include "ISerialize.h"
#include "GameUtils.h"
#include <ICryAnimation.h>
#include <IGameTokens.h>
#include "GameRules.h"
#include <IMaterialEffects.h>

#include "PlayerComponents.h"

#include "HUD/UIManager.h"

#include "IAgent.h"

#include "IFacialAnimation.h"
#include "IAIActor.h"

//------------------------------------------------------------------------
CActor::CActor()
	: m_isClient(false)
	, m_isMigrating(false)
{
}

//------------------------------------------------------------------------
CActor::~CActor()
{
	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(0);
	if (pCharacter)
		pCharacter->GetISkeletonPose()->SetPostProcessCallback(0, 0);

	for(int i = 0; i < GetEntity()->GetSlotCount(); i++)
		GetEntity()->FreeSlot(i);

	GetGameObject()->EnablePhysicsEvent(false, eEPE_OnPostStepImmediate);

	GetGameObject()->ReleaseView(this);

	if(g_pGame && g_pGame->GetIGameFramework() && g_pGame->GetIGameFramework()->GetIActorSystem())
		g_pGame->GetIGameFramework()->GetIActorSystem()->RemoveActor(GetEntityId());

	if(m_pComponentPrePhysicsUpdate)
		GetEntity()->RegisterComponent(m_pComponentPrePhysicsUpdate, EComponentFlags_Disable);
}

//-----------------------------------------------------------------------
void CActor::GetMemoryUsage(ICrySizer *s) const
{
	s->Add(*this);
}

//------------------------------------------------------------------------
bool CActor::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	if (!GetGameObject()->CaptureView(this))
		return false;

	IEntity *pEntity = GetEntity();

	g_pGame->GetIGameFramework()->GetIActorSystem()->AddActor(pEntity->GetId(), this);

	GetGameObject()->EnablePhysicsEvent(true, eEPE_OnPostStepImmediate);

	GetGameObject()->EnablePrePhysicsUpdate(ePPU_Always);

	if (!GetGameObject()->BindToNetwork())
		return false;

	GetEntity()->SetFlags(pEntity->GetFlags() | (ENTITY_FLAG_ON_RADAR | ENTITY_FLAG_CUSTOM_VIEWDIST_RATIO | ENTITY_FLAG_TRIGGER_AREAS | ENTITY_FLAG_UPDATE_HIDDEN));
	
	GetGameObject()->EnablePostUpdates(this);

	if (IEntityRenderProxy* pProxy = (IEntityRenderProxy*)pEntity->GetProxy(ENTITY_PROXY_RENDER))
	{
		if (IRenderNode* pRenderNode = pProxy->GetRenderNode())
			pRenderNode->SetRndFlags(ERF_REGISTER_BY_POSITION, true);
	}

	pEntity->PrePhysicsActivate(true);

	return true;
}

void CActor::PostInit( IGameObject * pGameObject )
{
	pGameObject->EnableUpdateSlot(this, 0);	
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_Always);
	pGameObject->EnablePostUpdates(this);

	const int requiredEvents[] = { eCGE_Ragdoll, eCGE_EnablePhysicalCollider, eCGE_DisablePhysicalCollider, eGFE_BecomeLocalPlayer, eGFE_OnCollision };
	GetGameObject()->RegisterExtForEvents(this, requiredEvents, sizeof(requiredEvents) / sizeof(int));

	m_pComponentPrePhysicsUpdate = ComponentCreateAndRegister<CPlayerComponent_PrePhysicsUpdate>(CPlayerComponent_PrePhysicsUpdate::SComponentInitializerEx(GetEntity(), this));
}

//------------------------------------------------------------------------
void CActor::Revive()
{
	SetHealth(PLAYER_MAX_HEALTH);

	IEntity *pEntity = GetEntity();

	if(pEntity->IsHidden())
		pEntity->Hide(false);

	pEntity->SetFlags(pEntity->GetFlags() | (ENTITY_FLAG_CASTSHADOW));
	pEntity->SetSlotFlags(0, pEntity->GetSlotFlags(0) | ENTITY_SLOT_RENDER);
	pEntity->PrePhysicsActivate(true);
	
	SetActorModel(); // set the model before physicalizing

	// Disable motion blur
	// SetMotionBlur is a bit confusing, true means it'll be disabled.
	if(IEntityRenderProxy *pRenderProxy = static_cast<IEntityRenderProxy *>(pEntity->GetProxy(ENTITY_PROXY_RENDER)))
		pRenderProxy->SetMotionBlur(true);

	if (gEnv->bServer)
		GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Alive);

	// Reset physical state, if it exists
	if (IPhysicalEntity *pPhysics = pEntity->GetPhysics())
	{
		pe_action_reset resetAction;
		pPhysics->Action(&resetAction);
	}

	if (IsClient())
		gEnv->p3DEngine->ResetPostEffects();

	if (ICharacterInstance *pCharacter = pEntity->GetCharacter(0))
	{
		pCharacter->EnableProceduralFacialAnimation(GetMaxHealth() > 0);
		pCharacter->EnableStartAnimation(true);
	}
}

//------------------------------------------------------------------------
void CActor::Despawn()
{
	IEntity *pEntity = GetEntity();

	ResetActorModel();

	pEntity->Hide(true);
}

//------------------------------------------------------------------------
void CActor::Physicalize()
{
	IEntity *pEntity = GetEntity();
	ICharacterInstance *pCharacter = pEntity->GetCharacter(0);

	// Do the actual physicalization, is specific per actor type so split into a separate function.
	DoPhysicalize();

	if(pEntity->GetPhysics() == nullptr)
		CryFatalError("Failed to physicalize player");

	PostPhysicalize();
}

//------------------------------------------------------------------------
void CActor::Dephysicalize()
{
	IEntity *pEntity = GetEntity();

	if (pEntity->GetPhysics())
	{
		SEntityPhysicalizeParams nop;
		nop.type = PE_NONE;
		pEntity->Physicalize(nop);
	}
}

//------------------------------------------------------------------------
void CActor::PostPhysicalize()
{
	IEntity *pEntity = GetEntity();

	GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameServerDynamic | eEA_GameClientStatic | eEA_GameServerStatic);
	
	// TODO: Initialize Mannequin here
}

//------------------------------------------------------------------------
void CActor::Ragdollize()
{
	if (GetLinkedVehicle())
		return;

	GetGameObject()->SetAutoDisablePhysicsMode(eADPM_Never);

	ICharacterInstance *pCharacter = GetEntity()->GetCharacter(0);
	if (pCharacter)
	{
		// dead guys shouldn't blink
		pCharacter->EnableProceduralFacialAnimation(false);
		//Anton :: SetDefaultPose on serialization
		if(gEnv->pSystem->IsSerializingFile() && pCharacter->GetISkeletonPose())
			pCharacter->GetISkeletonPose()->SetDefaultPose();
	}

	SEntityPhysicalizeParams pp;

	pp.type = PE_ARTICULATED;
	pp.nSlot = 0;
	pp.bCopyJointVelocities = true;

	if(pp.mass <= 0)
		pp.mass = 80.0f; //never ragdollize without mass [Anton]

	pp.fStiffnessScale = 1200;

	GetEntity()->Physicalize(pp);
}

void CActor::ResetActorModel()
{
	IEntity *pEntity = GetEntity();

	for (int i = 0; i < pEntity->GetSlotCount(); i++)
		pEntity->FreeSlot(i);

	Dephysicalize();
}

void CActor::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_START_GAME:
		GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameServerDynamic | eEA_GameClientStatic | eEA_GameServerStatic);
		break;

		case ENTITY_EVENT_RESET:
		case ENTITY_EVENT_PRE_SERIALIZE:
		{
			GetGameObject()->RequestRemoteUpdate(eEA_Physics | eEA_GameClientDynamic | eEA_GameServerDynamic | eEA_GameClientStatic | eEA_GameServerStatic);
		}
		break;
	}  
}

void CActor::SetAuthority(bool auth)
{
	// we've been given authority of this entity, mark the physics as changed
	// so that we send a current position, failure to do this can result in server/client
	// disagreeing on where the entity is. most likely to happen on restart
	if (auth)
	{
		CHANGED_NETWORK_STATE(this, eEA_Physics);
	}
}

//------------------------------------------------------------------------
void CActor::RequestFacialExpression(const char* pExpressionName /* = NULL */, f32* sequenceLength /*= NULL*/)
{
	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	IFacialInstance* pFacialInstance = (pCharacter ? pCharacter->GetFacialInstance() : 0);
	IFacialAnimSequence* pSequence = (pFacialInstance ? pFacialInstance->LoadSequence(pExpressionName) : 0);
	if (pFacialInstance)
		pFacialInstance->PlaySequence(pSequence, eFacialSequenceLayer_AIExpression);
}

void CActor::PrecacheFacialExpression(const char* pExpressionName)
{
	ICharacterInstance* pCharacter = GetEntity()->GetCharacter(0);
	IFacialInstance* pFacialInstance = (pCharacter ? pCharacter->GetFacialInstance() : 0);
	if (pFacialInstance)
		pFacialInstance->PrecacheFacialExpression(pExpressionName);
}

bool CActor::IsLocalClient() const
{
	return GetEntityId() == gEnv->pGame->GetIGameFramework()->GetClientActorId();
}

bool CActor::PrePhysicsUpdate()
{
	FUNCTION_PROFILER(GetISystem(), PROFILE_GAME);

	IEntity *pEnt = GetEntity();
	if (pEnt->IsHidden() && !(GetEntity()->GetFlags() & ENTITY_FLAG_UPDATE_HIDDEN))
		return false;

	if(IsDead())
		return false;

	const float frameTime = gEnv->pTimer->GetFrameTime();

	// Do any custom physics handling here

	return true;
}

void CActor::InitLocalPlayer()
{
	GetGameObject()->SetUpdateSlotEnableCondition(this, 0, eUEC_Always);
}

void CActor::HandleEvent( const SGameObjectEvent& event )
{
	if (event.event == eCGE_Ragdoll)
	{
		GetGameObject()->SetAspectProfile(eEA_Physics, eAP_Ragdoll);
	}
	else if (event.event == eGFE_BecomeLocalPlayer)
	{
		IEntity *pEntity = GetEntity();
		pEntity->SetFlags(GetEntity()->GetFlags() | ENTITY_FLAG_TRIGGER_AREAS);
		// Invalidate the matrix in order to force an update through the area manager
		pEntity->InvalidateTM(ENTITY_XFORM_POS);

		m_isClient = true;
		GetGameObject()->EnablePrePhysicsUpdate( ePPU_Always );

		// always update client's character
		if (ICharacterInstance * pCharacter = GetEntity()->GetCharacter(0))
			pCharacter->SetFlags(pCharacter->GetFlags() | CS_FLAG_UPDATE_ALWAYS);
	}
	else if (event.event == eGFE_OnCollision)
	{
		EventPhysCollision *physCollision = reinterpret_cast<EventPhysCollision *>(event.ptr);

		// Handle collision events here
	}
}

bool CActor::GetEntityPoolSignature( TSerialize signature )
{
	signature.BeginGroup("Actor");
	signature.EndGroup();
	return true;
}

bool CActor::ReloadExtension( IGameObject *pGameObject, const SEntitySpawnParams &params )
{
	CRY_ASSERT(GetGameObject() == pGameObject);

	ResetGameObject();

	if (!GetGameObject()->CaptureView(this))
		return false;

	// Re-enable the physics post step callback and CollisionLogging (were cleared during ResetGameObject()).
	GetGameObject()->EnablePhysicsEvent( true, eEPE_OnPostStepImmediate | eEPE_OnCollisionLogged | eEPE_OnCollisionImmediate);

	if (!GetGameObject()->BindToNetwork())
		return false;

	g_pGame->GetIGameFramework()->GetIActorSystem()->RemoveActor(params.prevId);
	g_pGame->GetIGameFramework()->GetIActorSystem()->AddActor(GetEntityId(), this);

	return true;
}

void CActor::PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	CRY_ASSERT(GetGameObject() == pGameObject);

	GetGameObject()->EnablePrePhysicsUpdate(ePPU_Always);

	GetEntity()->SetFlags(GetEntity()->GetFlags() |
		(ENTITY_FLAG_ON_RADAR | ENTITY_FLAG_CUSTOM_VIEWDIST_RATIO | ENTITY_FLAG_TRIGGER_AREAS));
}