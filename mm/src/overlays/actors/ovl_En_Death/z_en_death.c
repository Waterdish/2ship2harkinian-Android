/*
 * File: z_en_death.c
 * Overlay: ovl_En_Death
 * Description: Gomess
 */

#include "z_en_death.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_Arrow_Light/z_arrow_light.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_20 | ACTOR_FLAG_IGNORE_QUAKE)

#define THIS ((EnDeath*)thisx)

void EnDeath_Init(Actor* thisx, PlayState* play);
void EnDeath_Destroy(Actor* thisx, PlayState* play);
void EnDeath_Update(Actor* thisx, PlayState* play);
void EnDeath_Draw(Actor* thisx, PlayState* play);
void EnDeath_IntroCutscenePart1(EnDeath* this, PlayState* play);
void EnDeath_IntroCutscenePart2(EnDeath* this, PlayState* play);
void EnDeath_IntroCutscenePart3(EnDeath* this, PlayState* play);
void EnDeath_IntroCutscenePart4(EnDeath* this, PlayState* play);
void EnDeath_IntroCutscenePart5(EnDeath* this, PlayState* play);
void EnDeath_ApproachPlayer(EnDeath* this, PlayState* play);
void EnDeath_SwingAttack(EnDeath* this, PlayState* play);
void EnDeath_EndSwingAttack(EnDeath* this, PlayState* play);
void EnDeath_BlockProjectile(EnDeath* this, PlayState* play);
void EnDeath_SpinAttack(EnDeath* this, PlayState* play);
void EnDeath_Damaged(EnDeath* this, PlayState* play);
void EnDeath_DeathCutscenePart1(EnDeath* this, PlayState* play);
void EnDeath_DeathCutscenePart2(EnDeath* this, PlayState* play);
void EnDeath_DeathCutscenePart3(EnDeath* this, PlayState* play);
void EnDeath_DeathCutscenePart4(EnDeath* this, PlayState* play);
void EnDeath_Dead(EnDeath* this, PlayState* play);
void func_808C7B88(EnDeath* this, PlayState* play);
void func_808C7C88(EnDeath* this, PlayState* play);
void EnDeath_PlayCutscene(EnDeath* this, PlayState* play);
void EnDeath_BeginWithoutCutscene(EnDeath* this, PlayState* play);

void EnDeath_SetupIntroCutscenePart2(EnDeath* this, PlayState* play);
void EnDeath_SetupIntroCutscenePart3(EnDeath* this);
void EnDeath_SetupIntroCutscenePart4(EnDeath* this, PlayState* play);
void EnDeath_SetupIntroCutscenePart5(EnDeath* this, PlayState* play);
void EnDeath_SetupApproachPlayer(EnDeath* this);
void EnDeath_SetupSwingAttack(EnDeath* this);
void EnDeath_SetupEndSwingAttack(EnDeath* this);
void EnDeath_SetupBlockProjectile(EnDeath* this);
void EnDeath_SetupSpinAttack(EnDeath* this);
void EnDeath_SetupDeathCutscenePart2(EnDeath* this, PlayState* play);
void EnDeath_SetupDeathCutscenePart3(EnDeath* this);
void EnDeath_SetupDead(EnDeath* this, PlayState* play);
void func_808C7AEC(EnDeath* this);
void func_808C7C04(EnDeath* this);
void EnDeath_SetupPlayCutscene(EnDeath* this);
void EnDeath_SetupBeginWithoutCutscene(EnDeath* this);
void EnDeath_SetupDeathCutscenePart4(EnDeath* this);

const ActorInit En_Death_InitVars = {
    ACTOR_EN_DEATH,
    ACTORCAT_ENEMY,
    FLAGS,
    OBJECT_DEATH,
    sizeof(EnDeath),
    (ActorFunc)EnDeath_Init,
    (ActorFunc)EnDeath_Destroy,
    (ActorFunc)EnDeath_Update,
    (ActorFunc)EnDeath_Draw,
};

static ColliderSphereInit sSphereInit = {
    {
        COLTYPE_HIT3,
        AT_NONE,
        AC_NONE | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_TYPE_1,
        COLSHAPE_SPHERE,
    },
    {
        ELEMTYPE_UNK0,
        { 0xF7CFFFFF, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_NONE,
    },
    { 1, { { 0, 0, 0 }, 22 }, 100 },
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_NONE,
        OCELEM_ON,
    },
    { 35, 90, 20, { 0, 0, 0 } },
};

static ColliderTrisElementInit sTrisElementsInit[2] = {
    {
        {
            ELEMTYPE_UNK2,
            { 0xF7CFFFFF, 0x04, 0x20 },
            { 0xF7CFFFFF, 0x00, 0x00 },
            TOUCH_ON | TOUCH_SFX_NORMAL,
            BUMP_ON,
            OCELEM_NONE,
        },
        { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    },
    {
        {
            ELEMTYPE_UNK2,
            { 0xF7CFFFFF, 0x04, 0x20 },
            { 0xF7CFFFFF, 0x00, 0x00 },
            TOUCH_ON | TOUCH_SFX_NORMAL,
            BUMP_ON,
            OCELEM_NONE,
        },
        { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    },
};

static ColliderTrisInit sTrisInit = {
    {
        COLTYPE_METAL,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_HARD | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_TYPE_1,
        COLSHAPE_TRIS,
    },
    2,
    sTrisElementsInit, // sTrisElementsInit,
};

static ColliderQuadInit sQuadInit = {
    {
        COLTYPE_NONE,
        AT_NONE | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_2,
        COLSHAPE_QUAD,
    },
    {
        ELEMTYPE_UNK0,
        { 0xF7CFFFFF, 0x04, 0x20 },
        { 0x00000000, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL | TOUCH_UNK7,
        BUMP_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

typedef enum {
    DMGEFF_NONE = 0,
    DMGEFF_FIRE_ARROW = 2,
    DMGEFF_ICE_ARROW = 3,
    DMGEFF_LIGHT_ARROW = 4,
    DMGEFF_EXPLOSIVES = 15
} EnDeathDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Deku Stick     */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, DMGEFF_EXPLOSIVES),
    /* Zora boomerang */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Normal arrow   */ DMG_ENTRY(1, DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Goron punch    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(1, DMGEFF_FIRE_ARROW),
    /* Ice arrow      */ DMG_ENTRY(1, DMGEFF_ICE_ARROW),
    /* Light arrow    */ DMG_ENTRY(2, DMGEFF_LIGHT_ARROW),
    /* Goron spikes   */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Zora barrier   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Normal shield  */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, DMGEFF_EXPLOSIVES),
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 20, 28, 90, 20, 100 };

static EffectBlureInit2 D_808C9A3C = {
    0, 8, 0, { 100, 50, 100, 200 }, { 100, 0, 0, 64 }, { 100, 0, 0, 20 }, { 50, 0, 0, 0 }, 8,
    0, 2, 0, { 0, 0, 0, 0 },        { 0, 0, 0, 0 },
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F(scale, 0, ICHAIN_CONTINUE),
    ICHAIN_S8(hintId, TATL_HINT_ID_GOMESS, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 6000, ICHAIN_CONTINUE),
    ICHAIN_U8(targetMode, TARGET_MODE_5, ICHAIN_STOP),
};

void EnDeath_Init(Actor* thisx, PlayState* play) {
    EnDeath* this = THIS;
    PlayState* play2 = play;
    f32 yOffset = 15.0f;
    s16 yRot = 0;
    s32 i;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 5500.0f, ActorShadow_DrawCircle, 80.0f);

    SkelAnime_InitFlex(play2, &this->skelAnime, &gGomessSkel, &gGomessFloatAnim, this->jointTable, this->morphTable,
                       GOMESS_LIMB_MAX);

    Collider_InitAndSetSphere(play2, &this->coreCollider, &this->actor, &sSphereInit);
    Collider_InitAndSetCylinder(play2, &this->bodyCollider, &this->actor, &sCylinderInit);
    Collider_InitAndSetQuad(play2, &this->unk_788, &this->actor, &sQuadInit);
    Collider_InitAndSetTris(play2, &this->unk_860, &this->actor, &sTrisInit, this->unk_880);

    this->coreCollider.dim.worldSphere.radius = this->coreCollider.dim.modelSphere.radius;

    CollisionCheck_SetInfo2(&this->actor.colChkInfo, &sDamageTable, &sColChkInfoInit);

    Effect_Add(play2, &this->effectIndex, EFFECT_BLURE2, 0, 0, &D_808C9A3C);

    if (!(gSaveContext.eventInf[6] & 8)) {
        this->actor.world.pos.y += 400.0f;
    }

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i] = (EnMinideath*)Actor_SpawnAsChild(
            &play2->actorCtx, &this->actor, play2, ACTOR_EN_MINIDEATH, this->actor.world.pos.x,
            this->actor.world.pos.y + yOffset, this->actor.world.pos.z, 0, yRot, 0, i);
        if (this->miniDeaths[i] == NULL) {
            Actor_Kill(&this->actor);
        }
        yRot += 0x3A00;
        yOffset += 4.0f;
    }

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths) - 1; i++) {
        this->miniDeaths[i]->actor.child = &this->miniDeaths[i + 1]->actor;
    }

    this->bodyMatAnim = Lib_SegmentedToVirtual(&gGomessBodyMatAnim);
    this->coreMatAnim = Lib_SegmentedToVirtual(&gGomessCoreMatAnim);

    if (gSaveContext.eventInf[6] & 8) {
        // Watched intro cutscene
        this->unk_18E = true;
        Actor_SetScale(&this->actor, 0.01f);
        this->unk_191 = 0x17;
        this->actor.params = 20;
        this->scytheScale = 1.0f;
        EnDeath_SetupBeginWithoutCutscene(this);
    } else {
        // Start intro cutscene
        this->unk_192 = true;
        this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
        EnDeath_SetupPlayCutscene(this);
    }
}

