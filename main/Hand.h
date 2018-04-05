#pragma once

#include <boost/asio.hpp>

#include "Config.h"

#include "HandAudioCallback.h"
#include "LeapSoundController.h"

#include <Gamma/AudioIO.h>

#include <memory>
#include <iostream>

class Hand
{
private:
	std::unique_ptr< HandAudioCallback > audio_callback_;

	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::socket server_socket_;
	boost::asio::ip::tcp::acceptor acceptor_;
	bool is_central_connected_ = false;

	LeapSoundController leap_;

public:
	Hand()
		: server_socket_( io_service_ )
		, acceptor_( io_service_, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), 8080 ) )
	{
	}

	void start()
	{
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

		controller.addListener( leap_ );

		controller.setPolicy( Leap::Controller::POLICY_BACKGROUND_FRAMES );
		controller.setPolicy( Leap::Controller::POLICY_ALLOW_PAUSE_RESUME );

		// std::cout << controller.config().getFloat( "Gesture.Swipe.MinLength" ) << std::endl;

		audio_callback_ = std::make_unique< HandAudioCallback >( *this, in, out, leap_ );

		std::thread server_thread = start_server();

		audio_callback_->start( false );

		int x;
		std::cin >> x;

		audio_callback_->stop();

		// std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

		controller.removeListener( leap_ );

		config.save_file( "./config.txt" );

		server_thread.detach();
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