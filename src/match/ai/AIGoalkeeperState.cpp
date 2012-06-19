#include "common/Math.h"

#include "match/ai/AIHelpers.h"
#include "match/MatchHelpers.h"
#include "match/ai/AIPlayStates.h"
#include "match/PlayerActions.h"

AIGoalkeeperState::AIGoalkeeperState(Player* p, AIPlayController* m)
	: AIState(p, m),
	mHoldBallTimer(1.0f)
{
	setPivotPoint();
}

boost::shared_ptr<PlayerAction> AIGoalkeeperState::actOnBall(double time)
{
	if(mPlayer->getMatch()->getBall()->grabbed() && mPlayer->getMatch()->getBall()->getGrabber() == mPlayer) {
		// holding the ball
		mHoldBallTimer.doCountdown(time);
		if(mHoldBallTimer.check()) {
			return mPlayController->switchState(boost::shared_ptr<AIState>(new AIKickBallState(mPlayer, mPlayController)), time);
		}
		else {
			return boost::shared_ptr<PlayerAction>(new IdlePA());
		}
	}
	else {
		// not holding the ball
		if(MatchHelpers::myTeamInControl(*mPlayer) || !MatchHelpers::canGrabBall(*mPlayer)) {
			return mPlayController->switchState(boost::shared_ptr<AIState>(new AIKickBallState(mPlayer, mPlayController)), time);
		}
		else {
			mHoldBallTimer.rewind();
			return boost::shared_ptr<PlayerAction>(new GrabBallPA());
		}
	}
}

boost::shared_ptr<PlayerAction> AIGoalkeeperState::actNearBall(double time)
{
	return actOffBall(time);
}

boost::shared_ptr<PlayerAction> AIGoalkeeperState::actOffBall(double time)
{
	const Ball* ball = mPlayer->getMatch()->getBall();
	AbsVector3 ballpos = ball->getPosition();
	Vector3 futureballpos = ballpos.v + ball->getVelocity().v * 5.0f;
	Vector3 goalmiddlepoint = MatchHelpers::ownGoalPosition(*mPlayer).v;

	float balltowardsgoal = Common::Math::pointToLineDistance(ballpos.v, futureballpos, goalmiddlepoint);
	if(balltowardsgoal < GOAL_WIDTH) {
		Vector3 tgtpos = Common::Math::lineLineIntersection2D(ballpos.v,
				futureballpos,
				Vector3(-GOAL_WIDTH, goalmiddlepoint.y, 0),
				Vector3(GOAL_WIDTH, goalmiddlepoint.y, 0));
		// tgtpos may be null when the ball doesn't move
		if(!tgtpos.null()) {
			if(MatchHelpers::attacksUp(*mPlayer))
				tgtpos.y += 1.0f;
			else
				tgtpos.y -= 1.0f;

			return AIHelpers::createMoveActionTo(*mPlayer, AbsVector3(tgtpos));
		}
	}

	AbsVector3 diffvec = ballpos.v - mPivotPoint.v;
	AbsVector3 vectoball = MatchEntity::vectorFromTo(*mPlayer, *mPlayer->getMatch()->getBall());
	if(vectoball.v.length() < 10.0f) {
		AbsVector3 vectogoal = MatchHelpers::ownGoalPosition(*mPlayer).v - mPlayer->getPosition().v;
		if(vectogoal.v.length() < 10.0f) {
			return AIHelpers::createMoveActionTo(*mPlayer, ballpos);
		}
	}
	AbsVector3 point = mPivotPoint.v + diffvec.v.normalized() * mDistanceFromPivot;
	point.v.x = Common::clamp(-3.0f, point.v.x, 3.0f);
	if(mPivotPoint.v.y > 0)
		point.v.y = std::min(mPlayer->getMatch()->getPitchHeight() * 0.5f, point.v.y);
	else
		point.v.y = std::max(mPlayer->getMatch()->getPitchHeight() * -0.5f, point.v.y);

	return AIHelpers::createMoveActionTo(*mPlayer, point);
}

void AIGoalkeeperState::matchHalfChanged(MatchHalf m)
{
	setPivotPoint();
}

void AIGoalkeeperState::setPivotPoint()
{
	if(MatchHelpers::attacksUp(*mPlayer)) {
		mPivotPoint = mPlayer->getMatch()->convertRelativeToAbsoluteVector(RelVector3(0, -1.2, 0));
	}
	else {
		mPivotPoint = mPlayer->getMatch()->convertRelativeToAbsoluteVector(RelVector3(0, 1.2, 0));
	}
	mDistanceFromPivot = mPlayer->getMatch()->getPitchHeight() * 0.13f;
}


