#include "../../../main.h"
#include "../../gui.h"
#include "../../../game/game.h"
#include "../../../net/netgame.h"

extern CGame* pGame;
extern CNetGame* pNetGame;

/* CDialog Class Implementation */

CDialog::CDialog()
{
	m_pTitle = new CDialogTitle();
	this->addChild(m_pTitle);

	m_pContent = new CDialogContent();
	this->addChild(m_pContent);

	m_pButton = new CDialogButton();
	this->addChild(m_pButton);

	m_iID = -1;
	m_fWidth = 600.0f;
	m_fHeight = 300.0f;
}

static bool IsSAMPButtonEmpty(const std::string& str)
{
	if (str.empty()) return true;
	for (char c : str) { if ((unsigned char)c > 32) return false; }
	return true;
}

void CDialog::Show(int16_t iID, DialogStyle iStyle, const std::string& szTitle, const std::string& szInfo, const std::string& szButton1, const std::string& szButton2)
{
	if (iID >= 0)
	{
		m_iID = iID;

		this->SetTitleCaption(szTitle);
		this->SetButton1Caption(szButton1);

		if (IsSAMPButtonEmpty(szButton2)) {
			m_pButton->m_pButton2->setVisible(false);
		} else {
			this->SetButton2Caption(szButton2);
			m_pButton->m_pButton2->setVisible(true);
		}

		m_pContent->SetActive(iStyle, szInfo);
		this->performLayout();
		this->setVisible(true);

		if (pGame) pGame->DisplayHUD(false);
	}
}

void CDialog::Hide()
{
	this->setVisible(false);
	if (pGame) {
		if (!pNetGame || !pNetGame->GetTextDrawPool() || !pNetGame->GetTextDrawPool()->GetState()) {
			pGame->DisplayHUD(true);
		}
	}
}

void CDialog::SetTitleCaption(const std::string& szCaption) { m_pTitle->SetTitle(szCaption); }
void CDialog::SetButton1Caption(const std::string& szCaption) { m_pButton->SetButton1Caption(szCaption); }
void CDialog::SetButton2Caption(const std::string& szCaption) { m_pButton->SetButton2Caption(szCaption); }

void CDialog::performLayout()
{
	Widget* pActive = m_pContent->GetActiveWidget();
	if (!pActive) return;

	DialogStyle iStyle = m_pContent->GetActiveStyle();

	/* 1. Deep Measure Phase (cx, cy) */
	float cx = 190.0f; // Minimal content width (230 total - 40 side padding)
	float cy = 0.0f;

	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST || iStyle == DialogStyle::TABLIST_HEADERS)
	{
		pActive->performLayout();
		if (!pActive->children().empty()) {
			Widget* pPanel = pActive->children()[0];
			float fMinContentWidth = 360.0f;

			// Add 20px extra to content width to create internal right padding (PC Style)
			cx = ImMax(fMinContentWidth, pPanel->width() + 20.0f);
			cy = pPanel->height();
		}
	}
	else if (iStyle == DialogStyle::INPUT || iStyle == DialogStyle::PASSWORD)
	{
		if (pActive->children().size() >= 2) {
			Widget* pMsgBox = pActive->children()[0];
			if (!pMsgBox->children().empty()) {
				Widget* pLabel = pMsgBox->children()[0];
				pLabel->performLayout();
				cx = ImMax(cx, pLabel->width());
				cy = pLabel->height() + 40.0f + 10.0f; // InputHeight(40) + Gap(10)
			}
		}
	}
	else // MSGBOX
	{
		if (!pActive->children().empty()) {
			Widget* pLabel = pActive->children()[0];
			pLabel->performLayout();
			cx = ImMax(cx, pLabel->width());
			cy = pLabel->height();
		}
	}

	/* 2. PC Dimension Values (1:1 CDialog.cpp) */
	float fHeaderHeight = 20.0f;    // PC Caption height (approx size.cy + 4)
	float fSidePadding = 40.0f;     // Default side padding (20px left + 20px right)
	float fContentX = 20.0f;        // Always 20px from left for symmetry

	float fStaticHeaderHeight = 0.0f;
	float fColumnGap = 0.0f;
	if (iStyle == DialogStyle::TABLIST_HEADERS) {
		fStaticHeaderHeight = UISettings::dialogListItemHeight(); // 18.0f
		fColumnGap = 15.0f;
	}

	float fMaxListHeight = 380.0f;
	float fMinListHeight = 0.0f;
	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST || iStyle == DialogStyle::TABLIST_HEADERS) {
		fMinListHeight = 230.0f; // PC Default height 300 - 70 overhead
	}

	float fListHeight = ImMin(ImMax(cy, fMinListHeight), fMaxListHeight);
	float fClientWidth = cx;

	// PC Total Height Logic:
	// Caption 20 + TopMargin 0 + MidGap 10 + ButtonArea 30 + BottomPadding 5 + GapToButtons 20 = 85px overhead
	float fClientHeight = fListHeight + fStaticHeaderHeight + fColumnGap;
	m_fWidth = fClientWidth + fSidePadding;
	m_fHeight = fClientHeight + 85.0f;

	this->setSize(ImVec2(m_fWidth, m_fHeight));

	/* 3. Positioning Components */
	m_pTitle->setFixedSize(ImVec2(m_fWidth, fHeaderHeight));
	m_pTitle->performLayout();
	m_pTitle->setPosition(ImVec2(0.0f, 0.0f));

	float fContainerWidth = fClientWidth;

	if (iStyle == DialogStyle::INPUT || iStyle == DialogStyle::PASSWORD) {
		fContentX = 5.0f;
		fContainerWidth = fClientWidth + 30.0f;
	}

	m_pContent->setFixedSize(ImVec2(fContainerWidth, fClientHeight));
	m_pContent->performLayout();

	float fContentY = fHeaderHeight + 10.0f;
	if (iStyle == DialogStyle::TABLIST_HEADERS) {
		fContentY = fHeaderHeight; // Gap 0 from header to static title
	}
	m_pContent->setPosition(ImVec2(fContentX, fContentY));

	m_pButton->setFixedSize(ImVec2(m_fWidth, 30.0f));
	m_pButton->performLayout();
	m_pButton->setPosition(ImVec2(0.0f, m_fHeight - 35.0f));

	this->setPosition(ImVec2((parent()->width() - width()) / 2, (parent()->height() - height()) / 2));
}

