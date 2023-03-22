#pragma once

#include "fmod_studio.hpp"
#include <string>


class AudioSource
{
	public:
		FMOD::Sound* sound;
		FMOD::Channel* channel;
		FMOD_VECTOR* pos;
		AudioSource();
		AudioSource(const std::string& file, FMOD_MODE mode);
		~AudioSource();
		void LoadAudio(const std::string& file, FMOD_MODE mode);
		void Play();
		void SetVolume(float volume);
		void Pause(bool pause);
		void UnPause();
		void update(float x, float y, float z);
	private:

};
