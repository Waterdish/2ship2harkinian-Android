#ifndef SONGS_H
#define SONGS_H

void RegisterEnableSunsSong();
void RegisterFasterSongPlayback();
void RegisterZoraEggCount();
void RegisterPauseOwlWarp();
void RegisterSkipScarecrowSong();

#ifdef __cplusplus
extern "C" {
#endif

bool PauseOwlWarp_IsOwlWarpEnabled(void);

#ifdef __cplusplus
}
#endif

#endif // SONGS_H
