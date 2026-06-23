#include "../main.h"

#include "gui.h"

#include "../game/game.h"
#include "../net/netgame.h"

#include "../spawnscreen.h"
#include "../playertags.h"
#include "../dialog.h"
#include "../keyboard.h"
#include "../settings.h"
#include "../scoreboard.h"
#include "../deathmessage.h"

#include "../util/patch.h"

#include "interface.h"
#include "buttons.h"
#include "gamescreen.h"

#include "../debug.h"

extern CChatWindow *pChatWindow;
extern CSpawnScreen *pSpawnScreen;
extern CPlayerTags *pPlayerTags;
extern CDialogWindow *pDialogWindow;
extern CSettings *pSettings;
extern CKeyBoard *pKeyBoard;
extern CNetGame *pNetGame;
extern CScoreBoard *pScoreBoard;
extern CDeathMessage* pDeathMessage;
extern CGame *pGame;
extern CGameScreen *pGameScreen;
extern CDebug *pDebug;

/* imgui_impl_renderware.h */
void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_Init();
void ImGui_ImplRenderWare_NewFrame();
void ImGui_ImplRenderWare_ShutDown();

/*
	Все координаты GUI-элементов задаются
	относительно разрешения 1920x1080
*/

//Yellow 						ImVec4(0.96f, 0.56f, 0.19f, 1.0f)
//Dark Red 						ImVec4(0.7f, 0.12f, 0.12f, 1.0f)

#define PRIMARY_COLOR 			ImVec4(0.96f, 0.56f, 0.19f, 1.0f)
#define SECONDARY_COLOR 		ImVec4(0.7f, 0.12f, 0.12f, 1.0f)

#define MULT_X	1.0f / 1920
#define MULT_Y	1.0f / 1080

CGUI::CGUI()
{
    Log("Initializing GUI..");

    // setup ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplRenderWare_Init();

    // scale
    m_vecScale.x    = io.DisplaySize.x * MULT_X;
    m_vecScale.y    = io.DisplaySize.y * MULT_Y;
    // font Size
    m_fFontSize     = ScaleY( pSettings->Get().fFontSize );

    // mouse/touch
    m_bMousePressed = false;
    m_vecMousePos   = ImVec2(0, 0);

    Log("GUI | Scale factor: %f, %f Font size: %f", m_vecScale.x, m_vecScale.y, m_fFontSize);

    // ImGui::StyleColorsClassic();
    ImGui::StyleColorsDark();
    SetupDefaultStyle();

    m_bKeysStatus   = false;
    m_bTabStatus    = false;

    m_bRenderCBbg	= pSettings->Get().szChatBG;
    m_bRenderTextBg	= pSettings->Get().szTextBG;

    m_RenderSpeedID = 0;

    m_CurrentExp = 1;
    m_ToUpExp = 1;
    m_Eat = 0;

    radar = pSettings->Get().szRadar;
    timestamp = pSettings->Get().szTimeStamp;

    bShowDebugLabels = false;

    m_fuel = 0;
    bLabelBackground = pSettings->Get().bLabelBg;

    // load fonts
    char path[0xFF];
    sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, pSettings->Get().szFont);
    // cp1251 ranges
    static const ImWchar ranges[] =
    {
        0x0020, 0x0080,
        0x00A0, 0x00C0,
        0x0400, 0x0460,
        0x0490, 0x04A0,
        0x2010, 0x2040,
        0x20A0, 0x20B0,
        0x2110, 0x2130,
        0
    };
    Log("GUI | Loading font: %s", pSettings->Get().szFont);
    m_pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
    Log("GUI | ImFont pointer = 0x%X", m_pFont);

    Log("GUI | Loading font: chatkill.ttf");
	m_pFontGTAWeap = LoadFont("chtakill.ttf", 0);
	Log("GUI | ImFont pointer = 0x%X", m_pFontGTAWeap);
}

CGUI::~CGUI()
{
    ImGui_ImplRenderWare_ShutDown();
    ImGui::DestroyContext();
    SaveMenuSettings();
}

