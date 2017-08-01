/*	Gamma - Generic processing library
See COPYRIGHT file for authors and license information

Example:	Beating
Author:		Lance Putnam, 2012

Description:
This demonstrates the beating effect produced by summing two sinusoids with 
equal amplitude and nearly equal frequency. The sum of two sinusoids with 
frequencies f1 and f2 is equivalent to the product of two sinusoids with 
frequencies (f1+f2)/2 and (f1-f2)/2.
*/

#include "LeapSoundController.h"
#include "Config.h"
#include "chase_value.h"

#include "AudioApp.h"
#include <Gamma/Oscillator.h>
#include <Gamma/Envelope.h>
#include <Gamma/SamplePlayer.h>
#include <Gamma/Effects.h>

#include <boost/asio.hpp>

#define NOMINMAX
#include <windows.h>

#include <fstream>
#include <algorithm>
#include <thread>


using namespace gam;

#ifdef _DEBUG
#pragma comment( lib, "gamma_debug.lib" )
#else
#pragma comment( lib, "gamma.lib" )
#endif

void send_fire();

LeapSoundController leap;

class MyApp : public AudioApp
{
public:
	int page = 0;
	int step = 0;		// 16 分音符の step 0 .. 15
	int p_step = 0;		// 1 小節の step 0 .. 3

	bool finished = false;

	Accum<> timer;

	SamplePlayer<> tap, page_down, page_up, kick, snare;
	ADSR<> kick_env, snare_env;

	SamplePlayer < float, gam::ipl::Cubic, gam::phsInc::Loop > bass, lead_l, lead_r;

	Delay<> delay;

	common::chase_value< float > bass_volume = common::chase_value< float >( 0.f, 0.f, 0.05f );
	common::chase_value< float > lead_l_volume = common::chase_value< float >( 0.f, 0.f, 0.1f );
	common::chase_value< float > lead_r_volume = common::chase_value< float >( 0.f, 0.f, 0.1f );

	MyApp( int in, int out )
		: AudioApp( in, out )
		, kick_env()
		, snare_env()
	{
		timer.freq( 120.f / 60.f * 16.f );
		timer.phaseMax();	

		page_down.load( "page_down.wav" );
		page_up.load( "page_up.wav" );
		
		tap.load( "tap.wav" );
		
		page_down.phase( 0.99 );
		page_up.phase( 0.99 );
		tap.phase( 0.99 );

		kick.load( "kick.wav" );
		snare.buffer( kick );

		bass.load( "a.wav" );
		lead_l.buffer( bass );
		lead_r.buffer( bass );

		delay.maxDelay( 1.5f );
		delay.delay( 60.f / 120.f );

		// audioIO().gain( 0.f );

		leap.set_r_slider( 2, 0.5f );
		leap.set_r_slider( 3, 0.5f );
	}

	void key_input()
	{
		if ( GetAsyncKeyState( VK_LEFT ) & 0b1000000000000000 )
		{
			leap.decrement_page();
		}
		if ( GetAsyncKeyState( VK_RIGHT ) & 0b1000000000000000 )
		{
			leap.increment_page();
		}

		const float speed = 0.02f;

		if ( GetAsyncKeyState( VK_NUMPAD1 ) & 0b1000000000000000 )
		{
			leap.move_l_slider_force( -speed );
		}
		if ( GetAsyncKeyState( VK_NUMPAD7 ) & 0b1000000000000000 )
		{
			leap.move_l_slider_force( +speed );
		}
		if ( GetAsyncKeyState( VK_NUMPAD3 ) & 0b1000000000000000 )
		{
			leap.move_r_slider_force( -speed );
		}
		if ( GetAsyncKeyState( VK_NUMPAD9 ) & 0b1000000000000000 )
		{
			leap.move_r_slider_force( +speed );
		}
	}

