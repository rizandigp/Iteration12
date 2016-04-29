#pragma once

#include <Windows.h>


class Timer
{
public:
	Timer()
	{
		QueryPerformanceFrequency( &m_frequency ); 
		Start(); 
	};

	__forceinline void Start();

	__forceinline double GetSeconds();
	__forceinline double GetMiliseconds();
	__forceinline double GetMicroseconds();
	
private:
	LARGE_INTEGER m_frequency, m_counterStart, temp;
};

__forceinline void Timer::Start()
{
	QueryPerformanceCounter( &m_counterStart );
}

__forceinline double Timer::GetSeconds()
{
	QueryPerformanceCounter( &temp );
	return (temp.QuadPart - m_counterStart.QuadPart)/(double)m_frequency.QuadPart;
}

__forceinline double Timer::GetMiliseconds()
{
	QueryPerformanceCounter( &temp );
	return ((temp.QuadPart - m_counterStart.QuadPart)/(double)m_frequency.QuadPart)*1000.0;
}

__forceinline double Timer::GetMicroseconds()
{
	QueryPerformanceCounter( &temp );
	return ((temp.QuadPart - m_counterStart.QuadPart)/(double)m_frequency.QuadPart)*1000000.0;
}