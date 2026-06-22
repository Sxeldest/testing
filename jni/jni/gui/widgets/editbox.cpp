#include "../gui.h"

extern UI* pUI;

EditBox::EditBox()
{
	m_label = new Label(" ", ImColor(1.0f, 1.0f, 1.0f), false, UISettings::fontSize());//...
	this->addChild(m_label);
	m_passwordMode = false;
}

void EditBox::performLayout()
{
	m_label->performLayout();

	m_label->setPosition(ImVec2(
		UISettings::padding(),

		(height() - m_label->height()) / 2
	));
}

void EditBox::draw(ImGuiRenderer* renderer)
{
	// Draw background (PC: D3DCOLOR_ARGB(240, 5, 5, 5))
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(5, 5, 5, 240), true);

	// PC uses a thinner greyish border for editbox usually, or none.
	// We'll use a subtle border.
	renderer->drawRect(
		absolutePosition(),
		absolutePosition() + size(),
		ImColor(150, 150, 150, 100), false, 1.0f);

	Widget::draw(renderer);
}

void EditBox::touchPopEvent()
{
	pUI->keyboard()->show(this);
}

void EditBox::keyboardEvent(const std::string& input)
{
	m_input = input;
	if (m_passwordMode) {
		std::string masked(input.length(), '*');
		m_label->setText(masked);
	} else {
		m_label->setText(Encoding::cp2utf(input));
	}
}