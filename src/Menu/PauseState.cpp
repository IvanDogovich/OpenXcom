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
#include "PauseState.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "AbandonGameState.h"
#include "ListLoadState.h"
#include "ListSaveState.h"
#include "../Engine/Options.h"
#include "OptionsVideoState.h"
#include "OptionsGeoscapeState.h"
#include "OptionsBattlescapeState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Pause window.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 */
PauseState::PauseState(Game *game, OptionsOrigin origin) : State(game), _origin(origin)
{
	_screen = false;

	int x;
	if (_origin == OPT_GEOSCAPE)
	{
		x = 20;
	}
	else
	{
		x = 52;
	}

	// Create objects
	_window = new Window(this, 216, 160, x, 20, POPUP_BOTH);
	_btnLoad = new TextButton(180, 18, x+18, 52);
	_btnSave = new TextButton(180, 18, x+18, 74);
	_btnAbandon = new TextButton(180, 18, x+18, 96);
	_btnOptions = new TextButton(180, 18, x+18, 122);
	_btnCancel = new TextButton(180, 18, x+18, 150);
	_txtTitle = new Text(206, 17, x+5, 32);

	// Set palette
	if (_origin == OPT_BATTLESCAPE)
	{
		setPalette("PAL_BATTLESCAPE");
	}
	else
	{
		setPalette("PAL_GEOSCAPE", 0);
	}

	add(_window);
	add(_btnLoad);
	add(_btnSave);
	add(_btnAbandon);
	add(_btnOptions);
	add(_btnCancel);
	add(_txtTitle);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(Palette::blockOffset(15)-1);
	_window->setBackground(_game->getResourcePack()->getSurface("BACK01.SCR"));

	_btnLoad->setColor(Palette::blockOffset(15)-1);
	_btnLoad->setText(tr("STR_LOAD_GAME"));
	_btnLoad->onMouseClick((ActionHandler)&PauseState::btnLoadClick);

	_btnSave->setColor(Palette::blockOffset(15)-1);
	_btnSave->setText(tr("STR_SAVE_GAME"));
	_btnSave->onMouseClick((ActionHandler)&PauseState::btnSaveClick);

	_btnAbandon->setColor(Palette::blockOffset(15)-1);
	_btnAbandon->setText(tr("STR_ABANDON_GAME"));
	_btnAbandon->onMouseClick((ActionHandler)&PauseState::btnAbandonClick);

	_btnOptions->setColor(Palette::blockOffset(15)-1);
	_btnOptions->setText(tr("STR_GAME_OPTIONS"));
	_btnOptions->onMouseClick((ActionHandler)&PauseState::btnOptionsClick);

	_btnCancel->setColor(Palette::blockOffset(15)-1);
	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&PauseState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&PauseState::btnCancelClick, Options::keyCancel);
	if (origin == OPT_GEOSCAPE)
	{
		_btnCancel->onKeyboardPress((ActionHandler)&PauseState::btnCancelClick, Options::keyGeoOptions);
	}
	else if (origin == OPT_BATTLESCAPE)
	{
		_btnCancel->onKeyboardPress((ActionHandler)&PauseState::btnCancelClick, Options::keyBattleOptions);
	}

	_txtTitle->setColor(Palette::blockOffset(15)-1);
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_OPTIONS_UC"));

	if (_origin == OPT_BATTLESCAPE)
	{
		applyBattlescapeTheme();
	}

	if (_game->getSavedGame()->isIronman())
	{
		_btnLoad->setVisible(false);
		_btnSave->setVisible(false);
		_btnAbandon->setText(tr("STR_SAVE_AND_ABANDON_GAME"));
	}
}

/**
 *
 */
PauseState::~PauseState()
{

}

/**
 * Opens the Load Game screen.
 * @param action Pointer to an action.
 */
void PauseState::btnLoadClick(Action *)
{
	_game->pushState(new ListLoadState(_game, _origin));
}

/**
 * Opens the Save Game screen.
 * @param action Pointer to an action.
 */
void PauseState::btnSaveClick(Action *)
{
	_game->pushState(new ListSaveState(_game, _origin));
}

/**
* Opens the Game Options screen.
* @param action Pointer to an action.
*/
void PauseState::btnOptionsClick(Action *)
{
	Options::backupDisplay();
	if (_origin == OPT_GEOSCAPE)
	{
		_game->pushState(new OptionsGeoscapeState(_game, _origin));
	}
	else if (_origin == OPT_BATTLESCAPE)
	{
		_game->pushState(new OptionsBattlescapeState(_game, _origin));
	}
	else
	{
		_game->pushState(new OptionsVideoState(_game, _origin));
	}
}

/**
 * Opens the Abandon Game window.
 * @param action Pointer to an action.
 */
void PauseState::btnAbandonClick(Action *)
{
	_game->pushState(new AbandonGameState(_game, _origin));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void PauseState::btnCancelClick(Action *)
{
	_game->popState();
}

}
