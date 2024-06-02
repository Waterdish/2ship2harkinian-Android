#ifndef SAVING_ENHANCEMENTS_H
#define SAVING_ENHANCEMENTS_H

void RegisterSavingEnhancements();
void RegisterAutosave();

#ifdef __cplusplus
extern "C" {
#endif

int SavingEnhancements_GetSaveEntrance();
bool SavingEnhancements_CanSave();

#ifdef __cplusplus
}
#endif

#endif // SAVING_ENHANCEMENTS_H
