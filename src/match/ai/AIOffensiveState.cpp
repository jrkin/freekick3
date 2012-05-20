#include "match/ai/AIPlayStates.h"
#include "match/PlayerActions.h"
#include "match/MatchHelpers.h"
#include "match/ai/AIHelpers.h"

AIOffensiveState::AIOffensiveState(Player* p, AIPlayController* m)
	: AIState(p, m)
{
}

std::shared_ptr<PlayerAction> AIOffensiveState::actOnBall(double time)
{
	return mPlayController->switchState(std::shared_ptr<AIState>(new AIKickBallState(mPlayer, mPlayController)), time);
}

std::shared_ptr<PlayerAction> AIOffensiveState::actNearBall(double time)
{
	mDescription = std::string("Offensive");
	return AIHelpers::createMoveActionToBall(*mPlayer);
}

std::shared_ptr<PlayerAction> AIOffensiveState::actOffBall(double time)
{
	if(mPlayer->getPlayerPosition() != Soccer::PlayerPosition::Forward &&
			!MatchHelpers::myTeamInControl(*mPlayer)) {
		return mPlayController->switchState(std::shared_ptr<AIState>(new AIDefendState(mPlayer, mPlayController)), time);
	}
	else {
		mDescription = std::string("Supporting");
		return AIHelpers::createMoveActionTo(*mPlayer,
				AIHelpers::getShotPosition(*mPlayer));
	}
}


