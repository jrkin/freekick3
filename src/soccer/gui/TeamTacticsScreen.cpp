#include <algorithm>

#include "soccer/Match.h"
#include "soccer/Team.h"

#include "soccer/gui/TeamTacticsScreen.h"
#include "soccer/gui/Menu.h"

namespace Soccer {

TeamTacticsScreen::TeamTacticsScreen(boost::shared_ptr<ScreenManager> sm, Match& m,
			std::function<void (Match& m)> cont)
	: Screen(sm),
	mMatch(m),
	mChosenplnum(0),
	mHumanTeam(-1),
	mPitchRect(0.61f, 0.12f, 0.35f, 0.58f),
	mCont(cont)
{
	addButton("Back",   Common::Rectangle(0.01f, 0.90f, 0.23f, 0.06f));
	mToggleButtons[0] = addButton("Home",  Common::Rectangle(0.51f, 0.90f, 0.23f, 0.06f));
	mToggleButtons[1] = addButton("Away",  Common::Rectangle(0.51f, 0.90f, 0.23f, 0.06f));
	mToggleButtons[0]->hide();
	mTeamLabels[0] = addButton(m.getTeam(0)->getName().c_str(), Common::Rectangle(0.40f, 0.05f, 0.23f, 0.06f));
	mTeamLabels[1] = addButton(m.getTeam(1)->getName().c_str(), Common::Rectangle(0.40f, 0.05f, 0.23f, 0.06f));
	mTeamLabels[1]->hide();

	addButton("Match",  Common::Rectangle(0.76f, 0.90f, 0.23f, 0.06f), true, SDLK_m);

	addImage("share/pitch.png", mPitchRect);

	for(int i = 0; i < 2; i++) {
		setupTeamDisplay(i);
		if(mMatch.getTeam(i)->getController().HumanControlled)
			Menu::setButtonHumanColor(mTeamLabels[i]);
		else
			Menu::setButtonComputerColor(mTeamLabels[i]);
	}

	if(mHumanTeam == 1) {
		buttonPressed(mToggleButtons[1]);
	}
}

void TeamTacticsScreen::setupTeamDisplay(int i)
{
	float x = 0.05f;
	float y = 0.15f;
	const float namewidth = 0.20f;
	const float nameheight = 0.03f;

	int teamd = 0;
	int teamm = 0;
	int teamf = 0;

	for(auto p : mPlayers[i]) {
		removeButton(p.first);
	}
	mPlayers[i].clear();

	for(auto p : mPlayerControllerButtons[i]) {
		removeButton(p.first);
	}
	mPlayerControllerButtons[i].clear();

	for(auto p : mTacticsSliders[i]) {
		removeSlider(p);
	}
	mTacticsSliders[i].clear();

	for(auto b : mPlayerLabels[i]) {
		removeButton(b);
	}
	mPlayerLabels[i].clear();

	for(auto b : mFormationButtons[i]) {
		removeButton(b);
	}
	mFormationButtons[i].clear();

	mFormationNumbers[i] = "";

	for(auto b : mSkillLabels[i]) {
		removeButton(b);
	}
	mSkillLabels[i].clear();

	for(auto b : mTop3Labels[i]) {
		removeButton(b);
	}
	mTop3Labels[i].clear();

	if(mMatch.getTeam(i)->getController().HumanControlled) {
		if(mHumanTeam != -1 && mHumanTeam != i) {
			// both teams are human-controlled - don't allow choosing anything.
			mHumanTeam = -1;
			mChosenplnum = 0;
		}
		else {
			mHumanTeam = i;
			mChosenplnum = mMatch.getTeam(i)->getController().PlayerShirtNumber;
		}
	}
	int plnum = 0;
	for(auto p : mMatch.getTeam(i)->getTactics().mTactics) {
		plnum++;
		auto pl = mMatch.getTeam(i)->getPlayerById(p.first);
		assert(pl);

		// player skill labels
		addPlayerLabels(pl, i, x, y, namewidth, nameheight);

		// player controller button
		{
			boost::shared_ptr<Button> b = addButton("C", Common::Rectangle(x + namewidth + 0.07f, y, nameheight, nameheight));
			b->setCenteredText(TextAlignment::Centered);
			mPlayerControllerButtons[i].insert(std::make_pair(b, plnum));
		}

		// player label on the pitch
		{
			float pitchx = mPitchRect.x + (-p.second.WidthPosition * 1.15f + 1.0f) * 0.5f * mPitchRect.w;
			float pitchy = mPitchRect.y + (int)(p.second.Position) * 0.22f * mPitchRect.h;

			// give text some more space
			if((fabs(p.second.WidthPosition) > 0.35f) == (p.second.Position != PlayerPosition::Forward &&
						p.second.Position != PlayerPosition::Goalkeeper))
				pitchy += 0.05f;

			mPlayerLabels[i].push_back(addLabel(Player::getShorterName(*pl).c_str(), pitchx, pitchy,
						TextAlignment::Centered, 0.5f, Common::Color::White));
		}

		y += 0.04f;

		if(p.second.Position == PlayerPosition::Defender) {
			teamd++;
		}
		else if(p.second.Position == PlayerPosition::Midfielder) {
			teamm++;
		}
		else if(p.second.Position == PlayerPosition::Forward) {
			teamf++;
		}
	}

	y += 0.04f;

	// substitutes
	for(auto p : mMatch.getTeam(i)->getPlayers()) {
		auto it = mMatch.getTeam(i)->getTactics().mTactics.find(p->getId());
		if(it == mMatch.getTeam(i)->getTactics().mTactics.end()) {
			addPlayerLabels(p, i, x, y, namewidth, nameheight);

			y += 0.04f;

			/* TODO: handle case with more substitutes, by adding some scrolling thingy */
			if(y >= 0.90f - 0.04f)
				break;
		}
	}

	int defs[] = {4, 5, 3};
	int midfs[] = {4, 5, 3};
	int forws[] = {2, 1, 3};

	int formbutton = 0;

	for(auto def : defs) {
		for(auto midf : midfs) {
			for(auto forw : forws) {
				if(formbutton < 9 && def + midf + forw == 10) {
					std::stringstream ss;
					ss << def << "-" << midf << "-" << forw;
					if(teamd == def && teamm && midf && teamf == forw) {
						assert(mFormationNumbers[i].empty());
						mFormationNumbers[i] = ss.str();
					}
					addFormationButton(i, formbutton, ss.str());
					formbutton++;
				}
			}
		}
	}

	if(mFormationNumbers[i].empty() && formbutton < 9) {
		std::stringstream ss;
		ss << teamd << "-" << teamm << "-" << teamf;
		addFormationButton(i, formbutton, ss.str());
	}

	const float sliderX = 0.40f;
	float sliderY = 0.22f;
	const float sliderW = 0.20f;
	const float sliderH = 0.05f;

	mTacticsSliders[i].push_back(addSlider("-      Pressure      +",
				Common::Rectangle(sliderX, sliderY, sliderW, sliderH),
				mMatch.getTeam(i)->getTactics().Pressure));

	sliderY += sliderH * 1.05f;
	mTacticsSliders[i].push_back(addSlider("-     Long balls     +",
				Common::Rectangle(sliderX, sliderY, sliderW, sliderH),
				mMatch.getTeam(i)->getTactics().LongBalls));

	sliderY += sliderH * 1.05f;
	mTacticsSliders[i].push_back(addSlider("-    Fast passing    +",
				Common::Rectangle(sliderX, sliderY, sliderW, sliderH),
				mMatch.getTeam(i)->getTactics().FastPassing));

	sliderY += sliderH * 1.05f;
	mTacticsSliders[i].push_back(addSlider("-    Close shots    +",
				Common::Rectangle(sliderX, sliderY, sliderW, sliderH),
				mMatch.getTeam(i)->getTactics().ShootClose));

	for(auto s : mTacticsSliders[i]) {
		s->setColor1(Common::Color(112, 95, 64));
	}

	setupPlrLabels();
}

void TeamTacticsScreen::addPlayerLabels(const boost::shared_ptr<Player> pl, int i, float x,
		float y, float namewidth, float nameheight)
{
	// player name button
	{
		boost::shared_ptr<Button> b = addButton(pl->getName().c_str(), Common::Rectangle(x, y, namewidth, nameheight));
		b->setCenteredText(TextAlignment::MiddleLeft);
		mPlayers[i].insert(std::make_pair(b, pl->getId()));
	}

	bool gk = false;
	// player skill index label
	{
		Common::Color col = Common::Color::White;
		int skill = Player::getSkillIndex(*pl);
		if(skill < 0) {
			col = Common::Color::Yellow;
			gk = true;
		}
		mSkillLabels[i].push_back(addLabel(std::to_string(abs(skill / 40)).c_str(),
					x + namewidth + 0.01f, y + nameheight * 0.5f,
					TextAlignment::MiddleLeft,
					0.5f, col));
	}

	// top three skills label
	if(!gk) {
		const std::string top = Player::getTopSkillsString(*pl);
		mTop3Labels[i].push_back(addLabel(top.c_str(),
					x + namewidth + 0.030, y + nameheight * 0.5f,
					TextAlignment::MiddleLeft,
					0.5f, Common::Color::White));
	}
}

void TeamTacticsScreen::addFormationButton(int i, int formbutton, const std::string& str)
{
	const float formx = 0.51f;
	const float formy = 0.71f;

	Common::Rectangle rect(formx + (formbutton % 3) * 0.16f,
			formy + formbutton / 3 * 0.06f,
			0.15f, 0.05f);
	mFormationButtons[i].push_back(addButton(str.c_str(), rect));
}

void TeamTacticsScreen::setupPlrLabels()
{
	for(int i = 0; i < 2; i++) {
		for(auto l : mFormationButtons[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0)) {
				l->show();
				if(mFormationNumbers[i] == l->getText()) {
					Menu::setButtonSelectedColor(l);
				}
				else {
					Menu::setButtonDefaultColor(l);
				}
			}
			else
				l->hide();
		}
		for(auto l : mPlayerLabels[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l->show();
			else
				l->hide();
		}
		for(auto l : mSkillLabels[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l->show();
			else
				l->hide();
		}
		for(auto l : mTop3Labels[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l->show();
			else
				l->hide();
		}
		for(auto l : mPlayers[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l.first->show();
			else
				l.first->hide();
		}
		for(auto l : mPlayerControllerButtons[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l.first->show();
			else
				l.first->hide();

			if(mHumanTeam == i && (l.second == mChosenplnum || mChosenplnum == 0)) {
				Menu::setButtonHumanColor(l.first);
				l.first->setText("H");
			}
			else {
				Menu::setButtonComputerColor(l.first);
				l.first->setText("C");
			}
		}
		for(auto l : mTacticsSliders[i]) {
			if((mToggleButtons[0]->hidden()) == (i == 0))
				l->show();
			else
				l->hide();
			if(i == mHumanTeam)
				l->activate();
			else
				l->deactivate();
		}
	}
}

void TeamTacticsScreen::buttonPressed(boost::shared_ptr<Button> button)
{
	const std::string& buttonText = button->getText();
	if(buttonText == "Back") {
		setTeamTactics();
		mScreenManager->dropScreen();
	}
	else if(buttonText == "Home") {
		mToggleButtons[0]->hide();
		mToggleButtons[1]->show();
		mTeamLabels[0]->show();
		mTeamLabels[1]->hide();
		if(mSelectedPlayer.first) {
			Menu::setButtonDefaultColor(mSelectedPlayer.first);
			mSelectedPlayer.first = boost::shared_ptr<Button>();
		}
		setupPlrLabels();
	}
	else if(buttonText == "Away") {
		mToggleButtons[0]->show();
		mToggleButtons[1]->hide();
		mTeamLabels[0]->hide();
		mTeamLabels[1]->show();
		if(mSelectedPlayer.first) {
			Menu::setButtonDefaultColor(mSelectedPlayer.first);
			mSelectedPlayer.first = boost::shared_ptr<Button>();
		}
		setupPlrLabels();
	}
	else if(buttonText == "Match") {
		setTeamTactics();
		mCont(mMatch);
	}
	else {
		if(mHumanTeam != -1) {
			// check if player name button clicked
			{
				auto it = mPlayers[mHumanTeam].find(button);
				if(it != mPlayers[mHumanTeam].end()) {
					if(!mSelectedPlayer.first) {
						mSelectedPlayer.first = it->first;
						mSelectedPlayer.second = it->second;
						Menu::setButtonComputerColor(mSelectedPlayer.first);
					}
					else if(mSelectedPlayer.first == it->first) {
						Menu::setButtonDefaultColor(mSelectedPlayer.first);
						mSelectedPlayer.first = boost::shared_ptr<Button>();
					}
					else {
						// switch players
						auto pl1 = mMatch.getTeam(mHumanTeam)->getTactics().mTactics.find(mSelectedPlayer.second);
						auto pl2 = mMatch.getTeam(mHumanTeam)->getTactics().mTactics.find(it->second);
						if(pl1 != mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end() &&
						   pl2 != mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end()) {
							// both in lineup
							std::swap(pl1->second, pl2->second);
						}
						else if((pl1 == mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end()) !=
							(pl2 == mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end())) {
							// only one in lineup
							auto& value = pl1 == mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end() ?
								pl2->second : pl1->second;
							auto key = pl1 == mMatch.getTeam(mHumanTeam)->getTactics().mTactics.end() ?
								mSelectedPlayer.second : it->second;
							mMatch.getTeam(mHumanTeam)->getTactics().mTactics.erase(mSelectedPlayer.second);
							mMatch.getTeam(mHumanTeam)->getTactics().mTactics.erase(it->second);
							mMatch.getTeam(mHumanTeam)->getTactics().mTactics.insert(std::make_pair(key, value));
							assert(mMatch.getTeam(mHumanTeam)->getTactics().mTactics.size() == 11);
						}
						Menu::setButtonDefaultColor(mSelectedPlayer.first);
						mSelectedPlayer.first = boost::shared_ptr<Button>();
						setupTeamDisplay(mHumanTeam);
					}
				}
			}

			// check if player controller button clicked
			{
				auto it = mPlayerControllerButtons[mHumanTeam].find(button);
				if(it != mPlayerControllerButtons[mHumanTeam].end() && it->second != -1) {
					if(mChosenplnum != 0 && mChosenplnum == it->second)
						mChosenplnum = 0;
					else
						mChosenplnum = it->second;
					setupPlrLabels();
					return;
				}
			}

			// check if formation changed
			{
				auto it = std::find(mFormationButtons[mHumanTeam].begin(),
						mFormationButtons[mHumanTeam].end(), button);
				if(it != mFormationButtons[mHumanTeam].end() &&
						(*it)->getText() != mFormationNumbers[mHumanTeam]) {
					unsigned int def, mid, forw;
					if(sscanf((*it)->getText().c_str(), "%u-%u-%u", &def, &mid, &forw) == 3) {
						assert(def + mid + forw == 10);
						mMatch.getTeam(mHumanTeam)->setTactics(
								AITactics::updateTeamTactics(mMatch.getTeam(mHumanTeam)->getTactics(),
									def, mid, forw));
						setupTeamDisplay(mHumanTeam);
					}
				}
			}
		}

	}
}

const std::string TeamTacticsScreen::ScreenName = std::string("Team Tactics Screen");

const std::string& TeamTacticsScreen::getName() const
{
	return ScreenName;
}

void TeamTacticsScreen::setTeamTactics()
{
	if(mHumanTeam != -1) {
		assert(mTacticsSliders[mHumanTeam].size() == 4);
		mMatch.getTeam(mHumanTeam)->getTactics().Pressure    = mTacticsSliders[mHumanTeam][0]->getValue();
		mMatch.getTeam(mHumanTeam)->getTactics().LongBalls   = mTacticsSliders[mHumanTeam][1]->getValue();
		mMatch.getTeam(mHumanTeam)->getTactics().FastPassing = mTacticsSliders[mHumanTeam][2]->getValue();
		mMatch.getTeam(mHumanTeam)->getTactics().ShootClose  = mTacticsSliders[mHumanTeam][3]->getValue();
		mMatch.getTeam(mHumanTeam)->getController().PlayerShirtNumber = mChosenplnum;
	}
}

}


