#include "HandAudioCallback.h"
#include "Hand.h"

void HandAudioCallback::on_step( bool on_beat, bool on_bar )
{
	hand.on_step( on_beat, on_bar );
}

void HandAudioCallback::send_fire()
{
	hand.send_fire();
}

