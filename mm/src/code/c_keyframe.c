#include "global.h"
#include "BenPort.h"

#define FMOD(x, mod) ((x) - ((s32)((x) * (1.0f / (mod))) * (f32)(mod)))

// cKF_FrameControl_zeroClera
void FrameCtrl_Reset(FrameControl* frameCtrl) {
    frameCtrl->duration = 0.0f;
    frameCtrl->curTime = 0.0f;
    frameCtrl->speed = 0.0f;
    frameCtrl->end = 0.0f;
    frameCtrl->start = 0.0f;
    frameCtrl->animMode = KEYFRAME_ANIM_ONCE;
}

// cKF_FrameControl_ct
void FrameCtrl_Init(FrameControl* frameCtrl) {
    FrameCtrl_Reset(frameCtrl);
}

// cKF_FrameControl_setFrame
void FrameCtrl_SetProperties(FrameControl* frameCtrl, f32 startTime, f32 endTime, f32 duration, f32 t, f32 speed,
                             s32 animMode) {
    frameCtrl->start = startTime;
    frameCtrl->end = (endTime < 1.0f) ? duration : endTime;
    frameCtrl->duration = duration;
    frameCtrl->speed = speed;
    frameCtrl->curTime = t;
    frameCtrl->animMode = animMode;
}

// cKF_FrameControl_passCheck
s32 FrameCtrl_PassCheck(FrameControl* frameCtrl, f32 t, f32* remainingTime) {
    f32 curTime;
    f32 speed;

    *remainingTime = 0.0f;
    curTime = frameCtrl->curTime;

    if (t == curTime) {
        return false;
    }

    speed = ((frameCtrl->start < frameCtrl->end) ? frameCtrl->speed : -frameCtrl->speed) * 1.5f;

    if (((speed >= 0.0f) && (curTime < t) && (t <= curTime + speed)) ||
        ((speed < 0.0f) && (t < curTime) && (curTime + speed <= t))) {

        *remainingTime = curTime + speed - t;
        return true;
    }
    return false;
}

// cKF_FrameControl_stop_proc
s32 FrameCtrl_UpdateOnce(FrameControl* frameCtrl) {
    f32 remainingTime;

    if (frameCtrl->curTime == frameCtrl->end) {
        return KEYFRAME_DONE_ONCE;
    }
    if (FrameCtrl_PassCheck(frameCtrl, frameCtrl->end, &remainingTime)) {
        frameCtrl->curTime = frameCtrl->end;
        return KEYFRAME_DONE_ONCE;
    }
    if (FrameCtrl_PassCheck(frameCtrl, frameCtrl->start, &remainingTime)) {
        frameCtrl->curTime = frameCtrl->end;
        return KEYFRAME_DONE_ONCE;
    }
    return KEYFRAME_NOT_DONE;
}

// cKF_FrameControl_repeat_proc
s32 FrameCtrl_UpdateLoop(FrameControl* frameCtrl) {
    f32 remainingTime;

    if (FrameCtrl_PassCheck(frameCtrl, frameCtrl->end, &remainingTime)) {
        frameCtrl->curTime = frameCtrl->start + remainingTime;
        return KEYFRAME_DONE_LOOP;
    }
    if (FrameCtrl_PassCheck(frameCtrl, frameCtrl->start, &remainingTime)) {
        frameCtrl->curTime = frameCtrl->end + remainingTime;
        return KEYFRAME_DONE_LOOP;
    }
    return KEYFRAME_NOT_DONE;
}

// cKF_FrameControl_play
s32 FrameCtrl_Update(FrameControl* frameCtrl) {
    s32 result;
    f32 speed;

    if (frameCtrl->animMode == KEYFRAME_ANIM_ONCE) {
        result = FrameCtrl_UpdateOnce(frameCtrl);
    } else {
        result = FrameCtrl_UpdateLoop(frameCtrl);
    }

    if (result == KEYFRAME_NOT_DONE) {
        speed = (frameCtrl->start < frameCtrl->end) ? frameCtrl->speed : -frameCtrl->speed;
        frameCtrl->curTime = frameCtrl->curTime + speed * 1.5f;
    }

    if (frameCtrl->curTime < 1.0f) {
        frameCtrl->curTime = (frameCtrl->curTime - 1.0f) + frameCtrl->duration;
    } else if (frameCtrl->duration < frameCtrl->curTime) {
        frameCtrl->curTime = (frameCtrl->curTime - frameCtrl->duration) + 1.0f;
    }

    return result;
}