void EnDeath_Destroy(Actor* thisx, PlayState* play) {
    EnDeath* this = THIS;

    Collider_DestroySphere(play, &this->coreCollider);
    Collider_DestroyCylinder(play, &this->bodyCollider);
    Collider_DestroyQuad(play, &this->unk_788);
    Collider_DestroyTris(play, &this->unk_860);
    AudioSfx_StopByPos(&this->scytheScreenPos);
    Effect_Destroy(play, this->effectIndex);
}

void EnDeath_DimLights(PlayState* play) {
    EnvLightSettings* lightSettings;
    s32 i;

    play->envCtx.lightSettingOverride = 20;
    play->envCtx.lightSetting = 20;
    play->envCtx.prevLightSetting = 20;
    play->envCtx.lightBlend = 1.0f;
    lightSettings = &play->envCtx.lightSettingsList[20];

    for (i = 0; i != 3; i++) {
        lightSettings->light1Dir[i] = play->envCtx.lightSettings.light1Dir[i];
        lightSettings->light2Dir[i] = play->envCtx.lightSettings.light2Dir[i];
    }
}

void EnDeath_Float(EnDeath* this) {
    Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_SHARP_FLOAT - SFX_FLAG);

    if (this->floatTimer == 0) {
        this->floatTimer = 40;
    }
    this->floatTimer--;
    this->actor.world.pos.y = this->actor.home.pos.y + (1.0f - Math_CosS(this->floatTimer * 0x666)) * 7.5f;
}

s32 EnDeath_ProjectileApproaching(EnDeath* this, PlayState* play) {
    Actor* projectileActor =
        func_800BC270(play, &this->actor, 80.0f, 0x10000 | 0x2000 | 0x1000 | 0x800 | 0x80 | 0x20 | 0x10);
    s16 ret;

    if (projectileActor != NULL &&
        (ret = Actor_WorldYawTowardActor(&this->actor, projectileActor) - this->actor.shape.rot.y,
         ABS_ALT(ret) < 0x2000) &&
        (ret = Actor_WorldPitchTowardPoint(projectileActor, &this->actor.focus.pos) - projectileActor->world.rot.x,
         ABS_ALT(ret) < 0x3000)) {
        return true;
    }
    return false;
}

void EnDeath_UpdateSpinAttackTris(EnDeath* this) {
    f32 sinRotY = Math_SinS(this->actor.shape.rot.y);
    f32 cosRotY = Math_CosS(this->actor.shape.rot.y);
    Vec3f sp54;
    Vec3f sp48;
    Vec3f sp3C;

    sp3C.x = this->actor.world.pos.x - 70.0f * cosRotY + 75.0f * sinRotY;
    sp3C.y = this->actor.world.pos.y + -10.0f;
    sp3C.z = this->actor.world.pos.z + 70.0f * sinRotY + 75.0f * cosRotY;

    sp48.x = this->actor.world.pos.x + 70.0f * cosRotY + 65.0f * sinRotY;
    sp48.y = sp3C.y + 150.0f;
    sp48.z = this->actor.world.pos.z - 70.0f * sinRotY + 65.0f * cosRotY;

    sp54.x = sp3C.x;
    sp54.x += -10.0f * sinRotY;
    sp54.y = sp48.y;
    sp54.z = sp3C.z;
    sp54.z += -10.0f * cosRotY;
    Collider_SetTrisVertices(&this->unk_860, 0, &sp3C, &sp48, &sp54);

    sp54.x = sp48.x;
    sp54.x += 10.0f * sinRotY;
    sp54.y = sp3C.y;
    sp54.z = sp48.z;
    sp54.z += 10.0f * cosRotY;
    Collider_SetTrisVertices(&this->unk_860, 1, &sp3C, &sp54, &sp48);
}

f32 EnDeath_UpdateCoreVelocityAndRotation(EnDeath* this) {
    f32 tmp;

    this->coreVelocity = this->actor.world.pos.y - this->actor.home.pos.y;
    this->coreVelocity = CLAMP_MIN(this->coreVelocity, 0.0f);
    this->coreVelocity *= 0.02f;
    this->coreVelocity = SQ(this->coreVelocity) * 3.0f + 80.0f;

    tmp = 22.5f / (this->coreVelocity * (2 * M_PI));
    this->coreRotation += (s16)(65536.0f * tmp);
}

void EnDeath_SetupIntroCutscenePart1(EnDeath* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f eye;
    Vec3f at;

    this->actor.shape.rot.y = this->actor.shape.rot.y - 0x657A;
    this->actionTimer = 60;

    eye.x = (Math_SinS(this->actor.home.rot.y + 0x98) * 542.0f) + this->actor.world.pos.x;
    eye.z = (Math_CosS(this->actor.home.rot.y + 0x98) * 542.0f) + this->actor.world.pos.z;
    eye.y = this->actor.home.pos.y + 3.0f;

    at.x = this->actor.world.pos.x;
    at.z = this->actor.world.pos.z;
    at.y = this->actor.world.pos.y - 116.0f;

    player->actor.world.pos.x = Math_SinS(this->actor.home.rot.y - 0x370) * 463.0f + this->actor.world.pos.x;
    player->actor.world.pos.z = Math_CosS(this->actor.home.rot.y - 0x370) * 463.0f + this->actor.world.pos.z;
    player->actor.shape.rot.y = Actor_WorldYawTowardActor(&player->actor, &this->actor);

    Play_SetCameraAtEye(play, this->camId, &at, &eye);
    Play_SetCameraFov(play, this->camId, 77.0f);
    Player_SetCsAction(play, &this->actor, 0x15);

    this->actionFunc = EnDeath_IntroCutscenePart1;
}

void EnDeath_IntroCutscenePart1(EnDeath* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    player->actor.world.pos.x = Math_SinS(this->actor.home.rot.y - 0x370) * 463.0f + this->actor.world.pos.x;
    player->actor.world.pos.z = Math_CosS(this->actor.home.rot.y - 0x370) * 463.0f + this->actor.world.pos.z;
    player->actor.shape.rot.y = Actor_WorldYawTowardActor(&player->actor, &this->actor);

    if (this->actionTimer == 25) {
        Player_SetCsAction(play, &this->actor, 0x51);
    }
    if (this->actionTimer > 0) {
        this->actionTimer--;
    } else {
        EnDeath_SetupIntroCutscenePart2(this, play);
    }
    EnDeath_UpdateCoreVelocityAndRotation(this);
}

void EnDeath_SetupIntroCutscenePart2(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);
    s32 i;

    EnDeath_DimLights(play);

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_7;
    }

    this->actionTimer = 50;
    this->camEyeTarget.x = Math_SinS(this->actor.home.rot.y) * 140.0f + this->actor.world.pos.x;
    this->camEyeTarget.z = Math_CosS(this->actor.home.rot.y) * 140.0f + this->actor.world.pos.z;
    this->camEyeTarget.y = this->actor.home.pos.y + 20.0f;
    this->camAtTarget.x = this->actor.world.pos.x;
    this->camAtTarget.z = this->actor.world.pos.z;
    this->camAtTarget.y = this->actor.home.pos.y + 50.0f;
    this->camEyeSpeed = Math_Vec3f_DistXYZ(&camera->eye, &this->camEyeTarget) * 0.0225f;
    this->camAtSpeed = Math_Vec3f_DistXYZ(&camera->at, &this->camAtTarget) * 0.0225f;
    this->actionFunc = EnDeath_IntroCutscenePart2;
}

void EnDeath_IntroCutscenePart2(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);

    if (this->actionTimer == 42) {
        Player_SetCsAction(play, &this->actor, 4);
    } else if (this->actionTimer == 27) {
        Player_SetCsAction(play, &this->actor, 0x7B);
    }
    if (this->actionTimer <= 0) {
        if (this->actor.world.pos.y < (this->actor.home.pos.y + 400.0f) - 225.0f) {
            play->envCtx.lightSettingOverride = 26;
        }
        Math_Vec3f_StepTo(&camera->eye, &this->camEyeTarget, this->camEyeSpeed);
        Math_Vec3f_StepTo(&camera->at, &this->camAtTarget, this->camAtSpeed);
        Play_SetCameraAtEye(play, this->camId, &camera->at, &camera->eye);
        if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y, 9.0f) != 0) {
            this->actionTimer--;
            if (this->actionTimer == -20) {
                EnDeath_SetupIntroCutscenePart3(this);
            }
        }
    } else {
        this->actionTimer--;
    }
    EnDeath_UpdateCoreVelocityAndRotation(this);
}

