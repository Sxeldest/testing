#include "../../main.h"
#include "../gui.h"
#include "../../game/game.h"
#include "../../net/netgame.h"
#include "../../net/localplayer.h"

extern UI* pUI;
extern CNetGame* pNetGame;
extern CGame *pGame;

extern bool bNeedEnterVehicle;
extern bool OpenButton;
int Tab = 0;

ButtonPanel::ButtonPanel()
	: Layout(Orientation::HORIZONTAL)
{
	CButton* m_bTab = new CButton("TAB", UISettings::fontSize());
	OButton* m_bOpen = new OButton(">>", UISettings::fontSize());
	CButton* m_bClose = new CButton("<<", UISettings::fontSize());
	CButton* m_bEsc = new CButton("ESC", UISettings::fontSize());
	CButton* m_bAlt = new CButton("ALT", UISettings::fontSize());
	m_bH = new CButton("H", UISettings::fontSize());
	CButton* m_bY = new CButton("Y", UISettings::fontSize());
	CButton* m_bN = new CButton("N", UISettings::fontSize());
	CButton* m_bF = new CButton("F", UISettings::fontSize());
	CButton* m_bG = new CButton("G", UISettings::fontSize());
	CButton* m_b2 = new CButton("2", UISettings::fontSize());

	m_bTab->setCallback([]()
	{
		if (OpenButton && pUI) {
			if (pUI->playertablist()->visible())
				pUI->playertablist()->hide();
			else
				pUI->playertablist()->show();
		}
	});

	m_bOpen->setCallback([]() { OpenButton = true; });
	m_bClose->setCallback([]() { OpenButton = false; });

	m_bEsc->setCallback([m_bEsc]() {
		if (pNetGame && m_bEsc->visible() && OpenButton) {
			CTextDrawPool* pTextDrawPool = pNetGame->GetTextDrawPool();
			if (pTextDrawPool) {
				pTextDrawPool->SetSelectState(false, 0);
			}
		}
	});

	m_bAlt->setCallback([m_bAlt]() {
		if(m_bAlt->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
	});

	m_bH->setCallback([this]() {
		if(m_bH->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
	});

	m_bY->setCallback([m_bY]() {
		if(m_bY->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
	});

	m_bN->setCallback([m_bN]() {
		if(m_bN->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
	});

	m_bF->setCallback([m_bF]() {
		if(m_bF->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
	});

	m_bG->setCallback([m_bG]() {
		if(m_bG->visible() && OpenButton)
			bNeedEnterVehicle = true;
	});

	m_b2->setCallback([m_b2]() {
		if(m_b2->visible() && OpenButton)
			LocalPlayerKeys.bKeys[ePadKeys::KEY_SUBMISSION] = true;
	});

	this->addChild(m_bClose);
	this->addChild(m_bTab);
	this->addChild(m_bEsc);
	this->addChild(m_bAlt);
	this->addChild(m_bH);
	this->addChild(m_bF);
	this->addChild(m_bY);
	this->addChild(m_bN);
	this->addChild(m_bG);
	this->addChild(m_b2);
	this->addChild(m_bOpen);
}
