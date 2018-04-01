#ifndef GAMMA_AUDIO_APP_H_INC
#define GAMMA_AUDIO_APP_H_INC
/*	Gamma - Generic processing library
	See COPYRIGHT file for authors and license information
	
	Description: A very basic audio application abstraction
*/

#include <stdio.h>
#include "Gamma/AudioIO.h"
#include "Gamma/Domain.h"

namespace gam{

class AudioApp : public AudioCallback
{
public:
	AudioApp( int in_device_no, int out_device_no )
	{
		mAudioIO.append( *this );
		
		auto device_in = AudioDevice( in_device_no );
		auto device_out = AudioDevice( out_device_no );

		std::cout << "in  : " << device_in.name() << std::endl;
		std::cout << "out : " << device_out.name() << std::endl;

		mAudioIO.deviceIn( device_in );
		mAudioIO.deviceOut( device_out );

		initAudio( 44100, 64, 2, 1 );
	}

	void initAudio( double framesPerSec, unsigned framesPerBuffer, unsigned outChans, unsigned inChans )
	{
		mAudioIO.framesPerSecond(framesPerSec);
		mAudioIO.framesPerBuffer(framesPerBuffer);
		mAudioIO.channelsOut(outChans);
		mAudioIO.channelsIn(inChans);
		sampleRate(framesPerSec);
	}

	AudioIO& audioIO() { return mAudioIO; }
	const AudioIO& audioIO() const { return mAudioIO; }

	void start( bool block = true )
	{
		mAudioIO.start();

		if ( block )
		{		
			printf( "Press 'enter' to quit...\n" );
			
			std::string x;
			std::cin >> x;

			mAudioIO.stop();
		}
	}
	
private:
	AudioIO mAudioIO;
};

}

#endif
