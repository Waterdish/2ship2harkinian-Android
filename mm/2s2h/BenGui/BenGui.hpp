#ifndef BenGui_hpp
#define BenGui_hpp

#include <stdio.h>
#include "BenMenuBar.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/ActorViewer.h"
#include "DeveloperTools/CollisionViewer.h"
#include "DeveloperTools/EventLog.h"
#include "BenInputEditorWindow.h"
#include "Menu.h"

namespace BenGui {
    void SetupHooks();
    void SetupGuiElements();
    void Draw();
    void Destroy();
}

#endif /* BenGui_hpp */
