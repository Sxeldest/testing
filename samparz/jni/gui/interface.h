#pragma once

#include "../vendor/imgui/imgui.h"
#include "../game/RW/RenderWare.h"

enum typeState
{
    OFF = 0,
    ON
};

class CInterface {

public:
    CInterface();
    ~CInterface();

    void RenderMenu();
    void RenderSpeedometer();
    static void SetupKeyboardStyle();
    void ToggleMenuState() { m_bRenderMenu = !m_bRenderMenu; }

    void EngineState(bool b) { bEngine = b; }
    void LightState(bool b) { bLight = b; }
    void LockState(bool b) { bLock = b; }
    void BeltState(bool b) { bBelt = b; }

private:
    bool m_bRenderMenu;
	
    ImFont* speedFont;
    RwTexture *engineTex0, *engineTex1,
     *lockTex0, *lockTex1, 
     *lightTex0, *lightTex1,
     *beltTex0, *beltTex1,
     *fuelTex, *btt,
     *healthTex;
    bool bEngine, bLight, bLock, bBelt;
    int iMileage;
};