ImFont* CGUI::LoadFont(char *font, float fontsize)
{
	ImGuiIO &io = ImGui::GetIO();

	// load fonts
	char path[0xFF];
	sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, font);
	
	// ranges
	static const ImWchar ranges[] = 
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x04FF, // Russia
		0x0E00, 0x0E7F, // Thai
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
		0xF020, 0xF0FF, // Half-width characters
		0
	};
	
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
	return pFont;
}

void CGUI::RenderTextDeathMessage(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size, ImFont *font, bool bOutlineUseTextColor)
{
	int iOffset = bOutlineUseTextColor ? 1 : pSettings->Get().iFontOutline;
	if(bOutline)
	{
		// left
		posCur.x -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x += iOffset;
		// right
		posCur.x += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.x -= iOffset;
		// above
		posCur.y -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y += iOffset;
		// below
		posCur.y += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, bOutlineUseTextColor ? ImColor(col) : ImColor(IM_COL32_BLACK), text_begin, text_end);
		posCur.y -= iOffset;
	}

	ImGui::GetBackgroundDrawList()->AddText(font == nullptr ? GetFont() : font, font_size == 0.0f ? GetFontSize() : font_size, posCur, col, text_begin, text_end);
}


void CGUI::SetupDefaultStyle() {

    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->ScrollbarSize = ScaleY(45.0f);
	style->WindowPadding = ImVec2(8, 4);
	style->WindowBorderSize = 0.0f;
	style->ChildBorderSize = 0.0f;
	style->FrameBorderSize = 3.0f;
	style->WindowRounding = 0.75f;
	style->FrameRounding = 1.00f;
	style->ChildRounding = 0.75f;

    colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

	colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.79f);

    colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 2.0f);
    
    colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(255.0f, 255.0f, 255.0f, 1.0f);
	colors[ImGuiCol_ButtonActive] = ImVec4(255.0f, 255.0f, 255.0f, 1.0f);
	
	colors[ImGuiCol_ScrollbarGrabHovered] = SECONDARY_COLOR;
	colors[ImGuiCol_ScrollbarGrabActive] = SECONDARY_COLOR;
	
	colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

	colors[ImGuiCol_HeaderActive] = SECONDARY_COLOR;
	colors[ImGuiCol_Header] = SECONDARY_COLOR;
	colors[ImGuiCol_HeaderHovered] = SECONDARY_COLOR;

	colors[ImGuiCol_Border] = PRIMARY_COLOR;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void CGUI::Render()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplRenderWare_NewFrame();
    ImGui::NewFrame();

    // RenderVersion();
    // RenderPosition();
    RenderFPS();
    
    if(pNetGame)
    {   
        pNetGame->GetPlayerBubblePool()->Draw();
        pNetGame->GetLabelPool()->Draw();
        pNetGame->GetTextDrawPool()->Draw();
    }

    if (pGameScreen) 
    {

        pGameScreen->GetInterface()->RenderMenu();
        pGameScreen->GetButtons()->Render();
    }
        
    if(pPlayerTags)     pPlayerTags->Render();
    if(pChatWindow)     pChatWindow->Render();
    if(pDialogWindow)   pDialogWindow->Render();
    if(pSpawnScreen)    pSpawnScreen->Render();
    if(pKeyBoard)       pKeyBoard->Render();
    if(pScoreBoard)     pScoreBoard->Draw();
    if(pDeathMessage)   pDeathMessage->Render();
    
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());

    if(m_bNeedClearMousePos)
    {
        io.MousePos = ImVec2(-1, -1);
        m_bNeedClearMousePos = false;
    }
}

bool CGUI::OnTouchEvent(int type, bool multi, int x, int y)
{
    ImGuiIO& io = ImGui::GetIO();

    if(pNetGame) {
        if (pChatWindow && !pChatWindow->OnTouchEvent(type, multi, x, y))
            return false;

        if (pKeyBoard && !pKeyBoard->OnTouchEvent(type, multi, x, y))
            return false;

        pNetGame->GetTextDrawPool()->OnTouchEvent(type, multi, x, y);
    }

    switch(type)
    {
        case TOUCH_PUSH:
            io.MousePos = ImVec2(x, y);
            io.MouseDown[0] = true;
            break;

        case TOUCH_POP:
            io.MouseDown[0] = false;
            m_bNeedClearMousePos = true;
            break;

        case TOUCH_MOVE:
            io.MousePos = ImVec2(x, y);

            ScrollDialog(x, y);

            m_iLastPosY = y;
            break;
    }

    return true;
}

