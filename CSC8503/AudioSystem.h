#pragma once

#include "fmod_studio.hpp"
#include <string>

class AudioSystem
{
public:
	static AudioSystem* GetInstance()
	{
		if (_instance == nullptr)
		{
			_instance = new AudioSystem();
		}
		return _instance;
	}
	FMOD::Sound* loadSound(const std::string& file, FMOD_MODE mode) const;
	FMOD::Channel* playSound(FMOD::Sound* sound) const;
	FMOD::Channel* playSound(FMOD::Sound* sound,float volume) const;
	FMOD::Channel* playSound(FMOD::Sound* sound,  FMOD::Channel* channel) const;
	FMOD::Channel* pauseSound(FMOD::Sound* sound) const;
	void update();
	AudioSystem();
	~AudioSystem();
protected:
	static AudioSystem* _instance;
	FMOD::Studio::System* system;
	FMOD::System* lowLevelSystem;
};