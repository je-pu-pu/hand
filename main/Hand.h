#pragma once

#include <boost/asio.hpp>

#include "Config.h"

#include "HandAudioCallback.h"
#include "LeapSoundController.h"

#include <Gamma/AudioIO.h>

#include <memory>
#include <iostream>
#include <functional>

class Hand
{
private:
	std::unique_ptr< HandAudioCallback > audio_callback_;

	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::socket server_socket_;
	boost::asio::ip::tcp::acceptor acceptor_;
	bool is_central_connected_ = false;

	std::thread server_thread_;

	Config config_;
	LeapSoundController leap_;
	Leap::Controller controller_;

	std::function< void( bool, bool ) > on_step_;

public:
	Hand()
		: server_socket_( io_service_ )
		, acceptor_( io_service_, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), 8080 ) )
	{

	}

	~Hand()
	{
		stop();
	}

	void start( bool block = true )
	{
		gam::AudioDevice::printAll();

		int in = 0;
		int out = 0;

		if ( config_.load_file( "./config.txt" ) )
		{
			in = config_.get( "device.input_device_no", 0 );
			out = config_.get( "device.output_device_no", 0 );
		}
		else
		{
			std::cout << "input device no : ";
			std::cin >> in;

			std::cout << "output device no : ";
			std::cin >> out;

			config_.set( "device.input_device_no", in );
			config_.set( "device.output_device_no", out );
		}

		controller_.addListener( leap_ );

		controller_.setPolicy( Leap::Controller::POLICY_BACKGROUND_FRAMES );
		controller_.setPolicy( Leap::Controller::POLICY_ALLOW_PAUSE_RESUME );

		// std::cout << controller.config().getFloat( "Gesture.Swipe.MinLength" ) << std::endl;

		audio_callback_ = std::make_unique< HandAudioCallback >( *this, in, out, leap_ );
		audio_callback_->set_mic_volume( config_.get( "hand.mic_volume", HandAudioCallback::DEFAULT_MIC_VOLUME ) );
		audio_callback_->set_bgm_volume( config_.get( "hand.bgm_volume", HandAudioCallback::DEFAULT_BGM_VOLUME ) );

		server_thread_ = start_server();

		audio_callback_->start( false );
	}

	void set_on_step( std::function< void( bool, bool ) > f ) { on_step_ = f; }
	void on_step( bool on_beat, bool on_bar ) { if ( on_step_ ) { on_step_( on_beat, on_bar ); } };

	const HandAudioCallback& audio() const { return *audio_callback_; }
	const LeapSoundController& leap() const { return leap_; }

	void stop()
	{
		audio_callback_->stop();

		// std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

		controller_.removeListener( leap_ );

		config_.set( "hand.mic_volume", audio_callback_->get_mic_volume() );
		config_.set( "hand.bgm_volume", audio_callback_->get_bgm_volume() );
		config_.save_file( "./config.txt" );

		server_thread_.detach();
	}

	std::thread start_server()
	{
		return std::thread( [this] () {
			try
			{
				acceptor_.accept( server_socket_ );
				is_central_connected_ = true;
			}
			catch ( std::exception e )
			{
				std::cout << "error : " << e.what();
			}
		} );
	}

	void send_fire()
	{
		if ( ! is_central_connected_ )
		{
			std::cout << "central is not connected." << std::endl;
			return;
		}

		size_t s = boost::asio::write( server_socket_, boost::asio::buffer( "fire" ) );
		std::cout << "write " << s << std::endl;
	}
};