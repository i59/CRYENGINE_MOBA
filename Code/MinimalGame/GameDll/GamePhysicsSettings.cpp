/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2012.
-------------------------------------------------------------------------
History:
- 04:05:2012: Created by Stan Fichele

*************************************************************************/
#include "StdAfx.h"
#include "GamePhysicsSettings.h"
#include <BitFiddling.h>
#include "Utility/AutoEnum.h"

AUTOENUM_BUILDNAMEARRAY(s_collision_class_names, COLLISION_CLASSES);
AUTOENUM_BUILDNAMEARRAY(s_game_collision_class_names, GAME_COLLISION_CLASSES);

const int k_num_collision_classes = sizeof(s_collision_class_names)/sizeof(s_collision_class_names[0]);
const int k_num_game_collision_classes = sizeof(s_game_collision_class_names)/sizeof(s_game_collision_class_names[0]);;

const char* CGamePhysicsSettings::GetCollisionClassName(unsigned int bitIndex)
{
	return (bitIndex<MAX_COLLISION_CLASSES) ? m_names[bitIndex] : "";
}


int CGamePhysicsSettings::GetBit(uint32 a)
{
	int bit = IntegerLog2(a);
	bool valid = a!=0 && ((a-1)&a)==0;
	return valid ? bit : MAX_COLLISION_CLASSES;
}

void CGamePhysicsSettings::Init()
{
	COMPILE_TIME_ASSERT((k_num_collision_classes+k_num_game_collision_classes)<=MAX_COLLISION_CLASSES);

	// Automatically construct a list of string names for the collision clas enums

	for (int i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		m_names[i] = "";
		m_classIgnoreMap[i] = 0;
	}

	#define GP_ASSIGN_NAME(a,...) m_names[GetBit(a)] = #a;
	#define GP_ASSIGN_NAMES(list) list(GP_ASSIGN_NAME)
	GP_ASSIGN_NAMES(COLLISION_CLASSES);

	#undef GP_ASSIGN_NAME
	#define GP_ASSIGN_NAME(a,b,...) m_names[GetBit(b)] = #a;
	GP_ASSIGN_NAMES(GAME_COLLISION_CLASSES);

	// Set up the default ignore flags.
	SetIgnoreMap(gcc_player_all, gcc_ragdoll);
}

void CGamePhysicsSettings::ExportToLua()
{
	// Export enums to lua and construct a global table g_PhysicsCollisionClass
	
	IScriptSystem * pScriptSystem = gEnv->pScriptSystem;
	IScriptTable* physicsCollisionClassTable = pScriptSystem->CreateTable();
	physicsCollisionClassTable->AddRef();
	physicsCollisionClassTable->BeginSetGetChain();
	for (int i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		if (m_names[i][0])
		{
			if (i>=23)
				CryFatalError("LUA can't support flags beyond bit 23 due to using floats");

			stack_string name;
			name.Format("bT_%s", m_names[i]);  // Annoyingly we need to prefix with a b to make it a bool in lua
			physicsCollisionClassTable->SetValueChain(name.c_str(), 1<<i);
		}
	}
	physicsCollisionClassTable->EndSetGetChain();
	pScriptSystem->SetGlobalValue("g_PhysicsCollisionClass", physicsCollisionClassTable);
	physicsCollisionClassTable->Release();
	
	for (int i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		if (m_names[i][0])
			pScriptSystem->SetGlobalValue(m_names[i], 1<<i);
	}

#undef GP_ASSIGN_NAME
#undef GP_ASSIGN_NAMES

#define GP_ASSIGN_NAMES(list) list(GP_ASSIGN_NAME)
#define GP_ASSIGN_NAME(a,b,...) pScriptSystem->SetGlobalValue(#a, b);
	GP_ASSIGN_NAMES(GAME_COLLISION_CLASS_COMBOS);

#undef GP_ASSIGN_NAME
#undef GP_ASSIGN_NAMES

}

void CGamePhysicsSettings::AddIgnoreMap( uint32 gcc_classTypes, const uint32 ignoreClassTypesOR, const uint32 ignoreClassTypesAND )
{
	for(int i=0; i<MAX_COLLISION_CLASSES && gcc_classTypes; i++,gcc_classTypes>>=1)
	{
		if(gcc_classTypes&0x1)
		{
			m_classIgnoreMap[i] |= ignoreClassTypesOR;
			m_classIgnoreMap[i] &= ignoreClassTypesAND;
		}
	}
}

void CGamePhysicsSettings::SetIgnoreMap( uint32 gcc_classTypes, const uint32 ignoreClassTypes )
{
	AddIgnoreMap( gcc_classTypes, ignoreClassTypes, ignoreClassTypes );
}

void CGamePhysicsSettings::SetCollisionClassFlags( IPhysicalEntity& physEnt, uint32 gcc_classTypes, const uint32 additionalIgnoreClassTypesOR /*= 0*/, const uint32 additionalIgnoreClassTypesAND /*= 0xFFFFFFFF */ )
{
	const uint32 defaultIgnores = GetIgnoreTypes(gcc_classTypes); 
	pe_params_collision_class gcc_params;
	gcc_params.collisionClassOR.type = gcc_params.collisionClassAND.type = gcc_classTypes;
	gcc_params.collisionClassOR.ignore = gcc_params.collisionClassAND.ignore = (defaultIgnores|additionalIgnoreClassTypesOR)&additionalIgnoreClassTypesAND;
	physEnt.SetParams(&gcc_params);
}

void CGamePhysicsSettings::AddCollisionClassFlags( IPhysicalEntity& physEnt, uint32 gcc_classTypes, const uint32 additionalIgnoreClassTypesOR /*= 0*/, const uint32 additionalIgnoreClassTypesAND /*= 0xFFFFFFFF */ )
{
	const uint32 defaultIgnores = GetIgnoreTypes(gcc_classTypes); 
	pe_params_collision_class gcc_params;
	gcc_params.collisionClassOR.type = gcc_classTypes;
	gcc_params.collisionClassOR.ignore = defaultIgnores|additionalIgnoreClassTypesOR;
	gcc_params.collisionClassAND.ignore = additionalIgnoreClassTypesAND;
	physEnt.SetParams(&gcc_params);
}

uint32 CGamePhysicsSettings::GetIgnoreTypes( uint32 gcc_classTypes ) const
{
	uint32 ignoreTypes = 0;
	for(int i=0; i<MAX_COLLISION_CLASSES && gcc_classTypes; i++,gcc_classTypes>>=1)
	{
		if(gcc_classTypes&0x1)
		{
			ignoreTypes |= m_classIgnoreMap[i];
		}
	}
	return ignoreTypes;
}