void EnDeath_SetupIntroCutscenePart3(EnDeath* this) {
    gSaveContext.eventInf[6] |= 8;
    Animation_Change(&this->skelAnime, &object_death_Anim_00B284, 0.0f, 0.0f,
                     Animation_GetLastFrame(&object_death_Anim_00B284), ANIMMODE_ONCE, 0.0f);
    this->actor.scale.y = 0.01f;
    this->actor.shape.rot.y += 0x777F;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_APPEAR);
    this->actionFunc = EnDeath_IntroCutscenePart3;
}

void EnDeath_IntroCutscenePart3(EnDeath* this, PlayState* play) {
    s32 stepDone = Math_StepToF(&this->actor.scale.x, 0.01f, 0.0005f);
    f32 temp;

    this->actor.scale.z = this->actor.scale.x;
    temp = 0.01f - this->actor.scale.x;
    if (temp > 0.0025f) {
        temp = 0.0025f;
    }
    this->actor.shape.rot.y += (s16)(temp * 4505600.0f);
    if (stepDone) {
        this->unk_192 = false;
        EnDeath_SetupIntroCutscenePart4(this, play);
    }
    EnDeath_UpdateCoreVelocityAndRotation(this);
}

void EnDeath_SetupIntroCutscenePart4(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);
    f32 invAnimDuration = 1.0f / Animation_GetLastFrame(&object_death_Anim_00B284);

    this->actionTimer = 10;
    this->skelAnime.playSpeed = 1.0f;
    this->actor.shape.rot.y = this->actor.home.rot.y;
    this->camEyeTarget.x = Math_SinS(this->actor.home.rot.y) * 50.0f + this->actor.world.pos.x;
    this->camEyeTarget.z = Math_CosS(this->actor.home.rot.y) * 50.0f + this->actor.world.pos.z;
    this->camEyeTarget.y = this->actor.home.pos.y + 95.0f;
    this->camAtTarget.x = this->actor.world.pos.x;
    this->camAtTarget.z = this->actor.world.pos.z;
    this->camAtTarget.y = this->actor.world.pos.y + 100.0f;
    this->camEyeSpeed = Math_Vec3f_DistXYZ(&camera->eye, &this->camEyeTarget) * invAnimDuration;
    this->camAtSpeed = Math_Vec3f_DistXYZ(&camera->at, &this->camAtTarget) * invAnimDuration;

    this->actionFunc = EnDeath_IntroCutscenePart4;
}

void EnDeath_IntroCutscenePart4(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);

    if (this->unk_191 < 23) {
        this->unk_191++;
    }
    Math_Vec3f_StepTo(&camera->eye, &this->camEyeTarget, this->camEyeSpeed);
    Math_Vec3f_StepTo(&camera->at, &this->camAtTarget, this->camAtSpeed);
    Play_SetCameraAtEye(play, this->camId, &camera->at, &camera->eye);
    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->actionTimer > 0) {
            this->actionTimer--;
        } else {
            EnDeath_SetupIntroCutscenePart5(this, play);
        }
    }
    EnDeath_UpdateCoreVelocityAndRotation(this);
}

void EnDeath_SetupIntroCutscenePart5(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);

    Animation_PlayOnce(&this->skelAnime, &object_death_Anim_00CB2C);
    this->unk_191 = 0x17;
    Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
    this->camEyeTarget.x = Math_SinS(this->actor.home.rot.y) * 230.0f + this->actor.world.pos.x;
    this->camEyeTarget.z = Math_CosS(this->actor.home.rot.y) * 230.0f + this->actor.world.pos.z;
    this->camEyeTarget.y = this->actor.home.pos.y + 40.0f;
    this->camAtTarget.y = this->actor.world.pos.y + 85.0f;
    this->camEyeSpeed = Math_Vec3f_DistXYZ(&camera->eye, &this->camEyeTarget) * 0.14285715f;
    this->camAtSpeed = Math_Vec3f_DistXYZ(&camera->at, &this->camAtTarget) * 0.14285715f;
    Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_VOICE);
    this->actionFunc = EnDeath_IntroCutscenePart5;
}

void EnDeath_IntroCutscenePart5(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);
    f32 scale;

    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        play->envCtx.lightSettingOverride = 27;
    } else if (Animation_OnFrame(&this->skelAnime, 25.0f)) {
        play->envCtx.lightSettingOverride = 26;
    } else if (Animation_OnFrame(&this->skelAnime, 38.0f)) {
        s32 i;

        for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
            this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_9;
        }
    } else if (Animation_OnFrame(&this->skelAnime, 14.0f)) {
        s32 i;

        for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
            this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_8;
            this->miniDeaths[i]->actor.world.pos.x =
                (this->miniDeaths[i]->actor.world.pos.x - this->actor.world.pos.x) * 1.8f + this->actor.world.pos.x;
            this->miniDeaths[i]->actor.world.pos.z =
                (this->miniDeaths[i]->actor.world.pos.z - this->actor.world.pos.z) * 1.8f + this->actor.world.pos.z;
            this->miniDeaths[i]->actor.world.pos.y =
                (this->miniDeaths[i]->actor.world.pos.y - this->actor.world.pos.y) * 1.7f + this->actor.world.pos.y;
        }
        this->unk_18E = true;
        this->scytheScale = 1.0f;
    }
    Math_Vec3f_StepTo(&camera->eye, &this->camEyeTarget, this->camEyeSpeed);
    Math_Vec3f_StepTo(&camera->at, &this->camAtTarget, this->camAtSpeed);
    Math_StepToF(&camera->fov, 60.0f, 2.4285715f);
    Play_SetCameraFov(play, this->camId, camera->fov);
    Play_SetCameraAtEye(play, this->camId, &camera->at, &camera->eye);

    scale = CLAMP(this->skelAnime.curFrame, 10.0f, 14.0f) - 10.0f;
    this->scytheScale = scale * 0.25f;
    if (scale < 14.0f) {
        EnDeath_UpdateCoreVelocityAndRotation(this);
    }
    if (SkelAnime_Update(&this->skelAnime)) {
        CutsceneManager_Stop(this->actor.csId);
        Player_SetCsAction(play, &this->actor, 6);
        this->actor.flags |= ACTOR_FLAG_TARGETABLE;
        this->coreCollider.base.acFlags |= AC_ON;
        EnDeath_SetupApproachPlayer(this);
    }
}

void EnDeath_SetupApproachPlayer(EnDeath* this) {
    Animation_MorphToLoop(&this->skelAnime, &gGomessFloatAnim, 10.0f);

    this->actor.speed = 1.5f;
    if (this->actionFunc == EnDeath_EndSwingAttack || this->actionFunc == EnDeath_SpinAttack) {
        this->actionTimer = 140;
    } else {
        this->actionTimer = 100;
    }
    this->actionFunc = EnDeath_ApproachPlayer;
}

void EnDeath_ApproachPlayer(EnDeath* this, PlayState* play) {
    if (play->envCtx.lightSettingOverride == 20) {
        play->envCtx.lightSettingOverride = 26;
    }
    EnDeath_Float(this);
    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);

    if (this->actionTimer > 0) {
        this->actionTimer--;
    }
    if (this->actor.params < 5 && Rand_ZeroOne() < 0.4f && EnDeath_ProjectileApproaching(this, play)) {
        EnDeath_SetupBlockProjectile(this);
    } else if (this->actionTimer < 100 && this->actor.xzDistToPlayer < 200.0f) {
        EnDeath_SetupSwingAttack(this);
    } else if (this->actionTimer == 0) {
        if (this->actor.params >= 5) {
            func_808C7AEC(this);
        } else {
            EnDeath_SetupSpinAttack(this);
        }
    }
}

void EnDeath_SetupSwingAttack(EnDeath* this) {
    Animation_Change(&this->skelAnime, &gGomessScytheSwingAnim, 1.0f, 0.0f, 10.0f, ANIMMODE_ONCE, -3.0f);
    this->actionTimer = 0;
    this->floatTimer = 0;
    this->actor.speed = 8.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_ATTACK);
    this->actionFunc = EnDeath_SwingAttack;
}

void EnDeath_SwingAttack(EnDeath* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 4, 0x1000, 0x200);
    Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y, 3.0f);
    this->actor.shape.rot.x = (s16)((100.0f - this->actor.xzDistToPlayer) * 0.01f * 0x400);
    this->actor.shape.rot.x = CLAMP_MIN(this->actor.shape.rot.x, 0);

    if (this->actionTimer > 0) {
        this->actionTimer++;
        if (this->actionTimer == 3) {
            EnDeath_SetupEndSwingAttack(this);
        }
    } else if (SkelAnime_Update(&this->skelAnime) && this->actor.xzDistToPlayer < 100.0f) {
        Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_SCYTHE);
        this->actionTimer++;
        this->actor.speed = 0.0f;
    }
}

