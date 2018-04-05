#pragma once

#include "LeapSoundController.h"
#include <Gamma/AudioIO.h>
#include <Gamma/Domain.h>
#include <iostream>

class AudioCallback : public gam::AudioCallback
{
private:
	gam::AudioIO mAudioIO;

public:
	AudioCallback( int in_device_no, int out_device_no )
	{
		mAudioIO.append( *this );
		
		auto device_in = gam::AudioDevice( in_device_no );
		auto device_out = gam::AudioDevice( out_device_no );

		std::cout << "in  : " << device_in.name() << std::endl;
		std::cout << "out : " << device_out.name() << std::endl;

		mAudioIO.deviceIn( device_in );
		mAudioIO.deviceOut( device_out );

		initAudio( 44100, 64, 2, 1 );
	}

	~AudioCallback()
	{
		mAudioIO.remove( *this );
		mAudioIO.stop();
	}

	void initAudio( double framesPerSec, unsigned framesPerBuffer, unsigned outChans, unsigned inChans )
	{
		mAudioIO.framesPerSecond(framesPerSec);
		mAudioIO.framesPerBuffer(framesPerBuffer);
		mAudioIO.channelsOut(outChans);
		mAudioIO.channelsIn(inChans);
		gam::sampleRate(framesPerSec);
	}

	gam::AudioIO& audioIO() { return mAudioIO; }
	const gam::AudioIO& audioIO() const { return mAudioIO; }

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

	void stop()
	{
		mAudioIO.stop();
	}
};