// cKF_SkeletonInfo_R_zeroClear
void Keyframe_ResetFlex(KFSkelAnimeFlex* kfSkelAnime) {
    kfSkelAnime->skeleton = NULL;
    kfSkelAnime->animation = NULL;
    kfSkelAnime->jointTable = NULL;
    kfSkelAnime->callbacks = NULL;
    kfSkelAnime->morphTable = NULL;
    kfSkelAnime->morphFrames = 0.0f;
}

// cKF_SkeletonInfo_R_ct
void Keyframe_InitFlex(KFSkelAnimeFlex* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                       Vec3s* jointTable, Vec3s* morphTable, UnkKeyframeCallback* callbacks) {
    if (ResourceMgr_OTRSigCheck(skeleton)) {
        skeleton = (KeyFrameSkeleton*)ResourceMgr_LoadKeyFrameSkelByName(skeleton);
    }
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = (KeyFrameAnimation*)ResourceMgr_LoadKeyFrameAnimByName(animation);
    }

    Keyframe_ResetFlex(kfSkelAnime);
    FrameCtrl_Init(&kfSkelAnime->frameCtrl);
    kfSkelAnime->skeleton = (KeyFrameSkeleton*)Lib_SegmentedToVirtual(skeleton);
    kfSkelAnime->animation = (KeyFrameAnimation*)Lib_SegmentedToVirtual(animation);
    kfSkelAnime->jointTable = jointTable;
    kfSkelAnime->morphTable = morphTable;
    kfSkelAnime->callbacks = callbacks;
}

// cKF_SkeletonInfo_R_dt
void Keyframe_DestroyFlex(KFSkelAnimeFlex* kfSkelAnime) {
}

void Keyframe_FlexChangeAnim(KFSkelAnimeFlex* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                             f32 startTime, f32 endTime, f32 t, f32 speed, f32 morphFrames, s32 animMode);

void Keyframe_FlexPlayOnce(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, 0.0f,
                            KEYFRAME_ANIM_ONCE);
}

void Keyframe_FlexPlayOnceWithSpeed(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation, f32 speed) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, speed, 0.0f,
                            KEYFRAME_ANIM_ONCE);
}

void Keyframe_FlexPlayOnceWithMorph(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation, f32 morphFrames) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, morphFrames,
                            KEYFRAME_ANIM_ONCE);
}

void Keyframe_FlexPlayLoop(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, 0.0f,
                            KEYFRAME_ANIM_LOOP);
}

void Keyframe_FlexPlayLoopWithSpeed(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation, f32 speed) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, speed, 0.0f,
                            KEYFRAME_ANIM_LOOP);
}

void Keyframe_FlexPlayLoopWithMorph(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation, f32 morphFrames) {
    Keyframe_FlexChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                            ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, morphFrames,
                            KEYFRAME_ANIM_LOOP);
}

void Keyframe_FlexChangeAnim(KFSkelAnimeFlex* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                             f32 startTime, f32 endTime, f32 t, f32 speed, f32 morphFrames, s32 animMode) {
    if (ResourceMgr_OTRSigCheck(skeleton)) {
        skeleton = ResourceMgr_LoadKeyFrameSkelByName(skeleton);
    }
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = ResourceMgr_LoadKeyFrameAnimByName(animation);
        // Overwrite the end time after fetching the animation data as the parent caller most likely
        // passed in garbage data from the "fake" animation pointer (casted from resource string)
        endTime = animation->duration;
    }

    kfSkelAnime->morphFrames = morphFrames;

    if (kfSkelAnime->skeleton != skeleton) {
        kfSkelAnime->skeleton = (KeyFrameSkeleton*)Lib_SegmentedToVirtual(skeleton);
    }
    kfSkelAnime->animation = (KeyFrameAnimation*)Lib_SegmentedToVirtual(animation);

    FrameCtrl_SetProperties(&kfSkelAnime->frameCtrl, startTime, endTime, kfSkelAnime->animation->duration, t, speed,
                            animMode);
}

void Keyframe_FlexChangeAnimQuick(KFSkelAnimeFlex* kfSkelAnime, KeyFrameAnimation* animation) {
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = ResourceMgr_LoadKeyFrameAnimByName(animation);
    }

    kfSkelAnime->animation = (KeyFrameAnimation*)Lib_SegmentedToVirtual(animation);
    kfSkelAnime->frameCtrl.duration = kfSkelAnime->animation->duration;
}

// cKF_HermitCalc
// hermite interpolation
f32 Keyframe_Interpolate(f32 t, f32 delta, f32 x0, f32 x1, f32 v0, f32 v1) {
    f32 p = 3.0f * SQ(t) - 2.0f * CB(t);

    return (1.0f - p) * x0 + p * x1 + ((CB(t) - 2.0f * SQ(t) + t) * v0 + (CB(t) - SQ(t)) * v1) * delta;
}

