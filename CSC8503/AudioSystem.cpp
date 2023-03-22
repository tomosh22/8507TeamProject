#include "AudioSystem.h"
#include <stdio.h>
#include <fmod_errors.h>

AudioSystem* AudioSystem::_instance = nullptr;
FMOD::Sound* AudioSystem::loadSound(const std::string& file, FMOD_MODE mode) const {
	FMOD::Sound* pSound;
	std::string path = "../../Assets/Audio/" + file;
	this->lowLevelSystem->createSound(path.c_str(), mode, nullptr, &pSound);
	return pSound;
}

FMOD::Channel* AudioSystem::playSound(FMOD::Sound* sound) const {
	FMOD::Channel* channel;
	lowLevelSystem->playSound(sound, nullptr, false, &channel);
	return channel;
}

FMOD::Channel* AudioSystem::playSound(FMOD::Sound* sound, float volume) const
{
	FMOD::Channel* channel;
	lowLevelSystem->playSound(sound, nullptr, false, &channel);
	channel->setVolume(volume);
	return channel;
}

FMOD::Channel* AudioSystem::playSound(FMOD::Sound* sound, FMOD::Channel* channel) const
{
	lowLevelSystem->playSound(sound, nullptr, false, &channel);
	return channel;
}

FMOD::Channel* AudioSystem::pauseSound(FMOD::Sound* sound) const
{
	FMOD::Channel* channel;
	lowLevelSystem->playSound(sound, nullptr, true, &channel);
	return channel;
}

void AudioSystem::update(float x, float y, float z) {
	system->update();

	pos->x = x;
	pos->y = y;
	pos->z = z;

	lowLevelSystem->set3DListenerAttributes(0, pos, 0, 0, 0);

	lowLevelSystem->update();
}

AudioSystem::AudioSystem() : system(NULL) {
	FMOD_RESULT result;
	result = FMOD::Studio::System::create(&system); // Create the Studio System object.
	if (result != FMOD_OK) {
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}

	// Initialize FMOD Studio, which will also initialize FMOD Low Level
	result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK) {
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
	system->getCoreSystem(&lowLevelSystem);
	pos = new FMOD_VECTOR();
}


AudioSystem::~AudioSystem() {
	system->unloadAll();
	system->release();
	delete _instance;
	delete system;
	delete lowLevelSystem;
	delete pos;

}

