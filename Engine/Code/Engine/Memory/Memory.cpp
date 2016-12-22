#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/Memory/Memory.hpp"


//--------------------------------------------------------------------------------------------------------------
int g_numberOfAllocations = 0;
int g_totalAllocatedBytes = 0;


//--------------------------------------------------------------------------------------------------------------
void* operator new( size_t numBytes )
{
	size_t* ptr = (size_t*)malloc( numBytes + sizeof( size_t ) );
	//DebuggerPrintf( "Alloc %p of %u bytes.\n", ptr, numBytes );
	++g_numberOfAllocations;
	g_totalAllocatedBytes += numBytes;

	*ptr = numBytes;
	ptr++;
	return ptr;
}


//--------------------------------------------------------------------------------------------------------------
void operator delete( void* ptr )
{
	size_t* ptrSize = (size_t*)ptr;
	--ptrSize;
	size_t numBytes = *ptrSize;

	--g_numberOfAllocations;
	g_totalAllocatedBytes -= numBytes;

	free( ptrSize ); //Free knows how to free the entire malloc, it's not tied to the size_t.
}
