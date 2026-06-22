#include "../../gui.h"

InputWidget::InputWidget()
{
	m_msgbox = new MsgBoxWidget();
	this->addChild(m_msgbox);

	m_editbox = new EditBox();
	this->addChild(m_editbox);
}

void InputWidget::performLayout()
{
	float editBoxHeight = 40.0f; // PC Standard + 2px height boost

	// Measure the label inside the msgbox
	if (m_msgbox->children().empty()) return;
	Widget* label = m_msgbox->children()[0];
	label->performLayout();

	// PC Style: Message has more side padding (20px total) than EditBox (5px total)
	// InputWidget is placed at x=5 in Dialog, so we indent message by another 15px.
	float messageIndent = 15.0f;
	float messageWidth = this->width() - (messageIndent * 2.0f);
	float contentHeight = label->height();

	// Set msgbox (ScrollPanel) width to be indented
	m_msgbox->setFixedSize(ImVec2(messageWidth, contentHeight));
	m_msgbox->performLayout();

	// Editbox takes full container width (so it stays at 5px from dialog edge)
	m_editbox->setSize(ImVec2(this->width(), editBoxHeight));
	m_editbox->performLayout();

	m_msgbox->setPosition(ImVec2(messageIndent, 0.0f));
	m_editbox->setPosition(ImVec2(0.0f, contentHeight + 10.0f));
}

void InputWidget::setMessage(const std::string& message) { m_msgbox->setMessage(message); }
const std::string& InputWidget::inputString() const { return m_editbox->text(); }
