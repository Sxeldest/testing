#define IMGUI_DEFINE_MATH_OPERATORS
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_internal.h"

#include <string>
#include "uisettings.h"
#include "../main.h"
#include "../settings.h"

extern CSettings* pSettings;

/* scaling */
ImVec2 UISettings::m_baseSize = ImVec2(640.0f, 480.0f);
ImVec2 UISettings::m_scaleFactor = ImVec2(1.0f, 1.0f);

/* font (PC standard) */
float UISettings::m_fontSize = 16.0f;
float UISettings::m_smallFontSize = 14.0f;
float UISettings::m_outlineSize = 1.0f;

/* ////////////////// sizes ////////////////// */

/* widgets */
float UISettings::m_padding = 10.0f;

/* keyboard */
ImVec2 UISettings::m_keyboardPos = ImVec2(0.0f, 180.0f);
ImVec2 UISettings::m_keyboardSize = ImVec2(640.0f, 300.0f);
float UISettings::m_keyboardRowHeight = 52.0f;

/* splashscreen */
ImVec2 UISettings::m_splashscreenLogoPos = ImVec2(0.0f, 0.0f);
ImVec2 UISettings::m_splashscreenLogoSize = ImVec2(640.0f, 480.0f);
ImVec2 UISettings::m_splashScreenPBarPos = ImVec2(100.0f, 460.0f);
ImVec2 UISettings::m_splashScreenPBarSize = ImVec2(440.0f, 10.0f);

/* chat (PC standard) */
int UISettings::m_chatDispMesssages = 10;
int UISettings::m_chatMaxMessages = 20;
ImVec2 UISettings::m_chatPos = ImVec2(20.0f, 20.0f);
ImVec2 UISettings::m_chatPosNoRadar = ImVec2(20.0f, 20.0f);
ImVec2 UISettings::m_chatSize = ImVec2(420.0f, 0.0f);
ImVec2 UISettings::m_chatItemSize = ImVec2(420.0f, 14.0f);

/* spawn */
ImVec2 UISettings::m_spawnPos = ImVec2(200.0f, 420.0f);
ImVec2 UISettings::m_spawnSize = ImVec2(240.0f, 50.0f);

/* nametag */
ImVec2 UISettings::m_nametagBarSize = ImVec2(25.0f, 6.0f);

/* dialog (EXACT PC VALUES FROM CDialog.cpp) */
ImVec2 UISettings::m_dialogButtonPanelSize = ImVec2(100.0f, 30.0f); // m_i20, m_i24
ImVec2 UISettings::m_dialogMinSize = ImVec2(230.0f, 150.0f);
ImVec2 UISettings::m_dialogMaxSize = ImVec2(2000.0f, 2000.0f);
float UISettings::m_dialogTitleHeight = 25.0f;
float UISettings::m_dialogListItemHeight = 18.0f;
float UISettings::m_dialogInputHeight = 38.0f;

/* buttonpanel */
ImVec2 UISettings::m_buttonPanelPos = ImVec2(5.0f, 300.0f);
ImVec2 UISettings::m_buttonPanelSize = ImVec2(450.0f, 100.0f);

/* voice button */
ImVec2 UISettings::m_buttonVoicePos = ImVec2(500.0f, 180.0f);
ImVec2 UISettings::m_buttonVoiceSize = ImVec2(55.0f, 80.0f);

/* scoreboard */
ImVec2 UISettings::m_scoreboardSize = ImVec2(640.0f, 480.0f); // Standard PC Default
float UISettings::m_scoreboardHeaderHeight = 60.0f; // Standard PC Header

/* ////////////////// colors ////////////////// */
ImColor UISettings::m_buttonColor = ImColor(0.11f, 0.11f, 0.11f, 0.80f);
ImColor UISettings::m_buttonFocusedColor = ImColor(0xFF, 0xCC, 0x01);
ImColor UISettings::m_keyboardBackgroundColor = ImColor(0, 0, 0, 150);
ImColor UISettings::m_dialogBackgroundColor = ImColor(5, 5, 5, 220);
ImColor UISettings::m_dialogTitleBackgroundColor = ImColor(5, 5, 5, 220);
ImColor UISettings::m_scoreboardBackgroundColor = ImColor(5, 5, 5, 220);

void UISettings::Initialize(const ImVec2& display_size)
{
	m_scaleFactor = ImVec2(1.0f, 1.0f); // Disable scaling for PC-like absolute sizes

	if (pSettings)
	{
		m_fontSize = pSettings->Get().fFontSize;
		m_outlineSize = (float)pSettings->Get().iFontOutline;
		m_chatDispMesssages = pSettings->Get().iChatMaxMessages;
	}

	m_smallFontSize = m_fontSize - 2.0f;
	m_chatItemSize.y = m_fontSize + 2.0f;
	m_chatSize.y = m_chatItemSize.y * (float)m_chatDispMesssages;

	if (display_size.x > 800.0f) {
		m_scoreboardSize = ImVec2(800.0f, 600.0f);
	} else {
		m_scoreboardSize = ImVec2(640.0f, 480.0f);
	}
	m_scoreboardHeaderHeight = 60.0f;
}
