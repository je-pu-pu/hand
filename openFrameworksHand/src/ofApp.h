#pragma once

#include "ofMain.h"
#include <ofxOsc.h>
#include <string>

class Hand;
class HandAudioCallback;
class LeapSoundController;

class ofApp : public ofBaseApp
{
public:
	typedef HandAudioCallback HandAudio;

	const float POINT_SIZE_MIN = 100.f;
	const float POINT_SIZE_MAX = 200.f;

	/// OSC の送信先。設定ファイルの osc.host / osc.port で変更できる
	static constexpr const char* OSC_DEFAULT_HOST = "localhost";
	static const int OSC_DEFAULT_PORT = 7700;

protected:
	Hand& hand_;
	ofTrueTypeFont font_big_;
	ofTrueTypeFont font_small_;

	ofFloatColor bg_color_;
	float lh_point_size_;
	float rh_point_size_;

	/// 最後に /page で送ったページ名。変わったときだけ送るために持つ
	std::string last_sent_page_name_;

	void draw_text( const ofTrueTypeFont&, const std::string&, float, float = 4.f ) const;

	void send_osc_string( const std::string& address, const std::string& value );
	void send_osc_float( const std::string& address, float value );

	const HandAudioCallback& audio() const;
	const LeapSoundController& leap() const;

	ofxOscSender osc_sender_;

public:
	ofApp( Hand& );

	void setup();
	void update();
	void draw();


	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
};
