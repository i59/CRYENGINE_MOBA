#pragma once

#include <IGameObject.h>

class CNativeEntity
	: public CGameObjectExtensionHelper<CNativeEntity, IGameObjectExtension>
{
public:
	CNativeEntity();
	~CNativeEntity();

	// IGameObjectExtension
	virtual void GetMemoryUsage(ICrySizer *pSizer) const {}

	virtual bool Init(IGameObject *pGameObject);
	virtual void PostInit(IGameObject *pGameObject) {}

	virtual void InitClient(int channelId) {}
	virtual void PostInitClient(int channelId) {}

	virtual bool ReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) { return true; }
	virtual void PostReloadExtension(IGameObject * pGameObject, const SEntitySpawnParams &params) {}

	virtual bool GetEntityPoolSignature(TSerialize signature) { return true; }

	virtual void Release() { delete this; }

	virtual void FullSerialize(TSerialize ser) {}
	virtual bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int pflags) { return true; }
	virtual void PostSerialize() {}

	virtual void SerializeSpawnInfo(TSerialize ser) {}
	virtual ISerializableInfoPtr GetSpawnInfo() { return nullptr; }

	virtual void Update(SEntityUpdateContext& ctx, int updateSlot) {}

	virtual void HandleEvent(const SGameObjectEvent& event) {}
	virtual void ProcessEvent(SEntityEvent& event) {}

	virtual void SetChannelId(uint16 id) {}

	virtual void SetAuthority(bool auth) {}

	virtual void PostUpdate(float frameTime) {}

	virtual void PostRemoteSpawn() {}
	// ~IGameObjectExtension

	const char *GetPropertyValue(int index);
	void SetPropertyValue(int index, const char *value);

	float GetPropertyFloat(int index);
	int GetPropertyInt(int index);
	ColorF GetPropertyColor(int index);
	bool GetPropertyBool(int index);
	Vec3 GetPropertyVec3(int index);

	void SetPropertyFloat(int index, float value);
	void SetPropertyInt(int index, int value);
	void SetPropertyBool(int index, bool value);
	void SetPropertyVec3(int index, Vec3 value);
};

