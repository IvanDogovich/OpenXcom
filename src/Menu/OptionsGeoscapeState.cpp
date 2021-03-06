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
#include "OptionsGeoscapeState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/Slider.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/TextButton.h"
#include "../Interface/ComboBox.h"
#include "../Engine/Action.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape Options screen.
 * @param game Pointer to the core game.
 * @param origin Game section that originated this state.
 */
OptionsGeoscapeState::OptionsGeoscapeState(Game *game, OptionsOrigin origin) : OptionsBaseState(game, origin)
{
	setCategory(_btnGeoscape);

	// Create objects
	_txtDragScroll = new Text(114, 9, 210, 8);
	_cbxDragScroll = new ComboBox(this, 100, 16, 210, 18);

	_txtScrollSpeed = new Text(114, 9, 94, 8);
	_slrScrollSpeed = new Slider(100, 16, 94, 18);

	_txtDogfightSpeed = new Text(114, 9, 210, 40);
	_slrDogfightSpeed = new Slider(100, 16, 210, 50);

	_txtClockSpeed = new Text(114, 9, 94, 40);
	_slrClockSpeed = new Slider(100, 16, 94, 50);

	_txtGlobeDetails = new Text(114, 9, 94, 82);
	_btnGlobeCountries = new ToggleTextButton(100, 16, 94, 92);
	_btnGlobeRadars = new ToggleTextButton(100, 16, 94, 110);
	_btnGlobePaths = new ToggleTextButton(100, 16, 94, 128);

	_txtOptions = new Text(114, 9, 210, 82);
	_btnShowFunds = new ToggleTextButton(100, 16, 210, 92);

	add(_txtScrollSpeed);
	add(_slrScrollSpeed);

	add(_txtDogfightSpeed);
	add(_slrDogfightSpeed);

	add(_txtClockSpeed);
	add(_slrClockSpeed);

	add(_txtGlobeDetails);
	add(_btnGlobeCountries);
	add(_btnGlobeRadars);
	add(_btnGlobePaths);

	add(_txtOptions);
	add(_btnShowFunds);
	
	add(_txtDragScroll);
	add(_cbxDragScroll);

	centerAllSurfaces();

	// Set up objects
	_txtDragScroll->setColor(Palette::blockOffset(8)+10);
	_txtDragScroll->setText(tr("STR_DRAG_SCROLL"));
	
	std::vector<std::string> dragScrolls;
	dragScrolls.push_back("STR_NONE");
	dragScrolls.push_back("STR_LEFT_MOUSE_BUTTON");
	dragScrolls.push_back("STR_MIDDLE_MOUSE_BUTTON");
	dragScrolls.push_back("STR_RIGHT_MOUSE_BUTTON");

	_cbxDragScroll->setColor(Palette::blockOffset(15)-1);
	_cbxDragScroll->setOptions(dragScrolls);
	_cbxDragScroll->setSelected(Options::geoDragScrollButton);
	_cbxDragScroll->onChange((ActionHandler)&OptionsGeoscapeState::cbxDragScrollChange);
	_cbxDragScroll->setTooltip("STR_DRAG_SCROLL_DESC");
	_cbxDragScroll->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_cbxDragScroll->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_txtScrollSpeed->setColor(Palette::blockOffset(8)+10);
	_txtScrollSpeed->setText(tr("STR_SCROLL_SPEED"));

	_slrScrollSpeed->setColor(Palette::blockOffset(15)-1);
	_slrScrollSpeed->setRange(100, 10);
	_slrScrollSpeed->setValue(Options::geoScrollSpeed);
	_slrScrollSpeed->setTooltip("STR_SCROLL_SPEED_GEO_DESC");
	_slrScrollSpeed->onChange((ActionHandler)&OptionsGeoscapeState::slrScrollSpeedChange);
	_slrScrollSpeed->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_slrScrollSpeed->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_txtDogfightSpeed->setColor(Palette::blockOffset(8)+10);
	_txtDogfightSpeed->setText(tr("STR_DOGFIGHT_SPEED"));

	_slrDogfightSpeed->setColor(Palette::blockOffset(15)-1);
	_slrDogfightSpeed->setRange(50, 10);
	_slrDogfightSpeed->setValue(Options::dogfightSpeed);
	_slrDogfightSpeed->onChange((ActionHandler)&OptionsGeoscapeState::slrDogfightSpeedChange);
	_slrDogfightSpeed->setTooltip("STR_DOGFIGHT_SPEED_DESC");
	_slrDogfightSpeed->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_slrDogfightSpeed->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_txtClockSpeed->setColor(Palette::blockOffset(8)+10);
	_txtClockSpeed->setText(tr("STR_CLOCK_SPEED"));

	_slrClockSpeed->setColor(Palette::blockOffset(15)-1);
	_slrClockSpeed->setRange(250, 10);
	_slrClockSpeed->setValue(Options::geoClockSpeed);
	_slrClockSpeed->setTooltip("STR_CLOCK_SPEED_DESC");
	_slrClockSpeed->onChange((ActionHandler)&OptionsGeoscapeState::slrClockSpeedChange);
	_slrClockSpeed->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_slrClockSpeed->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_txtGlobeDetails->setColor(Palette::blockOffset(8)+10);
	_txtGlobeDetails->setText(tr("STR_GLOBE_DETAILS"));

	_btnGlobeCountries->setColor(Palette::blockOffset(15)-1);
	_btnGlobeCountries->setText(tr("STR_GLOBE_COUNTRIES"));
	_btnGlobeCountries->setPressed(Options::globeDetail);
	_btnGlobeCountries->onMouseClick((ActionHandler)&OptionsGeoscapeState::btnGlobeCountriesClick);
	_btnGlobeCountries->setTooltip("STR_GLOBE_COUNTRIES_DESC");
	_btnGlobeCountries->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_btnGlobeCountries->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_btnGlobeRadars->setColor(Palette::blockOffset(15)-1);
	_btnGlobeRadars->setText(tr("STR_GLOBE_RADARS"));
	_btnGlobeRadars->setPressed(Options::globeRadarLines);
	_btnGlobeRadars->onMouseClick((ActionHandler)&OptionsGeoscapeState::btnGlobeRadarsClick);
	_btnGlobeRadars->setTooltip("STR_GLOBE_RADARS_DESC");
	_btnGlobeRadars->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_btnGlobeRadars->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_btnGlobePaths->setColor(Palette::blockOffset(15)-1);
	_btnGlobePaths->setText(tr("STR_GLOBE_FLIGHT_PATHS"));
	_btnGlobePaths->setPressed(Options::globeFlightPaths);
	_btnGlobePaths->onMouseClick((ActionHandler)&OptionsGeoscapeState::btnGlobePathsClick);
	_btnGlobePaths->setTooltip("STR_GLOBE_FLIGHT_PATHS_DESC");
	_btnGlobePaths->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_btnGlobePaths->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);

	_txtOptions->setColor(Palette::blockOffset(8)+10);
	_txtOptions->setText(tr("STR_USER_INTERFACE_OPTIONS"));

	_btnShowFunds->setColor(Palette::blockOffset(15)-1);
	_btnShowFunds->setText(tr("STR_SHOW_FUNDS"));
	_btnShowFunds->setPressed(Options::showFundsOnGeoscape);
	_btnShowFunds->onMouseClick((ActionHandler)&OptionsGeoscapeState::btnShowFundsClick);
	_btnShowFunds->setTooltip("STR_SHOW_FUNDS_DESC");
	_btnShowFunds->onMouseIn((ActionHandler)&OptionsGeoscapeState::txtTooltipIn);
	_btnShowFunds->onMouseOut((ActionHandler)&OptionsGeoscapeState::txtTooltipOut);
}

