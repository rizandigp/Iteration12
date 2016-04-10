#pragma once
#include <Windows.h>

#include "Prerequisites.h"

#ifdef DEBUG
#define DEBUG_OUTPUT( string ) OutputDebugStringA( string );
#define DEBUG_OUTPUT_STD( std::string ) OutputDebugStringA( string.c_str() );
#define DEBUG_OUTPUTW( string ) OutputDebugStringW( string );
#else
#define DEBUG_OUTPUT( string ) OutputDebugStringA( string );
#define DEBUG_OUTPUT_STD( string ) OutputDebugStringA( string.c_str() );
#define DEBUG_OUTPUTW( string ) OutputDebugStringW( string );
#define DEBUG_OUTPUTW_STD( string ) OutputDebugStringW( string.c_str() );
#endif

void ErrorMsg( const char* file, const char* function, int line, std::string msg );

#define NGERROR( msg ) ErrorMsg( __FILE__, __FUNCTION__, __LINE__, msg );
#define NGWARNING( msg ) ErrorMsg( __FILE__, __FUNCTION__, __LINE__, msg );