void CGUI::RenderFPS()
{	
	CPatch::CallFunction<void>(g_libGTASA + 0x39A0C4 + 1);
	char buff[50];
	float count = CPatch::CallFunction<float>(g_libGTASA + 0x39A054 + 1);
	sprintf(buff, "FPS : %.f", count);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(5, 5), ImColor(0, 255, 127, 240), buff);
}

void CGUI::RenderVersion()
{
    ImGuiIO& io = ImGui::GetIO();
    char buffer[128];

    // --> Render Version --> --> --> --> -->
    ImVec2 pos = ImVec2(5, 1);
    RenderText(pos, ImColor(255, 255, 255, 255), true, "SAMP Android - zy", nullptr, 20);
}

void CGUI::RenderPosition()
{
    ImGuiIO& io = ImGui::GetIO();
    MATRIX4X4 matFromPlayer;

    CPlayerPed *pLocalPlayerPed = pGame->FindPlayerPed();
    pLocalPlayerPed->GetMatrix(&matFromPlayer);

    ImVec2 _ImVec2 = ImVec2(ScaleX(10), io.DisplaySize.y - ImGui::GetFontSize() * 0.85);

    char text[128];
    sprintf(text, "\t\tPosition > X: %.4f - Y: %.4f - Z: %.4f", matFromPlayer.pos.X, matFromPlayer.pos.Y, matFromPlayer.pos.Z);

    RenderText(_ImVec2, ImColor(255, 255, 255, 255), true, text, nullptr, ImGui::GetFontSize() * 0.85);
}

void CGUI::RenderRakNetStatistics()
{
    // TODO: добавить вывод статистики ракнета
    //StatisticsToString(rss, message, 0);

    /*ImGui::GetOverlayDrawList()->AddText(
        ImVec2(ScaleX(10), ScaleY(400)),
        ImColor(IM_COL32_BLACK), message);*/
}

void CGUI::SetupKeyboardStyle() {

    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->WindowBorderSize = 0.0f;
    style->ChildBorderSize  = 0.0f;
    style->PopupBorderSize  = 0.0f;
    style->FrameBorderSize  = 0.0f;

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);

}

void CGUI::RenderText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size)
{
    int iOffset = pSettings->Get().iFontOutline;

    if (bOutline)
    {
        auto oAlpha = (col & 0xFF000000) >> 24;
        auto oColor = ImColor(IM_COL32(0, 0, 0, oAlpha));

        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(nullptr, font_size, posCur, oColor, text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(nullptr, font_size, posCur, oColor, text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(nullptr, font_size, posCur, oColor, text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(nullptr, font_size, posCur, oColor, text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(nullptr, font_size, posCur, col, text_begin, text_end);
}

void CGUI::RenderOverlayText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
    int iOffset = pSettings->Get().iFontOutline;

    if(bOutline)
    {
        posCur.x -= iOffset;
        ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetOverlayDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetOverlayDrawList()->AddText(posCur, col, text_begin, text_end);
}

void CGUI::ScrollDialog(float x, float y)
{
    if (m_imWindow != nullptr)
    {
        // --> Scroll Window --> --> --> --> -->
        if (m_iLastPosY > y)
            ImGui::SetWindowScrollY(m_imWindow, m_imWindow->Scroll.y + ImGui::GetFontSize() / 2);

        if (m_iLastPosY < y)
            ImGui::SetWindowScrollY(m_imWindow, m_imWindow->Scroll.y - ImGui::GetFontSize() / 2);
    }
}

void CGUI::RenderTextWithSize(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size)
{
    int iOffset = pSettings->Get().iFontOutline;

    if (bOutline)
    {
        // left
        posCur.x -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.x += iOffset;
        // right
        posCur.x += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.x -= iOffset;
        // above
        posCur.y -= iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.y += iOffset;
        // below
        posCur.y += iOffset;
        ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
        posCur.y -= iOffset;
    }

    ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
}