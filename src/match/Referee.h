#ifndef REFEREE_H
#define REFEREE_H

#include <boost/shared_ptr.hpp>

#include "match/MatchEntity.h"
#include "match/Distance.h"
#include "match/Clock.h"

class Match;
class RefereeAction;
class Player;

enum class MatchHalf;

class Referee {
	public:
		Referee();
		void setMatch(Match* m);
		boost::shared_ptr<RefereeAction> act(double time);
		bool ballKicked(const Player& p, const AbsVector3& vel);
		bool isFirstTeamInControl() const;
		void ballGrabbed(const Player& p);
		void matchHalfChanged(MatchHalf m);
		const Player* getPlayerInControl() const;
	private:
		bool allPlayersOnOwnSideAndReady() const;
		boost::shared_ptr<RefereeAction> setOutOfPlay();
		Match* mMatch;
		bool mFirstTeamInControl;
		AbsVector3 mRestartPosition;
		Countdown mOutOfPlayClock;
		Countdown mWaitForResumeClock;
		const Player* mPlayerInControl;
};

#endif

