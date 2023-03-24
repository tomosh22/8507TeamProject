#include "AudioSource.h"
#include "AudioSystem.h"

AudioSource::AudioSource()
{

}

AudioSource::AudioSource(const std::string& file, FMOD_MODE mode)
{
	sound = AudioSystem::GetInstance()->loadSound(file, mode);
	pos = new FMOD_VECTOR();
}

AudioSource::~AudioSource()
{
	delete sound;
	delete channel;
	delete pos;
}

void AudioSource::LoadAudio(const std::string& file, FMOD_MODE mode)
{
	sound = AudioSystem::GetInstance()->loadSound(file, mode);
}

void AudioSource::Play()
{
	if (sound != nullptr)
	{
		channel = AudioSystem::GetInstance()->playSound(sound,channel);
	}
}

void AudioSource::SetVolume(float volume)
{
	channel->setVolume(volume);
}

void AudioSource::UnPause()
{
	channel->setPaused(false);
}

// updateSourcePos
void AudioSource::update(float x,float y ,float z)
{	
	pos->x = x;
	pos->y = y;
	pos->z = z;
	channel->set3DAttributes(pos, 0);
}

void AudioSource::Pause(bool pause)
{
	channel->setPaused(pause);
}