void CDialog::draw(ImGuiRenderer* renderer)
{
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), UISettings::dialogBackgroundColor(), true);
	Widget::draw(renderer);
}

void CDialog::touchEvent(const ImVec2& pos, TouchType type)
{
	if (m_pButton->contains(pos)) {
		m_pButton->touchEvent(pos, type);
		return;
	}
	Widget::touchEvent(pos, type);
}

/* CDialogTitle Class Implementation */

CDialogTitle::CDialogTitle()
{
	m_pLabel = new Label("Title", ImColor(1.0f, 1.0f, 1.0f), false, UISettings::fontSize());
	this->addChild(m_pLabel);
}

void CDialogTitle::SetTitle(const std::string& szTitle) { m_pLabel->setText(szTitle); }

void CDialogTitle::performLayout()
{
	m_pLabel->performLayout();
	m_pLabel->setPosition(ImVec2(10.0f, (height() - m_pLabel->height()) / 2));
}

void CDialogTitle::draw(ImGuiRenderer* renderer)
{
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), UISettings::dialogTitleBackgroundColor(), true);
	Widget::draw(renderer);
}

/* CDialogButton Class Implementation */

CDialogButton::CDialogButton() : Layout(Orientation::HORIZONTAL)
{
	m_pButton1 = new CDialogButton1();
	m_pButton1->setFixedSize(UISettings::dialogButtonSize());
	this->addChild(m_pButton1);

	m_pButton2 = new CDialogButton2();
	m_pButton2->setFixedSize(UISettings::dialogButtonSize());
	this->addChild(m_pButton2);
}

void CDialogButton::SetButton1Caption(const std::string& szCaption) { m_pButton1->setCaption(szCaption); }
void CDialogButton::SetButton2Caption(const std::string& szCaption) { m_pButton2->setCaption(szCaption); }

void CDialogButton::performLayout()
{
	m_pButton1->performLayout();
	m_pButton2->performLayout();

	float fCenterX = width() / 2.0f;
	float fButtonY = (height() - m_pButton1->height()) / 2.0f;

	if (m_pButton2->visible())
	{
		// EXACT SAMP PC POSITIONING (from CDialog.cpp UpdateLayout)
		// Button 1: (Width / 2) - (ButtonWidth - 10)
		// Button 2: (Width / 2) + 10
		m_pButton1->setPosition(ImVec2(fCenterX - 110.0f, fButtonY));
		m_pButton2->setPosition(ImVec2(fCenterX + 10.0f, fButtonY));
	}
	else
	{
		// Single button perfectly centered
		m_pButton1->setPosition(ImVec2(fCenterX - 50.0f, fButtonY));
	}
}

void CDialogButton::CDialogButton1::touchPopEvent()
{
	CDialog* pDialog = dynamic_cast<CDialog*>(this->parent()->parent());
	if (pNetGame) pNetGame->SendDialogResponse(pDialog->GetID(), 1, pDialog->GetContent()->GetListItem(), pDialog->GetContent()->GetInputString().c_str());
	pDialog->Hide();
}

void CDialogButton::CDialogButton2::touchPopEvent()
{
	CDialog* pDialog = dynamic_cast<CDialog*>(this->parent()->parent());
	if (pNetGame) pNetGame->SendDialogResponse(pDialog->GetID(), 0, pDialog->GetContent()->GetListItem(), pDialog->GetContent()->GetInputString().c_str());
	pDialog->Hide();
}

CDialogButton::CDialogButton1::CDialogButton1() : Button("", UISettings::fontSize() + 2.0f) {}
CDialogButton::CDialogButton2::CDialogButton2() : Button("", UISettings::fontSize() + 2.0f) {}
