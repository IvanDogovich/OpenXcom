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
#define _USE_MATH_DEFINES
#include "MonthlyReportState.h"
#include <sstream>
#include <cmath>
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/GameTime.h"
#include "PsiTrainingState.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Ruleset/RuleCountry.h"
#include "DefeatState.h"
#include "Globe.h"
#include "../Savegame/AlienBase.h"
#include "../Engine/Options.h"
#include "../Menu/SaveGameState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Monthly Report screen.
 * @param game Pointer to the core game.
 */
MonthlyReportState::MonthlyReportState(Game *game, bool psi, Globe *globe) : State(game), _psi(psi), _gameOver(false), _ratingTotal(0), _fundingDiff(0), _lastMonthsRating(0), _happyList(0), _sadList(0), _pactList(0)
{
	_globe = globe;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_btnBigOk = new TextButton(120, 18, 100, 174);
	_txtTitle = new Text(300, 17, 16, 8);
	_txtMonth = new Text(110, 9, 16, 24);
	_txtRating = new Text(180, 9, 125, 24);
	_txtChange = new Text(300, 9, 16, 32);
	_txtDesc = new Text(280, 140, 16, 40);
	_txtFailure = new Text(290, 160, 15, 10);

	// Set palette
	setPalette("PAL_GEOSCAPE", 3);

	add(_window);
	add(_btnOk);
	add(_btnBigOk);
	add(_txtTitle);
	add(_txtMonth);
	add(_txtRating);
	add(_txtChange);
	add(_txtDesc);
	add(_txtFailure);

	centerAllSurfaces();

	// Set up objects
	_window->setColor(Palette::blockOffset(15)-1);
	_window->setBackground(_game->getResourcePack()->getSurface("BACK13.SCR"));

	_btnOk->setColor(Palette::blockOffset(8)+10);
	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&MonthlyReportState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyCancel);
	
	_btnBigOk->setColor(Palette::blockOffset(8)+10);
	_btnBigOk->setText(tr("STR_OK"));
	_btnBigOk->onMouseClick((ActionHandler)&MonthlyReportState::btnOkClick);
	_btnBigOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyOk);
	_btnBigOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyCancel);
	_btnBigOk->setVisible(false);

	_txtTitle->setColor(Palette::blockOffset(15)-1);
	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_XCOM_PROJECT_MONTHLY_REPORT"));

	_txtFailure->setColor(Palette::blockOffset(8)+10);
	_txtFailure->setBig();
	_txtFailure->setAlign(ALIGN_CENTER);
	_txtFailure->setVerticalAlign(ALIGN_MIDDLE);
	_txtFailure->setWordWrap(true);
	_txtFailure->setText(tr("STR_YOU_HAVE_FAILED"));
	_txtFailure->setVisible(false);

	calculateChanges();

	int month = _game->getSavedGame()->getTime()->getMonth() - 1, year = _game->getSavedGame()->getTime()->getYear();
	if (month == 0)
	{
		month = 12;
		year--;
	}
	std::string m;
	switch (month)
	{
	case 1: m = "STR_JAN"; break;
	case 2: m = "STR_FEB"; break;
	case 3: m = "STR_MAR"; break;
	case 4: m = "STR_APR"; break;
	case 5: m = "STR_MAY"; break;
	case 6: m = "STR_JUN"; break;
	case 7: m = "STR_JUL"; break;
	case 8: m = "STR_AUG"; break;
	case 9: m = "STR_SEP"; break;
	case 10: m = "STR_OCT"; break;
	case 11: m = "STR_NOV"; break;
	case 12: m = "STR_DEC"; break;
	default: m = "";
	}
	int difficulty_threshold = 100*((int)(_game->getSavedGame()->getDifficulty())-9);

	_txtMonth->setColor(Palette::blockOffset(15)-1);
	_txtMonth->setSecondaryColor(Palette::blockOffset(8)+10);
	_txtMonth->setText(tr("STR_MONTH").arg(tr(m)).arg(year));

	// Calculate rating
	std::wstring rating = tr("STR_RATING_TERRIBLE");
	if (_ratingTotal > difficulty_threshold-300)
	{
		rating = tr("STR_RATING_POOR");
	}
	if (_ratingTotal > difficulty_threshold)
	{
		rating = tr("STR_RATING_OK");
	}
	if (_ratingTotal > 0)
	{
		rating = tr("STR_RATING_GOOD");
	}
	if (_ratingTotal > 500)
	{
		rating = tr("STR_RATING_EXCELLENT");
	}

	_txtRating->setColor(Palette::blockOffset(15)-1);
	_txtRating->setSecondaryColor(Palette::blockOffset(8)+10);
	_txtRating->setText(tr("STR_MONTHLY_RATING").arg(_ratingTotal).arg(rating));

	std::wostringstream ss3;
	if (_fundingDiff > 0)
		ss3 << '+';
	ss3 << Text::formatFunding(_fundingDiff);

	_txtChange->setColor(Palette::blockOffset(15)-1);
	_txtChange->setSecondaryColor(Palette::blockOffset(8)+10);
	_txtChange->setText(tr("STR_FUNDING_CHANGE").arg(ss3.str()));

	_txtDesc->setColor(Palette::blockOffset(8)+10);
	_txtDesc->setWordWrap(true);

	// calculate satisfaction
	std::wostringstream ss4;
	std::wstring satisFactionString = tr("STR_COUNCIL_IS_DISSATISFIED");
	bool resetWarning = true;
	if (_ratingTotal > difficulty_threshold)
	{
		satisFactionString = tr("STR_COUNCIL_IS_GENERALLY_SATISFIED");
	}
	if (_ratingTotal > 500)
	{
		satisFactionString = tr("STR_COUNCIL_IS_VERY_PLEASED");
	}
	if (_lastMonthsRating <= difficulty_threshold && _ratingTotal <= difficulty_threshold)
	{
		satisFactionString = tr("STR_YOU_HAVE_NOT_SUCCEEDED");
		_pactList.erase(_pactList.begin(), _pactList.end());
		_happyList.erase(_happyList.begin(), _happyList.end());
		_sadList.erase(_sadList.begin(), _sadList.end());
		_gameOver = true;
	}
	ss4 << satisFactionString;

	if (!_gameOver)
	{
		if (_game->getSavedGame()->getFunds() <= -1000000)
		{
			if (_game->getSavedGame()->getWarned())
			{
				ss4.str(L"");
				ss4 << tr("STR_YOU_HAVE_NOT_SUCCEEDED");
				_pactList.erase(_pactList.begin(), _pactList.end());
				_happyList.erase(_happyList.begin(), _happyList.end());
				_sadList.erase(_sadList.begin(), _sadList.end());
				_gameOver = true;
			}
			else
			{
				ss4 << "\n\n" << tr("STR_COUNCIL_REDUCE_DEBTS");
				_game->getSavedGame()->setWarned(true);
				resetWarning = false;
			}
		}
	}

	if (resetWarning && _game->getSavedGame()->getWarned())
		_game->getSavedGame()->setWarned(false);

	ss4 << countryList(_happyList, "STR_COUNTRY_IS_PARTICULARLY_PLEASED", "STR_COUNTRIES_ARE_PARTICULARLY_HAPPY");
	ss4 << countryList(_sadList, "STR_COUNTRY_IS_UNHAPPY_WITH_YOUR_ABILITY", "STR_COUNTRIES_ARE_UNHAPPY_WITH_YOUR_ABILITY");
	ss4 << countryList(_pactList, "STR_COUNTRY_HAS_SIGNED_A_SECRET_PACT", "STR_COUNTRIES_HAVE_SIGNED_A_SECRET_PACT");
	
	_txtDesc->setText(ss4.str());
}