void EnDeath_SetupEndSwingAttack(EnDeath* this) {
    this->skelAnime.endFrame = Animation_GetLastFrame(&gGomessScytheSwingAnim);
    this->actionTimer = 0;
    this->unk_2EC = -1;
    this->unk_18C = true;
    this->actionFunc = EnDeath_EndSwingAttack;
    this->actor.speed = 0.0f;
}

void EnDeath_EndSwingAttack(EnDeath* this, PlayState* play) {
    this->actionTimer++;
    if (this->skelAnime.curFrame > 20.0f) {
        this->unk_18C = false;
        this->unk_788.base.atFlags &= ~AT_ON;
        Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x200);
    }
    if (SkelAnime_Update(&this->skelAnime)) {
        this->actor.shape.rot.x = 0;
        EnDeath_SetupApproachPlayer(this);
    }
}

void EnDeath_SetupBlockProjectile(EnDeath* this) {
    Animation_MorphToLoop(&this->skelAnime, &gGomessScytheSpinAnim, -3.0f);
    this->unk_2EC = 31;
    this->actionTimer = 30;
    this->actor.speed = 0.0f;
    EnDeath_UpdateSpinAttackTris(this);
    this->unk_18C = true;
    this->numScytheAfterImages = -3;
    this->actionFunc = EnDeath_BlockProjectile;
}

void EnDeath_BlockProjectile(EnDeath* this, PlayState* play) {
    EnDeath_Float(this);

    this->actionTimer--;
    if (this->numScytheAfterImages < 7) {
        this->numScytheAfterImages++;
    }
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_ROLL);
    }
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    EnDeath_UpdateSpinAttackTris(this);
    if (this->actionTimer == 0) {
        if (this->actor.xzDistToPlayer > 200.0f) {
            EnDeath_SetupSpinAttack(this);
        } else {
            EnDeath_SetupApproachPlayer(this);
        }
    }
}

void EnDeath_SetupSpinAttack(EnDeath* this) {
    if (this->actionFunc != EnDeath_BlockProjectile) {
        Animation_MorphToLoop(&this->skelAnime, &gGomessScytheSpinAnim, -3.0f);
        this->numScytheAfterImages = -3;
        this->actionTimer = 0;
        this->unk_2EC = -1;
        this->unk_18C = true;
        this->actor.speed = 0.0f;
    } else {
        // Starting straight out of blocking a projectile, already spinning so start moving immediately
        this->actionTimer = 10;
        this->unk_2EC = 9;
        this->actor.speed = 10.0f;
    }
    EnDeath_UpdateSpinAttackTris(this);
    this->actionFunc = EnDeath_SpinAttack;
}

void EnDeath_SpinAttack(EnDeath* this, PlayState* play) {
    EnDeath_Float(this);

    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_ROLL);
    }

    if (this->numScytheAfterImages < 7) {
        this->numScytheAfterImages++;
    }

    this->actionTimer++;
    if (this->actionTimer < 10) {
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    } else if (this->actionTimer == 10) {
        this->actor.speed = 10.0f;
    }

    EnDeath_UpdateSpinAttackTris(this);

    Math_ScaledStepToS(&this->unk_2F2, (s32)(Math_SinS(this->actionTimer * 0x2000) * 0x800) + 0x3000, 0x1000);
    Math_ScaledStepToS(&this->unk_2F4, (s32)(Math_SinS(this->actionTimer * 0x2000 - 0x8000) * 0x800), 0x1000);

    if ((this->actor.bgCheckFlags & 8) || (this->unk_788.base.atFlags & AT_HIT) ||
        (this->unk_860.base.atFlags & AT_HIT)) {
        this->unk_18C = false;
        this->unk_788.base.atFlags &= ~(AT_ON | AT_HIT);
        this->unk_860.base.atFlags &= ~AT_HIT;
        EnDeath_SetupApproachPlayer(this);
    }
}

void EnDeath_SetupDamaged(EnDeath* this) {
    Animation_PlayOnce(&this->skelAnime, &gGomessDamagedAnim);
    this->actor.speed = 10.0f;
    func_800BE568(&this->actor, &this->coreCollider);
    Actor_SetColorFilter(&this->actor, 0x4000, 255, 0, 15);
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_DAMAGE);
    this->actionFunc = EnDeath_Damaged;
}

void EnDeath_Damaged(EnDeath* this, PlayState* play) {
    EnDeath_Float(this);
    Math_StepToF(&this->actor.speed, 0.0f, 0.5f);
    if (SkelAnime_Update(&this->skelAnime)) {
        this->coreCollider.base.acFlags |= AC_ON;
        EnDeath_SetupSpinAttack(this);
    }
}

void EnDeath_SetupDeathCutscenePart1(EnDeath* this, PlayState* play) {
    Vec3f eye;
    Vec3f at;
    s32 i;

    this->actor.flags &= ~(ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_IGNORE_QUAKE);
    Animation_PlayOnce(&this->skelAnime, &gGomessBeginDeathAnim);

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_6;
    }

    this->unk_18D = false;
    this->coreCollider.base.acFlags &= ~AC_ON;
    Actor_SetColorFilter(&this->actor, 0x4000, 255, 0, 30);
    this->actionTimer = 35;
    this->actor.world.pos.y = this->actor.home.pos.y;
    this->actor.speed = 0.0f;
    this->camEyeTarget.x = Math_SinS(this->actor.shape.rot.y + 0x900) * 79.0f + this->actor.world.pos.x;
    this->camEyeTarget.z = Math_CosS(this->actor.shape.rot.y + 0x900) * 79.0f + this->actor.world.pos.z;
    this->camEyeTarget.y = this->actor.home.pos.y + 63.0f;
    at.x = this->actor.world.pos.x;
    at.y = this->actor.world.pos.y + 100.0f;
    at.z = this->actor.world.pos.z;
    eye.x = Math_SinS(this->actor.shape.rot.y + 0x900) * 179.0f + this->actor.world.pos.x;
    eye.z = Math_CosS(this->actor.shape.rot.y + 0x900) * 179.0f + this->actor.world.pos.z;
    eye.y = this->actor.home.pos.y + 30.0f;
    Play_SetCameraAtEye(play, this->camId, &at, &eye);
    this->camEyeSpeed = Math_Vec3f_DistXYZ(&eye, &this->camEyeTarget) * 0.05f;
    this->actor.shape.rot.y += 0x2000;
    Player_SetCsAction(play, &this->actor, 7);
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_DEAD);
    this->actionFunc = EnDeath_DeathCutscenePart1;
}

void EnDeath_DeathCutscenePart1(EnDeath* this, PlayState* play) {
    Camera* camera = Play_GetCamera(play, this->camId);
    f32 distToTarget = Math_Vec3f_StepTo(&camera->eye, &this->camEyeTarget, this->camEyeSpeed);

    Play_SetCameraAtEye(play, this->camId, &camera->at, &camera->eye);

    if (SkelAnime_Update(&this->skelAnime)) {
        this->actor.shape.rot.y += 0x2000;
        Animation_PlayLoop(&this->skelAnime, &gGomessDeathAnim);
    } else if (this->skelAnime.animation == &gGomessBeginDeathAnim) {
        this->actor.shape.rot.y += 0x2000;
        if (this->skelAnime.curFrame > 3.0f) {
            Matrix_Put(&this->unk_6A4);
            Matrix_RotateXS(-0x1000, MTXMODE_APPLY);
            Matrix_Get(&this->unk_6A4);
            this->unk_18E = false;
            this->unk_6A4.yw += 18.0f;
            Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_SCYTHE);
        }
    }

    if (distToTarget < 0.1f) {
        if (this->actionTimer == 35) {
            Audio_PlaySfx_AtPosWithVolumeTransition(&this->scytheScreenPos, NA_SE_EN_DEATH_SCYTHE_LEV, 0x41);
        }
        if (this->actionTimer > 0) {
            this->actionTimer--;
        }
        if (this->actionTimer == 0) {
            EnDeath_SetupDeathCutscenePart2(this, play);
        }
    }
}

void EnDeath_SetupDeathCutscenePart2(EnDeath* this, PlayState* play) {
    EnvLightSettings* lightSettings1;
    EnvLightSettings* lightSettings2;
    Player* player = GET_PLAYER(play);
    Vec3f eye;
    Vec3f at;
    f32 sinRotY;
    f32 cosRotY;
    s32 i;

    Animation_PlayLoop(&this->skelAnime, &gGomessDeathAnim);
    this->actionTimer = 30;
    player->actor.shape.rot.y = Actor_WorldYawTowardPoint(&player->actor, &this->actor.home.pos) + 0x1000;
    this->actor.shape.rot.y = player->actor.shape.rot.y + 0x6000;
    sinRotY = Math_SinS(player->actor.shape.rot.y);
    cosRotY = Math_CosS(player->actor.shape.rot.y);
    this->actor.world.pos.x = player->actor.world.pos.x + (260.0f * sinRotY);
    this->actor.world.pos.z = player->actor.world.pos.z + (260.0f * cosRotY);
    this->actor.world.pos.y = this->actor.home.pos.y + 15.0f;
    eye.x = (Math_SinS((s16)(player->actor.shape.rot.y - 0x2500)) * 182.0f) + this->actor.world.pos.x;
    eye.z = (Math_CosS((s16)(player->actor.shape.rot.y - 0x2500)) * 182.0f) + this->actor.world.pos.z;
    eye.y = this->actor.world.pos.y - 13.0f;
    at.x = player->actor.world.pos.x + (120.0f * sinRotY);
    at.y = player->actor.world.pos.y + 90.0f;
    at.z = player->actor.world.pos.z + (120.0f * cosRotY);
    Play_SetCameraAtEye(play, this->camId, &at, &eye);

    lightSettings1 = &play->envCtx.lightSettingsList[20];
    lightSettings2 = &play->envCtx.lightSettingsList[21];

    for (i = 0; i < 3; i++) {
        lightSettings1->light1Dir[i] = lightSettings2->light1Dir[i];
        lightSettings1->light2Dir[i] = lightSettings2->light2Dir[i];
    }

    this->actionFunc = EnDeath_DeathCutscenePart2;
}

