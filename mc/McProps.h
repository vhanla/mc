/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <list>
#include <string>
#include <string.h>

using namespace std;

class McGroup
{
public:
	McGroup(char *_groupName);
	void add(char *_progName);
	string groupName;
	list<string> stringList;
};

class McProps
{
public:
	McProps();
	~McProps();

	double getAspectWeight() { return aspectWeight; }
	double getPaddingFactor() { return paddingFactor; }
	double getSpacingFactor() { return spacingFactor; }
	double getAnnimationDuration() { return animationDuration; }
	double getDesktopPadding() { return desktopPadding; }

	BOOL		getIsExcluded(const char *_progName);
	McGroup*	getGroup(char *_grpName);
	const char*	getGroupName(const char *_progName);

	void read();
private:
	double paddingFactor;
	double spacingFactor;
	double animationDuration;
	double aspectWeight;
	double desktopPadding;

	list<McGroup *> masterList;
	list<string>	excludeList;

	void scanValue();
	void scanGroup();
	void scanExclude();
	char *token_holder;
};