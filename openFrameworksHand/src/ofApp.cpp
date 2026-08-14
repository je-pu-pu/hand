#include "../../main/Hand.h"
#include "../../main/HandAudioCallback.h"
#include "ofApp.h"

#include <boost/format.hpp>

namespace
{

/**
 * 手の形に対応する OSC メッセージを返す ( 対応が無ければ nullptr )
 *
 * HandAudioCallback::play_sound_by_hand_shape() で実際に音が鳴る組み合わせに合わせている。
 */
const ofApp::ShapeMessage* get_shape_message( HandAudioCallback::Page page, LeapSoundController::Hand::Shape shape )
{
	typedef LeapSoundController::Hand::Shape Shape;

	static const ofApp::ShapeMessage rps_rock      = { "/rps", "r" };
	static const ofApp::ShapeMessage rps_paper     = { "/rps", "p" };
	static const ofApp::ShapeMessage rps_scissors  = { "/rps", "s" };
	static const ofApp::ShapeMessage rps_thumbs_up = { "/rps", "thumbs_up" };

	static const ofApp::ShapeMessage fox_1   = { "/fox", "1" };
	static const ofApp::ShapeMessage fox_2   = { "/fox", "2" };
	static const ofApp::ShapeMessage fox_3   = { "/fox", "3" };
	static const ofApp::ShapeMessage fox_fox = { "/fox", "fox" };

	if ( page == HandAudioCallback::Page::RPS )
	{
		switch ( shape )
		{
		case Shape::ROCK:		return & rps_rock;
		case Shape::PAPER:		return & rps_paper;
		case Shape::SCISSORS:	return & rps_scissors;
		case Shape::THUMBS_UP:	return & rps_thumbs_up;
		default:				return nullptr;
		}
	}

	if ( page == HandAudioCallback::Page::FOX )
	{
		switch ( shape )
		{
		case Shape::INDEX:		return & fox_1;
		case Shape::SCISSORS:	return & fox_2;
		case Shape::THREE:		return & fox_3;
		case Shape::FOX:		return & fox_fox;
		default:				return nullptr;
		}
	}

	return nullptr;
}

} // namespace

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

		// タップのフラグは 1 step の間しか立たないので、ここで拾っておく。
		// OSC の送信自体はオーディオコールバックからは行わず、update() に任せる。
		if ( audio().is_l_tapped() || audio().is_r_tapped() )
		{
			is_tap_pending_ = true;
		}

		// 手の形で音が鳴ったら、どの形だったかを照明側へ伝える
		if ( const ShapeMessage* message = get_shape_message( audio().get_page(), audio().get_played_shape() ) )
		{
			pending_shape_message_ = message;
		}

	} );

	osc_sender_.setup(
		hand_.config().get( "osc.host", std::string( OSC_DEFAULT_HOST ) ),
		hand_.config().get( "osc.port", OSC_DEFAULT_PORT ) );
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

	lh_point_size_ = math::clamp( lh_point_size_ * 0.9f, POINT_SIZE_MIN, POINT_SIZE_MAX );
	rh_point_size_ = math::clamp( rh_point_size_ * 0.9f, POINT_SIZE_MIN, POINT_SIZE_MAX );

	ofxOscMessage m;
	m.setAddress( "/bg" );
	m.addFloatArg( bg_color_.r );
	// m.addIntArg( 100 );
	// m.addFloatArg( 0.75f );
	osc_sender_.sendMessage( m );

	// 照明側 ( Chataigne ) でページごとの演出を切り替えるために送る。
	// 番号ではなくページ名を送るのは、設定を見て意味が分かるようにするためと、
	// Page の enum の順序を変えても照明側がずれないようにするため。
	const std::string& page_name = HandAudio::get_page_name( audio().get_page() );

	if ( page_name != last_sent_page_name_ )
	{
		send_osc_string( "/page", page_name );
		last_sent_page_name_ = page_name;
	}

	// BASS ページでは、両手の距離で決まるベースの音量を照明側にも渡す
	if ( audio().get_page() == HandAudio::Page::BASS )
	{
		send_osc_float( "/bass", audio().get_bass_volume() );
	}

	if ( is_tap_pending_.exchange( false ) )
	{
		send_osc_trigger( "/tap" );
	}

	if ( const ShapeMessage* message = pending_shape_message_.exchange( nullptr ) )
	{
		send_osc_string( message->address, message->value );
	}
}

/// 引数なしの OSC メッセージを送る ( 受信側ではトリガーとして扱われる )
void ofApp::send_osc_trigger( const std::string& address )
{
	ofxOscMessage m;
	m.setAddress( address );
	osc_sender_.sendMessage( m );
}

void ofApp::send_osc_float( const std::string& address, float value )
{
	ofxOscMessage m;
	m.setAddress( address );
	m.addFloatArg( value );
	osc_sender_.sendMessage( m );
}

void ofApp::send_osc_string( const std::string& address, const std::string& value )
{
	ofxOscMessage m;
	m.setAddress( address );
	m.addStringArg( value );
	osc_sender_.sendMessage( m );
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

	ofDrawBitmapStringHighlight( "FPS : " + std::to_string( ofGetFrameRate() ), 0, 10 );
	ofDrawBitmapStringHighlight( "MIC VOL : " + std::to_string( audio().get_mic_volume() ), 0, 30 );
	ofDrawBitmapStringHighlight( "BGM VOL : " + std::to_string( audio().get_bgm_volume() ), 0, 50 );

	ofDrawBitmapStringHighlight( "L Shape : " + std::string( leap().get_lh().get_shape_name() ), 0, 70 );
	ofDrawBitmapStringHighlight( "R Shape : " + std::string( leap().get_rh().get_shape_name() ), 0, 90 );
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
	// フルスクリーンの切り替え ( f / F11 / Alt + Enter )
	// アルファベットは実際に入力された文字コードが渡ってくるため 'F' も受ける
	// ( Caps Lock や Shift で大文字になる )
	case 'f':
	case 'F':
	case OF_KEY_F11:
		ofToggleFullscreen();
		break;

	case OF_KEY_RETURN:
		if ( ofGetKeyPressed( OF_KEY_ALT ) )
		{
			ofToggleFullscreen();
		}
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