void EnDeath_DeathCutscenePart2(EnDeath* this, PlayState* play) {
    f32 sin;
    f32 cos;
    s16 camYaw;

    SkelAnime_Update(&this->skelAnime);

    this->actionTimer--;
    if (this->actionTimer >= 0 && this->actionTimer < 3) {
        camYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;
        sin = Math_SinS(camYaw);
        cos = Math_CosS(camYaw);
        Matrix_Translate(this->actor.world.pos.x + (83.0f * sin) + (-38.0f * cos),
                         this->actor.world.pos.y + 53.0f + 15.0f * this->actionTimer,
                         this->actor.world.pos.z + (83.0f * cos) - (-38.0f * sin), MTXMODE_NEW);
        Matrix_RotateYS(camYaw - 0x3300, MTXMODE_APPLY);
        Matrix_RotateXS(0x1100 - this->actionTimer * 0x1800, MTXMODE_APPLY);
        Matrix_RotateZS(-0xA00, MTXMODE_APPLY);
        Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
        Matrix_Get(&this->unk_6A4);
        if (this->actionTimer == 0) {
            Camera_AddQuake(GET_ACTIVE_CAM(play), 2, 4, 6);
            Rumble_Request(this->actor.xyzDistToPlayerSq, 180, 20, 100);
            AudioSfx_StopByPosAndId(&this->scytheScreenPos, NA_SE_EN_DEATH_SCYTHE_LEV);
            Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_SCYTHE_ONGND);
        }
    }
    if (this->actionTimer < -25) {
        play->envCtx.lightSettingOverride = 20;
        EnDeath_SetupDeathCutscenePart3(this);
    }
}

void EnDeath_SetupDeathCutscenePart3(EnDeath* this) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_DIE;
    }

    this->actionTimer = 0;
    this->actionFunc = EnDeath_DeathCutscenePart3;
}

void EnDeath_DeathCutscenePart3(EnDeath* this, PlayState* play) {
    static Vec3f sparkleVel = { 0.0f, -1.5f, 0.0f };
    static Vec3f sparkleAccel = { 0.0f, -0.2f, 0.0f };
    static Color_RGBA8 sparklePrimColor = { 0, 0, 0, 0 };
    static Color_RGBA8 sparkleEnvColor = { 40, 40, 40, 0 };
    static s16 D_808C9A90[GOMESS_LIMB_MAX] = { 30, 30, 10, 2,  12, 17, 17, 17, 17, 17, 17,
                                               17, 14, 10, 12, 9,  9,  12, 9,  9,  30, 30 };
    s32 phi_v0;
    s32 phi_s0;
    u8* temp1A9;
    Vec3f* sparklePos;
    s32 pad;
    s32 pad1;
    f32 sin;
    f32 cos;
    f32 temp_fs0;
    s32 temp_v1;

    sparklePos = this->unk_404;
    temp1A9 = this->unk_1A9;
    for (phi_v0 = 0; (phi_v0 ^ 0) < ARRAY_COUNT(this->unk_404); phi_v0++) {
        sparklePos->y += 1.7f;
        temp_v1 = *temp1A9 - 30;
        *temp1A9 = CLAMP_MIN(temp_v1, 0);
        sparklePos++;
        temp1A9++;
    }

    temp_v1 = (-this->actionTimer * 14) + 252;
    this->actor.shape.shadowAlpha = CLAMP_MIN(temp_v1, 0);

    if (this->actionTimer < 18) {
        sparklePos = &this->unk_404[(this->actionTimer * 7) % ARRAY_COUNT(this->unk_404)];
        temp1A9 = &this->unk_1A9[(this->actionTimer * 7) % ARRAY_COUNT(this->unk_1A9)];

        sin = Math_SinS(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000);
        cos = Math_CosS(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000);

        for (phi_s0 = 0; phi_s0 < 7; phi_s0++) {
            temp_fs0 = Rand_CenteredFloat(100.0f) - 20.0f;
            sparklePos->x = this->actor.world.pos.x + (30.0f * sin) + (temp_fs0 * cos);
            sparklePos->y = (Rand_ZeroFloat(20.0f) + this->actor.world.pos.y + 4.0f * this->actionTimer) - 30.0f;
            sparklePos->z = (this->actor.world.pos.z + 30.0f * cos) - (temp_fs0 * sin);
            *temp1A9 = 255 - phi_s0 * 4;
            EffectSsKirakira_SpawnSmall(play, sparklePos, &sparkleVel, &sparkleAccel, &sparklePrimColor,
                                        &sparkleEnvColor);
            sparklePos++;
            temp1A9++;
        }
        Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_COMMON_EXTINCT_LEV - SFX_FLAG);
    }

    this->actionTimer++;

    for (phi_v0 = 0; phi_v0 < ARRAY_COUNT(this->unk_193); phi_v0++) {
        if (this->actionTimer == D_808C9A90[phi_v0]) {
            this->unk_193[phi_v0] = 1;
        }
    }

    if (this->actionTimer == 26) {
        EnDeath_SetupDeathCutscenePart4(this);
    }
}

void EnDeath_SetupDeathCutscenePart4(EnDeath* this) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(this->unk_1A9); i++) {
        this->unk_1A9[i] = 0;
    }

    Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.focus.pos);
    this->actor.bgCheckFlags &= ~1;
    this->actionTimer = 0;
    this->actionFunc = EnDeath_DeathCutscenePart4;
    this->actor.gravity = -1.0f;
    this->actor.shape.yOffset = 0.0f;
}

void EnDeath_DeathCutscenePart4(EnDeath* this, PlayState* play) {
    static Vec3f sparkleAccel = { 0.0f, -1.0f, 0.0f };
    static Color_RGBA8 sparklePrimColor = { 0, 200, 100, 0 };
    static Color_RGBA8 sparkleEnvColor = { 0, 100, 0, 0 };
    Vec3f sparkleVel;
    s32 i;

    if (this->actionTimer == 0) {
        if ((this->actor.bgCheckFlags & 1) || this->actor.floorHeight == -32000.0f) {
            Actor_SetScale(&this->actor, 0.0f);
            this->actionTimer++;

            for (i = 0; i < 40; i++) {
                sparkleAccel.y = -0.2f;
                sparkleVel.x = Rand_CenteredFloat(4.0f);
                sparkleVel.z = Rand_CenteredFloat(4.0f);
                sparkleVel.y = Rand_ZeroFloat(2.0f) + 3.0f;
                EffectSsKirakira_SpawnSmall(play, &this->actor.world.pos, &sparkleVel, &sparkleAccel, &sparklePrimColor,
                                            &sparkleEnvColor);
            }
            Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_HEARTBREAK);
        }
    } else {
        this->actionTimer++;
        if (this->actionTimer == 20) {
            EnDeath_SetupDead(this, play);
        }
    }
}

void EnDeath_SetupDead(EnDeath* this, PlayState* play) {
    CutsceneManager_Stop(this->actor.csId);
    Player_SetCsAction(play, &this->actor, 6);
    Flags_SetClearTemp(play, play->roomCtx.curRoom.num);
    this->actionTimer = 255;
    play->envCtx.lightSettingOverride = 255;
    this->actionFunc = EnDeath_Dead;
}

void EnDeath_Dead(EnDeath* this, PlayState* play) {
    if (this->actionTimer > 0) {
        this->actionTimer -= 5;
        if (this->actionTimer <= 0) {
            this->actionTimer = 0;
            Actor_Kill(&this->actor);
        }
    }
}

void func_808C7AEC(EnDeath* this) {
    s32 i;

    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_VOICE);
    Animation_MorphToPlayOnce(&this->skelAnime, &object_death_Anim_0015B4, 5.0f);
    this->actor.speed = 0.0f;

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_3;
    }

    this->floatTimer = 0;
    this->actionFunc = func_808C7B88;
}

void func_808C7B88(EnDeath* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y, 5.0f);

    if (SkelAnime_Update(&this->skelAnime)) {
        play->envCtx.lightSettingOverride = 27;
        func_808C7C04(this);
    }
}

void func_808C7C04(EnDeath* this) {
    s32 i;

    Animation_PlayLoop(&this->skelAnime, &object_death_Anim_001834);

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_4;
    }

    this->actionFunc = func_808C7C88;
    this->actor.speed = 0.0f;
}

