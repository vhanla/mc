/////////////////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT 2012 D. D. Steiner                                                            //
//                                                                                         //
// This work is licensed under a                                                           //
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.             //
// Terms of this license may be found at http://creativecommons.org/licenses/by-nc-sa/3.0/ //
// All other rights reserved.                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES

#include <Windows.h>
#include <list>
#include <cmath>
#include <string.h>

#include "mc.h"
#include "WindowList.h"
#include "WindowItem.h"
#include "McProps.h"

#include "McPiles.h"
using namespace std;

int McPiles::nextPileNumber = 0;

PileItem::PileItem(WindowItem *_wItem)
{
	wItem = _wItem;
	originalR.set(0, 0,
		wItem->getNormalRect()->getWidth(), wItem->getNormalRect()->getHeight());
}

PileItem::~PileItem()
{
}

McPile::McPile(const char *_pName)
{
	pName = new char[strlen(_pName) + 1];
	strcpy_s(pName, strlen(_pName) + 1, _pName);
	filledFirst = filledFinal = FALSE;
	row = 1;
	pileNumber = -1;  // We'll set this later
}

McPile::~McPile()
{
	delete pName;
	for (it = iList.begin(); it != iList.end();it++)
		delete *it;
}

BOOL McPile::matches(const char *_pName)
{
	return (0 == strcmp(pName, _pName));
}

void McPile::add(WindowItem *wItem)
{
	PileItem *pItem = new PileItem(wItem);
	iList.push_back(pItem);
}

void McPile::doLayout()
{
	if (1 == iList.size())
	{
		PileItem *pItem = *(iList.begin());
		boundingR.set(0, 0,
			pItem->wItem->getNormalRect()->getWidth(),
			pItem->wItem->getNormalRect()->getHeight());
		pItem->boundedR.set(&boundingR);
	}
	else
	{
		double nItem = iList.size();
		double nPad = floor(0.9 + log((double)nItem) / log(2.0));

		LONG maxW = 0;
		LONG maxH = 0;

		for (it = iList.begin(); it != iList.end(); it++)
		{
			maxW = max(maxW, (*it)->originalR.getWidth());
			maxH = max(maxH, (*it)->originalR.getHeight());
		}

		double paddingFactor = MC::getMC()->getProperties()->getPaddingFactor();
		double padX = floor(paddingFactor * maxW * nPad);
		double padY = floor(paddingFactor * maxH * nPad);
		double radius = (padX + padY) / 2.0;

		double width = maxW + padX;
		double height = maxH + padY;

		double radS = .5 * M_PI;
		double radI = 1.5 * M_PI / (nItem);

		boundingR.set((LONG)width, (LONG)height, 0, 0);
		double dIdx = 0;

		for (it = iList.begin(); it != iList.end(); it++)
		{
			PileItem *pItem = *it;
			double xM2 = 0.5 * (width + radius * cos(radS + dIdx*radI));
			double yM2 = 0.5 * (height - radius * sin(radS + dIdx*radI));
			double w2 = 0.5 *pItem->originalR.getWidth();
			double h2 = 0.5 * pItem->originalR.getHeight();

			pItem->boundedR.left = (LONG)(xM2 - w2);
			pItem->boundedR.top = (LONG)(yM2 - h2);
			pItem->boundedR.right = (LONG)(xM2 + w2);
			pItem->boundedR.bottom = (LONG)(yM2 + h2);

			boundingR.left = min(boundingR.left, pItem->boundedR.left);
			boundingR.top = min(boundingR.top, pItem->boundedR.top);
			boundingR.right = max(boundingR.right, pItem->boundedR.right);
			boundingR.bottom = max(boundingR.bottom, pItem->boundedR.bottom);

			dIdx += 1.0;
		}

		for (it = iList.begin(); it != iList.end(); it++)
		{
			PileItem *pItem = *it;
			pItem->boundedR.offset(-boundingR.left, -boundingR.top);
		}

		boundingR.offset(-boundingR.left, -boundingR.top);
	}

	if (85 < (rand() % 100))  // Mix it up occasionally
		for (it = iList.begin(); it != iList.end(); it++)
		{
			PileItem *pItem = *it;
			int oX = (boundingR.right - pItem->boundedR.right) - pItem->boundedR.left;
			pItem->boundedR.offset(oX, 0);
		}
}

