#pragma once

#include <iostream>
#include <array>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <Leap.h>

class LeapSoundController : public Leap::Listener
{
public:
	static const int PAGES = 11;

private:
	int page_ = 0;
	bool page_incremented_ = false;
	bool page_decremented_ = false;

	// この時間までスワイプジェスチャーを停止する
	std::chrono::system_clock::time_point swipe_stop_until_ = std::chrono::system_clock::now();

	std::array< float, PAGES > l_slider_;
	std::array< float, PAGES > r_slider_;

	bool l_tapped_ = false;
	bool r_tapped_ = false;

	Leap::Hand lh_;						/// 最新の左手
	Leap::Hand rh_;						/// 最新の右手

	Leap::Vector lh_pos_;				/// 最後の左手の位置
	Leap::Vector rh_pos_;				/// 最後の右手の位置

	bool is_l_slider_moving_ = false;	/// 現在左手がつまんでいるフラグ
	bool is_r_slider_moving_ = false;	/// 現在右手がつまんでいるフラグ

	float l_slider_start_value_;		/// 左スライダーの移動を開始した時点での値
	float r_slider_start_value_;		/// 右スライダーの移動を開始した時点での値

	float l_slider_start_hand_y_;		/// 左スライダーの移動を開始した時の手の位置
	float r_slider_start_hand_y_;		/// 右スライダーの移動を開始した時の手の位置

	int hand_count_ = 0;

protected:
	void start_l_slider_moving( float y )
	{
		is_l_slider_moving_ = true;
		l_slider_start_value_ = l_slider_[ page_ ];
		l_slider_start_hand_y_ = y;

		// std::cout << "L Slider : Start" << std::endl;
	}

	void stop_l_slider_moving()
	{
		if ( is_l_slider_moving_ )
		{
			is_l_slider_moving_ = false;

			// std::cout << "L Slider : Stop" << std::endl;
		}
	}

	void start_r_slider_moving( float y )
	{
		is_r_slider_moving_ = true;
		r_slider_start_value_ = r_slider_[ page_ ];
		r_slider_start_hand_y_ = y;

		// std::cout << "R Slider : Start" << std::endl;
	}

	void stop_r_slider_moving()
	{
		if ( is_r_slider_moving_ )
		{
			is_r_slider_moving_ = false;

			// std::cout << "R Slider : Stop" << std::endl;
		}
	}

	void move_l_slider( float y )
	{
		if ( is_l_slider_moving_ )
		{
			set_l_slider( page_, l_slider_start_value_ + ( y - l_slider_start_hand_y_ ) / 2000.f );
		}
	}

	void move_r_slider( float y )
	{
		if ( is_r_slider_moving_ )
		{
			set_r_slider( page_, r_slider_start_value_ + ( y - r_slider_start_hand_y_ ) / 2000.f );
		}
	}

public:
	LeapSoundController()
	{
		for ( int n = 0; n < PAGES; n++ )
		{
			l_slider_[ n ] = 0.f;
			r_slider_[ n ] = 0.f;
		}
	}

	bool is_lh_valid() const { return lh_.isValid(); }
	bool is_rh_valid() const { return rh_.isValid(); }

	const Leap::Vector& lh_pos() const { return lh_pos_; }
	const Leap::Vector& rh_pos() const { return rh_pos_; }

	bool is_l_slider_moving() const { return is_l_slider_moving_; }
	bool is_r_slider_moving() const { return is_r_slider_moving_; }

	const float l_slider( int page ) const
	{
		return l_slider_[ page ];
	}

	const float r_slider( int page ) const
	{
		return r_slider_[ page ];
	}

	void move_l_slider_force( float dy )
	{
		set_l_slider( page_, l_slider( page_ ) + dy );
	}

	void move_r_slider_force( float dy )
	{
		set_r_slider( page_, r_slider( page_ ) + dy );
	}

	void set_l_slider( int page, float value )
	{
		l_slider_[ page ] = std::max( 0.f, std::min( 1.f, value ) );

		// std::cout << "slider L : " << page_ << " : " << l_slider_[ page_ ] << std::endl;
	}

	void set_r_slider( int page, float value )
	{
		r_slider_[ page ] = std::max( 0.f, std::min( 1.f, value ) );

		// std::cout << "slider R : " << page_ << " : " << r_slider_[ page_ ] << std::endl;
	}

	float x_pos_to_rate( float x ) const
	{
		const float max_x = 200.f;

		float rate = ( x + max_x ) / ( max_x * 2.f );

		if ( rate < 0.f )
		{
			rate = 0.f;
		}
		if ( rate > 1.f )
		{
			rate = 1.f;
		}

		return rate;
	}