void func_808C7C88(EnDeath* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.params >= 5) {
        play->envCtx.lightSettingOverride = 26;
        EnDeath_SetupApproachPlayer(this);
    }
}

void EnDeath_SetupPlayCutscene(EnDeath* this) {
    CutsceneManager_Queue(this->actor.csId);
    this->actionFunc = EnDeath_PlayCutscene;
}

void EnDeath_PlayCutscene(EnDeath* this, PlayState* play) {
    // Wait for it's turn to play
    if (CutsceneManager_IsNext(this->actor.csId)) {
        // Start playing
        CutsceneManager_Start(this->actor.csId, &this->actor);
        this->camId = CutsceneManager_GetCurrentSubCamId(this->actor.csId);
        if (this->actor.colChkInfo.health == 0) {
            // Death cutscene
            EnDeath_SetupDeathCutscenePart1(this, play);
        } else {
            // Intro cutscene
            EnDeath_SetupIntroCutscenePart1(this, play);
        }
    } else {
        CutsceneManager_Queue(this->actor.csId);
    }
}

void EnDeath_SetupBeginWithoutCutscene(EnDeath* this) {
    this->actionFunc = EnDeath_BeginWithoutCutscene;
}

void EnDeath_BeginWithoutCutscene(EnDeath* this, PlayState* play) {
    if (!Play_InCsMode(play)) {
        EnDeath_DimLights(play);
        this->coreCollider.base.acFlags |= AC_ON;
        Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        EnDeath_SetupApproachPlayer(this);
    }
}

void EnDeath_UpdateCore(EnDeath* this, PlayState* play) {
    if (this->unk_18D) {
        // Core falling to the ground
        this->corePos.y += this->coreVelocity;
        this->coreVelocity -= 1.0f;
        this->coreRotation += 0x1800;
        if (this->corePos.y < this->actor.floorHeight) {
            // Hit the floor
            this->corePos.y = this->actor.floorHeight;
            func_800B3030(play, &this->corePos, &gZeroVec3f, &gZeroVec3f, 100, 0, 0);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->corePos, 11, NA_SE_EN_EXTINCT);
            this->unk_18D = false;
        }
    }
}

void EnDeath_UpdateDamage(EnDeath* this, PlayState* play) {
    s32 i;

    if (this->unk_2F8 > 0) {
        this->unk_2F8--;
    }
    if (this->coreCollider.base.acFlags & AC_HIT) {
        this->coreCollider.base.acFlags &= ~AC_HIT;

        if (this->actor.params >= 5) {
            this->unk_18D = true;
            this->coreVelocity = -1.0f;
            this->coreRotation = this->actor.shape.rot.y;
            Math_Vec3s_ToVec3f(&this->corePos, &this->coreCollider.dim.worldSphere.center);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->corePos, 30, NA_SE_EN_FFLY_DEAD);

            if (this->actor.colChkInfo.damageEffect == DMGEFF_LIGHT_ARROW && this->actionFunc != func_808C7B88) {
                for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
                    this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_1;
                }
                play->envCtx.lightSettingOverride = 28;

                this->unk_2FC = 20;
                if (this->actionFunc == EnDeath_ApproachPlayer) {
                    this->actionTimer = 100;
                }
            } else if (this->actor.colChkInfo.damageEffect == DMGEFF_EXPLOSIVES) {
                this->unk_2F8 = 0xA;
            }
        } else if (this->actor.colChkInfo.damageEffect != DMGEFF_EXPLOSIVES || this->unk_2F8 == 0) {
            this->unk_18C = false;
            this->unk_788.base.atFlags &= ~AT_ON;
            this->coreCollider.base.acFlags &= ~AC_ON;

            if (this->actor.colChkInfo.damageEffect == DMGEFF_LIGHT_ARROW) {
                this->dmgEffectAlpha = 3.0f;
                this->dmgEffectScale = 0.8f;
                this->dmgEffect = ACTOR_DRAW_DMGEFF_LIGHT_ORBS;
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->coreCollider.info.bumper.hitPos.x,
                            this->coreCollider.info.bumper.hitPos.y, this->coreCollider.info.bumper.hitPos.z, 0, 0, 0,
                            4);
            }
            if (play->envCtx.lightSettingOverride == 27) {
                play->envCtx.lightSettingOverride = 26;
            }
            this->actor.shape.rot.x = 0;

            if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
                Audio_RestorePrevBgm(); // Stop miniboss BGM
                EnDeath_SetupPlayCutscene(this);
            } else {
                for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
                    this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_2;
                }
                EnDeath_SetupDamaged(this);
            }
        }
    }
}

void EnDeath_Update(Actor* thisx, PlayState* play) {
    EnDeath* this = THIS;
    ArrowLight* lightArrow;
    s32 pad;

    if (this->unk_2FC > 0) {
        this->unk_2FC--;

        lightArrow = (ArrowLight*)SubS_FindActor(play, NULL, ACTORCAT_ITEMACTION, ACTOR_ARROW_LIGHT);
        if (lightArrow != NULL) {
            lightArrow->screenFillIntensity = -100.0f;
        }

        if (this->unk_2FC == 0) {
            play->envCtx.lightSettingOverride = 26;
        }
    }

    EnDeath_UpdateDamage(this, play);
    EnDeath_UpdateCore(this, play);
    if (this->actionFunc != EnDeath_SpinAttack) {
        Math_ScaledStepToS(&this->unk_2F2, 0, 0x800);
        Math_ScaledStepToS(&this->unk_2F4, 0, 0x800);
    }

    this->actionFunc(this, play);

    if (this->actionFunc != EnDeath_Damaged) {
        this->actor.world.rot.y = this->actor.shape.rot.y;
    }
    Actor_MoveWithGravity(&this->actor);

    Actor_UpdateBgCheckInfo(play, &this->actor, 40.0f, (this->actionFunc == EnDeath_SpinAttack) ? 50.0f : 100.0f, 40.0f,
                            4 | 2 | 1);

    this->bodyCollider.dim.pos.x = this->actor.world.pos.x + Math_SinS(this->actor.shape.rot.y) * 3.0f;
    this->bodyCollider.dim.pos.z = this->actor.world.pos.z + Math_CosS(this->actor.shape.rot.y) * 3.0f;
    this->bodyCollider.dim.pos.y = this->actor.world.pos.y;

    if (this->actor.params < 5) {
        this->coreCollider.dim.worldSphere.radius = this->coreCollider.dim.modelSphere.radius + 5;
    } else {
        this->coreCollider.dim.worldSphere.radius = this->coreCollider.dim.modelSphere.radius;
    }

    if (this->actionFunc != EnDeath_Dead) {
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->bodyCollider.base);
    }
    if (this->coreCollider.base.acFlags & AC_ON) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->coreCollider.base);
    }
    if (this->unk_788.base.atFlags & AT_ON) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->unk_788.base);
    }
    if (this->actionFunc == EnDeath_BlockProjectile || this->actionFunc == EnDeath_SpinAttack) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->unk_860.base);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->unk_860.base);
    }

    if (this->dmgEffectAlpha > 0.0f) {
        if (this->dmgEffect != ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) {
            Math_StepToF(&this->dmgEffectAlpha, 0.0f, 0.05f);
            this->dmgEffectScale = (this->dmgEffectAlpha + 1.0f) * 0.4f;

            this->dmgEffectScale = CLAMP_MAX(this->dmgEffectScale, 0.8f);
        } else if (Math_StepToF(&this->dmgEffectSteamScale, 0.8f, 0.02f) == 0) {
            Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_ICE_FREEZE - SFX_FLAG);
        }
    }
}

void EnDeath_DrawScytheSpinning(EnDeath* this, PlayState* play) {
    s32 i;
    Gfx* dl;
    s32 pad;

    func_800B8118(&this->actor, play, 0);
    Scene_SetRenderModeXlu(play, 1, 2);

    OPEN_DISPS(play->state.gfxCtx);
    dl = POLY_XLU_DISP;

    for (i = 1; i < this->numScytheAfterImages; i++) {
        FrameInterpolation_RecordOpenChild(this, i);
        gDPPipeSync(dl++);
        gDPSetEnvColor(dl++, 30, 30, 0, 255 - i * 35);

        Matrix_Put(&this->unk_6A4);
        Matrix_RotateXS(i * 0x2100, MTXMODE_APPLY);

        gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(dl++, object_death_DL_006F88); // scythe handle

        Matrix_Translate(0.0f, -1084.0f, 7012.0f, MTXMODE_APPLY);
        Matrix_RotateZYX(-0x4000, 0, -0x4000, MTXMODE_APPLY);

        gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(dl++, object_death_DL_0073D0); // scythe blade
        FrameInterpolation_RecordCloseChild();
    }

    POLY_XLU_DISP = dl;
    CLOSE_DISPS(play->state.gfxCtx);
}

