#pragma once

#include <Cry_Color.h>
#include <CryString.h>

#include <Cry_Math.h>

inline ColorF StringToColor(const char *sColor, bool adjustGamma)
{
	ColorF color(1.f);
	string colorString = sColor;

	for(int i = 0; i < 4; i++)
	{
		size_t pos = colorString.find_first_of(",");
		if(pos == string::npos)
			pos = colorString.size();

		const char *sToken = colorString.substr(0, pos);
		
		float fToken = (float)atof(sToken);

		// Convert to linear space
		if(adjustGamma)
			color[i] = powf(fToken / 255, 2.2f);
		else
			color[i] = fToken;

		if(pos == colorString.size())
			break;
		else
			colorString.erase(0, pos + 1);
	}

	return color;
}

inline Vec3 StringToVec3(const char *sVector)
{
	Vec3 v(ZERO);
	string vecString = sVector;

	for(int i = 0; i < 3; i++)
	{
		size_t pos = vecString.find_first_of(",");
		if(pos == string::npos)
			pos = vecString.size();

		const char *sToken = vecString.substr(0, pos);
		
		float fToken = (float)atof(sToken);

		v[i] = fToken;

		if(pos == vecString.size())
			break;
		else
			vecString.erase(0, pos + 1);
	}

	return v;
}