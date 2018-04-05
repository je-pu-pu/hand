#pragma once

#include <boost/asio.hpp>

#include "Tone.h"
#include "AudioCallback.h"
#include "chase_value.h"

#include <windows.h>

#include <Gamma/AudioIO.h>
#include <Gamma/Domain.h>
#include <Gamma/Oscillator.h>
#include <Gamma/Envelope.h>
#include <Gamma/SamplePlayer.h>
#include <Gamma/Effects.h>
#include <Gamma/FFT.h>
#include <Gamma/DFT.h>

#include <array>

class Hand;

class HandAudioCallback : public AudioCallback
{
public:
	const float RHYTHM_RATE_MIN = 0.5f;
	const float RHYTHM_RATE_MAX = 2.f;
	const float area_threashold_z = 100.f;

	enum class Page
	{
		TAP = 0,	// キータップのデモ
		PAD,		// パッドのデモ
		BASS,		// ベースのデモ
		KICK,		// キック
		SNARE,		// スネア
		DEMO,		// demo
		LEAD_R,		// リード R
		LEAD_L,		// リード L
		FREE,		// 自由演奏 ( 手品 )
		CLIMAX,		// クライマックス
		FINISH,		// 終了

		MAX
	};

	static const int PAGES = static_cast< int >( Page::MAX );

	static const std::string& get_page_name( Page page )
	{
		static std::array< std::string, static_cast< int >( Page::MAX ) > page_name_map = {
			"TAP",
			"PAD",
			"BASS",
			"KICK",
			"SNARE",
			"DEMO",
			"LEAD_R",
			"LEAD_L",
			"FREE",
			"CLIMAX",
			"FINISH",
		};

		return page_name_map[ static_cast< int >( page ) ];
	}

	enum class Part
	{
		KICK = 0, SNARE, BASS, LEAD_L, LEAD_R, TAP, BRIGHT, PAD,
		MAX
	};

	float get_part_volume( Part part ) const
	{
		static const float volume_table[ static_cast< int >( Part::MAX ) ][ PAGES ] = {
		//	{   TAP, PAD,    BASS,  KICK,SNARE, DEMO,    R,   L,   FREE,  MAX, FIN }
			{ 0.00f, 0.00f, 0.00f, 1.00f, 1.00f, 1.0f, 1.0f, 1.0f, 1.00f, 1.0f, 2.f }, // KICK
			{ 0.00f, 0.00f, 0.00f, 0.00f, 1.00f, 1.0f, 1.0f, 1.0f, 1.00f, 1.0f, 1.f }, // SNARE
			{ 0.00f, 0.00f, 0.50f, 0.50f, 0.50f, 1.0f, 1.0f, 1.0f, 1.00f, 1.0f, 0.f }, // BASS
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f, 0.f }, // LEAD_L
			{ 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.0f, 1.0f, 1.0f, 0.75f, 1.0f, 0.f }, // LEAD_R
			{ 1.00f, 0.25f, 0.00f, 0.00f, 0.00f, 1.0f, 0.0f, 1.0f, 1.00f, 1.0f, 1.f }, // TAP
			{ 0.00f, 0.00f, 0.10f, 0.10f, 0.10f, 0.1f, 0.5f, 0.5f, 1.00f, 1.0f, 1.f }, // BRIGHT
			{ 0.00f, 0.25f, 0.25f, 0.25f, 0.25f, 0.5f, 0.5f, 0.5f, 0.50f, 0.5f, 0.f }, // PAD
		};

