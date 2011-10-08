/*
 * Copyright 2011 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MiniMapView.h"
#include "../Savegame/Tile.h"
#include "Map.h"
#include "../Engine/Action.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Engine/Game.h"
#include "../Engine/SurfaceSet.h"
#include "../Resource/ResourcePack.h"
#include "../Savegame/SavedGame.h"
#include <sstream>

namespace OpenXcom
{
const int CELL_WIDTH = 4;
const int CELL_HEIGHT = 4;
const int MAX_LEVEL = 3;
const int MAX_FRAME = 2;

/**
 * Initializes all the elements in the MiniMapView.
 * @param w the MiniMapView width
 * @param h the MiniMapView height
 * @param x the MiniMapView x origin
 * @param y the MiniMapView y origin
 * @param game Pointer to the core game.
 * @param map The Battlescape map
 * @param battleGame Pointer to the SavedBattleGame
*/
MiniMapView::MiniMapView(int w, int h, int x, int y, Game * game, Map * map, SavedBattleGame * battleGame) : InteractiveSurface(w, h, x, y), _game(game), _map(map), _startX(0),  _startY(0), _lvl(_map->getViewHeight()), _battleGame(battleGame), _frame(0)
{
	_startX = _map->getCenterX () - ((getWidth () / CELL_WIDTH) / 2);
	_startY = _map->getCenterY () - ((getHeight () / CELL_HEIGHT) / 2);
}

/**
 * Blits this surface onto another one
 * @param surface Pointer to surface to blit onto.
 */
void MiniMapView::blit(Surface *surface)
{
	draw();
	InteractiveSurface::blit (surface);
}

/**
 * Draw the minimap
 */
void MiniMapView::draw()
{
	SavedBattleGame * battle = _game->getSavedGame()->getBattleGame();
	SurfaceSet * set = _game->getResourcePack()->getSurfaceSet("SCANG.DAT");
	int py = _startY;
	std::map<Tile *, BattleUnit *> battleUnits;
	std::vector<BattleUnit*> *const units (_battleGame->getUnits());
	// We can't access dead units from tiles. So we first compute a list of all battle units and their corresponding tile. We will use this later to display units on the minimap
	for(std::vector<BattleUnit*>::const_iterator it = units->begin (); it != units->end (); ++it)
	{
		battleUnits[(*it)->getTile()] = *it;
	}

	for (int y = getHeight () - CELL_HEIGHT; y >= 0; y-=CELL_HEIGHT)
	{
		int px = _startX;
		for (int x = 0; x < getWidth (); x += CELL_WIDTH)
		{
			SDL_Rect r;
			r.x = x;
			r.y = y;
			r.w = CELL_WIDTH;
			r.h = CELL_HEIGHT;
			MapData * data = 0;
			int lvl = _lvl;
			Tile * t = 0;
			while ((!t || !data) && lvl >= 0)
			{
				Position p (px, py, lvl--);
				t = battle->getTile(p);
				if (!t)
				{
					continue;
				}
				int i = MAX_LEVEL;
				while (!data && i >= 0)
				{
					data = t->getMapData(i--);
				}
			}
			Surface * s = 0;
			if(data && set)
			{
				s = set->getFrame (data->getMiniMapIndex()+35);
			}
			if(!s || !t->isDiscovered(2))
			{
				drawRect(&r, 0);
			}
			else
			{
				s->blitNShade(this, x, y, t->getShade());
			}
			std::map<Tile *, BattleUnit *>::iterator itTile =  battleUnits.find (t);
			if(itTile != battleUnits.end () && itTile->second->getVisible())
			{
				s = set->getFrame (itTile->second->getMiniMapSpriteIndex ()+_frame);
				s->blitNShade(this, x, y, 0);
			}
			px++;
		}
		py++;
	}
	int centerX = getWidth() / 2;
	int centerY = getHeight() / 2;
	Uint8 color = 1;
	int xOffset = CELL_WIDTH / 2;
	int yOffset = CELL_HEIGHT / 2;
	drawLine(centerX - CELL_WIDTH, centerY - CELL_HEIGHT,
		 centerX - xOffset, centerY - yOffset,
		 color); // top left
	drawLine(centerX + xOffset, centerY - yOffset,
		 centerX + CELL_WIDTH, centerY - CELL_HEIGHT,
		 color); // top right
	drawLine(centerX - CELL_WIDTH, centerY + CELL_HEIGHT,
		 centerX - xOffset, centerY + yOffset,
		 color); // bottom left
	drawLine(centerX + CELL_WIDTH, centerY + CELL_HEIGHT,
		 centerX + xOffset, centerY + yOffset,
		 color); //bottom right
}

/**
 * Increment the displayed level
 */
void MiniMapView::up ()
{
	_lvl++;
	if (_lvl > MAX_LEVEL)
	{
		_lvl = 0;
	}
	else
	{
		draw();
	}
}

/**
 * Decrement the displayed level
 */
void MiniMapView::down ()
{
	_lvl--;
	if (_lvl < 0)
	{
		_lvl = MAX_LEVEL;
	}
	else
	{
		draw();
	}
}

/**
 * Handle click on the minimap. Will change the minimap center to the clicked point
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
*/
void MiniMapView::mouseClick (Action *action, State *state)
{
	InteractiveSurface::mouseClick(action, state);
	int origX = action->getRelativeXMouse() / action->getXScale();
	int origY = action->getRelativeYMouse() / action->getYScale();
	_startY -= (origY / CELL_HEIGHT) - ((getHeight () / 2) / CELL_HEIGHT);
	_startX += (origX / CELL_HEIGHT) - ((getWidth () / 2) / CELL_HEIGHT);
	draw();
}

/**
 * Get the displayed level
 * @return the displayed level
*/
int MiniMapView::getDisplayedLevel ()
{
	return _lvl;
}

/**
 * Change the displayed level
 * @param level the new displayed level
*/
void MiniMapView::setDisplayedLevel (int level)
{
	_lvl = level;
}

/**
 * Update minimap animation
*/
void MiniMapView::animate()
{
	_frame++;
	if(_frame > MAX_FRAME)
	{
		_frame = 0;
	}
}
}
