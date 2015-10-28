#include "Windows.h"
#include "GameTimer.h"

GameTimer::GameTimer(): mSecondPerCount(0.0), mDeltaTime(0.0), mBaseTime(0), mPausedTime(0), mStopTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{ 
	__int64 countsPerSec; 
	QueryPerformanceFrequency((LARGE_INTEGER*) & countsPerSec); 
	mSecondPerCount = 1.0 / (double)countsPerSec; 
}


float GameTimer::TotalTime() const
{
	if (mStopped)
		return (float)((mStopTime - mBaseTime - mPausedTime)*mSecondPerCount);
	else
		return (float)((mCurrTime - mBaseTime - mPausedTime)*mSecondPerCount);
}
float GameTimer::DeltaTime() const
{
	return (float)mDeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = currTime;
	mStopped = false;
}
void GameTimer::Resume()
{
	__int64 resumeTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&resumeTime);
	if (mStopped)
	{
		mPausedTime += (resumeTime - mStopTime);
		mPrevTime = resumeTime;
		mStopTime = 0;
		mStopped = false;
	}
}
void GameTimer::Pause()
{
	if (!mStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mStopTime = currTime;
		mStopped = true;
	}
}
void GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondPerCount;
	mPrevTime = mCurrTime;
	if (mDeltaTime < 0.0)
		mDeltaTime = 0.0;
}