/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////
#include <list>

using namespace std;

class PileItem
{
public:
	PileItem(WindowItem *_wItem);
	~PileItem();

	McRect		originalR;
	McRect		boundedR;

	WindowItem*	wItem;
};

class WindowList;

class McPile
{
public:
	McPile(const char *);
	~McPile();

	void add(WindowItem *wItem);
	void doLayout();
	void positionThumbs(double _sRatio);
	BOOL matches(const char *);

	McRect	boundingR;

	McRect	placementR;
	BOOL	filledFirst;
	BOOL	filledFinal;

	int		getPileNumber() { return pileNumber; }
	void	setPileNumber(int _pileNumber) { pileNumber = _pileNumber; }
	void	setPileNumbers();

	char*	pName;

	int		row;

	double	sRatio;
private:
	list<PileItem *>iList;
	list<PileItem *>::iterator it;

	WindowList *wList;
	int			pileNumber;
};

class McPiles
{
public:
	McPiles();
	~McPiles();

	void createPiles();
	void layoutPiles();
	void positionThumbs();
private:
	list<McPile*>			pList;
	list<McPile*>::iterator	it;

	McRect placementR;
	McRect boundingR;

	BOOL checkOverlap(McRect *);
	void tuneLayout();

	BOOL placeNextWindow(McPile *);
	void placeWindow(McPile *);

	LONG wSpacing;
	double xyAspect;
	double sRatio;

	list<McPile*> placedPiles;
public:
	static int getNextPileNumber() { return nextPileNumber++; }
private:
	static int nextPileNumber;
};

