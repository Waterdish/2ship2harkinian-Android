#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

#define ELFMSG3_GET_SWITCH_FLAG(thisx) (((thisx)->params & 0x7F00) >> 8)

extern "C" {
#include "z64.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern u32 gBitFlags[];
void Flags_SetWeekEventReg(s32 flag);
void Flags_SetSwitch(PlayState* play, s32 flag);
void Actor_Kill(Actor* actor);
}

void RegisterSkipTatlInterrupts() {
    // First time entering Clock Town Interupt
    REGISTER_VB_SHOULD(VB_PLAY_TRANSITION_CS, {
        if (gSaveContext.save.entrance == ENTRANCE(SOUTH_CLOCK_TOWN, 0) && gSaveContext.save.cutsceneIndex == 0 &&
            !CHECK_WEEKEVENTREG(WEEKEVENTREG_59_04) &&
            CVarGetInteger("gEnhancements.Cutscenes.SkipMiscInteractions", 0)) {
            Flags_SetWeekEventReg(WEEKEVENTREG_59_04);
        }
    });

    // General interupt
    REGISTER_VB_SHOULD(VB_TATL_INTERUPT_MSG3, {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipMiscInteractions", 0) && *should) {
            Actor* actor = va_arg(args, Actor*);
            *should = false;
            if (ELFMSG3_GET_SWITCH_FLAG(actor) != 0x7F) {
                Flags_SetSwitch(gPlayState, ELFMSG3_GET_SWITCH_FLAG(actor));
            }
            Actor_Kill(actor);
        }
    });

    // General interupt 2 (the flags were directly copied from the original code)
    REGISTER_VB_SHOULD(VB_TATL_INTERUPT_MSG6, {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipMiscInteractions", 0) && *should) {
            Actor* actor = va_arg(args, Actor*);
            *should = false;
            switch (actor->textId) {
                case 0x224:
                    SET_WEEKEVENTREG(WEEKEVENTREG_79_10);
                    break;

                case 0x25B:
                    SET_WEEKEVENTREG(WEEKEVENTREG_88_20);
                    break;

                case 0x216:
                    SET_WEEKEVENTREG(WEEKEVENTREG_31_04);
                    break;

                case 0x231:
                    SET_WEEKEVENTREG(WEEKEVENTREG_31_01);
                    break;

                case 0x232:
                    SET_WEEKEVENTREG(WEEKEVENTREG_31_02);
                    break;

                case 0x233:
                    SET_WEEKEVENTREG(WEEKEVENTREG_80_04);
                    break;
            }
            if (CHECK_QUEST_ITEM(QUEST_REMAINS_ODOLWA)) {
                SET_WEEKEVENTREG(WEEKEVENTREG_87_10);
            }

            if (CHECK_QUEST_ITEM(QUEST_REMAINS_GOHT)) {
                SET_WEEKEVENTREG(WEEKEVENTREG_87_20);
            }

            if (CHECK_QUEST_ITEM(QUEST_REMAINS_GYORG)) {
                SET_WEEKEVENTREG(WEEKEVENTREG_87_40);
            }

            if (CHECK_QUEST_ITEM(QUEST_REMAINS_TWINMOLD)) {
                SET_WEEKEVENTREG(WEEKEVENTREG_87_80);
            }
            Actor_Kill(actor);
        }
    });
}