void McPile::positionThumbs(double scaleRatio)
{
	for (it = iList.begin(); it != iList.end(); it++)
	{
		PileItem *pItem = *it;
		WindowItem *wItem = pItem->wItem;

		McRect tRect(&pItem->boundedR);
		tRect.scale(scaleRatio / sRatio);
		tRect.offset(placementR.left, placementR.top);

		wItem->setThumbRect(&tRect);
	}
}

void McPile::setPileNumbers()
{
	for (it = iList.begin(); it != iList.end(); it++)
	{
		WindowItem* wItem = (*it)->wItem;
		wItem->setPileNumber(pileNumber);
	}
}

McPiles::McPiles()
{
	nextPileNumber = 0;
}

McPiles::~McPiles()
{
	for (it = pList.begin(); it != pList.end(); it++)
		delete *it;
}

void McPiles::createPiles()
{

	WindowList *wList = MC::getMC()->getWindowList();

	for (WindowItem *wItem = wList->getFirstItem(); wItem; wItem = wList->getNextItem())
	{
		McPile *pile = nullptr;
		const char *group = MC::getMC()->getProperties()->getGroupName(wItem->getProgName()->c_str());
		for (it = pList.begin(); it != pList.end(); it++)
		{
			pile = *it;
			if (pile->matches(group))
				break;
		}

		if (it == pList.end())
		{
			pile = new McPile(group);
			pList.push_back(pile);
		}

		pile->add(wItem);
	}

	for (it = pList.begin(); it != pList.end(); it++)
		(*it)->doLayout();
}

BOOL tileCompare(McPile *first, McPile *second)
{
	return first->boundingR.getArea() > second->boundingR.getArea();
}

void McPiles::layoutPiles()
{
	MC *mc = MC::getMC();
	McRect *mdR = &mc->getDisplayGeometry()->mdR;
	pList.sort(tileCompare);
	wSpacing = (LONG)(mc->getProperties()->getSpacingFactor() * mdR->getWidth());

	placementR.set(mdR);
	placementR.scale(0.25);

	double monW = mdR->getWidth();
	double monH = mdR->getHeight();
	xyAspect = mc->getDisplayGeometry()->mdAspect;

	boundingR.clear();

	for (list<McPile *>::iterator l_it = pList.begin(); l_it != pList.end(); l_it++)
		placeWindow(*l_it);

	McRect bounds(placementR.right, placementR.bottom, placementR.left, placementR.top);

	for (it = pList.begin(); it != pList.end(); it++)
	{
		bounds.left = min(bounds.left, (*it)->placementR.left);
		bounds.top = min(bounds.top, (*it)->placementR.top);
	}

	for (it = pList.begin(); it != pList.end(); it++)
		(*it)->placementR.offset(-bounds.left, -bounds.top);

	sRatio = min(1.0, min(monW / placementR.right, monH / placementR.bottom));
	placementR.scale(sRatio);

	placementR.right = max(placementR.right, (LONG)monW);
	placementR.bottom = max(placementR.bottom, (LONG)monH);

	for (it = pList.begin(); it != pList.end(); it++)
		(*it)->placementR.scale(sRatio);

	tuneLayout();
}

BOOL McPiles::checkOverlap(McRect *targetR)
{
	for (list<McPile *>::iterator cit = placedPiles.begin(); cit != placedPiles.end(); cit++)
		if (targetR->intersection(&(*cit)->placementR) > 0.0)
			return TRUE;
	return FALSE;
}

#define MCP_First 0
#define MCP_Final 1

