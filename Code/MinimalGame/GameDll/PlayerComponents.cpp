#include "StdAfx.h"
#include "PlayerComponents.h"

#include "Actor.h"

void CPlayerComponent_PrePhysicsUpdate::Initialize(const SComponentInitializer& init)
{
	m_pEntity = init.m_pEntity;

	auto initEx = static_cast<const SComponentInitializerEx&>(init);
	m_pPlayer = initEx.m_pPlayer;

	RegisterEvent(ENTITY_EVENT_PREPHYSICSUPDATE, IComponent::EComponentFlags_Enable);
}

IComponent::ComponentEventPriority CPlayerComponent_PrePhysicsUpdate::GetEventPriority(const int eventID) const
{
	switch (eventID)
	{
		case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			int priority = ENTITY_PROXY_LAST - ENTITY_PROXY_USER + EEntityEventPriority_PrepareAnimatedCharacterForUpdate;

			if (m_pPlayer->IsClient())
				priority += EEntityEventPriority_Client;

			return priority;
		}
	}

	return(ENTITY_PROXY_LAST - ENTITY_PROXY_USER);
}

void CPlayerComponent_PrePhysicsUpdate::ProcessEvent(SEntityEvent &event)
{
	switch (event.event)
	{
		case ENTITY_EVENT_PREPHYSICSUPDATE:
		{
			// Send prephysics update event to the actor
			// Movement update will be done here
			m_pPlayer->PrePhysicsUpdate();
		}
		break;
	}
}