// cKF_KeyCalc
s16 Keyframe_KeyCalc(s16 kfStart, s16 kfNum, KeyFrame* keyFrames, f32 t) {
    KeyFrame* keyFrames2 = &keyFrames[kfStart];
    f32 delta;
    s16 kf2;
    s16 kf1;

    if (t <= keyFrames2->frame) {
        return keyFrames2->value;
    }
    if (keyFrames2[kfNum - 1].frame <= t) {
        return keyFrames2[kfNum - 1].value;
    }

    for (kf2 = 1, kf1 = 0; true; kf2++, kf1++) {
        if (t < keyFrames2[kf2].frame) {
            delta = keyFrames2[kf2].frame - keyFrames2[kf1].frame;

            if (!IS_ZERO(delta)) {
                return nearbyint(Keyframe_Interpolate((t - keyFrames2[kf1].frame) / delta, delta * (1.0f / 30),
                                                      keyFrames2[kf1].value, keyFrames2[kf2].value,
                                                      keyFrames2[kf1].velocity, keyFrames2[kf2].velocity));
            } else {
                return keyFrames2[kf1].value;
            }
        }
    }
}

/**
 * Morph interpolator for rotation
 */
void Keyframe_RotationInterpolate(f32 t, s16* out, s16 rot1, s16 rot2) {
    u16 urot1 = rot1;
    s32 pad;
    u16 urot2 = rot2;
    f32 rot1f = rot1;
    f32 signedDiff = rot2 - rot1f;
    f32 urot1f = urot1;
    f32 unsignedDiff = urot2 - urot1f;

    if (fabsf(signedDiff) < fabsf(unsignedDiff)) {
        *out = rot1f + signedDiff * t;
    } else {
        *out = urot1f + unsignedDiff * t;
    }
}

/**
 * Morph interpolator for translation and scale
 */
void Keyframe_MorphInterpolate(s16* jointData, s16* morphData, f32 t) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (*jointData != *morphData) {
            f32 f1 = *jointData;
            f32 f2 = *morphData;
            *jointData = f1 + (f2 - f1) * t;
        }
        jointData++;
        morphData++;
    }
}

void Keyframe_UpdateFlexLimbs(KFSkelAnimeFlex* kfSkelAnime) {
    Vec3s* jointTable = kfSkelAnime->jointTable;
    Vec3s* morphTable = kfSkelAnime->morphTable;
    f32 t = 1.0f / fabsf(kfSkelAnime->morphFrames);
    s32 limbIndex;

    for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++) {
        Vec3s frameRot;
        Vec3s morphRot;

        // Interpolate scale
        Keyframe_MorphInterpolate((s16*)jointTable, (s16*)morphTable, t);
        jointTable++;
        morphTable++;

        // Read rotation
        frameRot.x = jointTable->x;
        frameRot.y = jointTable->y;
        frameRot.z = jointTable->z;

        morphRot.x = morphTable->x;
        morphRot.y = morphTable->y;
        morphRot.z = morphTable->z;

        // Interpolate rotation
        if (frameRot.x != morphRot.x || frameRot.y != morphRot.y || frameRot.z != morphRot.z) {
            Vec3s frameRotInv;
            f32 norm1;
            f32 norm2;

            frameRotInv.x = 0x7FFF + frameRot.x;
            frameRotInv.y = 0x7FFF - frameRot.y;
            frameRotInv.z = 0x7FFF + frameRot.z;

            // Compute L1 norms
            norm1 = fabsf((f32)morphRot.x - frameRot.x) + fabsf((f32)morphRot.y - frameRot.y) +
                    fabsf((f32)morphRot.z - frameRot.z);
            norm2 = fabsf((f32)morphRot.x - frameRotInv.x) + fabsf((f32)morphRot.y - frameRotInv.y) +
                    fabsf((f32)morphRot.z - frameRotInv.z);

            if (norm1 < norm2) {
                // frameRot is closer to morphRot than frameRotInv, interpolate between these two
                Keyframe_RotationInterpolate(t, &jointTable->x, frameRot.x, morphRot.x);
                Keyframe_RotationInterpolate(t, &jointTable->y, frameRot.y, morphRot.y);
                Keyframe_RotationInterpolate(t, &jointTable->z, frameRot.z, morphRot.z);
            } else {
                // frameRotInv is closer to morphRot than frameRot, interpolate between these two
                Keyframe_RotationInterpolate(t, &jointTable->x, frameRotInv.x, morphRot.x);
                Keyframe_RotationInterpolate(t, &jointTable->y, frameRotInv.y, morphRot.y);
                Keyframe_RotationInterpolate(t, &jointTable->z, frameRotInv.z, morphRot.z);
            }
        }
        morphTable++;
        jointTable++;

        // Interpolate translation
        Keyframe_MorphInterpolate((s16*)jointTable, (s16*)morphTable, t);
        jointTable++;
        morphTable++;
    }
}

