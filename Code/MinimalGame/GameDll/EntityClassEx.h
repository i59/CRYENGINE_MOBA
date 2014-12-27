#pragma once

#include <IEntityClass.h>
#include <IEntitySystem.h>

struct SEntityProperties
{
	SEntityProperties(IEntity *pEnt, int numProperties)
		: pEntity(pEnt) 
	{
		properties.resize(numProperties);
	}
	
	IEntity *pEntity;
	std::vector<string> properties;
};

class CEntityClassExPropertyHandler
	: public IEntityPropertyHandler
	, public IEntitySystemSink
{
public:
	CEntityClassExPropertyHandler(IEntityPropertyHandler::SPropertyInfo *pProperties, int numProperties, uint32 scriptFlags)
		: m_pProperties(pProperties)
		, m_numProperties(numProperties)
		, m_scriptFlags(scriptFlags) 
	{
		gEnv->pEntitySystem->AddSink(this, IEntitySystem::OnRemove, 0);
	}

	~CEntityClassExPropertyHandler()
	{
		delete[] m_pProperties;
	}

	// IEntityPropertyHandler interface
	virtual void GetMemoryUsage(ICrySizer *pSizer) const { pSizer->Add(m_pProperties); }
	virtual void RefreshProperties() {}
	virtual void LoadEntityXMLProperties(IEntity* entity, const XmlNodeRef& xml);
	virtual void LoadArchetypeXMLProperties(const char* archetypeName, const XmlNodeRef& xml) {}
	virtual void InitArchetypeEntity(IEntity* entity, const char* archetypeName, const SEntitySpawnParams& spawnParams) {}

	virtual int GetPropertyCount() const { return m_numProperties; }

	virtual bool GetPropertyInfo(int index, SPropertyInfo &info) const
	{
		if (index >= m_numProperties)
			return false;

		info = m_pProperties[index];
		return true;
	}

	virtual void SetProperty(IEntity* entity, int index, const char* value);

	virtual const char* GetProperty(IEntity* entity, int index) const;

	virtual const char* GetDefaultProperty(int index) const
	{
		if (index >= m_numProperties)
			return "";

		// TODO: Store default values for properties here
		return "";
		//return m_pProperties[index].defaultVal;
	}

	virtual void PropertiesChanged(IEntity *entity);

	virtual uint32 GetScriptFlags() const { return m_scriptFlags; }
	// -IEntityPropertyHandler

	// IEntitySystemSink
	virtual bool OnBeforeSpawn(SEntitySpawnParams &params) { return false; }
	virtual void OnSpawn(IEntity *pEntity, SEntitySpawnParams &params) {}
	virtual void OnReused(IEntity *pEntity, SEntitySpawnParams &params) {}
	virtual void OnEvent(IEntity *pEntity, SEntityEvent &event) {}
	virtual bool OnRemove(IEntity *pEntity);
	// ~IEntitySystemSink

	SEntityProperties *QueryEntityProperties(IEntity *pEntity);
	
	void LoadEntityXMLGroupProperties(IEntity *pEntity, const XmlNodeRef &groupNode, bool bRootNode);

protected:
	IEntityPropertyHandler::SPropertyInfo *m_pProperties;
	int m_numProperties;

	int m_scriptFlags;

	std::vector<SEntityProperties> m_entityProperties;
};