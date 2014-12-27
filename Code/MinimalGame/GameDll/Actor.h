#ifndef __Actor_H__
#define __Actor_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IActorSystem.h>

#define PLAYER_MAX_HEALTH 100

struct SMFXRunTimeEffectParams;

class IActionController;
struct SAnimationContext;

struct IAttachment;
struct IAttachmentObject;

class CPlayerComponent_PrePhysicsUpdate;

class CActor :
	public CGameObjectExtensionHelper<CActor, IActor>,
	public IGameObjectView
{
public:
	CActor();
	virtual ~CActor();

	// IActor
	virtual void GetMemoryUsage(ICrySizer * s) const;

	virtual void ProcessEvent(SEntityEvent& event);
	virtual IAnimatedCharacter * GetAnimatedCharacter() { return nullptr; }
	virtual const IAnimatedCharacter * GetAnimatedCharacter() const { return nullptr; }

	virtual void SetAuthority(bool auth);
	virtual void Release() { delete this; };
	virtual void ResetAnimGraph() {}
	virtual void NotifyAnimGraphTransition(const char *anim0){};
	virtual void NotifyAnimGraphInput(int id, const char *value) {};
	virtual void NotifyAnimGraphInput(int id, int value) {};
	virtual void FullSerialize(TSerialize ser) {}
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) { return true; }
	virtual void PostSerialize() {}
	virtual void SerializeSpawnInfo( TSerialize ser ) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return nullptr; }
	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) {}
	virtual void SetChannelId(uint16 id) {}
	virtual void  SerializeLevelToLevel( TSerialize &ser ) {}
	virtual void	SerializeXML( XmlNodeRef& node, bool bLoading ) {}

	virtual bool IsPlayer() const { return GetChannelId() != 0; }
	virtual bool IsClient() const { return m_isClient; }
	virtual bool IsMigrating() const { return m_isMigrating; }
	virtual void SetMigrating(bool isMigrating) { m_isMigrating = isMigrating; }
	virtual IMaterial *GetReplacementMaterial() { return nullptr; };

	virtual bool Init( IGameObject * pGameObject );
	virtual void InitClient( int channelId ) {}
	virtual void PostInit(IGameObject * pGameObject);
	virtual void PostInitClient(int channelId) {};
	virtual bool ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual void PostReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params );
	virtual bool GetEntityPoolSignature( TSerialize signature );

	virtual void InitLocalPlayer();

	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void PostUpdate(float frameTime) {}
	virtual void PostRemoteSpawn() {};

	virtual bool IsThirdPerson() const { return true; };
	virtual void ToggleThirdPerson(){}

	virtual void SetFacialAlertnessLevel(int alertness) {}
	virtual void RequestFacialExpression(const char* pExpressionName /* = NULL */, f32* sequenceLength /*= NULL*/);
	virtual void PrecacheFacialExpression(const char* pExpressionName);

	virtual void NotifyInventoryAmmoChange(IEntityClass* pAmmoClass, int amount) {}
	virtual EntityId	GetGrabbedEntityId() const { return 0; }

	virtual void HideAllAttachments(bool isHiding) {}

	virtual void SetIKPos(const char *pLimbName, const Vec3& goalPos, int priority) {}

	virtual void OnAIProxyEnabled(bool enabled) {};
	virtual void OnReturnedToPool() {};
	virtual void OnPreparedFromPool() {};

	virtual bool ShouldMuteWeaponSoundStimulus() const { return false; }

	virtual void OnReused(IEntity *pEntity, SEntitySpawnParams &params) {}
	// ~IActor

	// IGameObjectView
	virtual void UpdateView(SViewParams &viewParams) {}
	virtual void PostUpdateView(SViewParams &viewParams) {}
	// IGameObjectView

	// Handle physics / movement updates here
	bool PrePhysicsUpdate();

	// Called to revive the player into a playable state
	void Revive();

	// Remove the player from the world
	void Despawn();

	// Create the physical proxy, in order to allow for the player to collide
	void Physicalize();
	void DoPhysicalize() {}
	// Dephysicalize the player, ridding it of its physics proxy. Can not collide after this!
	void Dephysicalize();
	// Called after physicalization, animations etc should be initialized here
	void PostPhysicalize();

	// Put the actor character into a ragdoll state
	void Ragdollize();

	// Used to check if the actor is local, otherwise AI or remote MP player
	bool IsLocalClient() const;

protected:
	void ResetActorModel();
	virtual void SetActorModel(const char *filePath = nullptr) {}

protected:
	bool	m_isClient;
	bool	m_isMigrating;
	
	boost::shared_ptr<CPlayerComponent_PrePhysicsUpdate> m_pComponentPrePhysicsUpdate;
};

#endif //__Actor_H__