s32 Keyframe_UpdateFlex(KFSkelAnimeFlex* kfSkelAnime) {
    s32 limbIndex;
    s32 pad[2];
    u16* bitFlags;
    s16* outputValues;
    s32 kfn = 0;
    s32 presetIndex = 0;
    s32 kfStart = 0;
    s16* presetValues;
    KeyFrame* keyFrames;
    s16* kfNums;
    u32 bit;
    s32 i;
    s32 j;

    if (kfSkelAnime->morphFrames != 0.0f) {
        outputValues = (s16*)kfSkelAnime->morphTable;
    } else {
        outputValues = (s16*)kfSkelAnime->jointTable;
    }

    // Array of preset values to pull from
    presetValues = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->presetValues);

    // Array of number of keyframes belonging to each limb
    kfNums = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->kfNums);

    // Array of keyframes, ordered by frame number
    keyFrames = (KeyFrame*)Lib_SegmentedToVirtual(kfSkelAnime->animation->keyFrames);

    // Array of flags for each limb.
    // Each limb takes up 16 bits, the 9 least significant bits are used to indicate whether to interpolate with
    // keyframes (if the bit is set) otherwise to pull a preset value (if the bit is not set)
    //  [8] : Scale x
    //  [7] : Scale y
    //  [6] : Scale z
    //  [5] : Rotate x
    //  [4] : Rotate y
    //  [3] : Rotate z
    //  [2] : Translate x
    //  [1] : Translate y
    //  [0] : Translate z
    bitFlags = (u16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->bitFlagsFlex);

    // For each limb
    for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++) {
        bit = 1 << (3 * 3 - 1);

        // 3 iter (scale, rotate, translate)
        for (i = 0; i < 3; i++) {
            // 3 iter (x, y, z ?)
            for (j = 0; j < 3; j++, outputValues++) {
                if (bitFlags[limbIndex] & bit) {
                    // If the bit is set, interpolate with keyframes
                    *outputValues = Keyframe_KeyCalc(kfStart, kfNums[kfn], keyFrames, kfSkelAnime->frameCtrl.curTime);
                    kfStart += kfNums[kfn];
                    kfn++;
                } else {
                    // If the bit is not set, pull from preset values
                    *outputValues = presetValues[presetIndex];
                    presetIndex++;
                }

                bit >>= 1;

                if (i == 1) {
                    *outputValues = FMOD(*outputValues * 0.1f, 360) * 182.04445f;
                }
            }
        }
    }

    if (IS_ZERO(kfSkelAnime->morphFrames)) {
        return FrameCtrl_Update(&kfSkelAnime->frameCtrl);
    } else if (kfSkelAnime->morphFrames > 0.0f) {
        Keyframe_UpdateFlexLimbs(kfSkelAnime);
        kfSkelAnime->morphFrames -= 1.0f;
        if (kfSkelAnime->morphFrames <= 0.0f) {
            kfSkelAnime->morphFrames = 0.0f;
        }
        return KEYFRAME_NOT_DONE;
    } else {
        Keyframe_UpdateFlexLimbs(kfSkelAnime);
        kfSkelAnime->morphFrames += 1.0f;
        if (kfSkelAnime->morphFrames >= 0.0f) {
            kfSkelAnime->morphFrames = 0.0f;
        }
        return FrameCtrl_Update(&kfSkelAnime->frameCtrl);
    }
}