	void onAudio( AudioIOData& io )
	{
		if ( timer() && ( ! finished || leap.is_page_decremented() ) )
		{
			key_input();

			page = leap.page();

			if ( leap.pop_page_decremented() )
			{
				page_down.reset();

				on_page_changed( page, false );
			}
			if ( leap.pop_page_incremented() )
			{
				page_up.reset();

				on_page_changed( page, true );
			}

			if ( leap.pop_tapped() || page == 9 )
			{
				const int max_notes = 21;
				const int random_note_range = 5;
				const float tap_rate[ max_notes ] = { 130.813f, 146.832f, 164.814f, 195.998f, 220.000f, 261.626f, 293.665f, 329.628, 391.995, 440.000, 523.251f, 587.330f, 659.255f, 783.991f, 880.000f, 1046.502f, 1174.659f, 1318.510f, 1567.982, 1760.000f, 2093.005f };
				tap.rate( tap_rate[ leap.y_pos_to_index( leap.rh_if_tip_pos().y, max_notes ) + rand() % random_note_range ] / 220.f );

				tap.reset();
			}

			const bool kick_on[ 3 ][ 2 ][ 16 ] = {
				{
					{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
				},
				{
					{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0 },
					{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
				},
				{
					{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1 },
					{ 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1 },
				}
			};

			const bool snare_on[ 3 ][ 2 ][ 16 ] = {
				{
					{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
					{ 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1 }
				},
				{
					{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
					{ 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1 }
				},
				{
					{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
					{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
				}
			};

			const float hh_on[ 16 ] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

			const int is_fill_in = p_step < 3 ? 0 : 1;

			const int  kick_pattern[ LeapSoundController::PAGES ] = { 0, 0, 0, 0, 1, 0, 0, 1, 2, 0 };
			const int snare_pattern[ LeapSoundController::PAGES ] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 0 };

			step = ( step + 1 ) % 16;

			if ( kick_on[ kick_pattern[ page ] ][ is_fill_in ][ step ] )
			{
				kick.range( std::min( 0.9f, leap.l_slider( 2 ) ), 0.2f );
				kick.rate( 0.5f + leap.r_slider( 2 ) );
				kick.reset();
				kick_env.reset();
			}

			if ( snare_on[ snare_pattern[ page ] ][ is_fill_in ][ step ] )
			{
				snare.range( std::min( 0.9f, leap.l_slider( 3 ) ), 0.2f );
				snare.rate( 0.5f + leap.r_slider( 3 ) );
				snare.reset();
				snare_env.reset();
			}

			if ( step == 0 )
			{
				p_step = ( p_step + 1 ) % 4;
				page = leap.page();

				bass.reset();
				// bass_volume.fit_to_target();
			}

			if ( page > 1 )
			{
				bass_volume.fit( 1.f );
			}

			const float bass_rate[ 4 ] = { 87.307f, 97.999f, 110.f, 130.813f };
			bass.rate( bass_rate[ p_step ] / 110.f );

			if ( page == 1 )
			{
				// ベースのボリュームが左右の手の距離によって変わる
				if ( leap.hand_count() == 2 )
				{
					bass_volume.target_value() = leap.lh_pos().distanceTo( leap.rh_pos() ) / 1000.f;
					bass_volume.target_value() = std::max( bass_volume.target_value(), 0.f );
					bass_volume.target_value() = std::min( bass_volume.target_value(), 1.f );
				}

				bass_volume.chase();

				std::cout << "bass volume : " << bass_volume.value() << ", " << bass_volume.target_value() << std::endl;

				// ベースの
				/*
				float hands_pos = ( leap.lh_pos().y + leap.rh_pos().y ) / 2.f / 1000.f;
				hands_pos = std::max( hands_pos, 0.f );
				hands_pos = std::min( hands_pos, 1.f );

				int bass_note_index = static_cast< int >( hands_pos * 4.f );
				bass_note_index = std::max( bass_note_index, 0 );
				bass_note_index = std::min( bass_note_index, 3 );

				bass.rate( bass_rate[ bass_note_index ] / 110.f );
				*/
			}
			else if ( page == 5 )
			{
				if ( leap.rh_pos().z < -30.f )
				{
					lead_r_volume.target_value() = 1.f;
				}
			}
			else if ( page == 6 )
			{
				if ( leap.lh_pos().z < -30.f )
				{
					lead_l_volume.target_value() = 1.f;
				}
			}
			else if ( page == 7 )
			{
				if ( step / 2 % 2 == 1 )
				{
					bass.rate( bass.rate() * 2.f );
				}
			}
			else if ( page == 8 )
			{
				if ( step % 4 == 2 )
				{
					bass.rate( bass.rate() * 2 );
				}
				else if ( step % 4 != 0 )
				{
					bass.rate( bass.rate() * 3.f / 2.f );
				}
			}
			else if ( page == 9 )
			{
				finished = true;
				kick.rate( 1.f );
				kick.range( 0.f, 10.f );
				kick.reset();
			}

			lead_l_volume.chase();
			lead_r_volume.chase();
		}

		const float lead_rate[ 16 ] = { 261.626f, 293.665f, 329.628, 391.995, 440.000, 523.251f, 587.330f, 659.255f, 783.991f, 880.000f, 1046.502f, 1174.659f, 1318.510f, 1567.982, 1760.000f, 2093.005f };
		lead_l.rate( lead_rate[ leap.y_pos_to_index( leap.lh_pos().y, 16 ) ] / 110.f );
		lead_r.rate( lead_rate[ leap.y_pos_to_index( leap.rh_pos().y, 16 ) ] / 110.f );

		while ( io() )
		{
			/**
			 * 0 : キータップのデモ
			 * 1 : ベースのデモ
			 * 2 : キック
			 * 3 : スネア
			 * 4 : demo
			 * 5 : リード R
			 * 6 : リード L
			 * 7 : free 1
			 * 8 : free 2
			 * 9 : finish
			 */

			const float    kick_volume_of_page[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 2.f };
			const float   snare_volume_of_page[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 1.f };
			const float    bass_volume_of_page[ LeapSoundController::PAGES ] = { 0.f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 0.f };
			const float  lead_l_volume_of_page[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.f, 1.f, 0.f };
			const float  lead_r_volume_of_page[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.f, 1.f, 0.f };
			const float key_tap_volume_of_page[ LeapSoundController::PAGES ] = { 1.f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.f, 1.f, 1.f };

			const float     delay_gain[ LeapSoundController::PAGES ] = { 0.25f, 0.10f, 0.10f, 0.10f, 0.10f, 0.20f, 0.20f, 0.20f, 0.30f, 0.50f };
			const float  delay_feedbak[ LeapSoundController::PAGES ] = { 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.25f };

			float s = 0.f;
			
			s +=   kick() *   kick_volume_of_page[ page ] * kick_env();
			s +=  snare() *  snare_volume_of_page[ page ] * snare_env();
			s +=   bass() *   bass_volume_of_page[ page ] * bass_volume.value();
			
			s += lead_l() * lead_l_volume_of_page[ page ] * lead_l_volume.value();
			s += lead_r() * lead_r_volume_of_page[ page ] * lead_r_volume.value();

			s += tap() * key_tap_volume_of_page[ page ];
			
			s += page_down() + page_up();

			s += delay( s * delay_gain[ page ] + delay() * delay_feedbak[ page ] );
			s *= 0.5f;

			io.out( 0 ) = s;
			io.out( 1 ) = s;
		}
	}

	void on_page_changed( int page, bool incremented )
	{
		if ( page == 1 )
		{
			bass_volume.target_value() = 0.f;
			std::cout << "pc : " << bass_volume.target_value() << std::endl;
		}

		if ( page == 5 )
		{
			lead_r_volume.target_value() = 0.2f;
		}
		else if ( page == 6 )
		{
			lead_l_volume.target_value() = 0.2f;
		}
		else if ( page == 7 )
		{
			lead_l_volume.target_value() = 1.f;
			lead_r_volume.target_value() = 1.f;
		}

		if ( incremented )
		{
			if ( page == 9 )
			{
				send_fire();
			}
		}
		else
		{
			if ( page == 1 )
			{
				step = 0;
				p_step = 0;
			}

			if ( page == 8 )
			{
				finished = false;
			}
		}
	}
};


boost::asio::io_service io_service;
boost::asio::ip::tcp::socket server_socket( io_service );
boost::asio::ip::tcp::acceptor acceptor( io_service, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), 8080 ) );
bool is_central_connected = false;

std::thread start_server()
{
	return std::thread( [] () {
		try
		{
			acceptor.accept( server_socket );
			is_central_connected = true;
		}
		catch ( std::exception e )
		{
			std::cout << "error : " << e.what();
		}
	} );
}

void send_fire()
{
	if ( ! is_central_connected )
	{
		std::cout << "central is not connected." << std::endl;
		return;
	}

	size_t s = boost::asio::write( server_socket, boost::asio::buffer( "fire" ) );
	std::cout << "write " << s << std::endl;
}

int main( int, char** )
{
	std::thread server_thread = start_server();

	gam::AudioDevice::printAll();

	int in = 0, out = 0;

	Config config;
	if ( config.load_file( "./config.txt" ) )
	{
		in = config.get( "input_device_no", 0 );
		out = config.get( "output_device_no", 0 );
	}
	else
	{
		std::cout << "input device no : ";
		std::cin >> in;

		std::cout << "output device no : ";
		std::cin >> out;

		config.set( "input_device_no", in );
		config.set( "output_device_no", out );
	}

	Leap::Controller controller;

	controller.addListener( leap );
	controller.setPolicy( Leap::Controller::POLICY_BACKGROUND_FRAMES );
	controller.setPolicy( Leap::Controller::POLICY_ALLOW_PAUSE_RESUME );

	controller.config().setFloat( "Gesture.Swipe.MinLength", 400.f );
	controller.config().setFloat( "Gesture.Swipe.MinVelocity", 100.f );
	bool b = controller.config().save();

	MyApp( in, out ).start();

	// std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

	controller.removeListener( leap );

	config.save_file( "./config.txt" );

	server_thread.detach();

	return 0;
}