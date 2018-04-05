#pragma once

#include "ofMain.h"
#include <string>

class Hand;
class HandAudioCallback;
class LeapSoundController;

class ofApp : public ofBaseApp
{
public:
	typedef HandAudioCallback HandAudio;

	protected:
		Hand& hand_;
		ofTrueTypeFont font;

		ofFloatColor bg_color_;

		void draw_text( const std::string&, float ) const;

		const HandAudioCallback& audio() const;
		const LeapSoundController& leap() const;

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
