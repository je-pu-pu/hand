#include "HandAudioCallback.h"
#include "Hand.h"

void HandAudioCallback::on_step( bool on_beat, bool on_bar, const std::string& cpn, const std::string& npn )
{
	hand.on_step( on_beat, on_bar, cpn, npn );
}

void HandAudioCallback::send_fire()
{
	hand.send_fire();
}

