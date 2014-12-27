#ifndef __GAMECVARS_H__
#define __GAMECVARS_H__

struct SCVars
{
	void InitCVars(IConsole *pConsole);
};

extern struct SCVars *g_pGameCVars;

#endif //__GAMECVARS_H__