	float y_pos_to_rate( float y ) const
	{
		const float min_y = 200.f;
		const float max_y = 800.f;

		return std::clamp( ( y - min_y ) / ( max_y - min_y ), 0.f, 1.f );
	}

	const int y_pos_to_index( float y, int size ) const
	{
		int index = static_cast< int >( y_pos_to_rate( y ) * size );

		if ( index < 0 )
		{
			index = 0;
		}
		if ( index >= size )
		{
			index = size - 1;
		}

		return index;
	}

	int page() const { return page_; }
	void set_page( int page ) { page_ = page; }

	bool decrement_page()
	{
		if ( page_ <= 0 || page_decremented_ )
		{
			return false;
		}

		stop_l_slider_moving();
		stop_r_slider_moving();

		page_--;
		page_decremented_ = true;

		return true;
	}

	bool increment_page()
	{
		if ( page_ >= PAGES - 1 || page_incremented_ )
		{
			return false;
		}

		stop_l_slider_moving();
		stop_r_slider_moving();

		page_++;
		page_incremented_ = true;

		return true;
	}

	bool pop_page_incremented()
	{
		bool result = page_incremented_;
		page_incremented_ = false;
		return result;
	}
	bool pop_page_decremented()
	{
		bool result = page_decremented_;
		page_decremented_ = false;
		return result;
	}

	bool is_page_incremented() const { return page_incremented_; }
	bool is_page_decremented() const { return page_decremented_; }

	bool pop_l_tapped()
	{
		bool result = l_tapped_;
		l_tapped_ = false;
		return result;
	}

	bool pop_r_tapped()
	{
		bool result = r_tapped_;
		r_tapped_ = false;
		return result;
	}

	int hand_count() const { return hand_count_;  }

public:
    void onInit( const Leap::Controller& ) override { std::cout << "Initialized" << std::endl; }
	
	void onConnect( const Leap::Controller& controller ) override
	{
		std::cout << "Connected" << std::endl;

		controller.enableGesture( Leap::Gesture::TYPE_CIRCLE );
		controller.enableGesture( Leap::Gesture::TYPE_SWIPE );
		controller.enableGesture( Leap::Gesture::TYPE_KEY_TAP );
		controller.enableGesture( Leap::Gesture::TYPE_SCREEN_TAP );

		controller.config().setFloat( "Gesture.Swipe.MinLength", 400.f );
		controller.config().setFloat( "Gesture.Swipe.MinVelocity", 100.f );
		controller.config().save();
	}

	void onDisconnect(const Leap::Controller&) override { std::cout << "Disconnected" << std::endl; }
	void onExit(const Leap::Controller&) override { std::cout << "Exited" << std::endl; }
	
	void onFrame(const Leap::Controller& controller ) override
	{
		const Leap::Frame frame = controller.frame();
		
		hand_count_ = frame.hands().count();

		// std::cout << "hands: " << frame.hands().count();
		// std::cout << ", extended fingers: " << frame.fingers().extended().count();

		for ( auto g : frame.gestures() )
		{
			if ( g.type() == Leap::Gesture::TYPE_CIRCLE )
			{
				on_circle_gesture( g );
			}
			else if ( g.type() == Leap::Gesture::TYPE_SWIPE )
			{
				on_swipe_gesture( g );
			}
			else if ( g.type() == Leap::Gesture::TYPE_KEY_TAP )
			{
				on_key_tap_gesture( g );
			}
			else if ( g.type() == Leap::Gesture::TYPE_SCREEN_TAP )
			{
				on_screen_tap_gesture( g );
			}
			
		}

		auto hands = frame.hands();

		lh_ = Leap::Hand::invalid();
		rh_ = Leap::Hand::invalid();

		for ( auto hl = hands.begin(); hl != hands.end(); ++hl )
		{
			const auto hand = *hl;

			if ( hand.confidence() < 1.f )
			{
				if ( hand.isLeft() )
				{
					stop_l_slider_moving();
				}
				if ( hand.isRight() )
				{
					stop_r_slider_moving();
				}

				// std::cout << "c : " << hand.confidence() << std::endl;

				continue;
			}

			const auto fingers = hand.fingers();

			for ( auto fl = fingers.begin(); fl != fingers.end(); ++fl )
			{
				const auto finger = *fl;
			}

			if ( hand.isLeft() )
			{
				lh_ = hand;
				lh_pos_ = hand.wristPosition();

				if ( ! is_l_slider_moving_ && hand.pinchStrength() >= 1.f )
				{
					start_l_slider_moving( hand.wristPosition().y );
				}
				else if ( is_l_slider_moving_ && hand.pinchStrength() <= 0.f )
				{
					stop_l_slider_moving();
				}

				move_l_slider( hand.wristPosition().y );
			}
			else if ( hand.isRight() )
			{
				rh_ = hand;
				rh_pos_ = hand.wristPosition();

				if ( ! is_r_slider_moving_ && hand.pinchStrength() >= 1.f )
				{
					start_r_slider_moving( hand.wristPosition().y );
				}
				else if ( is_r_slider_moving_ && hand.pinchStrength() <= 0.f )
				{
					stop_r_slider_moving();
				}

				move_r_slider( hand.wristPosition().y );
			}
		}
	}
	
