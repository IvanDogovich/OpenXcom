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
#ifndef OPENXCOM_ALLOCATEPSITRAININGSTATE_H
#define OPENXCOM_ALLOCATEPSITRAININGSTATE_H

#include "../Engine/State.h"

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class Base;
class TextList;
class Soldier;

/**
 * Screen shown monthly to allow changing
 * soldiers currently in psi training.
 */
class AllocatePsiTrainingState : public State
{
private:
	TextButton *_btnOk;
	Window *_window;
	Text *_txtTitle, *_txtTraining, *_txtName, *_txtRemaining;
	Text *_txtPsiStrength, *_txtPsiSkill;
	TextList *_lstSoldiers;
	std::vector<Soldier*> _soldiers;
	size_t _sel;
	int _labSpace;
	Base *_base;
public:
	/// Creates the Psi Training state.
	AllocatePsiTrainingState(Game *game, Base *base);
	/// Cleans up the Psi Training state.
	~AllocatePsiTrainingState();
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Soldiers list.
	void lstSoldiersClick(Action *action);
};

}

#endif