void Keyframe_DrawFlexLimb(PlayState* play, KFSkelAnimeFlex* kfSkelAnime, s32* limbIndex,
                           OverrideKeyframeDrawScaled overrideKeyframeDraw, PostKeyframeDrawScaled postKeyframeDraw,
                           void* arg, Mtx** mtxStack) {
    KeyFrameFlexLimb* limb = (KeyFrameFlexLimb*)Lib_SegmentedToVirtual(kfSkelAnime->skeleton->limbsFlex);
    s32 i;
    Gfx* newDList;
    Gfx* limbDList;
    u8 flags;
    Vec3f scale;
    Vec3s rot;
    Vec3f pos;
    Vec3s* jointData;

    OPEN_DISPS(play->state.gfxCtx);

    limb += *limbIndex;
    jointData = &kfSkelAnime->jointTable[*limbIndex * 3];

    scale.x = jointData->x * 0.01f;
    scale.y = jointData->y * 0.01f;
    scale.z = jointData->z * 0.01f;

    jointData++;

    rot.x = jointData->x;
    rot.y = jointData->y;
    rot.z = jointData->z;

    jointData++;

    pos.x = jointData->x;
    pos.y = jointData->y;
    pos.z = jointData->z;

    Matrix_Push();

    newDList = limbDList = limb->dList;
    flags = limb->flags;

    if (overrideKeyframeDraw == NULL ||
        (overrideKeyframeDraw != NULL &&
         overrideKeyframeDraw(play, kfSkelAnime, *limbIndex, &newDList, &flags, arg, &scale, &rot, &pos) != 0)) {
        if ((kfSkelAnime->callbacks == NULL) || (limb->callbackIndex == KF_CALLBACK_INDEX_NONE) ||
            (kfSkelAnime->callbacks[limb->callbackIndex] == NULL) ||
            kfSkelAnime->callbacks[limb->callbackIndex](play, kfSkelAnime, *limbIndex, &newDList, &flags, arg) != 0) {

            Matrix_TranslateRotateZYX(&pos, &rot);

            if (scale.x != 1.0f || scale.y != 1.0f || scale.z != 1.0f) {
                Matrix_Scale(scale.x, scale.y, scale.z, MTXMODE_APPLY);
            }

            if (newDList != NULL) {
                Matrix_ToMtx(*mtxStack);

                if (flags & KEYFRAME_DRAW_XLU) {
                    gSPMatrix(POLY_XLU_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(POLY_XLU_DISP++, newDList);
                } else {
                    gSPMatrix(POLY_OPA_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(POLY_OPA_DISP++, newDList);
                }

                (*mtxStack)++;
            } else if (limbDList != NULL) {
                gSPMatrix(POLY_OPA_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                Matrix_ToMtx(*mtxStack);

                (*mtxStack)++;
            }
        }
    }

    if (postKeyframeDraw != NULL) {
        postKeyframeDraw(play, kfSkelAnime, *limbIndex, &newDList, &flags, arg, &scale, &rot, &pos);
    }

    (*limbIndex)++;

    for (i = 0; i < limb->numChildren; i++) {
        Keyframe_DrawFlexLimb(play, kfSkelAnime, limbIndex, overrideKeyframeDraw, postKeyframeDraw, arg, mtxStack);
    }

    Matrix_Pop();
    CLOSE_DISPS(play->state.gfxCtx);
}

void Keyframe_DrawFlex(PlayState* play, KFSkelAnimeFlex* kfSkelAnime, Mtx* mtxStack,
                       OverrideKeyframeDrawScaled overrideKeyframeDraw, PostKeyframeDrawScaled postKeyframeDraw,
                       void* arg) {
    s32 limbIndex;

    if (mtxStack == NULL) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x0D, mtxStack);
    gSPSegment(POLY_XLU_DISP++, 0x0D, mtxStack);

    limbIndex = 0;
    Keyframe_DrawFlexLimb(play, kfSkelAnime, &limbIndex, overrideKeyframeDraw, postKeyframeDraw, arg, &mtxStack);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Keyframe_ResetStandard(KFSkelAnime* kfSkelAnime) {
    kfSkelAnime->skeleton = NULL;
    kfSkelAnime->animation = NULL;
    kfSkelAnime->jointTable = NULL;
    kfSkelAnime->morphTable = NULL;
    kfSkelAnime->rotOffsetsTable = NULL;
    kfSkelAnime->morphFrames = 0.0f;
}

void Keyframe_InitStandard(KFSkelAnime* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                           Vec3s* jointTable, Vec3s* morphTable) {
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = ResourceMgr_LoadKeyFrameAnimByName(animation);
    }
    if (ResourceMgr_OTRSigCheck(skeleton)) {
        skeleton = ResourceMgr_LoadKeyFrameSkelByName(skeleton);
    }

    Keyframe_ResetStandard(kfSkelAnime);
    FrameCtrl_Init(&kfSkelAnime->frameCtrl);
    kfSkelAnime->skeleton = (KeyFrameSkeleton*)Lib_SegmentedToVirtual(skeleton);
    kfSkelAnime->animation = (KeyFrameAnimation*)Lib_SegmentedToVirtual(animation);
    kfSkelAnime->jointTable = jointTable;
    kfSkelAnime->morphTable = morphTable;
}

void Keyframe_DestroyStandard(KFSkelAnime* kfSkelAnime) {
}

void Keyframe_StandardChangeAnim(KFSkelAnime* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                                 f32 startTime, f32 endTime, f32 t, f32 speed, f32 morphFrames, s32 animMode,
                                 Vec3s* rotOffsetsTable);

void Keyframe_StandardPlayOnce(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, 0.0f,
                                KEYFRAME_ANIM_ONCE, rotOffsetsTable);
}

void Keyframe_StandardPlayOnceWithSpeed(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable,
                                        f32 speed) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, speed, 0.0f,
                                KEYFRAME_ANIM_ONCE, rotOffsetsTable);
}

