#pragma once

#include "Debug.h"

void ErrorMsg( const char* file, const char* function, int line, std::string msg )
{
	OutputDebugStringA(file);
	OutputDebugStringA(function);
	char linestr[16];
	OutputDebugStringA(itoa(line,linestr,10));
	msg.append("\n");	// Newline
	OutputDebugStringA( msg.c_str() );
}