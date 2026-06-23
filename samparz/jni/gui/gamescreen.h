#pragma once
#include "buttons.h"
#include "interface.h"
#include "../game/game.h"

class CGameScreen
{
private:
    CButtons* m_pButtons;
    CInterface* m_pInterface;
public:
    CGameScreen(/* args */);
    ~CGameScreen();

    CButtons* GetButtons() { return m_pButtons; }
    CInterface* GetInterface() { return m_pInterface; }

    void Render();

private:
    RwTexture* LoadTex(const char* name) { return (RwTexture*)LoadTextureFromDB("flame", name ); }
    void LoadAllTextures();
    void Speedometer();
    RwTexture* panelBackground, *panelEngine[2];
};
