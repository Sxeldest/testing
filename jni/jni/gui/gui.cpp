#include "../main.h"
#include "../game/game.h"
#include "../net/netgame.h"
#include "gui.h"
#include "../playertags.h"
#include "../net/chatbubble.h"

// voice
#include "../voice_new/Plugin.h"
#include "../voice_new/MicroIcon.h"
#include "../voice_new/SpeakerList.h"
#include "../voice_new/Network.h"

#include "../gui/samp_widgets/voicebutton.h"
#include "../gui/samp_widgets/buttonpanel.h"
#include "../java/jniutil.h"

extern CNetGame* pNetGame;
extern CPlayerTags* pPlayerTags;
extern UI* pUI;
extern CJavaWrapper* pJavaWrapper;

RwTexture* UI::m_pSampGuiTexture = nullptr;

ImVec4 UI::rectButtonNormal = ImVec4(0.0f, 0.5f, 136.0f, 54.5f);
ImVec4 UI::rectListBoxMain  = ImVec4(13.0f, 124.0f, 241.0f, 265.0f);
ImVec4 UI::rectListBoxSelection = ImVec4(17.0f, 269.0f, 241.0f, 287.0f);
ImVec4 UI::rectEditBox     = ImVec4(8.0f, 82.0f, 246.0f, 121.0f);

ImVec4 UI::rectScrollTrack     = ImVec4(243.0f, 144.0f, 265.0f, 155.0f);
ImVec4 UI::rectScrollUpArrow   = ImVec4(243.0f, 124.0f, 265.0f, 144.0f);
ImVec4 UI::rectScrollDownArrow = ImVec4(243.0f, 155.0f, 265.0f, 176.0f);
ImVec4 UI::rectScrollThumb     = ImVec4(266.0f, 123.0f, 286.0f, 167.0f);

UI::UI(const ImVec2& display_size, const std::string& font_path)
	: Widget(), ImGuiWrapper(display_size, font_path)
{
	UISettings::Initialize(display_size);
	this->setFixedSize(display_size);
}

bool UI::initialize()
{
	if (!ImGuiWrapper::initialize()) return false;

	// Load shared textures
	if (!m_pSampGuiTexture)
		m_pSampGuiTexture = (RwTexture*)LoadTextureFromDB("samp", "sampgui");

	/*m_splashScreen = new SplashScreen();
	this->addChild(m_splashScreen);
	m_splashScreen->setFixedSize(size());
	m_splashScreen->setPosition(ImVec2(0.0f, 0.0f));
	m_splashScreen->setVisible(true);*/

	m_chat = new Chat();
	this->addChild(m_chat);
	m_chat->setFixedSize(UISettings::chatSize());
	m_chat->setPosition(UISettings::chatPos());
	m_chat->setVisible(true);

	m_buttonPanel = new ButtonPanel();
	this->addChild(m_buttonPanel);
	m_buttonPanel->setFixedSize(UISettings::buttonPanelSize());
	m_buttonPanel->setPosition(UISettings::buttonPanelPos());
	m_buttonPanel->setVisible(false);

	m_voiceButton = new VoiceButton();
	this->addChild(m_voiceButton);
	m_voiceButton->setFixedSize(UISettings::buttonVoiceSize());
	m_voiceButton->setPosition(UISettings::buttonVoicePos());
	m_voiceButton->setVisible(false);

	m_spawn = new Spawn();
	this->addChild(m_spawn);
	m_spawn->setFixedSize(UISettings::spawnSize());
	m_spawn->setPosition(UISettings::spawnPos());
	m_spawn->setVisible(false);

	m_dialog = new CDialog();
	this->addChild(m_dialog);
	m_dialog->setVisible(false);
	m_dialog->setMinSize(UISettings::dialogMinSize());
	m_dialog->setMaxSize(UISettings::dialogMaxSize());

	m_inputChat = new InputChat();
	this->addChild(m_inputChat);
	m_inputChat->setFixedSize(UISettings::inputChatSize());
	m_inputChat->setPosition(UISettings::inputChatPos());
	m_inputChat->setVisible(false);

	m_playerTabList = new PlayerTabList();
	this->addChild(m_playerTabList);
	m_playerTabList->setVisible(false);

	// mem
	/* Label* d_label1;

	d_label1 = new Label("beta 1.0", ImColor(1.0f, 1.0f, 1.0f), true, UISettings::fontSize() / 2);
	this->addChild(d_label1);
	d_label1->setPosition(ImVec2(3.0, 3.0));*/

    // ==== version ==== //
    //d_label = new Label("", ImColor(1.0f, 1.0f, 1.0f), true, UISettings::fontSize() / 2);
    //this->addChild(d_label);
   // d_label->setPosition(ImVec2(3.0, 55.0));

	return true;
}

