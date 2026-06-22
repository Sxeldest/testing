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
	: ListBox()
{
	this->setClipping(false);
}

void Chat::addChatMessage(const std::string& message, const std::string& nick, const ImColor& nick_color)
{
	addPlayerMessage(message, nick, nick_color);
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
	int maxMessages = pSettings ? pSettings->Get().iChatMaxMessages : 10;
	if (maxMessages <= 0) maxMessages = 10;

	if (this->itemsCount() >= maxMessages)
	{
		this->removeItem(0);
	}

	MessageItem* item = new MessageItem(message, color);
	this->addItem(item);
	/*if(!active())*/ this->setScrollY(1.0f);
}

void Chat::addPlayerMessage(const std::string& message, const std::string& nick, const ImColor& nick_color)
{
	int maxMessages = pSettings ? pSettings->Get().iChatMaxMessages : 10;
	if (maxMessages <= 0) maxMessages = 10;

	if (this->itemsCount() >= maxMessages)
	{
		this->removeItem(0);
	}

	PlayerMessageItem* item = new PlayerMessageItem(message, nick, nick_color);
	this->addItem(item);
	/*if(!active())*/ this->setScrollY(1.0f);
}

void Chat::draw(ImGuiRenderer* renderer)
{
	if (pSettings)
	{
		// 1. Anchor tetap di fChatPosY (Atas)
		this->setPosition(ImVec2(pSettings->Get().fChatPosX, pSettings->Get().fChatPosY));

		float fontSize = pSettings->Get().fFontSize;
		int maxLines = pSettings->Get().iChatMaxMessages;
		if (maxLines <= 0) maxLines = 10;

		float sizeX = pSettings->Get().fChatSizeX;

		// 2. Tinggi kotak murni dihitung dari MaxLines * FontSize
		// Karena Anchor kita di Top (fChatPosY), maka jika MaxLines ditambah,
		// kotak akan memanjang ke arah BAWAH.
		float sizeY = fontSize * (float)maxLines;

		if (sizeX > 1.0f && sizeY > 1.0f) {
			this->setFixedSize(ImVec2(sizeX, sizeY));
		}
	}

	ListBox::draw(renderer);
}

void Chat::activateEvent(bool active)
{
	if (active)
	{
		this->setScrollable(true);
	}
	else
	{
		this->setScrollable(false);
	}
}

void Chat::touchPopEvent()
{
	if (pUI->playertablist()->visible()) return;

	pUI->keyboard()->show(this);
}

void Chat::keyboardEvent(const std::string& input)
{
	if (input.length() > 0 && pNetGame)
	{
		if (input[0] == '/') pNetGame->SendChatCommand(input.c_str());
		else pNetGame->SendChatMessage(input.c_str());
	}
}