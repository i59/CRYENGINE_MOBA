/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2005.
  -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: Register the factory templates used to create classes from names
               e.g. REGISTER_FACTORY(pFramework, "Player", CPlayer, false);

               Since overriding this function creates template based linker errors,
               it's been replaced by a standalone function in its own cpp file.

  -------------------------------------------------------------------------
  History:
  - 17:8:2005   Created by Nick Hesketh - Refactor'd from Game.cpp/h

*************************************************************************/

#pragma once

#include "EntityClassEx.h"

#include <IGameFramework.h>

#include <IVehicleSystem.h>
#include <IGameRulesSystem.h>

#define HIDE_FROM_EDITOR(className)																																				\
  { IEntityClass *pItemClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(className);\
  pItemClass->SetFlags(pItemClass->GetFlags() | ECLF_INVISIBLE); }																				\

#define REGISTER_GAME_OBJECT(framework, name, script)\
	{\
		IEntityClassRegistry::SEntityClassDesc clsDesc;\
		clsDesc.sName = #name;\
		clsDesc.sScriptFile = script;\
		struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
			IGameObjectExtensionPtr Create()\
			{\
				return ComponentCreate_DeleteWithRelease<C##name>(); \
			}\
			void GetGameObjectExtensionRMIData( void ** ppRMI, size_t * nCount )\
			{\
			C##name::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		static C##name##Creator _creator;\
		framework->GetIGameObjectSystem()->RegisterExtension(#name, &_creator, &clsDesc);\
	}

#define REGISTER_GAME_OBJECT_EX(name, clsDesc, properties, numProperties, scriptFlags)\
	{\
		clsDesc.sName = #name;\
		struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
			IGameObjectExtensionPtr Create()\
			{\
				return ComponentCreate_DeleteWithRelease<C##name>(); \
			}\
			void GetGameObjectExtensionRMIData( void ** ppRMI, size_t * nCount )\
			{\
			C##name::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		\
		if(numProperties > 0)\
			clsDesc.pPropertyHandler = new CEntityClassExPropertyHandler(properties, numProperties, scriptFlags);\
		\
		static C##name##Creator _creator;\
		gEnv->pGame->GetIGameFramework()->GetIGameObjectSystem()->RegisterExtension(clsDesc.sName, &_creator, &clsDesc); \
	}

#define REGISTER_GAME_OBJECT_EXTENSION(framework, name)\
	{\
		struct C##name##Creator : public IGameObjectExtensionCreatorBase\
		{\
			IGameObjectExtensionPtr Create()\
			{\
			return ComponentCreate_DeleteWithRelease<C##name>(); \
			}\
			void GetGameObjectExtensionRMIData( void ** ppRMI, size_t * nCount )\
			{\
			C##name::GetGameObjectExtensionRMIData( ppRMI, nCount );\
			}\
		};\
		static C##name##Creator _creator;\
		framework->GetIGameObjectSystem()->RegisterExtension(#name, &_creator, NULL);\
	}

////////////////////////////////////////
// Property registration helpers
////////////////////////////////////////

// Used to aid registration of property groups
// Example use case:
// {
//		ENTITY_PROPERTY_GROUP("MyGroup", groupIndexStart, groupIndexEnd, pProperties);
// }
struct SEditorPropertyGroup
{
	SEditorPropertyGroup(const char *name, int indexBegin, int indexEnd, IEntityPropertyHandler::SPropertyInfo *pProperties)
	: m_name(name)
	, m_indexEnd(indexEnd)
	, m_pProperties(pProperties)
	{
		pProperties[indexBegin].name = name;
		pProperties[indexBegin].type = IEntityPropertyHandler::FolderBegin;
	}

	~SEditorPropertyGroup()
	{
		m_pProperties[m_indexEnd].name = m_name;
		m_pProperties[m_indexEnd].type = IEntityPropertyHandler::FolderEnd;
	}

protected:
	string m_name;

	int m_indexEnd;
	IEntityPropertyHandler::SPropertyInfo *m_pProperties;
};

#define ENTITY_PROPERTY_GROUP(name, begin, end, properties) SEditorPropertyGroup group = SEditorPropertyGroup(name, begin, end, properties);

// Entity registration helpers
template <typename T>
inline void RegisterEntityProperty(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	CRY_ASSERT_MESSAGE(false, "Entity property of invalid type was not registered");
}

template <>
inline void RegisterEntityProperty<Vec3>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::Vector;
	pProperties[index].description = desc;
	pProperties[index].editType = "";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}

template <>
inline void RegisterEntityProperty<ColorF>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::Vector;
	pProperties[index].description = desc;
	pProperties[index].editType = "color";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}

template <>
inline void RegisterEntityProperty<float>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::Float;
	pProperties[index].description = desc;
	pProperties[index].editType = "";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}

template <>
inline void RegisterEntityProperty<int>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::Int;
	pProperties[index].description = desc;
	pProperties[index].editType = "";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}

template <>
inline void RegisterEntityProperty<bool>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::Bool;
	pProperties[index].description = desc;
	pProperties[index].editType = "b";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = 0;
	pProperties[index].limits.max = 1;
}

template <>
inline void RegisterEntityProperty<string>(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::String;
	pProperties[index].description = desc;
	pProperties[index].editType = "";
	pProperties[index].flags = 0;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}

inline void RegisterEntityPropertyTexture(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::String;
	pProperties[index].description = desc;
	pProperties[index].editType = "tex";
	pProperties[index].flags = 0;
}

inline void RegisterEntityPropertyFlare(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::String;
	pProperties[index].description = desc;
	pProperties[index].editType = "flare_";
	pProperties[index].flags = 0;
}

inline void RegisterEntityPropertyObject(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc)
{
	pProperties[index].name = name;
	//pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::String;
	pProperties[index].description = desc;
	pProperties[index].editType = "obj";
	pProperties[index].flags = 0;
}

/*inline void RegisterEntityPropertyEnum(IEntityPropertyHandler::SPropertyInfo *pProperties, int index, const char *name, const char *defaultVal, const char *desc, float min, float max)
{
	pProperties[index].name = name;
	pProperties[index].defaultVal = defaultVal;
	pProperties[index].type = IEntityPropertyHandler::String;
	pProperties[index].description = desc;
	pProperties[index].editType = "";
	pProperties[index].flags = IEntityPropertyHandler::ePropertyFlag_UIEnum | IEntityPropertyHandler::ePropertyFlag_Unsorted;
	pProperties[index].limits.min = min;
	pProperties[index].limits.max = max;
}*/

#define ENTITY_PROPERTY(type, properties, name, defaultVal, desc, min, max) RegisterEntityProperty<type>(properties, eProperty_##name, #name, defaultVal, desc, min, max);