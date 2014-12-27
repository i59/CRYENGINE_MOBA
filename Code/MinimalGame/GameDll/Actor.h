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
	virtual void SetHealth(float health) {}
	virtual float GetHealth() const { return PLAYER_MAX_HEALTH; }
	virtual int	GetHealthAsRoundedPercentage() const { return (int)((GetHealth() / GetMaxHealth()) * 100); }
	virtual void SetMaxHealth(float maxHealth) {}
	virtual float GetMaxHealth() const { return PLAYER_MAX_HEALTH; }
	virtual int	GetArmor() const { return 0; }
	virtual int	GetMaxArmor() const { return 0; }
	virtual int	GetTeamId() const { return 0; }

	virtual bool IsFallen() const { return false; }
	virtual bool IsDead() const { return GetHealth() <= 0; }
	virtual int	IsGod() { return 0; }
	virtual void Fall(Vec3 hitPos = Vec3(0,0,0)) {}
	virtual bool AllowLandingBob() { return false; }

	virtual void PlayAction(const char *action,const char *extension, bool looping=false) {}
	virtual IAnimationGraphState *GetAnimationGraphState() { return nullptr; }
	virtual void ResetAnimationState() {}

	virtual void CreateScriptEvent(const char *event,float value,const char *str = NULL) {}
	virtual bool BecomeAggressiveToAgent(EntityId entityID) { return false; }

	virtual void SetFacialAlertnessLevel(int alertness) {}
	virtual void RequestFacialExpression(const char* pExpressionName /* = NULL */, f32* sequenceLength /*= NULL*/);
	virtual void PrecacheFacialExpression(const char* pExpressionName);

	virtual EntityId	GetGrabbedEntityId() const { return 0; }

	virtual void HideAllAttachments(bool isHiding) {}

	virtual void SetIKPos(const char *pLimbName, const Vec3& goalPos, int priority) {}

	virtual void SetViewInVehicle(Quat viewRotation) {}
	virtual void SetViewRotation( const Quat &rotation ) {}
	virtual Quat GetViewRotation() const { return IDENTITY; }

	virtual bool IsFriendlyEntity(EntityId entityId, bool bUsingAIIgnorePlayer = true) const { return false; }

	virtual Vec3 GetLocalEyePos() const { return ZERO; }

	virtual void CameraShake(float angle,float shift,float duration,float frequency,Vec3 pos,int ID,const char* source="") {}

	virtual IItem* GetHolsteredItem() const { return nullptr; }
	virtual void HolsterItem(bool holster, bool playSelect = true, float selectSpeedBias = 1.0f, bool hideLeftHandObject = true) {}
	virtual IItem* GetCurrentItem(bool includeVehicle=false) const { return nullptr; }
	virtual bool DropItem(EntityId itemId, float impulseScale=1.0f, bool selectNext=true, bool byDeath=false) { return false; }
	virtual IInventory *GetInventory() const { return nullptr; }
	virtual void NotifyCurrentItemChanged(IItem* newItem) {}

	virtual IMovementController * GetMovementController() const { return nullptr; }

	virtual IEntity *LinkToVehicle(EntityId vehicleId) { return nullptr; }

	virtual IEntity* GetLinkedEntity() const { return nullptr; }

	virtual uint8 GetSpectatorMode() const { return 0; }

	virtual void GetMemoryUsage(ICrySizer * s) const;

	virtual void ProcessEvent(SEntityEvent& event);
	virtual IAnimatedCharacter * GetAnimatedCharacter() { return nullptr; }
	virtual const IAnimatedCharacter * GetAnimatedCharacter() const { return nullptr; }

	virtual void PlayExactPositioningAnimation( const char* sAnimationName, bool bSignal, const Vec3& vPosition, const Vec3& vDirection, float startWidth, float startArcAngle, float directionTolerance ) {}
	virtual void CancelExactPositioningAnimation() {}
	virtual void PlayAnimation( const char* sAnimationName, bool bSignal ) {}

	virtual void EnableTimeDemo( bool bTimeDemo ) {}

	virtual void SwitchDemoModeSpectator(bool activate) {}

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

	virtual const char* GetActorClassName() const { return "CActor"; }
	virtual ActorClass GetActorClass() const { return 0; }

	virtual const char* GetEntityClassName() const { return "CActor"; }

	virtual void HandleEvent( const SGameObjectEvent& event );
	virtual void PostUpdate(float frameTime) {}
	virtual void PostRemoteSpawn() {};

	virtual bool IsThirdPerson() const { return true; };
	virtual void ToggleThirdPerson(){}

	virtual IVehicle *GetLinkedVehicle() const { return nullptr; }

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