/**
 *
 */
OptionsGeoscapeState::~OptionsGeoscapeState()
{

}

/**
 * Changes the Drag Scroll option.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::cbxDragScrollChange(Action *)
{
	Options::geoDragScrollButton = _cbxDragScroll->getSelected();
}

/**
 * Updates the scroll speed.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::slrScrollSpeedChange(Action *)
{
	Options::geoScrollSpeed = _slrScrollSpeed->getValue();
}

/**
 * Updates the dogfight speed.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::slrDogfightSpeedChange(Action *)
{
	Options::dogfightSpeed = _slrDogfightSpeed->getValue();
}

/**
 * Updates the clock speed.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::slrClockSpeedChange(Action *)
{
	Options::geoClockSpeed = _slrClockSpeed->getValue();
}

/**
 * Changes the Globe Country Borders option.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::btnGlobeCountriesClick(Action *)
{
	Options::globeDetail = _btnGlobeCountries->getPressed();
}

/**
 * Changes the Globe Radar Ranges option.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::btnGlobeRadarsClick(Action *)
{
	Options::globeRadarLines = _btnGlobeRadars->getPressed();
}

/**
 * Changes the Globe Flight Paths option.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::btnGlobePathsClick(Action *)
{
	Options::globeFlightPaths = _btnGlobePaths->getPressed();
}

/**
 * Changes the Show Funds option.
 * @param action Pointer to an action.
 */
void OptionsGeoscapeState::btnShowFundsClick(Action *)
{
	Options::showFundsOnGeoscape = _btnShowFunds->getPressed();
}

}