void Keyframe_StandardPlayOnceWithMorph(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable,
                                        f32 morphFrames) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f,
                                morphFrames, KEYFRAME_ANIM_ONCE, rotOffsetsTable);
}

void Keyframe_StandardPlayLoop(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f, 0.0f,
                                KEYFRAME_ANIM_LOOP, rotOffsetsTable);
}

void Keyframe_StandardPlayLoopWithSpeed(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable,
                                        f32 speed) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, speed, 0.0f,
                                KEYFRAME_ANIM_LOOP, rotOffsetsTable);
}

void Keyframe_StandardPlayLoopWithMorph(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation, Vec3s* rotOffsetsTable,
                                        f32 morphFrames) {
    Keyframe_StandardChangeAnim(kfSkelAnime, kfSkelAnime->skeleton, animation, 1.0f,
                                ((KeyFrameAnimation*)Lib_SegmentedToVirtual(animation))->duration, 1.0f, 1.0f,
                                morphFrames, KEYFRAME_ANIM_LOOP, rotOffsetsTable);
}

void Keyframe_StandardChangeAnim(KFSkelAnime* kfSkelAnime, KeyFrameSkeleton* skeleton, KeyFrameAnimation* animation,
                                 f32 startTime, f32 endTime, f32 t, f32 speed, f32 morphFrames, s32 animMode,
                                 Vec3s* rotOffsetsTable) {
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = ResourceMgr_LoadKeyFrameAnimByName(animation);
    }
    if (ResourceMgr_OTRSigCheck(skeleton)) {
        skeleton = ResourceMgr_LoadKeyFrameSkelByName(skeleton);
    }

    kfSkelAnime->morphFrames = morphFrames;
    kfSkelAnime->skeleton = (KeyFrameSkeleton*)Lib_SegmentedToVirtual(skeleton);
    kfSkelAnime->animation = (KeyFrameAnimation*)Lib_SegmentedToVirtual(animation);

    FrameCtrl_SetProperties(&kfSkelAnime->frameCtrl, startTime, endTime, kfSkelAnime->animation->duration, t, speed,
                            animMode);
    kfSkelAnime->rotOffsetsTable = rotOffsetsTable;
}

void Keyframe_StandardChangeAnimQuick(KFSkelAnime* kfSkelAnime, KeyFrameAnimation* animation) {
    if (ResourceMgr_OTRSigCheck(animation)) {
        animation = ResourceMgr_LoadKeyFrameAnimByName(animation);
    }

    kfSkelAnime->animation = Lib_SegmentedToVirtual(animation);
    kfSkelAnime->frameCtrl.duration = kfSkelAnime->animation->duration;
}

void Keyframe_UpdateStandardLimbs(KFSkelAnime* kfSkelAnime) {
    Vec3s* jointTable = kfSkelAnime->jointTable;
    Vec3s* morphTable = kfSkelAnime->morphTable;
    f32 t = 1.0f / fabsf(kfSkelAnime->morphFrames);
    s32 limbIndex;

    Keyframe_MorphInterpolate((s16*)jointTable, (s16*)morphTable, t);

    // Skip translation
    jointTable++;
    morphTable++;

    // Interpolate rotation
    for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++) {
        Vec3s frameRot;
        Vec3s morphRot;

        frameRot.x = jointTable->x;
        frameRot.y = jointTable->y;
        frameRot.z = jointTable->z;

        morphRot.x = morphTable->x;
        morphRot.y = morphTable->y;
        morphRot.z = morphTable->z;

        if (frameRot.x != morphRot.x || frameRot.y != morphRot.y || frameRot.z != morphRot.z) {
            Vec3s frameRotInv;
            f32 norm1;
            f32 norm2;

            frameRotInv.x = 0x7FFF + frameRot.x;
            frameRotInv.y = 0x7FFF - frameRot.y;
            frameRotInv.z = 0x7FFF + frameRot.z;

            norm1 = fabsf((f32)morphRot.x - frameRot.x) + fabsf((f32)morphRot.y - frameRot.y) +
                    fabsf((f32)morphRot.z - frameRot.z);
            norm2 = fabsf((f32)morphRot.x - frameRotInv.x) + fabsf((f32)morphRot.y - frameRotInv.y) +
                    fabsf((f32)morphRot.z - frameRotInv.z);

            if (norm1 < norm2) {
                Keyframe_RotationInterpolate(t, &jointTable->x, frameRot.x, morphRot.x);
                Keyframe_RotationInterpolate(t, &jointTable->y, frameRot.y, morphRot.y);
                Keyframe_RotationInterpolate(t, &jointTable->z, frameRot.z, morphRot.z);
            } else {
                Keyframe_RotationInterpolate(t, &jointTable->x, frameRotInv.x, morphRot.x);
                Keyframe_RotationInterpolate(t, &jointTable->y, frameRotInv.y, morphRot.y);
                Keyframe_RotationInterpolate(t, &jointTable->z, frameRotInv.z, morphRot.z);
            }
        }
        morphTable++;
        jointTable++;
    }
}

