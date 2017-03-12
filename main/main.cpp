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

#include "AudioApp.h"
#include "Gamma/Oscillator.h"
#include "Gamma/Envelope.h"
#include "Gamma/SamplePlayer.h"
#include "Gamma/Effects.h"

#define NOMINMAX
#include <windows.h>

#include <thread>


using namespace gam;

#ifdef _DEBUG
#pragma comment( lib, "gamma_debug.lib" )
#else
#pragma comment( lib, "gamma.lib" )
#endif

LeapSoundController leap;

class MyApp : public AudioApp
{
public:
	int step = 0;
	int p_step = 0;

	bool finished = false;

	Accum<> timer;

	SamplePlayer<> tap, page_down, page_up, kick, snare;
	SamplePlayer < float, gam::ipl::Cubic, gam::phsInc::Loop > bass, lead_l, lead_r;

	Delay<> delay;

	MyApp( int in, int out )
		: AudioApp( in, out )
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
		if ( timer() && ! finished )
		{
			key_input();

			if ( leap.pop_page_decremented() )
			{
				// kick.range( rand() / static_cast< float >( RAND_MAX ) , 0.01f );
				// snare.range( rand() / static_cast< float >( RAND_MAX ) , 0.01f );

				page_down.reset();
			}
			if ( leap.pop_page_incremented() )
			{
				// kick.range( rand() / static_cast< float >( RAND_MAX ) , 0.01f );
				// snare.range( rand() / static_cast< float >( RAND_MAX ) , 0.01f );

				page_up.reset();

				if ( leap.page() == 1 )
				{
					step = 0;
					p_step = 0;
				}

				if ( leap.page() == 8 )
				{
					// delay.delay( 60.f / 120.f / 2.f );
				}
			}

			if ( leap.pop_tapped() || leap.page() == 9 )
			{
				const float tap_rate[ 21 ] = { 130.813f, 146.832f, 164.814f, 195.998f, 220.000f, 261.626f, 293.665f, 329.628, 391.995, 440.000, 523.251f, 587.330f, 659.255f, 783.991f, 880.000f, 1046.502f, 1174.659f, 1318.510f, 1567.982, 1760.000f, 2093.005f };
				tap.rate( tap_rate[ leap.y_pos_to_index( leap.rh_if_tip_pos().y, 15 ) + rand() % 6 ] / 220.f );

				tap.reset();
			}

			const bool kick_on[ 3 ][ 2 ][ 16 ] = {
				{
					{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
				},
				{
					{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
				},
				{
					{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1 },
					{ 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1 },
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

			const int  kick_pattern[ LeapSoundController::PAGES ] = { 0, 0, 0, 1, 1, 0, 0, 1, 2, 0 };
			const int snare_pattern[ LeapSoundController::PAGES ] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 0 };

			if ( kick_on[ kick_pattern[ leap.page() ] ][ is_fill_in ][ step ] )
			{
				kick.range( std::min( 0.9f, leap.l_slider( 2 ) ), 0.25f );
				kick.rate( 0.5f + leap.r_slider( 2 ) );
				kick.reset();
			}

			if ( snare_on[ snare_pattern[ leap.page() ] ][ is_fill_in ][ step ] )
			{
				snare.range( std::min( 0.9f, leap.l_slider( 3 ) ), 0.25f );
				snare.rate( 0.5f + leap.r_slider( 3 ) );
				snare.reset();
			}

			step = ( step + 1 ) % 16;

			if ( step == 0 )
			{
				p_step = ( p_step + 1 ) % 4;

				bass.reset();
			}

			const float bass_rate[ 4 ] = { 87.307f, 97.999f, 110.f, 130.813f };
			bass.rate( bass_rate[ p_step ] / 110.f );

			if ( leap.page() == 7 )
			{
				if ( step / 2 % 2 == 1 )
				{
					bass.rate( bass.rate() * 2.f );
				}
			}
			else if ( leap.page() == 8 )
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
			else if ( leap.page() == 9 )
			{
				finished = true;
				kick.rate( 1.f );
				kick.range( 0.f, 10.f );
				kick.reset();
			}
		}

		const float lead_rate[ 16 ] = { 261.626f, 293.665f, 329.628, 391.995, 440.000, 523.251f, 587.330f, 659.255f, 783.991f, 880.000f, 1046.502f, 1174.659f, 1318.510f, 1567.982, 1760.000f, 2093.005f };
		lead_l.rate( lead_rate[ leap.y_pos_to_index( leap.lh_if_tip_pos().y, 16 ) ] / 110.f );
		lead_r.rate( lead_rate[ leap.y_pos_to_index( leap.rh_if_tip_pos().y, 16 ) ] / 110.f );

		while ( io() )
		{
			const float    kick_volume[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 2.f };
			const float   snare_volume[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 1.f };
			const float    bass_volume[ LeapSoundController::PAGES ] = { 0.f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.f, 1.f, 0.f };
			const float  lead_l_volume[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.f, 1.f, 0.f };
			const float  lead_r_volume[ LeapSoundController::PAGES ] = { 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.f, 1.f, 0.f };
			const float key_tap_volume[ LeapSoundController::PAGES ] = { 1.f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.f, 1.f, 1.f };

			const float     delay_gain[ LeapSoundController::PAGES ] = { 0.25f, 0.10f, 0.10f, 0.10f, 0.10f, 0.20f, 0.20f, 0.20f, 0.30f, 0.50f };
			const float  delay_feedbak[ LeapSoundController::PAGES ] = { 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.25f };

			float s = 0.f;
			
			s +=   kick() *   kick_volume[ leap.page() ];
			s +=  snare() *  snare_volume[ leap.page() ];
			s +=   bass() *   bass_volume[ leap.page() ];
			
			s += lead_l() * lead_l_volume[ leap.page() ];
			s += lead_r() * lead_r_volume[ leap.page() ];

			s += tap() * key_tap_volume[ leap.page() ];
			
			s += page_down() + page_up();

			s += delay( s * delay_gain[ leap.page() ] + delay() * delay_feedbak[ leap.page() ] );
			s *= 0.5f;

			io.out( 0 ) = s;
			io.out( 1 ) = s;
		}
	}
};

int main( int, char** )
{
	gam::AudioDevice::printAll();

	int in = 0, out = 0;

	std::cout << "input device no : ";
	std::cin >> in;

	std::cout << "output device no : ";
	std::cin >> out;

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

	return 0;
}

