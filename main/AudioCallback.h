#pragma once

#include "LeapSoundController.h"
#include <Gamma/AudioIO.h>
#include <Gamma/Domain.h>
#include <portaudio.h>
#include <cstdio>
#include <iostream>

class AudioCallback : public gam::AudioCallback
{
private:
	gam::AudioIO mAudioIO;

public:
	/// デバイスのホスト API 名 ( "ASIO" / "MME" / "Windows DirectSound" など ) を返す
	static const char* get_host_api_name( int device_no )
	{
		gam::AudioDevice::numDevices();		// Gamma 側で PortAudio を初期化させる

		const PaDeviceInfo* device = Pa_GetDeviceInfo( device_no );

		if ( ! device )
		{
			return "?";
		}

		const PaHostApiInfo* host_api = Pa_GetHostApiInfo( device->hostApi );

		return host_api ? host_api->name : "?";
	}

	/// オーディオデバイスの一覧をホスト API 名つきで表示する
	static void print_devices()
	{
		gam::AudioDevice::numDevices();		// Gamma 側で PortAudio を初期化させる

		for ( int n = 0; n < Pa_GetDeviceCount(); n++ )
		{
			const PaDeviceInfo* device = Pa_GetDeviceInfo( n );

			if ( ! device )
			{
				continue;
			}

			std::printf( "[%2d] %-20s %-40s in %2d, out %2d, %6.0f Hz\n",
				n,
				get_host_api_name( n ),
				device->name,
				device->maxInputChannels,
				device->maxOutputChannels,
				device->defaultSampleRate );
		}
	}

	AudioCallback( int in_device_no, int out_device_no )
	{
		mAudioIO.append( *this );

		auto device_in = gam::AudioDevice( in_device_no );
		auto device_out = gam::AudioDevice( out_device_no );

		std::cout << "in  : [" << in_device_no  << "] " << get_host_api_name( in_device_no )  << " : " << device_in.name()  << std::endl;
		std::cout << "out : [" << out_device_no << "] " << get_host_api_name( out_device_no ) << " : " << device_out.name() << std::endl;

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