		return volume_table[ static_cast< int >( part ) ][ get_page_index() ];
	}

	// BPM
	int get_bpm() const { return 120; }

	Page page = Page::TAP;
	
	int step = 0;		// 16 分音符のカウント ( 0 .. 15 )
	int beat = 0;		//  4 分音符のカウント ( 0 ..  3 )
	int bar = 0;		//  1 小節のカウント   ( 0 ..  3 )

	bool finished = false;

	gam::Accum<> timer;

	gam::SamplePlayer<> tap, page_down, page_up, kick, snare;
	gam::AD<> tap_env, kick_env, snare_env;

	gam::SamplePlayer < float, gam::ipl::Cubic, gam::phsInc::Loop > bass, lead_l, lead_r, pad1, pad2, pad3;
	gam::ADSR<> bass_env;
	
	gam::SamplePlayer<> bright;
	gam::AD<> bright_env;

	gam::Biquad<> bq_filter;
	gam::Delay<> delay;

	common::chase_value< float > bass_volume = common::chase_value< float >( 0.f, 0.f, 0.0001f );
	common::chase_value< float > lead_l_volume = common::chase_value< float >( 0.f, 0.f, 0.0001f );
	common::chase_value< float > lead_r_volume = common::chase_value< float >( 0.f, 0.f, 0.0001f );

	unsigned int rec = 0;
	gam::Array<float> rec_buf;

	int rec_frame_index = 0;

	bool is_on_step_;					/// たった今 16 分音符の頭
// 	bool is_on_beat_;					/// たった今  4 分音符の頭
// 	bool is_on_bar_;					/// たった今小節の頭

private:
	Hand& hand;
	LeapSoundController& leap;

protected:
	bool is_on_step() const { return is_on_step_; }

	float get_slider_value_l( Page page ) const { return leap.l_slider( static_cast< int >( page ) ); }
	float get_slider_value_r( Page page ) const { return leap.r_slider( static_cast< int >( page ) ); }
	void set_slider_value_l( Page page, float value ) { leap.set_l_slider( static_cast< int >( page ), value ); }
	void set_slider_value_r( Page page, float value ) { leap.set_r_slider( static_cast< int >( page ), value ); }