	void onFocusGained(const Leap::Controller&) override { std::cout << "Focus Gained" << std::endl; }
	void onFocusLost(const Leap::Controller&) override { std::cout << "Focus Lost" << std::endl; }
	
	void onDeviceChange( const Leap::Controller& controller ) override
	{
		std::cout << "Device Changed" << std::endl;
		const auto devices = controller.devices();

		for (int i = 0; i < devices.count(); ++i) {
			std::cout << "id: " << devices[i].toString() << std::endl;
			std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
			std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
			std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl;
		}
	}

	void onServiceConnect(const Leap::Controller&) override
	{
		std::cout << "Service Connected" << std::endl;
	}

	void onServiceDisconnect(const Leap::Controller&) override
	{
		std::cout << "Service Disconnected" << std::endl;
	};

    void onServiceChange( const Leap::Controller& ) override
	{
		std::cout << "Service Changed" << std::endl;
	}

	void onDeviceFailure( const Leap::Controller& controller ) override
	{
		std::cout << "Device Error" << std::endl;
		const Leap::FailedDeviceList devices = controller.failedDevices();

		for ( Leap::FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl )
		{
			const auto device = *dl;
			std::cout << "  PNP ID:" << device.pnpId();
			std::cout << "    Failure type:" << device.failure();
		}
	}
	
	void onLogMessage( const Leap::Controller&, Leap::MessageSeverity s, int64_t t, const char* msg ) override
	{
		switch (s) {
		case Leap::MESSAGE_CRITICAL:
			std::cout << "[Critical]";
			break;
		case Leap::MESSAGE_WARNING:
			std::cout << "[Warning]";
			break;
		case Leap::MESSAGE_INFORMATION:
			std::cout << "[Info]";
			break;
		case Leap::MESSAGE_UNKNOWN:
			std::cout << "[Unknown]";
		}
		std::cout << "[" << t << "] ";
		std::cout << msg << std::endl;
	}

protected:

	void on_circle_gesture( const Leap::CircleGesture& circle )
	{
		// std::cout << "circle : " << circle.toString() << " : " << circle.progress() << std::endl;
	}

	void on_swipe_gesture( const Leap::SwipeGesture& swipe )
	{
		if ( swipe.state() != Leap::Gesture::STATE_STOP )
		{
			return;
		}

		if ( std::abs( swipe.direction().x ) < std::abs( swipe.direction().y ) )
		{
			return;
		}

		if ( std::abs( swipe.direction().x ) < std::abs( swipe.direction().z ) )
		{
			return;
		}

		auto now = std::chrono::system_clock::now();

		if ( now < swipe_stop_until_ )
		{
			return;
		}

		if ( swipe.direction().x > 0.f )
		{
			if ( decrement_page() )
			{
				swipe_stop_until_ = now + std::chrono::milliseconds( 500 );
			}
		}
		else
		{
			if ( increment_page() )
			{
				swipe_stop_until_ = now + std::chrono::milliseconds( 500 );
			}
		}

		// std::cout << "swipe : " << swipe.hands().isEmpty() << std::endl;
		// std::cout << "leap page : " << page_ << std::endl;
	}

	void on_key_tap_gesture( const Leap::KeyTapGesture& tap )
	{
		Leap::Finger f( tap.pointable() );

		// std::cout << "key tap : " << tap.toString() << " : " << ( f.hand().isLeft() ? "L" : "R" ) << f.type() << std::endl;

		if ( f.hand().isLeft() )
		{
			l_tapped_ = true;
		}
		else
		{
			r_tapped_ = true;
		}
	}

	void on_screen_tap_gesture( const Leap::ScreenTapGesture& tap )
	{
		Leap::Finger f( tap.pointable() );

		// std::cout << "screen tap : " << tap.toString() << " : " << ( f.hand().isLeft() ? "L" : "R" ) << f.type() << std::endl;
	}
};

