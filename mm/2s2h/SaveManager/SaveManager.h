
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <libultraship/libultraship.h>

#ifdef __cplusplus
std::string SaveManager_GetFileName(int fileNum, bool isBackup = false);
bool SaveManager_HandleFileDropped(std::string filePath);
bool BinarySaveConverter_HandleFileDropped(std::string filePath);
int SaveManager_GetOpenFileSlot();
void SaveManager_WriteSaveFile(std::filesystem::path fileName, nlohmann::json j);
#else
void SaveManager_SysFlashrom_WriteData(u8* addr, u32 pageNum, u32 pageCount);
s32 SaveManager_SysFlashrom_ReadData(void* addr, u32 pageNum, u32 pageCount);
#endif

#if defined(__ANDROID__)
#ifdef __cplusplus
extern "C"
{
#endif
void SaveManager_Init();
#ifdef __cplusplus
}
#endif
#endif

#endif // SAVE_MANAGER_H
