#include "../../main/Hand.h"
#include "../../main/HandAudioCallback.h"
#include "ofApp.h"

#include <boost/format.hpp>

ofApp::ofApp( Hand& hand )
	: hand_( hand )
	, lh_point_size_( POINT_SIZE_MIN )
	, rh_point_size_( POINT_SIZE_MIN )
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

		if ( audio().is_l_tapped() )
		{
			lh_point_size_ = POINT_SIZE_MAX;
		}
		if ( audio().is_r_tapped() )
		{
			rh_point_size_ = POINT_SIZE_MAX;
		}

	} );
}

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate( 60 );

	font_big_.load( "mplus-1m-bold.ttf", 300, true, false, true );
	font_small_.load( "mplus-1m-bold.ttf",  100, true, false, true );
}

//--------------------------------------------------------------
void ofApp::update()
{
	if ( audio().get_next_page() == HandAudio::Page::FINISH )
	{
		bg_color_ = ofColor::red;
	}
	else
	{
		bg_color_.r *= 0.85f;
		bg_color_.g *= 0.85f;
		bg_color_.b *= 0.85f;
	}

	lh_point_size_ = std::clamp( lh_point_size_ * 0.9f, POINT_SIZE_MIN, POINT_SIZE_MAX );
	rh_point_size_ = std::clamp( rh_point_size_ * 0.9f, POINT_SIZE_MIN, POINT_SIZE_MAX );
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear( bg_color_ );

	float lx = leap().x_pos_to_rate( leap().lh_pos().x ) * ofGetWindowWidth();
	float ly = ( 1.f - leap().y_pos_to_rate( leap().lh_pos().y ) ) * ofGetWindowHeight();

	ofSetColor( leap().is_lh_valid() ? ( leap().is_l_slider_moving() ? ofColor::yellowGreen : ofColor::white ) : ofColor::red );

	if ( audio().is_lh_lead_position() )
	{
		ofDrawCircle( ofPoint( lx, ly ), lh_point_size_ );
	}
	else
	{
		ofDrawRectRounded( ofRectangle( lx - lh_point_size_ / 2.f, ly - lh_point_size_ / 2.f, lh_point_size_, lh_point_size_ ), 5.f );
	}

	float rx = leap().x_pos_to_rate( leap().rh_pos().x ) * ofGetWindowWidth();
	float ry = ( 1.f - leap().y_pos_to_rate( leap().rh_pos().y ) ) * ofGetWindowHeight();

	ofSetColor( leap().is_rh_valid() ? ( leap().is_r_slider_moving() ? ofColor::yellowGreen : ofColor::white ) : ofColor::red );
	
	if ( audio().is_rh_lead_position() )
	{
		ofDrawCircle( ofPoint( rx, ry ), rh_point_size_ );
	}
	else
	{
		ofDrawRectRounded( ofRectangle( rx - rh_point_size_ / 2.f, ry - rh_point_size_ / 2.f, rh_point_size_, rh_point_size_ ), 5.f );
	}

	draw_text( font_big_, HandAudio::get_page_name( audio().get_page() ), ofGetWindowHeight() / 4 );
	draw_text( font_big_, HandAudio::get_page_name( audio().get_next_page() ), ofGetWindowHeight() / 4 * 3 );

	const std::string data_text = (
		boost::format( "%0.3f    %d-%d-%d    %0.3f " )
			% leap().l_slider( leap().page() )
			% ( ( audio().get_bar() / 4 % 4 ) + 1 )
			% ( ( audio().get_bar() % 4 ) + 1 )
			% ( audio().get_beat() + 1 )
			% leap().r_slider( leap().page() )
	).str();

	draw_text( font_small_, data_text, ofGetWindowHeight() / 2 );

	auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
	auto local_time = std::localtime( & now );
	std::stringstream time;
	time  << std::put_time( local_time, "%H:%M:%S " );

	draw_text( font_small_, time.str(), ( font_small_.getAscenderHeight() / 2 ) );

	ofDrawBitmapStringHighlight( std::to_string( ofGetFrameRate() ), 0, 11 );
	ofDrawBitmapStringHighlight( std::to_string( audio().get_mic_volume() ), 0, 31 );
	ofDrawBitmapStringHighlight( std::to_string( audio().get_bgm_volume() ), 0, 51 );
}

const HandAudioCallback& ofApp::audio() const
{
	return hand_.audio();
}

const LeapSoundController& ofApp::leap() const
{
	return hand_.leap();
}

void ofApp::draw_text( const ofTrueTypeFont& font, const std::string& s, float y, float border_w ) const
{
	ofColor font_color( 0.f );
	ofColor border_color( 255.f, 255.f, 255.f, 255.f );
	
	const float fw = font.stringWidth( s );
	const float fh = font.stringHeight( s );

	const float fx = ( ofGetWindowWidth() - fw ) / 2;
	const float fy = y + font.getLineHeight() / 2 + font.getDescenderHeight();

	ofSetColor( border_color );

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
