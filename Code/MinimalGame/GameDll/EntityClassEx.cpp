#include "StdAfx.h"
#include "EntityClassEx.h"

#include <IEntitySystem.h>

bool CEntityClassExPropertyHandler::OnRemove(IEntity *pEntity)
{
	for (auto it = m_entityProperties.begin(); it != m_entityProperties.end(); ++it)
	{
		if (it->pEntity == pEntity)
		{
			m_entityProperties.erase(it);
			return true;
		}
	}

	return true;
}

void CEntityClassExPropertyHandler::LoadEntityXMLProperties(IEntity *pEntity, const XmlNodeRef& xml)
{
	if(auto properties = xml->findChild("Properties"))
	{
		// Load default
		LoadEntityXMLGroupProperties(pEntity, properties, true);

		// Load folders
		for(int i = 0; i < properties->getChildCount(); i++)
		{
			XmlNodeRef groupNode = properties->getChild(i);
			LoadEntityXMLGroupProperties(pEntity, groupNode, false);
		}
	}

	PropertiesChanged(pEntity);
}

void CEntityClassExPropertyHandler::LoadEntityXMLGroupProperties(IEntity *pEntity, const XmlNodeRef &groupNode, bool bRootNode)
{
	const char *groupName = groupNode->getTag();
	bool bFoundGroup = bRootNode;

	for(int i = 0; i < GetPropertyCount(); i++)
	{
		SPropertyInfo info;
		GetPropertyInfo(i, info);

		if(!bFoundGroup)
		{
			if(!strcmp(info.name, groupName) && info.type == IEntityPropertyHandler::FolderBegin)
			{
				bFoundGroup = true;
				continue;
			}
		}
		else
		{
			if(info.type == IEntityPropertyHandler::FolderEnd || info.type == IEntityPropertyHandler::FolderBegin)
				break;

			for(int index = 0; index < groupNode->getNumAttributes(); index++)
			{
				const char *name;
				const char *value;

				groupNode->getAttributeByIndex(index, &name, &value);

				if(!strcmp(name, info.name))
				{
					SetProperty(pEntity, i, value);
					break;
				}
			}
		}
	}
}

void CEntityClassExPropertyHandler::SetProperty(IEntity *pIEntity, int index, const char *value)
{
	EntityId id = pIEntity->GetId();

	SEntityProperties *pEntityProperties = QueryEntityProperties(pIEntity);
	pEntityProperties->properties[index] = value;
}

const char *CEntityClassExPropertyHandler::GetProperty(IEntity *pIEntity, int index) const
{
	for(auto it = m_entityProperties.begin(); it != m_entityProperties.end(); ++it)
	{
		if(it->pEntity == pIEntity)
			return it->properties[index];
	}

	return GetDefaultProperty(index);
}

SEntityProperties *CEntityClassExPropertyHandler::QueryEntityProperties(IEntity *pEntity)
{
	for(auto it = m_entityProperties.begin(); it != m_entityProperties.end(); ++it)
	{
		if(it->pEntity == pEntity)
			return &(*it);
	}

	m_entityProperties.push_back(SEntityProperties(pEntity, m_numProperties));
	return &m_entityProperties.back();
}

void CEntityClassExPropertyHandler::PropertiesChanged(IEntity *pEntity)
{
	SEntityEvent propertiesChangedEvent = SEntityEvent(ENTITY_EVENT_EDITOR_PROPERTY_CHANGED);
	pEntity->SendEvent(propertiesChangedEvent);
}