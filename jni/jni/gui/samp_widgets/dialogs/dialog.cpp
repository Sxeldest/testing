#include "../../../main.h"
#include "../../gui.h"
#include "../../../game/game.h"
#include "../../../net/netgame.h"

extern CGame* pGame;
extern CNetGame* pNetGame;
extern UI* pUI;

/* CDialog Class */

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
		if (pUI && pUI->inputChat()->visible()) pUI->inputChat()->hide();

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

	float cx = 190.0f;
	float cy = 0.0f;

	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST || iStyle == DialogStyle::TABLIST_HEADERS)
	{
		pActive->performLayout();
		if (!pActive->children().empty()) {
			Widget* pPanel = pActive->children()[0];
			float fMinContentWidth = 360.0f;

			cx = ImMax(fMinContentWidth, pPanel->width() + 16.0f);
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
				cy = pLabel->height() + UISettings::dialogInputHeight() + 10.0f;
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

	float fHeaderHeight = 20.0f;
	float fSidePadding = 40.0f;
	float fContentX = 20.0f;

	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST || iStyle == DialogStyle::TABLIST_HEADERS) {
		fSidePadding = 20.0f;
		fContentX = 10.0f;
	}

	float fStaticHeaderHeight = 0.0f;
	float fColumnGap = 0.0f;
	if (iStyle == DialogStyle::TABLIST_HEADERS) {
		fStaticHeaderHeight = UISettings::dialogListItemHeight();
		fColumnGap = 15.0f;
	}

	float fMaxListHeight = 380.0f;
	float fMinListHeight = 0.0f;
	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST || iStyle == DialogStyle::TABLIST_HEADERS) {
		fMinListHeight = 230.0f;
	}

	float fListHeight = ImMin(ImMax(cy, fMinListHeight), fMaxListHeight);
	float fClientWidth = cx;
	float fClientHeight = fListHeight + fStaticHeaderHeight + fColumnGap;
	m_fWidth = fClientWidth + fSidePadding;

	float fHeightOverhead = 70.0f;
	if (iStyle == DialogStyle::LIST || iStyle == DialogStyle::TABLIST) {
		fHeightOverhead = 75.0f;
	} else if (iStyle == DialogStyle::TABLIST_HEADERS) {
		fHeightOverhead = 65.0f;
	}

	m_fHeight = fClientHeight + fHeightOverhead;

	this->setSize(ImVec2(m_fWidth, m_fHeight));

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
		fContentY = fHeaderHeight;
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

CDialogTitle::CDialogTitle()
{
	m_pLabel = new Label("Title", ImColor(1.0f, 1.0f, 1.0f), false, UISettings::fontSize() + 2.0f);
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
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(0, 0, 0, 255), true);
	Widget::draw(renderer);
}

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
		m_pButton1->setPosition(ImVec2(fCenterX - 110.0f, fButtonY));
		m_pButton2->setPosition(ImVec2(fCenterX + 10.0f, fButtonY));
	}
	else
	{
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

CDialogButton::CDialogButton1::CDialogButton1() : Button("", UISettings::fontSize() + 4.0f) {}
CDialogButton::CDialogButton2::CDialogButton2() : Button("", UISettings::fontSize() + 4.0f) {}
