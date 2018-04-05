#include "../../main/Hand.h"

#include "ofMain.h"
#include "ofApp.h"

#ifdef _DEBUG
#pragma comment( lib, "gamma_debug.lib" )
#else
#pragma comment( lib, "gamma.lib" )
#endif

//========================================================================
int main( ){
	Hand hand;
	hand.start( false );

	ofSetupOpenGL( 1920, 1200, OF_FULLSCREEN );			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp( hand ) );

}