void McPiles::placeWindow(McPile *pile)
{
	double aspectWeight = MC::getMC()->getProperties()->getAspectWeight();

	if (placedPiles.size() == 0)
		pile->placementR.set(0, 0, pile->boundingR.getWidth(), pile->boundingR.getHeight());
	else
	{
		McPile*			bestCandidate = NULL;
		double			bestMetric = 0.0;
		unsigned		bestLocation;
		McRect			bestR;

		McRect			targetR;

		// Try two placements, try to minimize metric

		for (list<McPile *>::reverse_iterator c_it = placedPiles.rbegin(); c_it != placedPiles.rend(); c_it++)
		{
			McPile *candidate = *c_it;
			for (unsigned pl = MCP_First; pl <= MCP_Final; pl++)
			{
				switch (pl)
				{
				case MCP_First:

					if (candidate->filledFirst) continue;

					targetR.left = candidate->placementR.right + 1;
					if (pile->boundingR.getHeight() > candidate->placementR.getHeight())
						targetR.top = candidate->placementR.top;
					else
						targetR.top = candidate->placementR.top +
						(candidate->placementR.getHeight() - pile->boundingR.getHeight()) / 2;
					break;

				case MCP_Final:

					if (candidate->filledFinal) continue;

					targetR.top = candidate->placementR.bottom + 1;
					if (pile->boundingR.getWidth() > candidate->placementR.getWidth())
						targetR.left = candidate->placementR.left;
					else
						targetR.left = candidate->placementR.left +
						(candidate->placementR.getWidth() - pile->boundingR.getWidth()) / 2;
					break;
				}

				targetR.right = targetR.left + pile->boundingR.getWidth();
				targetR.bottom = targetR.top + pile->boundingR.getHeight();

				double metric1 =
					((double)(placementR.bottom*placementR.right)) /
					(
					(placementR.bottom + ((double)max(0, targetR.bottom - placementR.bottom))) *
						(placementR.right + ((double)max(0, targetR.right - placementR.right)))
						);

				double aspect =
					((double)max(boundingR.right, targetR.right)) /
					((double)max(boundingR.bottom, targetR.bottom));

				double metric2 = (aspect < xyAspect) ? aspect / xyAspect : xyAspect / aspect;

				double metric = aspectWeight * metric2 + (1.0 - aspectWeight) * metric1;

				if ((metric > bestMetric) && !checkOverlap(&targetR))
				{
					bestMetric = metric;
					bestCandidate = candidate;
					bestLocation = pl;
					bestR.set(&targetR);
				}
			}
		}

		pile->placementR.set(&bestR);

		switch (bestLocation)
		{
		case MCP_First:

			bestCandidate->filledFirst = TRUE;
			pile->row = bestCandidate->row;
			break;

		case MCP_Final:

			bestCandidate->filledFinal = TRUE;
			pile->row = bestCandidate->row + 1;
			break;

		}
	}

	boundingR.right = max(boundingR.right, pile->placementR.right);
	boundingR.bottom = max(boundingR.bottom, pile->placementR.bottom);

	if (boundingR.right > placementR.right)
	{
		LONG deltaW = (boundingR.right - placementR.right);
		placementR.right += deltaW;
		placementR.bottom += (LONG)(deltaW / xyAspect);
	}

	if (boundingR.bottom > placementR.bottom)
	{
		LONG deltaH = (boundingR.bottom - placementR.bottom);
		placementR.bottom += deltaH;
		placementR.right += (LONG)(xyAspect * deltaH);
	}

	placedPiles.push_back(pile);

}