/**
 *
 */
MonthlyReportState::~MonthlyReportState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void MonthlyReportState::btnOkClick(Action *)
{
	if (!_gameOver)
	{
		_game->popState();
		if (_psi)
		{
			_game->pushState(new PsiTrainingState(_game));
		}
		// Autosave
		if (_game->getSavedGame()->isIronman())
		{
			_game->pushState(new SaveGameState(_game, OPT_GEOSCAPE, SAVE_IRONMAN));
		}
		else if (Options::autosave)
		{
			_game->pushState(new SaveGameState(_game, OPT_GEOSCAPE, SAVE_AUTO_GEOSCAPE));
		}
	}
	else
	{
		if (_txtFailure->getVisible())
		{
			_game->popState();
			_game->pushState(new DefeatState(_game));
		}
		else
		{
			_window->setColor(Palette::blockOffset(8)+10);
			_txtTitle->setVisible(false);
			_txtMonth->setVisible(false);
			_txtRating->setVisible(false);
			_txtChange->setVisible(false);
			_txtDesc->setVisible(false);
			_btnOk->setVisible(false);
			_btnBigOk->setVisible(true);
			_txtFailure->setVisible(true);
			_game->getResourcePack()->playMusic("GMLOSE");
		}
	}
}

/**
 * Update all our activity counters, gather all our scores, 
 * get our countries to make sign pacts, adjust their fundings,
 * assess their satisfaction, and finally calculate our overall
 * total score, with thanks to Volutar for the formulas.
 */
