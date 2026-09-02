#include "../../main.h"
#include "..//gui.h"
#include "button.h"

extern bool OpenButton;
extern bool isOpen;

Button::Button(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();
}

void Button::performLayout()
{
	float padding = UISettings::padding();
	m_label->performLayout();
	float btnHeight = 30.0f;
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	ImVec2 labelPos = (size() - m_label->size()) / 2;
	labelPos.y -= 1.5f;
	m_label->setPosition(labelPos);
}

void Button::draw(ImGuiRenderer* renderer)
{
	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 200);

		m_label->setColor(ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		ImVec4 rect = UI::rectButtonNormal;
		renderer->drawImageUV(p, p + s, ImVec2(rect.x/tw, rect.y/th), ImVec2(rect.z/tw, rect.w/th), (ImTextureID)UI::m_pSampGuiTexture->raster, currentTint);
	}

	Widget::draw(renderer);
}

void Button::touchPopEvent()
{
	if (m_callback) m_callback();
}

CButton::CButton(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();
}

void CButton::performLayout()
{
	float padding = UISettings::padding();

	m_label->performLayout();
	float btnHeight = 30.0f;
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	ImVec2 labelPos = (size() - m_label->size()) / 2;
	labelPos.y -= 1.5f;
	m_label->setPosition(labelPos);
}

void CButton::draw(ImGuiRenderer* renderer)
{
	if (OpenButton == false) return;

	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 200);
		m_label->setColor(ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		ImVec4 rect = UI::rectButtonNormal;
		renderer->drawImageUV(p, p + s, ImVec2(rect.x/tw, rect.y/th), ImVec2(rect.z/tw, rect.w/th), (ImTextureID)UI::m_pSampGuiTexture->raster, currentTint);
	}

	Widget::draw(renderer);
}

void CButton::touchPopEvent()
{
	if (m_callback) m_callback();
}

OButton::OButton(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();
}

void OButton::performLayout()
{
	float padding = UISettings::padding();

	m_label->performLayout();
	float btnHeight = 30.0f;
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	ImVec2 labelPos = (size() - m_label->size()) / 2;
	labelPos.y -= 1.5f;
	m_label->setPosition(labelPos);
}

void OButton::draw(ImGuiRenderer* renderer)
{
	if (OpenButton == true)
	{
		this->setPosition(ImVec2(-150.0f, -150.0f));
		return;
	}

	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 200);
		m_label->setColor(ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		ImVec4 rect = UI::rectButtonNormal;
		renderer->drawImageUV(p, p + s, ImVec2(rect.x/tw, rect.y/th), ImVec2(rect.z/tw, rect.w/th), (ImTextureID)UI::m_pSampGuiTexture->raster, currentTint);
	}

	Widget::draw(renderer);
	this->setPosition(ImVec2(15.0f, 15.0f));
}

void OButton::touchPopEvent() {
	if (m_callback) m_callback();
}
