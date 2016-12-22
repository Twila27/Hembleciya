#pragma once


//--------------------------------------------------------------------------------------------------------------
extern int g_numberOfAllocations;
extern int g_totalAllocatedBytes;


//--------------------------------------------------------------------------------------------------------------
void* operator new( size_t numBytes );
void* operator new[]( size_t numBytes );
void operator delete( void* ptr );
void operator delete[]( void* ptr );
