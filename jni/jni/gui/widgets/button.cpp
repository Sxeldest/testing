#include "../../main.h"
#include "..//gui.h"
#include "button.h"

extern bool OpenButton;
extern bool isOpen;

RwTexture* Button::m_guiTexture = nullptr;

Button::Button(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();

	if (!m_guiTexture)
		m_guiTexture = (RwTexture*)LoadTextureFromDB("samp", "sampgui");
}

void Button::performLayout()
{
	float padding = UISettings::padding();

	m_label->performLayout();

	// Gunakan tinggi minimal 30.0f sesuai standar PC agar tidak gepeng
	float btnHeight = ImMax(30.0f, m_label->size().y + (padding / 2 * 2));
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	m_label->setPosition((size() - m_label->size()) / 2);
}

void Button::draw(ImGuiRenderer* renderer)
{
	if (m_guiTexture)
	{
		float tw = (float)m_guiTexture->raster->width;
		float th = (float)m_guiTexture->raster->height;

		// Menggunakan warna full (Alpha 255) agar line terlihat jelas dan tebal
		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 255);
		m_label->setColor(focused() ? m_colorFocused : ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		// Menggunakan metode simple stretch (Gaya PC) agar line stabil
		// Texture (0, 0, 136, 54) ditarik mengikuti size widget
		renderer->drawImageUV(p, p + s, ImVec2(0.0f/tw, 0.5f/th), ImVec2(136.0f/tw, 54.5f/th), (ImTextureID)m_guiTexture->raster, currentTint);

		if (focused())
		{
			// Layer fill transparan gelap saat ditekan
			renderer->drawImageUV(p, p + s, ImVec2(136.0f/tw, 0.5f/th), ImVec2(272.0f/tw, 54.5f/th), (ImTextureID)m_guiTexture->raster, ImColor(0, 0, 0, 60));
		}
	}

	Widget::draw(renderer);
}

void Button::touchPopEvent()
{
	if (m_callback) m_callback();
}


//============== Custom Button=========================//
CButton::CButton(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();

	if (!Button::m_guiTexture)
		Button::m_guiTexture = (RwTexture*)LoadTextureFromDB("samp", "sampgui");
}

void CButton::performLayout()
{
	float padding = UISettings::padding();

	m_label->performLayout();
	float btnHeight = ImMax(30.0f, m_label->size().y + (padding / 2 * 2));
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	m_label->setPosition((size() - m_label->size()) / 2);
}

void CButton::draw(ImGuiRenderer* renderer)
{
	if (OpenButton == false) return;

	if (Button::m_guiTexture)
	{
		float tw = (float)Button::m_guiTexture->raster->width;
		float th = (float)Button::m_guiTexture->raster->height;

		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 255);
		m_label->setColor(focused() ? m_colorFocused : ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		renderer->drawImageUV(p, p + s, ImVec2(0.0f/tw, 0.0f/th), ImVec2(136.0f/tw, 54.0f/th), (ImTextureID)Button::m_guiTexture->raster, currentTint);

		if (focused())
		{
			renderer->drawImageUV(p, p + s, ImVec2(136.0f/tw, 0.0f/th), ImVec2(272.0f/tw, 54.0f/th), (ImTextureID)Button::m_guiTexture->raster, ImColor(0, 0, 0, 60));
		}
	}

	Widget::draw(renderer);
}

void CButton::touchPopEvent()
{
	if (m_callback) m_callback();
}
//=======================================//

//======== >> Button ====================//
OButton::OButton(const std::string& caption, float font_size)
{
	m_callback = nullptr;

	m_label = new Label(caption, ImColor(1.0f, 1.0f, 1.0f), false, font_size);
	this->addChild(m_label);

	m_color = UISettings::buttonColor();
	m_colorFocused = UISettings::buttonFocusedColor();

	if (!Button::m_guiTexture)
		Button::m_guiTexture = (RwTexture*)LoadTextureFromDB("samp", "sampgui");
}

void OButton::performLayout()
{
	float padding = UISettings::padding();

	m_label->performLayout();
	float btnHeight = ImMax(30.0f, m_label->size().y + (padding / 2 * 2));
	this->setSize(ImVec2(m_label->size().x + padding * 2, btnHeight));

	m_label->setPosition((size() - m_label->size()) / 2);
}

void OButton::draw(ImGuiRenderer* renderer)
{
	if (OpenButton == true)
	{
		//Set >> to hide position
		this->setPosition(ImVec2(-150.0f, -150.0f));
		return;
	}

	if (Button::m_guiTexture)
	{
		float tw = (float)Button::m_guiTexture->raster->width;
		float th = (float)Button::m_guiTexture->raster->height;

		ImColor currentTint = focused() ? m_colorFocused : ImColor(255, 255, 255, 255);
		m_label->setColor(focused() ? m_colorFocused : ImColor(255, 255, 255, 255));

		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		renderer->drawImageUV(p, p + s, ImVec2(0.0f/tw, 0.0f/th), ImVec2(136.0f/tw, 54.0f/th), (ImTextureID)Button::m_guiTexture->raster, currentTint);

		if (focused())
		{
			renderer->drawImageUV(p, p + s, ImVec2(136.0f/tw, 0.0f/th), ImVec2(272.0f/tw, 54.0f/th), (ImTextureID)Button::m_guiTexture->raster, ImColor(0, 0, 0, 60));
		}
	}

	Widget::draw(renderer);

	//Set >> to position
	this->setPosition(ImVec2(15.0f, 15.0f));
}

void OButton::touchPopEvent()
{
	if (m_callback) m_callback();
}
//======================================//