s32 Keyframe_UpdateStandard(KFSkelAnime* kfSkelAnime) {
    s32 limbIndex;
    u32 bit;
    u8* bitFlags;
    s32 i;
    s32 kfn = 0;
    s32 presetIndex = 0;
    s32 kfStart = 0;
    s16* presetValues;
    KeyFrame* keyFrames;
    s16* kfNums;
    s16* outputValues;

    if (kfSkelAnime->morphFrames != 0.0f) {
        outputValues = (s16*)kfSkelAnime->morphTable;
    } else {
        outputValues = (s16*)kfSkelAnime->jointTable;
    }

    presetValues = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->presetValues);
    kfNums = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->kfNums);
    keyFrames = (KeyFrame*)Lib_SegmentedToVirtual(kfSkelAnime->animation->keyFrames);
    bitFlags = (u8*)Lib_SegmentedToVirtual(kfSkelAnime->animation->bitFlags);

    // Translation
    bit = 1 << (3 * 2 - 1);

    for (i = 0; i < 3; i++, bit >>= 1) { // xyz
        if (*bitFlags & bit) {
            *outputValues = Keyframe_KeyCalc(kfStart, kfNums[kfn], keyFrames, kfSkelAnime->frameCtrl.curTime);
            kfStart += kfNums[kfn++];
        } else {
            *outputValues = presetValues[presetIndex++];
        }
        outputValues++;
    }

    // Rotation only
    for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++) {
        bit = 1 << (3 - 1);

        for (i = 0; i < 3; i++) { // xyz
            s32 pad;

            if (bitFlags[limbIndex] & bit) {
                *outputValues = Keyframe_KeyCalc(kfStart, kfNums[kfn], keyFrames, kfSkelAnime->frameCtrl.curTime);
                kfStart += kfNums[kfn++];
            } else {
                *outputValues = presetValues[presetIndex++];
            }
            *outputValues = FMOD(*outputValues * 0.1f, 360) * 182.04445f;
            bit >>= 1;
            outputValues++;
        }
    }

    if (kfSkelAnime->rotOffsetsTable != NULL) {
        Vec3s* table;

        if (kfSkelAnime->morphFrames != 0.0f) {
            table = kfSkelAnime->morphTable;
        } else {
            table = kfSkelAnime->jointTable;
        }

        table++; // Skip translation?

        // Add all offsets to rotations
        for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++, table++) {
            table->x = table->x + kfSkelAnime->rotOffsetsTable[limbIndex].x;
            table->y = table->y + kfSkelAnime->rotOffsetsTable[limbIndex].y;
            table->z = table->z + kfSkelAnime->rotOffsetsTable[limbIndex].z;
        }
    }

    if (IS_ZERO(kfSkelAnime->morphFrames)) {
        return FrameCtrl_Update(&kfSkelAnime->frameCtrl);
    }
    if (kfSkelAnime->morphFrames > 0.0f) {
        Keyframe_UpdateStandardLimbs(kfSkelAnime);
        kfSkelAnime->morphFrames -= 1.0f;
        if (kfSkelAnime->morphFrames <= 0.0f) {
            kfSkelAnime->morphFrames = 0.0f;
        }
        return KEYFRAME_NOT_DONE;
    }
    Keyframe_UpdateStandardLimbs(kfSkelAnime);
    kfSkelAnime->morphFrames += 1.0f;
    if (kfSkelAnime->morphFrames >= 0.0f) {
        kfSkelAnime->morphFrames = 0.0f;
    }
    return FrameCtrl_Update(&kfSkelAnime->frameCtrl);
}