void EnDeath_DrawScythe(EnDeath* this, PlayState* play) {
    Gfx* dl;
    s32 pad;

    if (this->actionFunc == EnDeath_Dead) {
        func_800B8118(&this->actor, play, 0);
        Scene_SetRenderModeXlu(play, 1, 2);
    }

    OPEN_DISPS(play->state.gfxCtx);

    if (this->actionFunc == EnDeath_Dead) {
        dl = POLY_XLU_DISP;

        gDPPipeSync(dl++);
        gDPSetEnvColor(dl++, 30, 30, 0, this->actionTimer);
    } else {
        dl = POLY_OPA_DISP;
    }

    Matrix_Put(&this->unk_6A4);
    Matrix_Scale(this->scytheScale, this->scytheScale, this->scytheScale, MTXMODE_APPLY);

    gSPMatrix(&dl[0], Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(&dl[1], object_death_DL_006F88); // scythe handle

    Matrix_Translate(0.0f, -1084.0f, 7012.0f, MTXMODE_APPLY);
    Matrix_RotateZYX(-0x4000, 0, -0x4000, MTXMODE_APPLY);

    gSPMatrix(&dl[2], Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(&dl[3], object_death_DL_0073D0); // scythe blade

    Matrix_MultZero(&this->scytheWorldPos);

    if (this->actionFunc == EnDeath_Dead) {
        POLY_XLU_DISP = &dl[4];
    } else {
        POLY_OPA_DISP = &dl[4];
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void EnDeath_DrawBats(EnDeath* this, PlayState* play) {
    // Each bat is animated by cycling through these display lists
    static Gfx* sMinideathDLs[MINIDEATH_ANIM_LENGTH] = {
        gGomessBatFrame1DL, gGomessBatFrame2DL, gGomessBatFrame3DL, gGomessBatFrame4DL, gGomessBatFrame5DL,
        gGomessBatFrame6DL, gGomessBatFrame7DL, gGomessBatFrame8DL, gGomessBatFrame9DL,
    };
    EnMinideath* miniDeath;
    MtxF* cmf;
    f32 phi_fs2;
    MiniDeathStruct* phi_s0;
    s32 j;
    f32 scale;
    Gfx* dl;
    s32 i;
    Vec3f sp9C;

    OPEN_DISPS(play->state.gfxCtx);
    dl = POLY_OPA_DISP;

    gSPDisplayList(
        dl++, &gSetupDLs[0x19]); // incementing by 0x30 it seems. this is odd. The increment size may be wrong? TODO
    gSPDisplayList(dl++, gGomessBatMaterialDL);

    cmf = Matrix_GetCurrent();

    if (this->actionFunc == EnDeath_Dead) {
        phi_fs2 = this->actionTimer * 0.000035f;
        scale = 0.007f;
        if (phi_fs2 > 0.007f) {
            phi_fs2 = 0.007f;
            scale = 0.007f;
        }
    } else {
        phi_fs2 = 0.007f;
        scale = 0.007f;
    }

    if (this->actor.flags & ACTOR_FLAG_IGNORE_QUAKE) {
        Math_Vec3f_Copy(&sp9C, &play->mainCamera.quakeOffset);
    } else {
        Math_Vec3f_Copy(&sp9C, &gZeroVec3f);
    }

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        if (this->actionFunc == EnDeath_BeginWithoutCutscene ||
            (this->miniDeaths[i]->actor.flags & ACTOR_FLAG_40) == ACTOR_FLAG_40) {
            miniDeath = this->miniDeaths[i];
            FrameInterpolation_RecordOpenChild(miniDeath, i);

            Matrix_RotateZYX(miniDeath->actor.shape.rot.x, miniDeath->actor.shape.rot.y, 0, MTXMODE_NEW);
            Matrix_Scale(phi_fs2, phi_fs2, phi_fs2, MTXMODE_APPLY);

            for (phi_s0 = miniDeath->unk_160, j = 0; j < ARRAY_COUNT(miniDeath->unk_160); j++, phi_s0++) {
                if (phi_s0->unk_1 == 0) {
                    FrameInterpolation_RecordOpenChild(phi_s0, 0);
                    cmf->mf[3][0] = phi_s0->unk_4.x + sp9C.x;
                    cmf->mf[3][1] = phi_s0->unk_4.y + sp9C.y;
                    cmf->mf[3][2] = phi_s0->unk_4.z + sp9C.z;

                    gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(dl++, sMinideathDLs[phi_s0->animFrame]);
                    FrameInterpolation_RecordCloseChild();
                }
            }

            for (phi_s0 = miniDeath->unk_160, j = 0; j < ARRAY_COUNT(miniDeath->unk_160); j++, phi_s0++) {
                if (phi_s0->unk_1 == 1) {
                    FrameInterpolation_RecordOpenChild(phi_s0, 1);
                    Matrix_RotateZYX(0x4000, phi_s0->unk_1E, 0, MTXMODE_NEW);
                    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
                    cmf->mf[3][0] = phi_s0->unk_4.x + sp9C.x;
                    cmf->mf[3][1] = phi_s0->unk_4.y + sp9C.y;
                    cmf->mf[3][2] = phi_s0->unk_4.z + sp9C.z;

                    gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(dl++, sMinideathDLs[phi_s0->animFrame]);
                    FrameInterpolation_RecordCloseChild();
                }
            }
            FrameInterpolation_RecordCloseChild();
        }
    }

    if (this->unk_192) {
        for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
            miniDeath = this->miniDeaths[i];
            Matrix_RotateZYX(miniDeath->actor.shape.rot.x, miniDeath->actor.shape.rot.y, 0, MTXMODE_NEW);
            Matrix_Scale(phi_fs2, phi_fs2, phi_fs2, MTXMODE_APPLY);

            for (phi_s0 = miniDeath->unk_160, j = 0; j < ARRAY_COUNT(miniDeath->unk_160); j++, phi_s0++) {
                FrameInterpolation_RecordOpenChild(phi_s0, 2);
                cmf->mf[3][0] = miniDeath->actor.world.pos.x - phi_s0->unk_10.x;
                cmf->mf[3][1] = miniDeath->actor.world.pos.y + (20.0f - phi_s0->unk_10.y);
                cmf->mf[3][2] = miniDeath->actor.world.pos.z - phi_s0->unk_10.z;

                gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(dl++, sMinideathDLs[phi_s0->animFrame]);
                FrameInterpolation_RecordCloseChild();
            }
        }
    }

    if (this->unk_18D) {
        Matrix_RotateZYX(0x4000, this->coreRotation, 0, MTXMODE_NEW);
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
        cmf->mf[3][0] = this->corePos.x + sp9C.x;
        cmf->mf[3][1] = this->corePos.y + sp9C.y;
        cmf->mf[3][2] = this->corePos.z + sp9C.z;

        gSPMatrix(dl++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(dl++, sMinideathDLs[0]);
    }

    POLY_OPA_DISP = dl;
    CLOSE_DISPS(play->state.gfxCtx);
}

void EnDeath_DrawFlames(EnDeath* this, PlayState* play2) {
    PlayState* play = play2;
    s32 i;
    MtxF* cmf;
    s32 phi_s7;
    u8* flameAlphas;
    Vec3f* flamePos;
    MiniDeathStruct* effect;
    u8 alpha;
    s32 j;

    phi_s7 = this->actionTimer * 7;
    if (phi_s7 > ARRAY_COUNT(this->unk_1A9)) {
        phi_s7 = ARRAY_COUNT(this->unk_1A9);
    }

    OPEN_DISPS(play->state.gfxCtx);

    gDPSetEnvColor(POLY_XLU_DISP++, 230, 135, 25, 0);

    Matrix_Put(&play->billboardMtxF);
    Matrix_Scale(0.00405f, 0.003f, 0.00405f, MTXMODE_APPLY);

    cmf = Matrix_GetCurrent();

    flamePos = this->unk_404;
    flameAlphas = this->unk_1A9;
    for (i = 0; i < phi_s7; i++) {
        if (*flameAlphas > 0) {
            FrameInterpolation_RecordOpenChild(flameAlphas, i);

            gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 165, 255, 215, *flameAlphas);
            gSPSegment(POLY_XLU_DISP++, 0x08,
                       Gfx_TwoTexScroll(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0,
                                        ((play->gameplayFrames + (i * 10)) * -20) & 0x1FF, 32, 128));

            cmf->mf[3][0] = flamePos->x;
            cmf->mf[3][1] = flamePos->y;
            cmf->mf[3][2] = flamePos->z;

            gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);
            FrameInterpolation_RecordCloseChild();
        }
        flamePos++;
        flameAlphas++;
    }

    Matrix_Put(&play->billboardMtxF);
    Matrix_Scale(0.00405f, 0.003f, 0.00405f, MTXMODE_APPLY);

    if (this->actionFunc == EnDeath_Dead) {
        alpha = this->actionTimer;
    } else {
        alpha = 255;
    }

    for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
        if ((this->miniDeaths[i]->actor.flags & ACTOR_FLAG_40) == ACTOR_FLAG_40) {
            effect = this->miniDeaths[i]->unk_160;

            for (j = 0; j < ARRAY_COUNT(this->miniDeaths[i]->unk_160); j++, effect++) {
                FrameInterpolation_RecordOpenChild(effect, 3);
                cmf->mf[3][0] = effect->unk_4.x;
                cmf->mf[3][1] = effect->unk_4.y - 12.0f;
                cmf->mf[3][2] = effect->unk_4.z;

                gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 165, 255, 215, alpha);
                gSPSegment(POLY_XLU_DISP++, 0x08,
                           Gfx_TwoTexScroll(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0,
                                            ((play->gameplayFrames + ((j + i) * 10)) * -20) & 511, 32, 128));
                gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);
                FrameInterpolation_RecordCloseChild();
            }
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void EnDeath_DrawCore(EnDeath* this, PlayState* play) {
    s32 pad;
    Gfx* dl;

    OPEN_DISPS(play->state.gfxCtx);
    dl = POLY_OPA_DISP;

    Matrix_ReplaceRotation(&play->billboardMtxF);

    gSPMatrix(&dl[0], Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(&dl[1], gGomessCoreDL);

    if (this->actor.params >= 5) {
        // bats in front of the core
        gSPDisplayList(&dl[2], gGomessBatsGuardingCoreDL);
        POLY_OPA_DISP = &dl[3];
    } else {
        POLY_OPA_DISP = &dl[2];
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

s32 EnDeath_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnDeath* this = THIS;
    f32 temp_f12;
    s32 temp_v0;

    if (this->unk_193[limbIndex] == 1) {
        *dList = NULL;
    } else if (limbIndex == GOMESS_LIMB_CLOAK_LOWER && this->actionFunc == EnDeath_DeathCutscenePart3) {
        temp_v0 = this->actionTimer - 5;
        if (temp_v0 > 0) {
            temp_f12 = 1.0f - (temp_v0 * 0.1f);
            Matrix_Scale(temp_f12, temp_f12, temp_f12, MTXMODE_APPLY);
        }
    }
    if (limbIndex == GOMESS_LIMB_SCYTHE_HANDLE || limbIndex == GOMESS_LIMB_SCYTHE_BLADE) {
        if (!this->unk_18E) {
            *dList = NULL;
        }
    } else if (limbIndex == GOMESS_LIMB_CLOAK_UPPER) {
        rot->z = rot->z + this->unk_2F2;
    } else if (limbIndex == GOMESS_LIMB_CLOAK_LOWER) {
        rot->z = rot->z + this->unk_2F4;
    }
    return 0;
}

void EnDeath_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    static Vec3f D_808C9AF4 = { -1000.0f, 12000.0f, 0.0f };
    static Vec3f D_808C9B00 = { -2000.0f, -2500.0f, 0.0f };
    static Vec3f D_808C9B0C[5] = {
        { 1000.0f, 2000.0f, 0.0f },    { 1500.0f, 1000.0f, 4000.0f },  { 1500.0f, 1000.0f, -4000.0f },
        { 4000.0f, 4000.0f, 2000.0f }, { 4000.0f, 4000.0f, -2000.0f },
    };
    static s8 D_808C9B48[GOMESS_LIMB_MAX] = { -1, -1, -1, 12, -1, 0, -1, -1, -1, -1, -1,
                                              -1, -1, 7,  1,  2,  3, 4,  5,  6,  -1, -1 };
    EnDeath* this = THIS;
    s8 index;

    if (limbIndex == GOMESS_LIMB_SCYTHE_BLADE) {
        Matrix_MultZero(&this->scytheWorldPos);

        if ((this->unk_788.base.atFlags & AT_ON) && this->unk_2EC != this->actionTimer) {
            Vec3f quad0;
            Vec3f quad1;
            Vec3f pos1;
            Vec3f pos2;

            Math_Vec3f_Copy(&quad0, &this->unk_788.dim.quad[0]);
            Math_Vec3f_Copy(&quad1, &this->unk_788.dim.quad[1]);

            if (this->actionFunc == EnDeath_EndSwingAttack) {
                Matrix_MultVecX(6000.0f, &pos1);
                Matrix_MultVec3f(&D_808C9AF4, &pos2);
            } else {
                Matrix_MultVecY(5000.0f, &pos2);
                Matrix_MultVec3f(&D_808C9B00, &pos1);
            }

            Collider_SetQuadVertices(&this->unk_788, &pos2, &pos1, &quad0, &quad1);
            EffectBlure_AddVertex(Effect_GetByIndex(this->effectIndex), &pos1, &pos2);

            this->unk_2EC = this->actionTimer;
        } else if (this->unk_18C) {
            if (this->actionFunc == EnDeath_EndSwingAttack) {
                Matrix_MultVecX(6000.0f, &this->unk_788.dim.quad[1]);
                Matrix_MultVec3f(&D_808C9AF4, &this->unk_788.dim.quad[0]);
            } else {
                Matrix_MultVecY(5000.0f, &this->unk_788.dim.quad[0]);
                Matrix_MultVec3f(&D_808C9B00, &this->unk_788.dim.quad[1]);
            }
            this->unk_18C = false;
            this->unk_788.base.atFlags |= AT_ON;
            this->unk_2EC = this->actionTimer;
        }
    } else if (limbIndex == GOMESS_LIMB_CORE_POS && this->actionFunc != EnDeath_Dead) {
        Matrix_Push();
        EnDeath_DrawCore(this, play);
        Matrix_Pop();
        Matrix_MultZero(&this->actor.focus.pos);

        this->coreCollider.dim.worldSphere.center.x = this->actor.focus.pos.x;
        this->coreCollider.dim.worldSphere.center.y = this->actor.focus.pos.y;
        this->coreCollider.dim.worldSphere.center.z = this->actor.focus.pos.z;

        if (this->actor.params < 5) {
            this->coreCollider.dim.worldSphere.center.y = this->actor.focus.pos.y + 5.0f;
        }
    } else if (limbIndex == GOMESS_LIMB_SCYTHE_HANDLE) {
        if (!(this->actionFunc != EnDeath_SpinAttack && this->actionFunc != EnDeath_BlockProjectile &&
              this->actionFunc != EnDeath_IntroCutscenePart5) ||
            (this->actionFunc == EnDeath_DeathCutscenePart1 && this->unk_18E == true)) {
            Matrix_Get(&this->unk_6A4);
        }
    }

    index = D_808C9B48[limbIndex];
    if (index != -1) {
        if (index < 7) {
            // get matrix translation as-is, occupies slots 0, 1, 2, 3, 4, 5, 6
            Matrix_MultZero(&this->dmgEffectPositions[index]);
        } else if (index == 7) {
            // 5 points using 5 offsets, occupies slots 7, 8, 9, 10, 11
            s32 i;
            Vec3f* offset;
            Vec3f* effPos;

            for (effPos = &this->dmgEffectPositions[index], offset = &D_808C9B0C[0], i = 0;
                 i != ARRAY_COUNT(D_808C9B0C); i++, offset++, effPos++) {
                Matrix_MultVec3f(offset, effPos);
            }
        } else if (index == 12) {
            // get matrix translation as-is and a point offset by -2000 from it, occupies slots 12, 13
            Matrix_MultZero(&this->dmgEffectPositions[index]);
            Matrix_MultVecY(-2000.0f, &this->dmgEffectPositions[index + 1]);
        }
    }
}

void EnDeath_Draw(Actor* thisx, PlayState* play) {
    EnDeath* this = THIS;
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    func_800B8050(&this->actor, play, 0);
    AnimatedMat_DrawStepOpa(play, this->bodyMatAnim, this->unk_191);
    AnimatedMat_DrawOpa(play, this->coreMatAnim);
    Scene_SetRenderModeXlu(play, 0, 1);

    gDPSetEnvColor(POLY_OPA_DISP++, 30, 30, 0, 255);

    if (this->actionFunc != EnDeath_Dead && this->actionFunc != EnDeath_DeathCutscenePart4) {
        SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                              EnDeath_OverrideLimbDraw, EnDeath_PostLimbDraw, &this->actor);
    }
    if (this->actionFunc == EnDeath_DeathCutscenePart4) {
        EnDeath_DrawCore(this, play);
    }
    if (!this->unk_18E) {
        EnDeath_DrawScythe(this, play);
    }
    if (this->actor.colorFilterTimer != 0) {
        func_800AE5A0(play);
    }
    Actor_DrawDamageEffects(play, &this->actor, this->dmgEffectPositions, ARRAY_COUNT(this->dmgEffectPositions),
                            this->dmgEffectScale, this->dmgEffectSteamScale, this->dmgEffectAlpha, this->dmgEffect);
    if (this->actionFunc == EnDeath_SpinAttack || this->actionFunc == EnDeath_BlockProjectile) {
        EnDeath_DrawScytheSpinning(this, play);
    }
    EnDeath_DrawBats(this, play);
    if (this->actionFunc == EnDeath_Dead || this->actionFunc == EnDeath_DeathCutscenePart4 ||
        (this->actionFunc == EnDeath_DeathCutscenePart3 && this->actionTimer > 0)) {
        EnDeath_DrawFlames(this, play);
    }
    SkinMatrix_Vec3fMtxFMultXYZ(&play->viewProjectionMtxF, &this->scytheWorldPos, &this->scytheScreenPos);

    CLOSE_DISPS(play->state.gfxCtx);
}
