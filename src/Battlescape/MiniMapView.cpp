/*
 * Copyright 2010-2013 OpenXcom Developers.
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
#include "Camera.h"
#include "../Engine/Action.h"
#include "../Interface/Cursor.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Engine/Game.h"
#include "../Engine/SurfaceSet.h"
#include "../Resource/ResourcePack.h"
#include "../Ruleset/Armor.h"
#include "../Engine/Options.h"
#include <sstream>

namespace OpenXcom
{
const int CELL_WIDTH = 4;
const int CELL_HEIGHT = 4;
const int MAX_LEVEL = 3;
const int MAX_FRAME = 2;

/**
 * Initializes all the elements in the MiniMapView.
 * @param w The MiniMapView width.
 * @param h The MiniMapView height.
 * @param x The MiniMapView x origin.
 * @param y The MiniMapView y origin.
 * @param game Pointer to the core game.
 * @param camera The Battlescape camera.
 * @param battleGame Pointer to the SavedBattleGame.
 */
MiniMapView::MiniMapView(int w, int h, int x, int y, Game * game, Camera * camera, SavedBattleGame * battleGame) : InteractiveSurface(w, h, x, y), _game(game), _camera(camera), _battleGame(battleGame), _frame(0), _isMouseScrolling(false), _isMouseScrolled(false), _xBeforeMouseScrolling(0), _yBeforeMouseScrolling(0), _mouseScrollX(0), _mouseScrollY(0), _totalMouseMoveX(0), _totalMouseMoveY(0), _mouseMovedOverThreshold(false)
{
	_set = _game->getResourcePack()->getSurfaceSet("SCANG.DAT");
}

/**
 * Draws the minimap.
 */
