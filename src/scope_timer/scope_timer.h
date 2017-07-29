#pragma once

#if DEBUG

#if TARGET_PRIZM
#include "tmu.h"
#define GetCycles() REG_TMU_TCNT_2
#else
#include <windows.h>
extern unsigned int ScopeTimer_Frequency;
extern LONGLONG ScopeTimer_Start;
inline unsigned int GetCycles() {
	LARGE_INTEGER result;
	if (QueryPerformanceCounter(&result) != 0) {
		return (unsigned int) ((result.QuadPart - ScopeTimer_Start) / ScopeTimer_Frequency);
	}

	return 0;
}
#endif

struct ScopeTimer {
	unsigned int cycleCount;
	unsigned int numCounts;

	const char* funcName;
	int line;

	ScopeTimer* nextTimer;

	ScopeTimer(const char* withFunctionName, int withLine);

	inline void AddTime(unsigned short cycles) {
		cycleCount += cycles;
		numCounts++;
	}

	static ScopeTimer* firstTimer;
	static char debugString[128];			// per application debug string (placed on last row), usually FPS or similar
	static void InitSystem();
	static void DisplayTimes();
	static void Shutdown();
};

struct TimedInstance {
	unsigned int start;
	ScopeTimer* myTimer;

	inline TimedInstance(ScopeTimer* withTimer) : start(GetCycles()), myTimer(withTimer) {
	}

	inline ~TimedInstance() {
#if TARGET_WINSIM
		int elapsed = (int)(GetCycles() - start);
#else
		int elapsed = (int)(start - GetCycles());
#endif

		if (elapsed >= 0) {
			myTimer->AddTime(elapsed);
		}
	}
};

#define TIME_SCOPE() static ScopeTimer __timer(__FUNCTION__, __LINE__); TimedInstance __timeMe(&__timer);
#define TIME_SCOPE_NAMED(Name) static ScopeTimer __timer(#Name, __LINE__); TimedInstance __timeMe(&__timer);
#else
struct ScopeTimer {
	static void InitSystem() {}
	static void DisplayTimes() {}
	static void Shutdown() {}
};
#endif

#ifndef TIME_SCOPE
#define TIME_SCOPE() 
#define TIME_SCOPE_NAMED(Name) 
#endif