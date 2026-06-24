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
	float editBoxHeight = UISettings::dialogInputHeight();

	if (m_msgbox->children().empty()) return;
	Widget* label = m_msgbox->children()[0];
	label->performLayout();

	float messageIndent = 15.0f;
	float messageWidth = this->width();
	float contentHeight = label->height();

	m_msgbox->setFixedSize(ImVec2(messageWidth, contentHeight));
	m_msgbox->performLayout();

	m_editbox->setSize(ImVec2(this->width(), editBoxHeight));
	m_editbox->performLayout();

	m_msgbox->setPosition(ImVec2(messageIndent, 0.0f));
	m_editbox->setPosition(ImVec2(0.0f, contentHeight + 10.0f));
}

void InputWidget::setMessage(const std::string& message) { m_msgbox->setMessage(message); }
const std::string& InputWidget::inputString() const { return m_editbox->text(); }
