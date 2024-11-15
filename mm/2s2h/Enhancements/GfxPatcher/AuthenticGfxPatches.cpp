#include "AuthenticGfxPatches.h"
#include "libultraship/libultraship.h"
#include <cstring>

extern "C" {
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_fz/object_fz.h"
#include "objects/object_ik/object_ik.h"
#include "overlays/ovl_En_Syateki_Okuta/ovl_En_Syateki_Okuta.h"
#include "overlays/ovl_Obj_Jgame_Light/ovl_Obj_Jgame_Light.h"

void ResourceMgr_PatchGfxByName(const char* path, const char* patchName, int index, Gfx instruction);
void ResourceMgr_UnpatchGfxByName(const char* path, const char* patchName);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
}

#define dgameplay_keep_Tex_00CA30_Overflow "__OTR__objects/gameplay_keep/gameplay_keep_Tex_00CA30_Overflow"
static const ALIGN_ASSET(2) char gameplay_keep_Tex_00CA30_Overflow[] = dgameplay_keep_Tex_00CA30_Overflow;

#define dgEffIceFragmentTex_Overflow "__OTR__objects/gameplay_keep/gEffIceFragmentTex_Overflow"
static const ALIGN_ASSET(2) char gEffIceFragmentTex_Overflow[] = dgEffIceFragmentTex_Overflow;

#define dgIronKnuckleFireTex_Overflow "__OTR__objects/object_ik/gIronKnuckleFireTex_Overflow"
static const ALIGN_ASSET(2) char gIronKnuckleFireTex_Overflow[] = dgIronKnuckleFireTex_Overflow;

typedef struct {
    const char* dlist;
    int startInstruction;
} DListPatchInfo;

static DListPatchInfo freezardBodyDListPatchInfos[] = {
    { object_fz_DL_001130, 5 }, { object_fz_DL_0021A0, 5 }, { object_fz_DL_002CA0, 5 },
    { object_fz_DL_003260, 5 }, { object_fz_DL_0033F0, 5 },
};

static DListPatchInfo ironKnuckleDListPatchInfos[] = {
    { gIronKnuckleVambraceLeftDL, 39 },
    { gIronKnuckleVambraceLeftDL, 59 },

    { gIronKnuckleArmLeftDL, 38 },

    { gIronKnuckleVambraceRightDL, 39 },
    { gIronKnuckleVambraceRightDL, 59 },

    { gIronKnuckleArmRightDL, 38 },

    { gIronKnuckleWaistDL, 8 },
    { gIronKnuckleWaistDL, 28 },

    { gIronKnucklePauldronLeftDL, 8 },
    { gIronKnucklePauldronLeftDL, 31 },

    { gIronKnuckleBootTipLeftDL, 15 },
    { gIronKnuckleBootTipLeftDL, 37 },
    { gIronKnuckleBootTipLeftDL, 52 },
    { gIronKnuckleBootTipLeftDL, 68 },

    { gIronKnuckleWaistArmorLeftDL, 27 },
    { gIronKnuckleWaistArmorLeftDL, 46 },
    { gIronKnuckleWaistArmorLeftDL, 121 },

    { gIronKnucklePauldronRightDL, 8 },
    { gIronKnucklePauldronRightDL, 32 },

    { gIronKnuckleBootTipRightDL, 15 },
    { gIronKnuckleBootTipRightDL, 37 },
    { gIronKnuckleBootTipRightDL, 52 },
    { gIronKnuckleBootTipRightDL, 68 },

    { gIronKnuckleWaistArmorRightDL, 23 },
    { gIronKnuckleWaistArmorRightDL, 42 },
    { gIronKnuckleWaistArmorRightDL, 106 },
};

static DListPatchInfo arrowTipDListPatchInfos[] = {
    { gameplay_keep_DL_013FF0, 46 },
    { gameplay_keep_DL_014370, 5 },
};