void MiniMapView::draw()
{
	int _startX = _camera->getCenterPosition().x - ((getWidth() / CELL_WIDTH) / 2);
	int _startY = _camera->getCenterPosition().y - ((getHeight() / CELL_HEIGHT) / 2);

	InteractiveSurface::draw();
	if(!_set)
	{
		return;
	}
	SDL_Rect current;
	current.x = current.y = 0;
	current.w = getWidth ();
	current.h = getHeight ();
	drawRect(&current, 0);
	this->lock();
	for (int lvl = 0; lvl <= _camera->getCenterPosition().z; lvl++)
	{
		int py = _startY;
		for (int y = Surface::getY(); y < getHeight () + Surface::getY(); y += CELL_HEIGHT)
		{
			int px = _startX;
			for (int x = Surface::getX(); x < getWidth () + Surface::getX(); x += CELL_WIDTH)
			{
				MapData * data = 0;
				Tile * t = 0;
				Position p (px, py, lvl);
				t = _battleGame->getTile(p);
				if (!t)
				{
					px++;
					continue;
				}
				int tileShade = 16;
				if (t->isDiscovered(2))
				{
					tileShade = t->getShade();
				}
				for(int i = 0; i < 4; i++)
				{
					data = t->getMapData(i);

					Surface * s = 0;
					if(data && data->getMiniMapIndex())
					{
						s = _set->getFrame (data->getMiniMapIndex()+35);
					}
					if(s)
					{
						s->blitNShade(this, x, y, tileShade);
					}
				}
				// alive units
				if (t->getUnit() && t->getUnit()->getVisible())
				{
					int frame = t->getUnit()->getMiniMapSpriteIndex();
					int size = t->getUnit()->getArmor()->getSize();
					frame += (t->getPosition().y - t->getUnit()->getPosition().y) * size;
					frame += t->getPosition().x - t->getUnit()->getPosition().x;
					frame += _frame * size * size;
					Surface * s = _set->getFrame(frame);
					s->blitNShade(this, x, y, 0);
				}
				// perhaps (at least one) item on this tile?
				if (t->isDiscovered(2) && !t->getInventory()->empty())
				{
					int frame = 9 + _frame;
					Surface * s = _set->getFrame(frame);
					s->blitNShade(this, x, y, 0);
				}

				px++;
			}
			py++;
		}
	}
	this->unlock();
	int centerX = getWidth() / 2 - 1;
	int centerY = getHeight() / 2 - 1;
	Uint8 color = 1 + _frame * 3;
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
 * Increments the displayed level.
 * @return New display level.
 */
int MiniMapView::up ()
{
	_camera->setViewLevel(_camera->getViewLevel()+1);
	_redraw = true;
	return _camera->getViewLevel();
}

/**
 * Decrements the displayed level.
 * @return New display level.
 */
int MiniMapView::down ()
{
	_camera->setViewLevel(_camera->getViewLevel()-1);
	_redraw = true;
	return _camera->getViewLevel();
}

/**
 * Handles mouse presses on the minimap. Enters mouse-moving mode when the right button is used.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void MiniMapView::mousePress(Action *action, State *state)
{
	InteractiveSurface::mousePress(action, state);

	if (action->getDetails()->button.button == Options::battleDragScrollButton)
	{
		_isMouseScrolling = true;
		_isMouseScrolled = false;
		SDL_GetMouseState(&_xBeforeMouseScrolling, &_yBeforeMouseScrolling);
		_posBeforeMouseScrolling = _camera->getCenterPosition();
		_mouseScrollX = 0; _mouseScrollY = 0;
		_totalMouseMoveX = 0; _totalMouseMoveY = 0;
		_mouseMovedOverThreshold = false;
		_mouseScrollingStartTime = SDL_GetTicks();
	}
}

/**
 * Handles mouse clicks on the minimap. Will change the camera center to the clicked point.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void MiniMapView::mouseClick (Action *action, State *state)
{
	InteractiveSurface::mouseClick(action, state);

	// The following is the workaround for a rare problem where sometimes
	// the mouse-release event is missed for any reason.
	// However if the SDL is also missed the release event, then it is to no avail :(
	// (this part handles the release if it is missed and now an other button is used)
	if (_isMouseScrolling) {
		if (action->getDetails()->button.button != Options::battleDragScrollButton
		&& 0==(SDL_GetMouseState(0,0)&SDL_BUTTON(Options::battleDragScrollButton))) { // so we missed again the mouse-release :(
			// Check if we have to revoke the scrolling, because it was too short in time, so it was a click
			if ((!_mouseMovedOverThreshold) && (SDL_GetTicks() - _mouseScrollingStartTime <= (Options::dragScrollTimeTolerance)))
				{ _camera->centerOnPosition(_posBeforeMouseScrolling); _redraw = true; }
			_isMouseScrolled = _isMouseScrolling = false;
		}
	}

	// Drag-Scroll release: release mouse-scroll-mode
	if (_isMouseScrolling)
	{
		// While scrolling, other buttons are ineffective
		if (action->getDetails()->button.button == Options::battleDragScrollButton) _isMouseScrolling = false; else return;
		// Check if we have to revoke the scrolling, because it was too short in time, so it was a click
		if ((!_mouseMovedOverThreshold) && (SDL_GetTicks() - _mouseScrollingStartTime <= (Options::dragScrollTimeTolerance)))
		{
			_isMouseScrolled = false;
			_camera->centerOnPosition(_posBeforeMouseScrolling);
			_redraw = true;
		}
		if (_isMouseScrolled) return;
	}

	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		int origX = action->getRelativeXMouse() / action->getXScale();
		int origY = action->getRelativeYMouse() / action->getYScale();
		// get offset (in cells) of the click relative to center of screen
		int xOff = (origX / CELL_WIDTH) - ((getWidth() / 2) / CELL_WIDTH);
		int yOff = (origY / CELL_HEIGHT) - ((getHeight() / 2) / CELL_HEIGHT);
		// center the camera on this new position
		int newX = _camera->getCenterPosition().x + xOff;
		int newY = _camera->getCenterPosition().y + yOff;
		_camera->centerOnPosition(Position(newX,newY,_camera->getViewLevel()));
		_redraw = true;
	}
	else if (action->getDetails()->button.button == SDL_BUTTON_RIGHT)
	{
    // Closes the window on right-click.
		_game->popState();
	}
}

/**
 * Handles moving over the minimap.
 * Will change the camera center when the mouse is moved in mouse-moving mode.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void MiniMapView::mouseOver(Action *action, State *state)
{
	InteractiveSurface::mouseOver(action, state);

	if (_isMouseScrolling && action->getDetails()->type == SDL_MOUSEMOTION)
	{
		// The following is the workaround for a rare problem where sometimes
		// the mouse-release event is missed for any reason.
		// However if the SDL is also missed the release event, then it is to no avail :(
		// (checking: is the dragScroll-mouse-button still pressed?)
		if (0==(SDL_GetMouseState(0,0)&SDL_BUTTON(Options::battleDragScrollButton))) { // so we missed again the mouse-release :(
			// Check if we have to revoke the scrolling, because it was too short in time, so it was a click
			if ((!_mouseMovedOverThreshold) && (SDL_GetTicks() - _mouseScrollingStartTime <= (Options::dragScrollTimeTolerance)))
				{ _camera->centerOnPosition(_posBeforeMouseScrolling); _redraw = true; }
			_isMouseScrolled = _isMouseScrolling = false;
			return;
		}

		_isMouseScrolled = true;

		// Set the mouse cursor back
		SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
		SDL_WarpMouse(_xBeforeMouseScrolling, _yBeforeMouseScrolling);
		SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

		// Check the threshold
		_totalMouseMoveX += action->getDetails()->motion.xrel;
		_totalMouseMoveY += action->getDetails()->motion.yrel;
		if (!_mouseMovedOverThreshold)
			_mouseMovedOverThreshold = ((std::abs(_totalMouseMoveX) > Options::dragScrollPixelTolerance) || (std::abs(_totalMouseMoveY) > Options::dragScrollPixelTolerance));

		// Calculate the move
		int newX;
		int newY;
		if (Options::battleDragScrollInvert)
		{
			_mouseScrollX += action->getDetails()->motion.xrel;
			_mouseScrollY += action->getDetails()->motion.yrel;
			newX = _posBeforeMouseScrolling.x + _mouseScrollX / 4;
			newY = _posBeforeMouseScrolling.y + _mouseScrollY / 4;

			// Keep the limits...
			if (newX < -1 || _camera->getMapSizeX() < newX)
			{
				_mouseScrollX -= action->getDetails()->motion.xrel;
				newX = _posBeforeMouseScrolling.x + _mouseScrollX / 4;
			}
			if (newY < -1 || _camera->getMapSizeY() < newY)
			{
				_mouseScrollY -= action->getDetails()->motion.yrel;
				newY = _posBeforeMouseScrolling.y + _mouseScrollY / 4;
			}
		}
		else
		{
			newX = _posBeforeMouseScrolling.x - (int)((double)_totalMouseMoveX / action->getXScale()) / 4;
			newY = _posBeforeMouseScrolling.y - (int)((double)_totalMouseMoveY / action->getYScale()) / 4;

			// Keep the limits...
			if (newX < -1) newX = -1;
			else if (_camera->getMapSizeX() < newX) newX = _camera->getMapSizeX();
			if (newY < -1) newY = -1;
			else if (_camera->getMapSizeY() < newY) newY = _camera->getMapSizeY();
		}

		// Scrolling
		_camera->centerOnPosition(Position(newX,newY,_camera->getViewLevel()));
		_redraw = true;

		// We don't want to look the mouse-cursor jumping :)
		action->getDetails()->motion.x=_xBeforeMouseScrolling; action->getDetails()->motion.y=_yBeforeMouseScrolling;
		_game->getCursor()->handle(action);
	}
}

/**
 * Handles moving into the minimap.
 * Stops the mouse-scrolling mode, if its left on.
 * @param action Pointer to an action.
 * @param state State that the action handlers belong to.
 */
void MiniMapView::mouseIn(Action *action, State *state)
{
	InteractiveSurface::mouseIn(action, state);

	_isMouseScrolling = false;
	setButtonPressed(SDL_BUTTON_RIGHT, false);
}

/**
 * Updates the minimap animation.
 */
void MiniMapView::animate()
{
	_frame++;
	if(_frame > MAX_FRAME)
	{
		_frame = 0;
	}
	_redraw = true;
}

}
