#include "Engine/Time/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineCommon.hpp"


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <time.h>


//--------------------------------------------------------------------------------------------------------------
void SeedWindowsRNG()
{
	srand( static_cast<unsigned int>( time( NULL ) ) );
}

//--------------------------------------------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency( &countsPerSecond );
	QueryPerformanceCounter( &out_initialTime );

	return ( 1.0 / static_cast<double>( countsPerSecond.QuadPart ) );
}


//--------------------------------------------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
	static LARGE_INTEGER initialTime; //Note static implies lifetime beyond function call.
	static double secondsPerCount = InitializeTime( initialTime ); //Never called again due to static.
	LARGE_INTEGER currentCount;
	
	QueryPerformanceCounter( &currentCount );
	
	LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;
	double currentSeconds = static_cast<double>(elapsedCountsSinceInitialTime) * secondsPerCount;
	
	return currentSeconds;
}


//--------------------------------------------------------------------------------------------------------------
float CalcDeltaSeconds()
{
	/* Records on Performance: Alienware m17x R2, GTX 980M
		Release build mode with below loop
			30 FPS - Unplugged any mode
			80-90 FPS - Plugged power saver
			90-120+ FPS - Plugged balanced or high performance
		Release build mode without below loop
			30 FPS - Unplugged any mode
			140-150 FPS - Plugged power saver
			500-600 FPS - Plugged balanced or high performance
	*/

	static double s_timeLastFrameBegan = GetCurrentTimeSeconds(); //Called only once due static.
	double timeThisFrameBegan = GetCurrentTimeSeconds();

	float deltaSeconds = static_cast<float>( timeThisFrameBegan - s_timeLastFrameBegan );

	const double FRAMES_PER_SECOND = 60.0;
	const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;
	const float MAX_DELTA = 1.0f;

	while ( deltaSeconds < SECONDS_PER_FRAME )
	{
		TODO( "Replace with more sophisticated framerate timer system." );
		Sleep( 0 );
		//Further apart because s_timeLastFrameBegan isn't updated:
		deltaSeconds = static_cast<float>( GetCurrentTimeSeconds() - s_timeLastFrameBegan );
	}

	s_timeLastFrameBegan = timeThisFrameBegan;

	return GetMin( deltaSeconds, MAX_DELTA );
}