void UI::render()
{
	ImGuiWrapper::render();

	uintptr_t ms_memoryUsed = *(uint32_t*)(g_libGTASA+0x00792B74);
	uintptr_t ms_memoryAvailable = *(uint32_t*)(g_libGTASA+0x00685FA0);
    std::string str = string_format(std::string ("%d/%d"), ms_memoryUsed>>20, ms_memoryAvailable>>20);
    //d_label->setText(str);

	if (m_bNeedClearMousePos) {
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(-1, -1);
		m_bNeedClearMousePos = false;
	}
}

void UI::shutdown()
{
	ImGuiWrapper::shutdown();
}

void UI::drawList()
{
	if (!visible()) return;

	/*Label* label;
	label = new Label("1.0.11", ImColor(1.0f, 1.0f, 1.0f), true, UISettings::fontSize() / 2);
	label->setPosition(ImVec2(0.0, 0.0));
	this->addChild(label);*/

	/*if (pNetGame) //render custom gui
	{
		CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
		if (pTextDrawPool) pTextDrawPool->DrawImage();
	}*/

	if (pPlayerTags) pPlayerTags->Render(renderer());
	if (pNetGame && pNetGame->GetTextLabelPool()) pNetGame->GetTextLabelPool()->Render(renderer());
	if (pNetGame && pNetGame->GetPlayerBubblePool()) pNetGame->GetPlayerBubblePool()->Render(renderer());

	draw(renderer());
}

void UI::touchEvent(const ImVec2& pos, TouchType type)
{
	/* 
		                    
		1 -           
		2 -       
		3 -    
	*/

	if (m_inputChat->visible())
	{
		if (m_inputChat->contains(pos)) {
			if (type == TouchType::push && pJavaWrapper && !pJavaWrapper->IsKeyboardShowing()) {
				pJavaWrapper->ShowKeyboard();
				pJavaWrapper->SetKeyboardText(m_inputChat->inputString().c_str());
			}
			m_inputChat->touchEvent(pos, type);
			return;
		}

		if (m_inputChat->getCaller() && m_inputChat->getCaller()->contains(pos)) {
			if (type == TouchType::push && pJavaWrapper && !pJavaWrapper->IsKeyboardShowing()) {
				pJavaWrapper->ShowKeyboard();
				pJavaWrapper->SetKeyboardText(m_inputChat->inputString().c_str());
			}
			m_inputChat->getCaller()->touchEvent(pos, type);
			return;
		}

		if (type == TouchType::push) {
			if (pJavaWrapper && pJavaWrapper->IsKeyboardShowing()) {
				pJavaWrapper->HideKeyboard();
			} else {
				m_inputChat->hide(true);
			}
			return;
		}
		return;
	}

	if (m_dialog->visible() && m_dialog->contains(pos))
	{
		m_dialog->touchEvent(pos, type);
		return;
	}

	if (m_playerTabList->visible() && m_playerTabList->contains(pos))
	{
		m_playerTabList->touchEvent(pos, type);
		return;
	}

	Widget::touchEvent(pos, type);
}

enum eTouchType
{
	TOUCH_POP = 1,
	TOUCH_PUSH = 2,
	TOUCH_MOVE = 3
};

bool UI::OnTouchEvent(int type, bool multi, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();

	/*
	switch (type)
	{
	case 1://TOUCH_PUSH:
		io.MousePos = ImVec2(x, y);
		io.MouseDown[0] = true;
		MyLog2("TOUCH_PUSH");
		break;

	case 2://TOUCH_POP:
		io.MouseDown[0] = false;
		m_bNeedClearMousePos = true;
		MyLog2("TOUCH_POP");
		break;

	case 3://TOUCH_MOVE:
		io.MousePos = ImVec2(x, y);
		MyLog2("TOUCH_MOVE");
		break;
	}*/
	VoiceButton* vbutton = pUI->voicebutton();
	switch (type)
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
		//if (vbutton->countdown > 50) vbutton->countdown = 20;
		break;
	}

	return true;
}
