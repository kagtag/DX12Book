#pragma once
class GameTimer
{
public:

	GameTimer();

	float TotalTime() const;
	float DeltaTime() const;

	void Reset(); 
	void Start();
	void Stop();
	void Tick();

private:
	double mSecondsPerCount;	// in seconds
	double mDeltaTime;			// in seconds

	__int64 mBaseTime;		// time point when Reset() gets called
	__int64 mPausedTime;	// how long game has been paused in total
	__int64 mStopTime;		// time point when game paused
	__int64 mPrevTime;		
	__int64 mCurrTime;

	bool mStopped;
};

