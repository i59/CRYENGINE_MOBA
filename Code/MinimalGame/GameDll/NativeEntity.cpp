#include "StdAfx.h"
#include "NativeEntity.h"

#include "GameFactory.h"

#include "GameStringUtils.h"

CNativeEntity::CNativeEntity()
{
}

CNativeEntity::~CNativeEntity()
{
}

bool CNativeEntity::Init(IGameObject *pGameObject)
{
	SetGameObject(pGameObject);

	return true;
}

const char *CNativeEntity::GetPropertyValue(int index)
{
	IEntity *pEntity = GetEntity();
	if (pEntity == nullptr)
		return "";

	IEntityClass *pEntityClass = pEntity->GetClass();

	IEntityPropertyHandler *pPropertyHandler = pEntityClass->GetPropertyHandler();
	if (pPropertyHandler)
	{
		return pPropertyHandler->GetProperty(pEntity, index);
	}

	return "";
}

void CNativeEntity::SetPropertyValue(int index, const char *value)
{
	IEntity *pEntity = GetEntity();
	if (pEntity == nullptr)
		return;

	IEntityClass *pEntityClass = pEntity->GetClass();

	IEntityPropertyHandler *pPropertyHandler = pEntityClass->GetPropertyHandler();
	if (pPropertyHandler)
	{
		pPropertyHandler->SetProperty(pEntity, index, value);
	}
}

float CNativeEntity::GetPropertyFloat(int index)
{
	return (float)atof(GetPropertyValue(index));
}

int CNativeEntity::GetPropertyInt(int index)
{
	return atoi(GetPropertyValue(index));
}

ColorF CNativeEntity::GetPropertyColor(int index)
{
	return StringToColor(GetPropertyValue(index), false);
}

bool CNativeEntity::GetPropertyBool(int index)
{
	return GetPropertyInt(index) != 0;
}

Vec3 CNativeEntity::GetPropertyVec3(int index)
{
	return StringToVec3(GetPropertyValue(index));
}

void CNativeEntity::SetPropertyFloat(int index, float value)
{
	string valueString;
	valueString.Format("%f", value);

	SetPropertyValue(index, valueString.c_str());
}

void CNativeEntity::SetPropertyInt(int index, int value)
{
	string valueString;
	valueString.Format("%i", value);

	SetPropertyValue(index, valueString.c_str());
}

void CNativeEntity::SetPropertyBool(int index, bool value)
{
	string valueString;
	valueString.Format("%i", value ? "1" : "0");

	SetPropertyValue(index, valueString.c_str());
}

void CNativeEntity::SetPropertyVec3(int index, Vec3 value)
{
	string valueString;
	valueString.Format("%f,%f,%f", value.x, value.y, value.z);

	SetPropertyValue(index, valueString.c_str());
}