public:
	HandAudioCallback( Hand& hand, int in, int out, LeapSoundController& leap )
		: AudioCallback( in, out )
		, hand( hand )
		, leap( leap )
		, tap_env( 0.01f, 0.5f )
		, kick_env( 0.01f, 0.25f )
		, snare_env( 0.01f, 0.25f )
		, bass_env( 0.01f )
	{
		timer.period( 60.f / get_bpm() / 4.f );
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
		delay.delay( 60.f / get_bpm() / 2.f );

		bq_filter.type( gam::HIGH_PASS );

		set_slider_value_r( Page::KICK,  range_to_rate( 1.f, RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
		set_slider_value_r( Page::SNARE, range_to_rate( 1.f, RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
	}

	Page get_page() const { return page; }
	Page get_next_page() const { return static_cast< Page >( leap.page() ); }

	bool is_recording() const
	{
		return rec & 0b1;
	}

	bool is_speaking_mode() const
	{
		return GetAsyncKeyState( VK_SPACE ) & 0b1000000000000000;
	}

	bool is_record_started() const
	{
		return ( !( rec & 0x10 ) ) && ( rec & 0b1 );
	}

	bool is_record_finished() const
	{
		return ( rec & 0b10 ) && !( rec & 0b1 );
	}

	static float range_to_rate( float value, float min, float max )
	{
		return ( value - min ) / ( max - min );
	}

	static float rate_to_range( float value, float min, float max )
	{
		if ( min > max )
		{
			return std::clamp( ( max - min ) * value + min, max, min );
		}

		return std::clamp( ( max - min ) * value + min, min, max );
	}

	static float range_to_range( float value, float min, float max, float new_min, float new_max )
	{
		return rate_to_range( range_to_rate( value, min, max ), new_min, new_max );
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

		rec <<= 1;
		rec |= static_cast< bool >( GetAsyncKeyState( 'R' ) & 0b1000000000000000 );

		if ( is_record_started() )
		{
			rec_frame_index = 0;
			rec_buf.resize( get_frames_per_beat() * 4, 0 );
		}
		else if ( is_record_finished() )
		{
			rec_buf.resize( std::max( rec_frame_index, 64 ) );
			
			// analysis( rec_buf );
			// print_edge( rec_buf );

			gam::arr::normalize( & rec_buf[ 0 ], rec_buf.size() );

			if ( page == Page::KICK )
			{
				kick.buffer( rec_buf, audioIO().framesPerSecond(), 1 );
				set_slider_value_r( Page::KICK, range_to_rate( 1.f, RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
			}
			else if ( page == Page::SNARE )
			{
				snare.buffer( rec_buf, audioIO().framesPerSecond(), 1 );
				set_slider_value_r( Page::SNARE, range_to_rate( 1.f, RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
			}
			else
			{
				tap.buffer( rec_buf, audioIO().framesPerSecond(), 1 );

				if ( page >= Page::PAD )
				{
					smoothing( rec_buf );
					// print_edge( rec_buf );

					bass.buffer( rec_buf, audioIO().framesPerSecond(), 1 );
					lead_l.buffer( bass );
					lead_r.buffer( bass );
					pad1.buffer( bass );
					pad2.buffer( bass );
					pad3.buffer( bass );

					kick.buffer( rec_buf, audioIO().framesPerSecond(), 1 );
					snare.buffer( kick );
					bright.buffer( kick );
				}
			}
		}

		// std::cout << rec << std::endl;
	}

	void smoothing( gam::Array<float>& buf )
	{
		if ( false )
		{
			const float range = 16;

			for ( int n = 0; n < buf.size(); n++ )
			{
				float v = 0.f;

				for ( int m = 0; m < range; m++ )
				{
					v += buf[ ( n + m ) % buf.size() ];
				}

				buf[ n ] = v / range;
			}
		}

		if ( true )
		{
			for ( int n = 0; n < buf.size() / 2; n++ )
			{
				int m = buf.size() / 2 + n;

				float r = static_cast< float >( n ) / ( buf.size() / 2 );
				float a = buf[ n ];
				float b = buf[ m ];

				buf[ n ] = ( a * r ) + ( b * ( 1.f - r ) );
				buf[ m ] = ( a * r ) + ( b * ( 1.f - r ) );
			}
		}
	}

	void sine( gam::Array<float>& buf, float tone )
	{
		gam::Sine<> s;
		s.freq( tone );

		buf.resize( audioIO().framesPerSecond() );

		for ( int n = 0; n < buf.size(); n++ )
		{
			buf[ n ] = s();
		}
	}

	void analysis( gam::Array<float>& buf )
	{
		gam::RFFT<float> rfft( buf.size() );
		
		rfft.forward( &buf[ 0 ] );

		// std::cout << std::endl;

		float max = 0.f;
		float freq = 0.f;

		for ( int n = 1; n < buf.size(); n += 2 )
		{
			const float f = static_cast< float >( n ) * static_cast< float >( audioIO().framesPerSecond() ) / buf.size();

			if ( max < buf[ n ] )
			{
				max = buf[ n ];
				freq = f;
			}

			if ( f >= 200 && f <= 500 )
			{
				std::cout << "f : " << f << " : " << buf[ n ] << std::endl;
			}

			// std::cout << buf[ n ] << "," << buf[ n + 1 ] << std::endl;
		}

		std::cout << "freq: " << freq << std::endl;

		rfft.inverse( &buf[ 0 ] );

		/*
		gam::STFT stft;

		if ( stft( buf[ 0 ] ) )
		{
			for ( int n = 0; n < stft.numBins(); n++ )
			{
				stft.bin( n );
			}
		}

		std::cout << "num bins: " << stft.numBins() << std::endl;
		std::cout << "bin freq: " << stft.binFreq() << std::endl;
		*/
	}

	void print_edge( gam::Array<float>& buf ) const
	{
		std::cout << "edge :" << std::endl;

		for ( int n = -5; n <= 5; n++ )
		{
			std::cout << n << ": " << buf[ ( buf.size() + n ) % buf.size() ] << std::endl;
		}
	}

	void onAudio( gam::AudioIOData& io )
	{
		key_input();

		if ( is_recording() )
		{
			for ( int n = 0; n < io.framesPerBuffer(); n++ )
			{
				if ( rec_frame_index < rec_buf.size() )
				{
					rec_buf[ rec_frame_index ] = io.in( 0, n );
					rec_frame_index++;
				}
			}
		}

		int n = 0;

		while ( io() )
		{
			is_on_step_ = false;
			// is_on_beat_ = false;
			// is_on_bar_  = false;

			if ( timer() )
			{
				on_note();
			}

			update_bass();
			update_lead();

			mix( io, n );

			n++;
		}
	}

	void update_bass()
	{
		if ( page == Page::BASS )
		{
			// ベースのボリュームが左右の手の距離によって変わる
			if ( leap.hand_count() == 2 )
			{
				bass_volume.target_value() = std::clamp( leap.lh_pos().distanceTo( leap.rh_pos() ) / 1000.f, 0.f, 1.f);
			}

			bass_volume.chase();
		}
	}

	bool is_lh_lead_position() const { return leap.is_lh_valid() && leap.lh_pos().z < area_threashold_z; }
	bool is_rh_lead_position() const { return leap.is_rh_valid() && leap.rh_pos().z < area_threashold_z; }

	void update_lead()
	{
		const bool is_position_valid_l = is_lh_lead_position();
		const bool is_position_valid_r = is_rh_lead_position();
		
		const bool is_ll = range_to_rate( leap.x_pos_to_rate( leap.lh_pos().x ), 0.10f, 0.25f ) < 0.5f; // 左手が左エリアの左側にある
		const bool is_rr = range_to_rate( leap.x_pos_to_rate( leap.rh_pos().x ), 0.75f, 0.90f ) > 0.5f; // 右手が右エリアの右側にある

		const bool is_portamento_l = page < Page::CLIMAX || is_ll;
		const bool is_portamento_r = page < Page::CLIMAX || is_rr;

		const float chase_speed_l = is_portamento_l ? 0.0001f : 10.f;
		const float chase_speed_r = is_portamento_r ? 0.0005f : 10.f;

		const std::array< float, 8 > tones_diatonic_low  = { Tone::C4, Tone::D4, Tone::E4, Tone::F4, Tone::G4, Tone::A4, Tone::B4, Tone::C5, };
		const std::array< float, 8 > tones_diatonic_high = { Tone::C5, Tone::D5, Tone::E5, Tone::F5, Tone::G5, Tone::A5, Tone::B5, Tone::C6, };
		const std::array< float, 6 > tones_pentatonic_low = { Tone::C4, Tone::D4, Tone::E4, Tone::G4, Tone::A4, Tone::C5 };
		const std::array< float, 6 > tones_pentatonic_mid = { Tone::C5, Tone::D5, Tone::E5, Tone::G5, Tone::A5, Tone::C6 };
		const std::array< float, 6 > tones_pentatonic_high = { Tone::C6, Tone::D6, Tone::E6, Tone::G6, Tone::A6,Tone::C7 };

		const auto& tones_l = tones_pentatonic_low;
		const auto& tones_r = page == Page::CLIMAX ? tones_pentatonic_high : tones_pentatonic_mid;
		
		const auto target_tone_l = is_lh_lead_position() ? tones_l[ leap.y_pos_to_index( leap.lh_pos().y, tones_l.size() ) ] : tones_l[ tones_l.size() - 1 ];
		const auto target_tone_r = is_rh_lead_position() ? tones_r[ leap.y_pos_to_index( leap.rh_pos().y, tones_r.size() ) ] : tones_r[ tones_r.size() - 1 ];

		// std::cout << chase_speed_l << ", " << chase_speed_r << std::endl;

		if ( is_on_step() || is_portamento_l )
		{
			lead_l.rate( math::chase( static_cast< float >( lead_l.rate() ), target_tone_l / Tone::C3, chase_speed_l ) );
		}
		if ( is_on_step() || is_portamento_r )
		{
			lead_r.rate( math::chase( static_cast< float >( lead_r.rate() ), target_tone_r / Tone::C3, chase_speed_r ) );
		}

		lead_l_volume.target_value() = is_position_valid_l ? 1.f : 0.f;
		lead_r_volume.target_value() = is_position_valid_r ? 1.f : 0.f;

		lead_l_volume.speed() = 0.0001f * ( is_position_valid_l ? 1.f : 0.2f );
		lead_r_volume.speed() = 0.0001f * ( is_position_valid_r ? 1.f : 0.2f );

		lead_l_volume.chase();
		lead_r_volume.chase();
	}

	int get_page_index() const
	{
		return static_cast< int >( page );
	}

	void mix( gam::AudioIOData& io, int io_step )
	{
		//                                                { TAP,   PAD,   BASS,  KICK, SNARE,  DEMO,    R,      L,  FREE,   MAX, FIN }
		const std::array< float, PAGES >     delay_gain = { 0.25f, 0.25f, 0.10f, 0.10f, 0.10f, 0.10f, 0.20f, 0.20f, 0.20f, 0.30f, 0.75f };
		const std::array< float, PAGES >  delay_feedbak = { 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f };

		float s = 0.f;
			
		s +=   kick() * get_part_volume( Part::KICK   ) *  kick_env();
		s +=  snare() * get_part_volume( Part::SNARE  ) * snare_env();
		s +=   bass() * get_part_volume( Part::BASS   ) * bass_volume.value() * bass_env(); // * ( step % 4 / 2 );
		s += lead_l() * get_part_volume( Part::LEAD_L ) * lead_l_volume.value();
		s += lead_r() * get_part_volume( Part::LEAD_R ) * lead_r_volume.value();
		s +=    tap() * get_part_volume( Part::TAP    ) * tap_env();

		const float pad = ( pad1() + pad2() + pad3() ) / 3.f;
		s += pad      * get_part_volume( Part::PAD );
		s += bright() * get_part_volume( Part::BRIGHT ) * bright_env();

		s += page_down() + page_up();
		s /= static_cast< float >( Part::MAX );

		bq_filter.freq( leap.y_pos_to_rate( leap.rh_pos().y ) * 1000.f );
		// s = bq_filter( s );

		s = compress( s );
		
		s += delay( s * delay_gain[ get_page_index() ] + delay() * delay_feedbak[ get_page_index() ] );
		s = compress( s );

		if ( is_recording() || is_speaking_mode() )
		{
			s *= 0.2f;
			s += io.in( 0, io_step ) * 0.3f;
		}

		io.out( 0 ) = s;
		io.out( 1 ) = s;
	}

	float compress( float level )
	{
		float compressor_max_level = 1.f;

		compressor_max_level = math::chase( compressor_max_level, 1.f, 0.0001f );
		compressor_max_level = std::max( std::abs( level ), compressor_max_level );

		level /= ( compressor_max_level + 0.01f );

		/*
		if ( max_level > 1.f )
		{
			std::cout << "max: " << max_level << std::endl;
		}
		*/

		/*
		if ( std::abs( level ) >= 1.f )
		{
			std::cout << level << ", " << max_level << std::endl;
		}
		*/

		compressor_max_level = std::min( compressor_max_level, 10.f );

		return level;
	}

	/**
	 * 16 分音符毎の処理
	 *
	 */
	void on_note()
	{
		is_on_step_ = true;

		if ( page != Page::FINISH || step < 15 )
		{
			step = ( step + 1 ) % 16;

			on_step( ( step % 4 ) == 0, step == 0 );

			if ( step == 0 )
			{
				bar = ( bar + 1 ) % 4;
			}
		}

		// 1 小節目の 1 拍目のみでページ変更を行う
		// ( 2 ～ 4 小節目でページ変更のジェスチャーした場合でも 1 小節目にページを変更する )
		if ( bar == 0 && step == 0 || page == Page::FINISH )
		{
			// ページの変更
			while ( get_page_index() < leap.page() )
			{
				page = static_cast< Page >( get_page_index() + 1 );
				on_page_changed( page, true );
			}

			while ( get_page_index() > leap.page() )
			{
				page = static_cast< Page >( get_page_index() - 1 );
				on_page_changed( page, false );
			}
		}

		if ( page == Page::FINISH && step > 0 )
		{
			return;
		}

		//
		if ( step == 0 )
		{
			bass_env.reset();
		}

		if ( leap.pop_page_decremented() )
		{
			page_down.reset();
		}
		if ( leap.pop_page_incremented() )
		{
			page_up.reset();
		}

		const bool l_tapped = leap.pop_l_tapped();
		const bool r_tapped = leap.pop_r_tapped();

		if ( l_tapped || r_tapped )
		{
			const int random_note_range = 5;
			const std::array< float, 21 > tap_note = {
				Tone::C3, Tone::D3, Tone::E3, Tone::G3, Tone::A3,
				Tone::C4, Tone::D4, Tone::E4, Tone::G4, Tone::A4,
				Tone::C5, Tone::D5, Tone::E5, Tone::G5, Tone::A5,
				Tone::C6, Tone::D6, Tone::E6, Tone::G6, Tone::A6,
				Tone::C7
			};
			const auto tapped_y = l_tapped ? leap.lh_pos().y : leap.rh_pos().y;
			const int tap_index = leap.y_pos_to_index( tapped_y, tap_note.size() - random_note_range ) + rand() % random_note_range;
			
			tap.rate( ( page == Page::TAP && ! leap.is_lh_valid() ) ? 1.f : ( tap_note[ tap_index ] / Tone::C4 ) );
			tap.reset();
			tap_env.reset();
		}

		update_sequencer();

		// const std::array< float, 4 > bass_rate = { Tone::C3, Tone::E3, Tone::F3, Tone::G3 };
		const std::array< float, 4 > bass_rate = { Tone::F1, Tone::G1, Tone::A1, Tone::C2 };
		bass.rate( page >= Page::DEMO ? bass_rate[ bar ] / Tone::C3 : 1.f );

		// const std::array< float, 4 > pad_1_tones = { Tone::C4, Tone::C4, Tone::A3, Tone::B3 };
		// const std::array< float, 4 > pad_2_tones = { Tone::G3, Tone::G3, Tone::F3, Tone::G3 };
		// const std::array< float, 4 > pad_3_tones = { Tone::E3, Tone::E3, Tone::C3, Tone::D3 };

		const std::array< float, 4 > pad_1_tones = { Tone::C4, Tone::D4, Tone::E4, Tone::D4 };
		const std::array< float, 4 > pad_2_tones = { Tone::A3, Tone::A3, Tone::C4, Tone::B3 };
		const std::array< float, 4 > pad_3_tones = { Tone::G3, Tone::F3, Tone::G3, Tone::G3 };

		pad1.rate( rate_to_range( get_slider_value_l( Page::PAD ), 1.f, pad_1_tones[ bar ] / Tone::C3 ) );
		pad2.rate( rate_to_range( get_slider_value_l( Page::PAD ), 1.f, pad_2_tones[ bar ] / Tone::C3 ) );
		pad3.rate( rate_to_range( get_slider_value_l( Page::PAD ), 1.f, pad_3_tones[ bar ] / Tone::C3 ) );

		if ( page >= Page::FREE )
		{
			if ( step / 2 % 2 == 1 )
			{
				bass.rate( bass.rate() * 2.f );
			}
		}
		if ( page == Page::FINISH )
		{
			finished = true;
			kick.rate( 1.f );
			kick.range( 0.f, 10.f );
			kick.reset();
		}
	}

	void on_step( bool, bool );
	

	void update_sequencer()
	{
		const bool kick_on[ 4 ][ 2 ][ 16 ] = {
			{
				{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
				{ 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
			},
			{
				{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0 },
				{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
			},
			{
				{ 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0 },
				{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			},
			{
				{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			}
		};

		const bool snare_on[ 4 ][ 2 ][ 16 ] = {
			{
				{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
				{ 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1 }
			},
			{
				{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
				{ 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1 }
			},
			{
				{ 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 },
				{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1 },
			},
			{
				{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			}
		};

		const float bright_phrase[ 3 ][ 2 ][ 16 ] = {
			{
				{ Tone::C5, Tone::__, Tone::G5, Tone::__,  Tone::C6, Tone::__, Tone::C5, Tone::G5,  Tone::__, Tone::C6, Tone::__, Tone::C6,  Tone::G5, Tone::__, Tone::C6, Tone::__, },
				{ Tone::C5, Tone::__, Tone::G5, Tone::__,  Tone::C6, Tone::__, Tone::C5, Tone::G5,  Tone::__, Tone::C6, Tone::__, Tone::C6,  Tone::G5, Tone::__, Tone::C6, Tone::__, },
			},
			{
				{ Tone::A5, Tone::__, Tone::G5, Tone::__,  Tone::E5, Tone::__, Tone::D5, Tone::C5,  Tone::__, Tone::D5, Tone::__, Tone::E5,  Tone::D5, Tone::__, Tone::C5, Tone::__, },
				{ Tone::A5, Tone::__, Tone::G5, Tone::__,  Tone::E5, Tone::__, Tone::D5, Tone::C5,  Tone::__, Tone::D5, Tone::__, Tone::G5,  Tone::D5, Tone::__, Tone::C5, Tone::__, },
			},
			{
				{ Tone::C5, },
				{ Tone::__, },
			},
		};

		const int is_fill_in = bar < 3 ? 0 : 1;

		const std::array< int, PAGES > kick_pattern = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3 };
		const std::array< int, PAGES > snare_pattern = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3 };

		if ( kick_on[ kick_pattern[ get_page_index() ] ][ is_fill_in ][ step ] )
		{
			kick.range( std::min( 0.9f, get_slider_value_l( Page::KICK ) ), 0.15f );
			kick.rate( rate_to_range( get_slider_value_r( Page::KICK ), RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
			kick.reset();
			kick_env.reset();
		}

		if ( snare_on[ snare_pattern[ get_page_index() ] ][ is_fill_in ][ step ] )
		{
			snare.range( std::min( 0.9f, get_slider_value_l( Page::SNARE ) ), 0.15f );
			snare.rate( rate_to_range( get_slider_value_r( Page::SNARE ), RHYTHM_RATE_MIN, RHYTHM_RATE_MAX ) );
			snare.reset();
			snare_env.reset();
		}

		const float bright_tone = bright_phrase[ page == Page::FINISH ? 2 : ( page >= Page::FREE ? 1 : 0 ) ][ is_fill_in ][ step ];

		if ( bright_tone  != Tone::__ )
		{
			bright.rate( bright_tone / Tone::C3 );
			bright.reset();
			bright_env.reset();
		}
	}

	void on_page_changed( Page page, bool incremented )
	{
		std::cout << "--------------------" << std::endl;
		std::cout << "page : " << get_page_name( page ) << std::endl;

		if ( page == Page::BASS )
		{
			bass_volume.target_value() = 0.f;
			// std::cout << "pc : " << bass_volume.target_value() << std::endl;
		}
		else if ( page > Page::BASS )
		{
			bass_volume.fit( 1.f );
		}

		if ( incremented )
		{
			if ( page == Page::FINISH )
			{
				send_fire();
			}
		}
		else
		{
			if ( page == Page::BASS )
			{
				step = 0;
				bar = 0;
			}

			if ( page == Page::CLIMAX )
			{
				finished = false;
			}
		}
	}

	int get_frames_per_beat() const
	{
		return static_cast< int >( audioIO().framesPerSecond() * 60 / get_bpm() );
	}

	int get_frames_per_step() const
	{
		return get_frames_per_beat() / 4;
	}

	void send_fire();
};