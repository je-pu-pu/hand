#include "Hand.h"

#ifdef _DEBUG
#pragma comment( lib, "gamma_debug.lib" )
#else
#pragma comment( lib, "gamma.lib" )
#endif

int main( int, char** )
{
	Hand hand;
	hand.start();

	return 0;
}