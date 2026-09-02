#include "../gui.h"
#include "../../main.h"
#include "../../game/game.h"
#include "../../net/netgame.h"
#include <algorithm>
#include "../settings.h"
#include "java/jniutil.h"

extern UI* pUI;
extern CGame* pGame;
extern CNetGame* pNetGame;
extern CSettings* pSettings;
extern CJavaWrapper *pJavaWrapper;

Chat::Chat()
	: Widget()
{
	m_messages.clear();
	m_active = false;
	m_scrollIdx = 0;
}

void Chat::addChatMessage(const std::string& message, const std::string& nick, const ImColor& nick_color)
{
	std::string full = nick + ": " + message;
	addMessage(full, nick_color);
}

void Chat::addInfoMessage(const std::string& format, ...)
{
	char tmp_buf[512];

	va_list args;
	va_start(args, format);
	vsprintf(tmp_buf, format.c_str(), args);
	va_end(args);

	addMessage(std::string(tmp_buf), ImColor(0x00, 0xc8, 0xc8));
}

void Chat::addDebugMessage(const std::string& format, ...)
{
	char tmp_buf[512];

	va_list args;
	va_start(args, format);
	vsprintf(tmp_buf, format.c_str(), args);
	va_end(args);

	addMessage(std::string(tmp_buf), ImColor(0xbe, 0xbe, 0xbe));
}

void Chat::addClientMessage(const std::string& message, const ImColor& color)
{
	addMessage(message, color);
}

void Chat::addMessage(const std::string& message, const ImColor& color)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	ChatEntry entry;
	entry.message = message;
	entry.color = color;

	m_messages.push_back(entry);
	if (m_messages.size() > MAX_MESSAGES) {
		m_messages.pop_front();
	}

	if (m_scrollIdx > 0) m_scrollIdx++;
	if (m_scrollIdx >= (int)m_messages.size()) m_scrollIdx = (int)m_messages.size() - 1;
}

void Chat::draw(ImGuiRenderer* renderer)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_messages.empty()) return;

	float fontSize = UISettings::fontSize();
	ImVec2 pos = absolutePosition();
	float x = pos.x;
	float y = pos.y;
	float line_height = fontSize + 1.0f;

	int max_messages = UISettings::chatDispMessages();
	int total = (int)m_messages.size();
	int start = total - max_messages - m_scrollIdx;
	if (start < 0) start = 0;
	int end = start + max_messages;
	if (end > total) end = total;

	for (int i = start; i < end; ++i) {
		const auto& entry = m_messages[i];
		renderer->drawText(ImVec2(x, y), entry.color, entry.message, true, fontSize, nullptr, true);
		y += line_height;
	}

	if (m_active && UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		float sbWidth = 20.0f;
		ImVec2 sbPos = ImVec2(10.0f, pos.y + 40.0f);
		float sbHeight = (max_messages * line_height) - 60.0f;
		if (sbHeight < 40.0f) sbHeight = 40.0f;

		ImVec4 rectTrack = UI::rectScrollTrack;
		renderer->drawImageUV(sbPos, sbPos + ImVec2(sbWidth, sbHeight),
			ImVec2(rectTrack.x / tw, rectTrack.y / th),
			ImVec2(rectTrack.z / tw, rectTrack.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		float arrowH = 16.0f;
		ImVec4 rectUp = UI::rectScrollUpArrow;
		renderer->drawImageUV(sbPos, sbPos + ImVec2(sbWidth, arrowH),
			ImVec2(rectUp.x / tw, rectUp.y / th),
			ImVec2(rectUp.z / tw, rectUp.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		ImVec4 rectDown = UI::rectScrollDownArrow;
		renderer->drawImageUV(sbPos + ImVec2(0.0f, sbHeight - arrowH), sbPos + ImVec2(sbWidth, sbHeight),
			ImVec2(rectDown.x / tw, rectDown.y / th),
			ImVec2(rectDown.z / tw, rectDown.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		if (total > max_messages)
		{
			float trackH = sbHeight - (arrowH * 2.0f);
			float thumbH = ImMax(20.0f, ((float)max_messages / (float)total) * trackH);

			float scrollRatio = (float)m_scrollIdx / (float)(total - max_messages);
			scrollRatio = ImSaturate(scrollRatio);
			float thumbY = (sbHeight - arrowH - thumbH) - (scrollRatio * (trackH - thumbH));

			ImVec4 rectThumb = UI::rectScrollThumb;
			renderer->drawImageUV(sbPos + ImVec2(0.0f, thumbY), sbPos + ImVec2(sbWidth, thumbY + thumbH),
				ImVec2(rectThumb.x / tw, rectThumb.y / th),
				ImVec2(rectThumb.z / tw, rectThumb.w / th),
				(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(0xB9, 0x22, 0x28, 255));
		}
	}

	Widget::draw(renderer);
}

void Chat::performLayout()
{
	if (pSettings)
	{
		this->setPosition(ImVec2(pSettings->Get().fChatPosX, pSettings->Get().fChatPosY));

		float sizeX = pSettings->Get().fChatSizeX;
		if (sizeX < 420.0f) sizeX = 420.0f;

		float fontSize = pSettings->Get().fFontSize;
		int maxLines = pSettings->Get().iChatMaxMessages;
		if (maxLines <= 0) maxLines = 10;

		this->setSize(ImVec2(sizeX, (fontSize + 1.0f) * maxLines));
	}
}

void Chat::activateEvent(bool active)
{
	m_active = active;
	if (!active) m_scrollIdx = 0;
}

void Chat::touchEvent(const ImVec2& pos, TouchType type)
{
	static ImVec2 lastTouchPos;
	if (type == TouchType::push) {
		lastTouchPos = pos;
	}
	else if (m_active && type == TouchType::move) {
		float deltaY = pos.y - lastTouchPos.y;
		float step = UISettings::fontSize() + 1.0f;

		if (fabs(deltaY) >= step) {
			int lines = (int)(deltaY / step);
			m_scrollIdx += lines;

			int maxScroll = (int)m_messages.size() - UISettings::chatDispMessages();
			if (m_scrollIdx < 0) m_scrollIdx = 0;
			if (m_scrollIdx > maxScroll) m_scrollIdx = maxScroll;

			lastTouchPos = pos;
		}
	}

	Widget::touchEvent(pos, type);
}

void Chat::touchPopEvent()
{
	if (pUI->playertablist()->visible()) return;

	pUI->inputChat()->show(this);
}

void Chat::keyboardEvent(const std::string& input)
{
}

void Chat::onSubmit()
{
	std::string input = pUI->inputChat()->inputString();
	if (input.length() > 0 && pNetGame)
	{
		if (input[0] == '/') pNetGame->SendChatCommand(input.c_str());
		else pNetGame->SendChatMessage(input.c_str());
	}
}
