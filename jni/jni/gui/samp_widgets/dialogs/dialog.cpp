#include "../../../main.h"
#include "../../gui.h"
#include "../../../game/game.h"
#include "../../../net/netgame.h"

extern CGame* pGame;
extern CNetGame* pNetGame;

/* Dialog */
Dialog::Dialog()
{
	m_title = new DialogTitle();
	this->addChild(m_title);

	m_content = new Content();
	this->addChild(m_content);

	m_buttonPanel = new DialogButtonPanel();
	this->addChild(m_buttonPanel);

	m_dialogID = -1;
}

static bool IsSAMPButtonEmpty(const std::string& str)
{
	if (str.empty()) return true;
	for (char c : str) { if ((unsigned char)c > 32) return false; }
	return true;
}

void Dialog::show(int16_t dialog_id, DialogStyle dialog_style, const std::string& title, const std::string& message, const std::string& button1, const std::string& button2)
{
	if (dialog_id >= 0) {
		this->setTitleCaption(title);
		this->setButton1Caption(button1);

		if (IsSAMPButtonEmpty(button2)) {
			m_buttonPanel->m_button2->setVisible(false);
		} else {
			this->setButton2Caption(button2);
			m_buttonPanel->m_button2->setVisible(true);
		}

		m_content->setActive(dialog_style, message);
		this->performLayout();
		this->setVisible(true);
		if (pGame) pGame->DisplayHUD(false);
		m_dialogID = dialog_id;
	}
}

void Dialog::hide()
{
	this->setVisible(false);
	if (pGame) {
		if (!pNetGame || !pNetGame->GetTextDrawPool() || !pNetGame->GetTextDrawPool()->GetState()) {
			pGame->DisplayHUD(true);
		}
	}
}

void Dialog::setTitleCaption(const std::string& caption) { m_title->setTitle(caption); }
void Dialog::setButton1Caption(const std::string& caption) { m_buttonPanel->setButton1Caption(caption); }
void Dialog::setButton2Caption(const std::string& caption) { m_buttonPanel->setButton2Caption(caption); }

void Dialog::performLayout()
{
	Widget* active = m_content->activeWidget();
	if (!active) return;

	/* 1. Deep Measure Phase */
	float preferredWidth = 230.0f; // Standard PC Min width (m_iWidth default check)
	float preferredHeight = 0.0f;
	DialogStyle style = m_content->activeStyle();

	if (style == DialogStyle::LIST || style == DialogStyle::TABLIST || style == DialogStyle::TABLIST_HEADERS) {
		active->performLayout();
		if (!active->children().empty()) {
			Widget* panel = active->children()[0];
			preferredWidth = ImMax(preferredWidth, panel->width());
			preferredHeight = panel->height();
		}
	} else if (style == DialogStyle::INPUT || style == DialogStyle::PASSWORD) {
		if (active->children().size() >= 2) {
			Widget* msgbox = active->children()[0];
			Widget* editbox = active->children()[1];
			if (!msgbox->children().empty()) {
				Widget* label = msgbox->children()[0];
				label->performLayout();
				preferredWidth = ImMax(preferredWidth, label->width());
				preferredHeight = label->height() + 40.0f + 10.0f; // InputHeight(40) + Gap(10)
			}
		}
	} else {
		// MSGBOX
		if (!active->children().empty()) {
			Widget* label = active->children()[0];
			label->performLayout();
			preferredWidth = ImMax(preferredWidth, label->width());
			preferredHeight = label->height();
		}
	}

	/* 2. PC EXACT VALUES (1:1 CDialog.cpp) */
	float headerHeight = 25.0f;    // Caption height
	float sidePadding = 40.0f;     // From: m_iWidth = cx + 40

	float staticHeaderHeight = 0.0f;
	float columnGap = 0.0f;
	if (style == DialogStyle::TABLIST_HEADERS) {
		staticHeaderHeight = UISettings::dialogListItemHeight(); // 18.0f
		columnGap = 15.0f; // Gap between static columns and list (Increased per feedback)
	}

	float maxListHeight = 350.0f;
	float listHeight = ImMin(preferredHeight, maxListHeight);
	float clientWidth = preferredWidth;

	// PC Total Height Logic Refined:
	// Caption 25 + TopMargin 0 + MidGap 10 + ButtonArea 30 + BottomPadding 5 = 70px overhead
	float clientHeight = listHeight + staticHeaderHeight + columnGap;
	float dialogWidth = clientWidth + sidePadding;
	float dialogHeight = clientHeight + 70.0f;

	this->setSize(ImVec2(dialogWidth, dialogHeight));

	/* 3. Position Components per PC Rules */
	m_title->setFixedSize(ImVec2(dialogWidth, headerHeight));
	m_title->performLayout();
	m_title->setPosition(ImVec2(0.0f, 0.0f));

	// x positioning logic from PC:
	// ListBox/TabList: SetLocation(10, 10) -> x = 10
	float contentX = 10.0f;
	float containerWidth = clientWidth + 20.0f;

	if (style == DialogStyle::INPUT || style == DialogStyle::PASSWORD) {
		contentX = 5.0f;
		containerWidth = clientWidth + 30.0f;
	} else if (style == DialogStyle::MSGBOX) {
		contentX = 20.0f;
		containerWidth = clientWidth;
	}

	m_content->setFixedSize(ImVec2(containerWidth, clientHeight));
	m_content->performLayout();
	// PC Precise: Caption(25) - 2px Offset to eliminate label internal padding = 23.0f
	m_content->setPosition(ImVec2(contentX, 23.0f));

	m_buttonPanel->setFixedSize(ImVec2(dialogWidth, 30.0f));
	m_buttonPanel->performLayout();
	// y = dialogHeight - ButtonArea(30) - BottomPadding(5) = Height - 35.0f
	m_buttonPanel->setPosition(ImVec2(0.0f, dialogHeight - 35.0f));

	this->setPosition(ImVec2((parent()->width() - width()) / 2, (parent()->height() - height()) / 2));
}

