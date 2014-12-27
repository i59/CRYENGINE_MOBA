#pragma once

#include <IComponent.h>

class CActor;

class CPlayerComponent_PrePhysicsUpdate : public IComponent
{
public:
	struct SComponentInitializerEx : public SComponentInitializer
	{
		SComponentInitializerEx(IEntity *pEntity, CActor *pPlayer)
			: SComponentInitializer(pEntity)
			, m_pPlayer(pPlayer)
		{
		}

		CActor *m_pPlayer;
	};

	// IComponent
	virtual void Initialize(const SComponentInitializer& init);

	virtual IComponent::ComponentEventPriority GetEventPriority(const int eventID) const;

	virtual	void ProcessEvent(SEntityEvent &event);
	// ~IComponent

protected:
	IEntity *m_pEntity;
	CActor *m_pPlayer;
};