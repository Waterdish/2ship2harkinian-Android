#ifndef GRAPHICS_H
#define GRAPHICS_H

void Register3DItemDrops();
void Register3DSClock();
void RegisterDisableBlackBars();
void MotionBlur_RenderMenuOptions();
void RegisterPlayAsKafei();
void RegisterTextBasedClock();

#ifdef __cplusplus
#include <libultraship/libultraship.h>

extern "C" {
#endif

void MotionBlur_Override(u8* status, s32* alpha);

#ifdef __cplusplus
};
#endif

#endif // GRAPHICS_H