void Keyframe_DrawStandardLimb(PlayState* play, KFSkelAnime* kfSkelAnime, s32* limbIndex,
                               OverrideKeyframeDraw overrideKeyframeDraw, PostKeyframeDraw postKeyframeDraw, void* arg,
                               Mtx** mtxStack) {
    KeyFrameStandardLimb* limb =
        *limbIndex + (KeyFrameStandardLimb*)Lib_SegmentedToVirtual(kfSkelAnime->skeleton->limbsStandard);
    s32 i;
    Gfx* newDList;
    Gfx* limbDList;
    u8 flags;
    Vec3s rot;
    Vec3s* jointData = &kfSkelAnime->jointTable[*limbIndex];
    Vec3f pos;

    if (*limbIndex != 0) {
        pos.x = limb->translation.x;
        pos.y = limb->translation.y;
        pos.z = limb->translation.z;
    } else {
        pos.x = jointData->x;
        pos.y = jointData->y;
        pos.z = jointData->z;
    }

    jointData++;

    rot.x = jointData->x;
    rot.y = jointData->y;
    rot.z = jointData->z;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();

    newDList = limbDList = limb->dList;
    flags = limb->flags;

    if (overrideKeyframeDraw == NULL ||
        (overrideKeyframeDraw != NULL &&
         overrideKeyframeDraw(play, kfSkelAnime, *limbIndex, &newDList, &flags, arg, &rot, &pos) != 0)) {

        Matrix_TranslateRotateZYX(&pos, &rot);

        if (newDList != NULL) {
            Matrix_ToMtx(*mtxStack);

            if (flags & KEYFRAME_DRAW_XLU) {
                gSPMatrix(POLY_XLU_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, newDList);
            } else {
                gSPMatrix(POLY_OPA_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_OPA_DISP++, newDList);
            }
            (*mtxStack)++;
        } else if (limbDList != NULL) {
            Matrix_ToMtx(*mtxStack);

            gSPMatrix(POLY_OPA_DISP++, *mtxStack, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            (*mtxStack)++;
        }
    }

    if (postKeyframeDraw != NULL) {
        postKeyframeDraw(play, kfSkelAnime, *limbIndex, &newDList, &flags, arg, &rot, &pos);
    }

    (*limbIndex)++;

    for (i = 0; i < limb->numChildren; i++) {
        Keyframe_DrawStandardLimb(play, kfSkelAnime, limbIndex, overrideKeyframeDraw, postKeyframeDraw, arg, mtxStack);
    }

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

void Keyframe_DrawStandard(PlayState* play, KFSkelAnime* kfSkelAnime, Mtx* mtxStack,
                           OverrideKeyframeDraw overrideKeyframeDraw, PostKeyframeDraw postKeyframeDraw, void* arg) {
    s32 limbIndex;

    if (mtxStack == NULL) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x0D, mtxStack);
    gSPSegment(POLY_XLU_DISP++, 0x0D, mtxStack);

    limbIndex = 0;
    Keyframe_DrawStandardLimb(play, kfSkelAnime, &limbIndex, overrideKeyframeDraw, postKeyframeDraw, arg, &mtxStack);

    CLOSE_DISPS(play->state.gfxCtx);
}

void Keyframe_FlexGetScales(KFSkelAnime* kfSkelAnime, s32 targetLimbIndex, s16* scalesOut) {
    s16* kfNums;
    s32 i;
    s32 kfn = 0;
    s32 presetIndex = 0;
    s32 kfStart = 0;
    s32 j;
    u16* bitFlags;
    s16* scales = scalesOut;
    s16* presetValues;
    KeyFrame* keyFrames;
    s32 limbIndex;

    presetValues = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->presetValues);
    kfNums = (s16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->kfNums);
    keyFrames = (KeyFrame*)Lib_SegmentedToVirtual(kfSkelAnime->animation->keyFrames);
    bitFlags = (u16*)Lib_SegmentedToVirtual(kfSkelAnime->animation->bitFlagsFlex);

    for (limbIndex = 0; limbIndex < kfSkelAnime->skeleton->limbCount; limbIndex++) {
        u32 bit = 1 << (3 * 3 - 1);

        for (i = 0; i < 3; i++) {
            if ((limbIndex == targetLimbIndex) && (i == 0)) {
                // Is the target limb and is scale data
                for (j = 0; j < 3; j++) {
                    if (bitFlags[limbIndex] & bit) {
                        *scales = Keyframe_KeyCalc(kfStart, kfNums[kfn], keyFrames, kfSkelAnime->frameCtrl.curTime);
                        kfStart += kfNums[kfn];
                        kfn++;
                    } else {
                        *scales = presetValues[presetIndex];
                        presetIndex++;
                    }
                    bit >>= 1;
                    scales++;
                }
            } else {
                // Not the target limb, step over values
                for (j = 0; j < 3; j++) {
                    if (bitFlags[limbIndex] & bit) {
                        kfStart += kfNums[kfn];
                        kfn++;
                    } else {
                        presetIndex++;
                    }
                    bit >>= 1;
                }
            }
        }
    }
}