void PatchArrowTipTexture() {
    // Custom texture for Arrow tips that accounts for overflow texture reading
    Gfx arrowTipTextureWithOverflowFixGfx =
        gsDPSetTextureImage(G_IM_FMT_RGBA, G_IM_SIZ_16b_LOAD_BLOCK, 1, gameplay_keep_Tex_00CA30_Overflow);

    // Gfx instructions to fix authentic vanilla bug where the Arrow tips texture is read as the wrong size
    Gfx arrowTipTextureWithSizeFixGfx[] = {
        gsDPLoadTextureBlock(gameplay_keep_Tex_00CA30, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
                             G_TX_MIRROR | G_TX_WRAP, 5, 5, 1, 1),
    };

    bool fixTexturesOOB = CVarGetInteger("gEnhancements.Fixes.FixTexturesOOB", 0);

    for (const auto& patchInfo : arrowTipDListPatchInfos) {
        const char* dlist = patchInfo.dlist;
        int start = patchInfo.startInstruction;

        // Patch using custom overflowed texture
        if (!fixTexturesOOB) {
            // Unpatch the other texture fix
            for (size_t i = 4; i < 8; i++) {
                int instruction = start + i;
                std::string unpatchName = "arrowTipTextureWithSizeFix_" + std::to_string(instruction);
                ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            }

            std::string patchName = "arrowTipTextureWithOverflowFix_" + std::to_string(start);
            std::string patchName2 = "arrowTipTextureWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), start, arrowTipTextureWithOverflowFixGfx);
            ResourceMgr_PatchGfxByName(dlist, patchName2.c_str(), start + 1, gsSPNoOp());
        } else { // Patch texture to use correct image size/fmt
            // Unpatch the other texture fix
            std::string unpatchName = "arrowTipTextureWithOverflowFix_" + std::to_string(start);
            std::string unpatchName2 = "arrowTipTextureWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName2.c_str());

            for (size_t i = 4; i < 8; i++) {
                int instruction = start + i;
                std::string patchName = "arrowTipTextureWithSizeFix_" + std::to_string(instruction);

                if (i == 0) {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction, gsSPNoOp());
                } else {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction,
                                               arrowTipTextureWithSizeFixGfx[i - 1]);
                }
            }
        }
    }
}

void PatchFreezardBodyTexture() {
    // Custom texture for Freezard effect that accounts for overflow texture reading
    Gfx freezardBodyTextureWithOverflowFixGfx =
        gsDPSetTextureImage(G_IM_FMT_IA, G_IM_SIZ_16b_LOAD_BLOCK, 1, gEffIceFragmentTex_Overflow);

    // Gfx instructions to fix authentic vanilla bug where the Freezard effect texture is read as the wrong format
    Gfx freezardBodyTextureWithFormatFixGfx[] = {
        gsDPLoadTextureBlock(gEffIceFragmentTex, G_IM_FMT_I, G_IM_SIZ_8b, 32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                             G_TX_NOMIRROR | G_TX_WRAP, 5, 5, G_TX_NOLOD, G_TX_NOLOD),
    };

    bool fixTexturesOOB = CVarGetInteger("gEnhancements.Fixes.FixTexturesOOB", 0);

    for (const auto& patchInfo : freezardBodyDListPatchInfos) {
        const char* dlist = patchInfo.dlist;
        int start = patchInfo.startInstruction;

        // Patch using custom overflowed texture
        if (!fixTexturesOOB) {
            // Unpatch the other texture fix
            for (size_t i = 0; i < 8; i++) {
                int instruction = start + i;
                std::string unpatchName = "freezardBodyTextureWithFormatFix_" + std::to_string(instruction);
                ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            }

            std::string patchName = "freezardBodyTextureWithOverflowFix_" + std::to_string(start);
            std::string patchName2 = "freezardBodyTextureWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), start, freezardBodyTextureWithOverflowFixGfx);
            ResourceMgr_PatchGfxByName(dlist, patchName2.c_str(), start + 1, gsSPNoOp());
        } else { // Patch texture to use correct image size/fmt
            // Unpatch the other texture fix
            std::string unpatchName = "freezardBodyTextureWithOverflowFix_" + std::to_string(start);
            std::string unpatchName2 = "freezardBodyTextureWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName2.c_str());

            for (size_t i = 0; i < 8; i++) {
                int instruction = start + i;
                std::string patchName = "freezardBodyTextureWithFormatFix_" + std::to_string(instruction);

                if (i == 0) {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction, gsSPNoOp());
                } else {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction,
                                               freezardBodyTextureWithFormatFixGfx[i - 1]);
                }
            }
        }
    }
}

void PatchIronKnuckleFireTexture() {
    // Custom texture for Iron Knuckle texture that accounts for overflow texture reading
    Gfx ironKnuckleFireTexWithOverflowFixGfx =
        gsDPSetTextureImage(G_IM_FMT_I, G_IM_SIZ_8b_LOAD_BLOCK, 1, gIronKnuckleFireTex_Overflow);

    // Gfx instructions to fix authentic vanilla bug where the Iron Knuckle texture is read as the wrong size
    Gfx ironKnuckleFireTexWithFormatFixGfx[] = {
        gsDPLoadTextureBlock_4b(gIronKnuckleFireTex, G_IM_FMT_I, 32, 64, 0, G_TX_MIRROR | G_TX_WRAP,
                                G_TX_MIRROR | G_TX_WRAP, 5, 6, G_TX_NOLOD, G_TX_NOLOD),
    };

    bool fixTexturesOOB = CVarGetInteger("gEnhancements.Fixes.FixTexturesOOB", 0);

    for (const auto& patchInfo : ironKnuckleDListPatchInfos) {
        const char* dlist = patchInfo.dlist;
        int start = patchInfo.startInstruction;

        // Patch using custom overflowed texture
        if (!fixTexturesOOB) {
            // Unpatch the other texture fix
            for (size_t i = 0; i < 8; i++) {
                int instruction = start + i;
                std::string unpatchName = "ironKnuckleFireTexWithSizeFix_" + std::to_string(instruction);
                ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            }

            std::string patchName = "ironKnuckleFireTexWithOverflowFix_" + std::to_string(start);
            std::string patchName2 = "ironKnuckleFireTexWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), start, ironKnuckleFireTexWithOverflowFixGfx);
            ResourceMgr_PatchGfxByName(dlist, patchName2.c_str(), start + 1, gsSPNoOp());
        } else { // Patch texture to use correct image size/fmt
            // Unpatch the other texture fix
            std::string unpatchName = "ironKnuckleFireTexWithOverflowFix_" + std::to_string(start);
            std::string unpatchName2 = "ironKnuckleFireTexWithOverflowFix_" + std::to_string(start + 1);
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName.c_str());
            ResourceMgr_UnpatchGfxByName(dlist, unpatchName2.c_str());

            for (size_t i = 0; i < 8; i++) {
                int instruction = start + i;
                std::string patchName = "ironKnuckleFireTexWithSizeFix_" + std::to_string(instruction);

                if (i == 0) {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction, gsSPNoOp());
                } else {
                    ResourceMgr_PatchGfxByName(dlist, patchName.c_str(), instruction,
                                               ironKnuckleFireTexWithFormatFixGfx[i - 1]);
                }
            }
        }
    }
}

