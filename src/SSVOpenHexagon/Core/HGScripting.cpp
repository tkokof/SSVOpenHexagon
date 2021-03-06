// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <fstream>
#include <SSVUtilsJson/SSVUtilsJson.hpp>
#include "SSVOpenHexagon/Global/Assets.hpp"
#include "SSVOpenHexagon/Utils/Utils.hpp"
#include "SSVOpenHexagon/Core/HexagonGame.hpp"
#include "SSVOpenHexagon/Components/CWall.hpp"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace sses;
using namespace ssvu;
using namespace ssvuj;

namespace hg
{
	void HexagonGame::initLua()
	{
		// Utils
		lua.writeVariable("u_log", 					[=](string mLog) 						{ lo("lua") << mLog << endl; });
		lua.writeVariable("u_execScript", 			[=](string mName) 						{ runLuaFile(levelData->packPath + "Scripts/" + mName); });
		lua.writeVariable("u_playSound", 			[=](string mId) 						{ assets.playSound(mId); });
		lua.writeVariable("u_isKeyPressed",			[=](int mKey) 							{ return window.isKeyPressed((KKey) mKey); });
		lua.writeVariable("u_isFastSpinning",		[=] 									{ return status.fastSpin > 0; });
		lua.writeVariable("u_forceIncrement", 		[=]				 						{ incrementDifficulty(); });
		lua.writeVariable("u_kill",					[=]										{ timeline.append<Do>([=]{ death(true); }); });
		lua.writeVariable("u_eventKill",			[=]										{ eventTimeline.append<Do>([=]{ death(true); }); });
		lua.writeVariable("u_getDifficultyMult",	[=] 									{ return difficultyMult; });
		lua.writeVariable("u_getSpeedMultDM",		[=]										{ return getSpeedMultDM(); });
		lua.writeVariable("u_getDelayMultDM",		[=]										{ return getDelayMultDM(); });

		// Messages
		lua.writeVariable("m_messageAdd", 			[=](string mMsg, float mDuration)		{ eventTimeline.append<Do>([=]{ if(firstPlay && Config::getShowMessages()) addMessage(mMsg, mDuration); }); });
		lua.writeVariable("m_messageAddImportant",	[=](string mMsg, float mDuration)		{ eventTimeline.append<Do>([=]{ if(Config::getShowMessages()) addMessage(mMsg, mDuration); }); });

		// Main timeline control
		lua.writeVariable("t_wait", 				[=](float mDuration) 					{ timeline.append<Wait>(mDuration); });
		lua.writeVariable("t_waitS", 				[=](float mDuration) 					{ timeline.append<Wait>(mDuration * 60.f); });
		lua.writeVariable("t_waitUntilS", 			[=](float mDuration)
		{
			timeline.append<Wait>(10);
			timeline.append<Do>([=]{ if(status.currentTime < mDuration) timeline.jumpTo(timeline.getCurrentIndex() - 2); });
		});

		// Event timeline control
		lua.writeVariable("e_eventStopTime",		[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration; }); });
		lua.writeVariable("e_eventStopTimeS",		[=](float mDuration) 					{ eventTimeline.append<Do>([=]{ status.timeStop = mDuration * 60.f; }); });
		lua.writeVariable("e_eventWait",			[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration); });
		lua.writeVariable("e_eventWaitS", 			[=](float mDuration) 					{ eventTimeline.append<Wait>(mDuration * 60.f); });
		lua.writeVariable("e_eventWaitUntilS", 		[=](float mDuration)
		{
			eventTimeline.append<Wait>(10);
			eventTimeline.append<Do>([=]{ if(status.currentTime < mDuration) eventTimeline.jumpTo(eventTimeline.getCurrentIndex() - 2); });
		});

		// Level control
		lua.writeVariable("l_setSpeedMult",			[=](float mValue)						{ levelStatus.speedMult = mValue; });
		lua.writeVariable("l_setSpeedInc",			[=](float mValue)						{ levelStatus.speedInc = mValue; });
		lua.writeVariable("l_setRotationSpeed",		[=](float mValue)						{ levelStatus.rotationSpeed = mValue; });
		lua.writeVariable("l_setRotationSpeedMax",	[=](float mValue)						{ levelStatus.rotationSpeedMax = mValue; });
		lua.writeVariable("l_setRotationSpeedInc",	[=](float mValue)						{ levelStatus.rotationSpeedInc = mValue; });
		lua.writeVariable("l_setDelayMult",			[=](float mValue)						{ levelStatus.delayMult = mValue; });
		lua.writeVariable("l_setDelayInc",			[=](float mValue)						{ levelStatus.delayInc = mValue; });
		lua.writeVariable("l_setFastSpin",			[=](float mValue)						{ levelStatus.fastSpin = mValue; });
		lua.writeVariable("l_setSides",				[=](int mValue)							{ levelStatus.sides = mValue; });
		lua.writeVariable("l_setSidesMin",			[=](int mValue)							{ levelStatus.sidesMin = mValue; });
		lua.writeVariable("l_setSidesMax",			[=](int mValue)							{ levelStatus.sidesMax = mValue; });
		lua.writeVariable("l_setIncTime",			[=](float mValue)						{ levelStatus.incTime = mValue; });
		lua.writeVariable("l_setPulseMin",			[=](float mValue)						{ levelStatus.pulseMin = mValue; });
		lua.writeVariable("l_setPulseMax",			[=](float mValue)						{ levelStatus.pulseMax = mValue; });
		lua.writeVariable("l_setPulseSpeed",		[=](float mValue)						{ levelStatus.pulseSpeed = mValue; });
		lua.writeVariable("l_setPulseSpeedR",		[=](float mValue)						{ levelStatus.pulseSpeedR = mValue; });
		lua.writeVariable("l_setPulseDelayMax",		[=](float mValue)						{ levelStatus.pulseDelayMax = mValue; });
		lua.writeVariable("l_setBeatPulseMax",		[=](float mValue)						{ levelStatus.beatPulseMax = mValue; });
		lua.writeVariable("l_setBeatPulseDelayMax",	[=](float mValue)						{ levelStatus.beatPulseDelayMax = mValue; });
		lua.writeVariable("l_setWallSkewLeft",		[=](float mValue)						{ levelStatus.wallSkewLeft = mValue; });
		lua.writeVariable("l_setWallSkewRight",		[=](float mValue)						{ levelStatus.wallSkewRight = mValue; });
		lua.writeVariable("l_setWallAngleLeft",		[=](float mValue)						{ levelStatus.wallAngleLeft = mValue; });
		lua.writeVariable("l_setWallAngleRight",	[=](float mValue)						{ levelStatus.wallAngleRight = mValue; });
		lua.writeVariable("l_setRadiusMin",			[=](float mValue)						{ levelStatus.radiusMin = mValue; });
		lua.writeVariable("l_setSwapEnabled",		[=](bool mValue)						{ levelStatus.swapEnabled = mValue; });
		lua.writeVariable("l_setTutorialMode",		[=](bool mValue)						{ levelStatus.tutorialMode = mValue; });
		lua.writeVariable("l_setIncEnabled",		[=](bool mValue)						{ levelStatus.incEnabled = mValue; });
		lua.writeVariable("l_addTracked",			[=](string mVar, string mName)			{ levelStatus.trackedVariables.emplace_back(mVar, mName); });
		lua.writeVariable("l_enableRndSideChanges", [=](bool mValue)						{ levelStatus.rndSideChangesEnabled = mValue; });
		lua.writeVariable("l_getRotationSpeed",		[=]										{ return levelStatus.rotationSpeed; });
		lua.writeVariable("l_getSides",				[=]										{ return levelStatus.sides; });
		lua.writeVariable("l_getSpeedMult",			[=]										{ return levelStatus.speedMult; });
		lua.writeVariable("l_getDelayMult",			[=]										{ return levelStatus.delayMult; });

		// Style control
		lua.writeVariable("s_setPulseInc",			[=](float mValue)						{ styleData.pulseIncrement = mValue; });
		lua.writeVariable("s_setHueInc",			[=](float mValue)						{ styleData.hueIncrement = mValue; });
		lua.writeVariable("s_getHueInc",			[=]										{ return styleData.hueIncrement; });

		// Wall creation
		lua.writeVariable("w_wall",					[=](int mSide, float mThickness)																					{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {getSpeedMultDM()}); }); });
		lua.writeVariable("w_wallAdj",				[=](int mSide, float mThickness, float mSpeedAdj)																	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, mSpeedAdj * getSpeedMultDM()); }); });
		lua.writeVariable("w_wallAcc",				[=](int mSide, float mThickness, float mSpeedAdj, float mAcceleration, float mMinSpeed, float mMaxSpeed)			{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {mSpeedAdj * getSpeedMultDM(), mAcceleration, mMinSpeed * getSpeedMultDM(), mMaxSpeed * getSpeedMultDM()}); }); });
		lua.writeVariable("w_wallHModSpeedData",	[=](float mHMod, int mSide, float mThickness, float mSAdj, float mSAcc, float mSMin, float mSMax, bool mSPingPong)	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {mSAdj * getSpeedMultDM(), mSAcc, mSMin, mSMax, mSPingPong}, mHMod); }); });
		lua.writeVariable("w_wallHModCurveData",	[=](float mHMod, int mSide, float mThickness, float mCAdj, float mCAcc, float mCMin, float mCMax, bool mCPingPong)	{ timeline.append<Do>([=]{ factory.createWall(mSide, mThickness, {getSpeedMultDM()}, {mCAdj, mCAcc, mCMin, mCMax, mCPingPong}, mHMod); }); });
	}
}