void McPiles::tuneLayout()
{

	list<McPile*> pLists[10]; //Allow for 10 rows, should be dynamic
	list<McPile*>::reverse_iterator r_it;

	int r, rows = 0;
	for (it = pList.begin(); it != pList.end(); it++)
		rows = max(rows, (*it)->row);
	for (r = 0;r<rows;r++)
		for (it = pList.begin(); it != pList.end(); it++)
			if ((*it)->row == 1 + r)
				pLists[r].push_back(*it);

	LONG maxB = 0;
	for (r = 0;r<rows;r++)
	{

		if (r > 0)
		{
			for (it = pLists[r].begin(); it != pLists[r].end(); it++)
				if ((*it)->placementR.top <= maxB)
					(*it)->placementR.offset(0, 1 + maxB - (*it)->placementR.top);
		}

		for (it = pLists[r].begin(); it != pLists[r].end(); it++)
			maxB = max(maxB, (*it)->placementR.bottom);
	}

	LONG oldCenter = 0, oldWidth = 0;
	for (r = 0; r < rows; r++)
	{
		long padding = placementR.getWidth();
		long count = pLists[r].size();

		for (it = pLists[r].begin(); it != pLists[r].end(); it++)
			padding -= (*it)->placementR.getWidth();

		padding = max(0, padding);

		if (r>1 && count == 1)
		{
			McPile *pile = *(pLists[r].begin());
			if (pile->placementR.getWidth() < oldWidth)
				padding = oldCenter - pile->placementR.getWidth() / 2;
			else
				padding = oldCenter - oldWidth / 2;
		}
		else
			padding /= count + 1;

		LONG cumX = 0;
		LONG w;
		if (r % 2)
			for (r_it = pLists[r].rbegin(); r_it != pLists[r].rend(); r_it++)
			{
				(*r_it)->setPileNumber(McPiles::getNextPileNumber());
				cumX += padding;
				w = (*r_it)->placementR.getWidth();
				(*r_it)->placementR.left = cumX;
				cumX += w;
				(*r_it)->placementR.right = cumX;
			}
		else
			for (it = pLists[r].begin(); it != pLists[r].end(); it++)
			{
				(*it)->setPileNumber(McPiles::getNextPileNumber());
				cumX += padding;
				w = (*it)->placementR.getWidth();
				(*it)->placementR.left = cumX;
				cumX += w;
				(*it)->placementR.right = cumX;
			}

		LONG oldBottom = placementR.bottom + 100;
		for (it = pLists[r].begin(); it != pLists[r].end(); it++)
		{
			McPile *pile = *it;
			if (pile->placementR.bottom < oldBottom)
			{
				oldBottom = pile->placementR.bottom;
				oldCenter = (pile->placementR.left + pile->placementR.right) / 2;
				oldWidth = pile->placementR.getWidth();
			}
		}
	}

	list<McPile *>::iterator jt;
	for (r = 1; r< rows; r++)
	{
		for (it = pLists[r].begin(); it != pLists[r].end(); it++)
		{
			BOOL finished = FALSE;
			while (!finished)
			{
				(*it)->placementR.offset(0, -1);
				if ((*it)->placementR.top < 0)
				{
					MC::getMC();
				}
				for (jt = pLists[r - 1].begin(); jt != pLists[r - 1].end(); jt++)
					if ((*it)->placementR.intersection(&(*jt)->placementR) > 0 || ((*it)->placementR.top < 0))
					{
						(*it)->placementR.offset(0, 1);
						finished = TRUE;
						break;
					}
			}
		}
	}

	for (it = pList.begin(); it != pList.end(); it++)
		(*it)->setPileNumbers();

	McRect bR(placementR.right, placementR.bottom, placementR.left, placementR.top);
	for (it = pList.begin(); it != pList.end(); it++)
	{
		bR.bottom = max(bR.bottom, (*it)->placementR.bottom);
		bR.right = max(bR.right, (*it)->placementR.right);
		bR.left = min(bR.left, (*it)->placementR.left);
		bR.top = min(bR.top, (*it)->placementR.top);
	}

	double finalScale = max((double)(bR.getWidth()) / (double)(placementR.getWidth()),
		(double)(bR.getHeight()) / (double)(placementR.getHeight()));

	if (finalScale > 1.0)
	{
		sRatio /= finalScale;
		for (it = pList.begin(); it != pList.end(); it++)
			(*it)->placementR.scale(1.0 / finalScale);
	}

	McRect bounds(placementR.right, placementR.bottom, 0, 0);

	for (it = pList.begin(); it != pList.end(); it++)
	{
		bounds.top = min(bounds.top, (*it)->placementR.top);
		bounds.left = min(bounds.left, (*it)->placementR.left);
		bounds.right = max(bounds.right, (*it)->placementR.right);
		bounds.bottom = max(bounds.bottom, (*it)->placementR.bottom);
	}

	for (it = pList.begin(); it != pList.end(); it++)
	{

		McRect *pr = &(*it)->placementR;
		pr->offset(
			(placementR.getWidth() - bounds.getWidth()) / 2 - bounds.left,
			(placementR.getHeight() - bounds.getHeight()) / 2 - bounds.top);
		double yxF = ((double)pr->getHeight()) / ((double)pr->getWidth());
		pr->left += wSpacing / 2;
		pr->right -= wSpacing / 2;
		pr->top += (LONG)(0.5*yxF*wSpacing);
		pr->bottom -= (LONG)(0.5*yxF*wSpacing);
		(*it)->sRatio = ((double)(pr->getWidth() + wSpacing)) / ((double)pr->getWidth());
	}
}

void McPiles::positionThumbs()
{
	for (it = pList.begin(); it != pList.end(); it++)
		(*it)->positionThumbs(sRatio);
}