void GfxPatcher_ApplyOverflowTexturePatches() {
    PatchArrowTipTexture();
    PatchFreezardBodyTexture();
    PatchIronKnuckleFireTexture();
}

void PatchMiniGameCrossAndCircleSymbols() {
    // The X and O displayed in mini-games are incorrectly set to FMT_I instead of FMT_IA,
    // Fast3D throws an assert and does nothing as FMT_I with SIZ_16 is not a valid texture type.
    // Patching all the relevant instructions to use FMT_IA matching the exported texture
    const char* dLists[] = { gShootingGalleryOctorokCrossDL, gShootingGalleryOctorokCircleDL, gObjJgameLightIncorrectDL,
                             gObjJgameLightCorrectDL };

    for (auto& dl : dLists) {
        Gfx* instructions = ResourceMgr_LoadGfxByName(dl);

        // Use a mask to unset and set the new format while preserving the rest of the instruction
        const uintptr_t mask = UINTPTR_MAX ^ (_SHIFTL(7, 21, 3));

        Gfx in3 = instructions[3]; // G_SETTIMG_OTR_HASH
        in3.words.w0 = (in3.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);
        Gfx in5 = instructions[5]; // G_SETTILE
        in5.words.w0 = (in5.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);
        Gfx in9 = instructions[9]; // G_SETTILE
        in9.words.w0 = (in9.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);

        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_3", 3, in3);
        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_5", 5, in5);
        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_9", 9, in9);
    }
}

void PatchKnifeChamberRoomGeometry() {
    if (CVarGetInteger("gEnhancements.Graphics.DisableSceneGeometryDistanceCheck", 0)) {
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_YADOYA/Z2_YADOYA_room_00DL_012280", "disableDistanceCheck1", 5,
                                   gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_YADOYA/Z2_YADOYA_room_00DL_012280", "disableDistanceCheck2", 6,
                                   gsSPNoOp());
    } else {
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_YADOYA/Z2_YADOYA_room_00DL_012280", "disableDistanceCheck1");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_YADOYA/Z2_YADOYA_room_00DL_012280", "disableDistanceCheck2");
    }
}

void PatchClockTownBuildingGeometry() {
    if (CVarGetInteger("gEnhancements.Graphics.DisableSceneGeometryDistanceCheck", 0)) {
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D490", "disableDistanceCheck1",
                                   5, gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D490", "disableDistanceCheck2",
                                   6, gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00CD70", "disableDistanceCheck1",
                                   5, gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00CD70", "disableDistanceCheck2",
                                   6, gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D9C8", "disableDistanceCheck1",
                                   5, gsSPNoOp());
        ResourceMgr_PatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D9C8", "disableDistanceCheck2",
                                   6, gsSPNoOp());
    } else {
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D490",
                                     "disableDistanceCheck1");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D490",
                                     "disableDistanceCheck2");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00CD70",
                                     "disableDistanceCheck1");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00CD70",
                                     "disableDistanceCheck2");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D9C8",
                                     "disableDistanceCheck1");
        ResourceMgr_UnpatchGfxByName("scenes/nonmq/Z2_00KEIKOKU/Z2_00KEIKOKU_room_00DL_00D9C8",
                                     "disableDistanceCheck2");
    }
}

void GfxPatcher_ApplyGeometryIssuePatches() {
    PatchKnifeChamberRoomGeometry();
    PatchClockTownBuildingGeometry();
}

// Applies required patches for authentic bugs to allow the game to play and render properly
void GfxPatcher_ApplyNecessaryAuthenticPatches() {
    PatchMiniGameCrossAndCircleSymbols();

    GfxPatcher_ApplyOverflowTexturePatches();

    GfxPatcher_ApplyGeometryIssuePatches();
}
