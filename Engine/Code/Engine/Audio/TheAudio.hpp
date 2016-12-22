#pragma once

//---------------------------------------------------------------------------
//#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library //Unnecessary IF we include the .lib in .sln's Engine/ThirdParty/fmod filter.

//---------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//---------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef FMOD::Channel* AudioChannelHandle;
const unsigned int MISSING_SOUND_ID = 0xffffffff;


//---------------------------------------------------------------------------
class AudioSystem;
extern AudioSystem* g_theAudio;


/////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();
	SoundID CreateOrGetSound( const std::string& soundFileName );
	AudioChannelHandle PlaySound( SoundID soundID, float volumeLevel = 1.f, bool loop = false );
	void StopChannel( AudioChannelHandle channel );
	bool isPlaying( AudioChannelHandle channel );
	void Update( ); // Must be called at regular intervals (e.g. every frame)

protected:
	void InitializeFMOD();
	void ValidateResult( FMOD_RESULT result );

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
};


//---------------------------------------------------------------------------
void InitializeAudio();