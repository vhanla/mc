/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <string>
#include <string.h>

#include "mc.h"
#include "McProps.h"

McGroup::McGroup(char *_groupName)
{
	groupName.assign(_groupName);
}

void McGroup::add(char *_progName)
{
	string progName(_progName);
	stringList.push_front(progName);
}

McProps::McProps()
{
	paddingFactor = 0.15;
	spacingFactor = 0.015;
	animationDuration = 0.3;
	aspectWeight = 0.2;
	desktopPadding = 0.1;
}

McProps::~McProps()
{
	for (list<McGroup*>::iterator g_it = masterList.begin(); g_it != masterList.end(); g_it++)
		delete (*g_it);
}

BOOL McProps::getIsExcluded(const char *_progName)
{
	for (list<string>::iterator s_it = excludeList.begin(); s_it != excludeList.end(); s_it++)
		if (0 == _stricmp(_progName, (*s_it).c_str()))
			return TRUE;
	return FALSE;
}

const char*  McProps::getGroupName(const char *_progName)
{
	for (list<McGroup*>::iterator g_it = masterList.begin(); g_it != masterList.end(); g_it++)
	{
		list<string> sList = (*g_it)->stringList;
		for (list<string>::iterator s_it = sList.begin(); s_it != sList.end(); s_it++)
			if (0 == _stricmp(_progName, (*s_it).c_str()))
				return (*g_it)->groupName.c_str();
	}
	return _progName;
}

McGroup* McProps::getGroup(char *_grpName)
{
	for (list<McGroup*>::iterator g_it = masterList.begin(); g_it != masterList.end(); g_it++)
		if (0 == _stricmp((*g_it)->groupName.c_str(), _grpName))
			return (*g_it);
	McGroup *g = new McGroup(_grpName);
	masterList.push_back(g);
	return g;
}

// REALLY UGLY!

void McProps::read()
{
	FILE *f;
	if (0 != fopen_s(&f, "mc.cfg", "r"))
		return;

	while (!feof(f))
	{
		char lineb[255];
		char *line = fgets(lineb, 255, f);
		if (line == NULL)
			continue;

		char *pch = strchr(line, '#');
		if (pch != NULL)
			*pch = 0;


		pch = strtok_s(line, " ,=\t", &token_holder);
		if (pch == NULL)
		{
			continue;
		}

		if (0 == _stricmp(pch, "value"))
			scanValue();
		else if (0 == _stricmp(pch, "group"))
			scanGroup();
		else if (0 == _stricmp(pch, "exclude"))
			scanExclude();
	}

	fclose(f);
}

void McProps::scanValue()
{
	char nbuff[255];
	char *name = strtok_s(NULL, " ,\t", &token_holder);
	if (name == NULL)
	{
		MC::Log("*** Expected setting name, didn't find.\n");
		return;
	}
	strcpy_s(nbuff, sizeof(nbuff), name);

	char *value = strtok_s(NULL, " ,=\t", &token_holder);
	if (value == NULL)
	{
		MC::Log("*** Expected setting value, didn't find.\n");
		return;
	}

	double val = atof(value);

	if (0 == _stricmp(name, "paddingfactor")) paddingFactor = val;
	else if (0 == _stricmp(name, "spacingFactor")) spacingFactor = val;
	else if (0 == _stricmp(name, "animationduration")) animationDuration = val;
	else if (0 == _stricmp(name, "aspectweight")) aspectWeight = val;
	else if (0 == _stricmp(name, "desktoppadding"))
	{
		desktopPadding = max(0.05, val);
		desktopPadding = min(val, 0.2);
	}
}

void McProps::scanExclude()
{

	char *ptr = token_holder;

	while (NULL != *ptr)
	{
		char *pattern;

		if (*ptr == '"')
		{
			token_holder++;
			pattern = "\"";
		}
		else if (isalpha(*ptr))
			pattern = " ,\t\n";
		else if (NULL == *ptr)
			break;
		else
		{
			ptr++;
			continue;
		}

		ptr = strtok_s(NULL, pattern, &token_holder);

		string ev(ptr);
		excludeList.push_back(ev);

		ptr = token_holder;
	}
}

void McProps::scanGroup()
{
	char *name = strtok_s(NULL, " ,\"\t", &token_holder);
	if (name == NULL)
	{
		MC::Log("*** Expected group name, didn't find.\n");
		return;
	}

	McGroup *g = this->getGroup(name);

	char *ptr = token_holder;

	while (NULL != *ptr)
	{
		char *pattern;

		if (*ptr == '"')
		{
			token_holder++;
			pattern = "\"";
		}
		else if (isalpha(*ptr))
			pattern = " ,\t\n";
		else if (NULL == *ptr)
			break;
		else
		{
			ptr++;
			continue;
		}

		ptr = strtok_s(NULL, pattern, &token_holder);

		g->add(ptr);
		ptr = token_holder;
	}
}