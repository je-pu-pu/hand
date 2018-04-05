#include "../../main/Hand.h"
#include "../../main/HandAudioCallback.h"
#include "ofApp.h"

ofApp::ofApp( Hand& hand )
	: hand_( hand )
{
	hand_.set_on_step( [this] ( bool on_beat, bool on_bar ) {
		if ( on_bar )
		{
			bg_color_ = ofColor( 255, 255, 255 );
		}
		else if ( on_beat )
		{
			bg_color_ = ofColor( 63, 63, 63 );
		}
	} );
}

//--------------------------------------------------------------
void ofApp::setup(){
	font.load( "mplus-1m-bold.ttf", 320, true, false, true );

	/*
	for ( int n = 0; n < 9; n++ )
	{
		std::cout << n << ":" << font.stringWidth( std::to_string( n ) ) << std::endl;
	}
	*/
}

//--------------------------------------------------------------
void ofApp::update()
{
	if ( audio().get_page() == HandAudio::Page::FINISH )
	{
		bg_color_ = ofColor::red;
	}
	else
	{
		bg_color_.r -= 0.05f;
		bg_color_.g -= 0.05f;
		bg_color_.b -= 0.05f;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear( bg_color_ );

	// std::string s = std::to_string( ofGetFrameRate() );

	float lx = leap().x_pos_to_rate( leap().lh_pos().x ) * ofGetWindowWidth();
	float ly = ( 1.f - leap().y_pos_to_rate( leap().lh_pos().y ) ) * ofGetWindowHeight();

	ofSetColor( leap().is_lh_valid() ? ( leap().is_l_slider_moving() ? ofColor::yellowGreen : ofColor::white ) : ofColor::red );
	ofDrawCircle( ofPoint( lx, ly ), audio().is_lh_lead_position() ? 100.f : 50.f );

	float rx = leap().x_pos_to_rate( leap().rh_pos().x ) * ofGetWindowWidth();
	float ry = ( 1.f - leap().y_pos_to_rate( leap().rh_pos().y ) ) * ofGetWindowHeight();

	ofSetColor( leap().is_rh_valid() ? ( leap().is_r_slider_moving() ? ofColor::yellowGreen : ofColor::white ) : ofColor::red );
	ofDrawCircle( ofPoint( rx, ry ), audio().is_rh_lead_position() ? 100.f : 50.f );

	draw_text( HandAudio::get_page_name( audio().get_page() ), ofGetWindowHeight() / 4 );
	draw_text( HandAudio::get_page_name( audio().get_next_page() ), ofGetWindowHeight() / 4 * 3 );
}

const HandAudioCallback& ofApp::audio() const
{
	return hand_.audio();
}

const LeapSoundController& ofApp::leap() const
{
	return hand_.leap();
}

void ofApp::draw_text( const std::string& s, float y ) const
{
	ofColor font_color( 0.f );
	ofColor border_color( 255.f, 255.f, 255.f, 255.f );
	
	const float fw = font.stringWidth( s );
	const float fh = font.stringHeight( s );

	const float fx = ( ofGetWindowWidth() - fw ) / 2;
	const float fy = y + fh / 2;

	ofSetColor( border_color );

	const float border_w = 4.f;

	for ( int y = -border_w; y <= border_w; y++ )
	{
		for ( int x = -border_w; x <= border_w; x++ )
		{
			font.drawStringAsShapes( s, fx + x, fy + y );
		}
	}
	
	ofSetColor( font_color );
	font.drawStringAsShapes( s, fx, fy );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch ( key )
	{
	case 'f':
		ofToggleFullscreen();
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}