void MonthlyReportState::calculateChanges()
{
	// initialize all our variables.
	_lastMonthsRating = 0;
	int xcomSubTotal = 0;
	int xcomTotal = 0;
	int alienTotal = 0;
	int monthOffset = _game->getSavedGame()->getFundsList().size() - 2;
	int lastMonthOffset = _game->getSavedGame()->getFundsList().size() - 3;
	if (lastMonthOffset < 0)
		lastMonthOffset += 2;

	// update activity meters, calculate a total score based on regional activity
	// and gather last month's score
	for (std::vector<Region*>::iterator k = _game->getSavedGame()->getRegions()->begin(); k != _game->getSavedGame()->getRegions()->end(); ++k)
	{
		(*k)->newMonth();
		if ((*k)->getActivityXcom().size() > 2)
			_lastMonthsRating += (*k)->getActivityXcom().at(lastMonthOffset)-(*k)->getActivityAlien().at(lastMonthOffset);
		xcomSubTotal += (*k)->getActivityXcom().at(monthOffset);
		alienTotal += (*k)->getActivityAlien().at(monthOffset);
	}

	// apply research bonus AFTER calculating our total, because this bonus applies to the council ONLY,
	// and shouldn't influence each country's decision.

	// the council is more lenient after the first month
	if (_game->getSavedGame()->getMonthsPassed() > 1)
		_game->getSavedGame()->getResearchScores().at(monthOffset) += 400;

	xcomTotal = _game->getSavedGame()->getResearchScores().at(monthOffset) + xcomSubTotal;


	if (_game->getSavedGame()->getResearchScores().size() > 2)
		_lastMonthsRating += _game->getSavedGame()->getResearchScores().at(lastMonthOffset);


	// now that we have our totals we can send the relevant info to the countries
	// and have them make their decisions weighted on the council's perspective.
	for (std::vector<Country*>::iterator k = _game->getSavedGame()->getCountries()->begin(); k != _game->getSavedGame()->getCountries()->end(); ++k)
	{
		// add them to the list of new pact members 
		// this is done BEFORE initiating a new month
		// because the _newPact flag will be reset in the
		// process
		if ((*k)->getNewPact())
		{
			_pactList.push_back((*k)->getRules()->getType());
		}

		// determine satisfaction level, sign pacts, adjust funding
		// and update activity meters,
		(*k)->newMonth(xcomTotal, alienTotal);

		// and after they've made their decisions, calculate the difference, and add
		// them to the appropriate lists.
		_fundingDiff += (*k)->getFunding().back()-(*k)->getFunding().at((*k)->getFunding().size()-2);
		switch((*k)->getSatisfaction())
		{
		case 1:
			_sadList.push_back((*k)->getRules()->getType());
			break;
		case 3:
			_happyList.push_back((*k)->getRules()->getType());
			break;
		default:
			break;
		}
	}

	//calculate total.
	_ratingTotal = xcomTotal - alienTotal;
}

/**
 * Builds a sentence from a list of countries, adding the appropriate
 * separators and pluralization.
 * @param countries List of country string IDs.
 * @param singular String ID to append at the end if the list is singular.
 * @param plural String ID to append at the end if the list is plural.
 */
std::wstring MonthlyReportState::countryList(const std::vector<std::string> &countries, const std::string &singular, const std::string &plural)
{
	std::wostringstream ss;
	if (!countries.empty())
	{
		ss << "\n\n";
		if (countries.size() == 1)
		{
			ss << tr(singular).arg(tr(countries.front()));
		}
		else
		{
			LocalizedText list = tr(countries.front());
			std::vector<std::string>::const_iterator i;
			for (i = countries.begin() + 1; i < countries.end() - 1; ++i)
			{
				list = tr("STR_COUNTRIES_COMMA").arg(list).arg(tr(*i));
			}
			list = tr("STR_COUNTRIES_AND").arg(list).arg(tr(*i));
			ss << tr(plural).arg(list);
		}
	}
	return ss.str();
}

}