void Dialog::draw(ImGuiRenderer* renderer)
{
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), UISettings::dialogBackgroundColor(), true);
	Widget::draw(renderer);
}

void Dialog::touchEvent(const ImVec2& pos, TouchType type)
{
	if (m_buttonPanel->contains(pos)) {
		m_buttonPanel->touchEvent(pos, type);
		return;
	}
	Widget::touchEvent(pos, type);
}

DialogTitle::DialogTitle() {
	m_label = new Label("Title", ImColor(1.0f, 1.0f, 1.0f), false, UISettings::fontSize());
	this->addChild(m_label);
}
void DialogTitle::setTitle(const std::string& caption) { m_label->setText(caption); }
void DialogTitle::performLayout() {
	m_label->performLayout();
	m_label->setPosition(ImVec2(10.0f, (height() - m_label->height()) / 2));
}
void DialogTitle::draw(ImGuiRenderer* renderer) {
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), UISettings::dialogTitleBackgroundColor(), true);
	Widget::draw(renderer);
}

DialogButtonPanel::DialogButtonPanel() : Layout(Orientation::HORIZONTAL) {
	m_button1 = new DialogButton1();
	m_button1->setFixedSize(UISettings::dialogButtonPanelSize());
	this->addChild(m_button1);
	m_button2 = new DialogButton2();
	m_button2->setFixedSize(UISettings::dialogButtonPanelSize());
	this->addChild(m_button2);
}
void DialogButtonPanel::setButton1Caption(const std::string& caption) { m_button1->setCaption(caption); }
void DialogButtonPanel::setButton2Caption(const std::string& caption) { m_button2->setCaption(caption); }
void DialogButtonPanel::performLayout() {
	m_button1->performLayout();
	m_button2->performLayout();
	float centerX = width() / 2.0f;
	if (m_button2->visible()) {
		// PC Centered Logic: Button1 ends at -10, Button2 starts at +10 (20px gap)
		m_button1->setPosition(ImVec2(centerX - 110.0f, (height() - m_button1->height()) / 2.0f));
		m_button2->setPosition(ImVec2(centerX + 10.0f, (height() - m_button2->height()) / 2.0f));
	} else {
		// Single button centered (Width 100)
		m_button1->setPosition(ImVec2(centerX - 50.0f, (height() - m_button1->height()) / 2.0f));
	}
}

void DialogButtonPanel::DialogButton1::touchPopEvent() {
	Dialog* dialog = dynamic_cast<Dialog*>(this->parent()->parent());
	if (pNetGame) pNetGame->SendDialogResponse(dialog->dialogID(), 1, dialog->content()->listItem(), dialog->content()->inputString().c_str());
	dialog->hide();
}
void DialogButtonPanel::DialogButton2::touchPopEvent() {
	Dialog* dialog = dynamic_cast<Dialog*>(this->parent()->parent());
	if (pNetGame) pNetGame->SendDialogResponse(dialog->dialogID(), 0, dialog->content()->listItem(), dialog->content()->inputString().c_str());
	dialog->hide();
}
DialogButtonPanel::DialogButton1::DialogButton1() : Button("", UISettings::fontSize()) {}
DialogButtonPanel::DialogButton2::DialogButton2() : Button("", UISettings::fontSize()) {}
