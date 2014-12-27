#include "StdAfx.h"
#include "Actor.h"

#include <IViewSystem.h>

void CActor::UpdateView(SViewParams &viewParams)
{
	// Default to 60 field of view
	// Note how we have to convert from degrees to radians
	viewParams.fov = DEG2RAD(60);

	IEntity *pEntity = GetEntity();
	Matrix34 actorTM = pEntity->GetWorldTM();

	viewParams.position = actorTM.GetTranslation();
	viewParams.rotation = Quat(actorTM);
}