#ifndef GAMETIMER_H
#define GAMETIMER_H


class GameTimer
{
public:
	GameTimer();
	float TotalTime() const;//return total time since game starts, unit: second
	float DeltaTime() const;//return the time span between two frames

	void Reset();
	void Resume();
	void Pause();
	void Tick();

private:
	double mSecondPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif