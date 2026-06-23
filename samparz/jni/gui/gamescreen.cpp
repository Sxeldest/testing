#include "main.h"
#include "gamescreen.h"
#include "gui.h"

extern CGUI* pGUI;

CGameScreen::CGameScreen()
{
    m_pInterface    = new CInterface();
    m_pButtons      = new CButtons();
}

void CGameScreen::LoadAllTextures()
{
}

void CGameScreen::Render()
{
}

void CGameScreen::Speedometer()
{   
}