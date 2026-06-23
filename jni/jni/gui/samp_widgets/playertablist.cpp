#include "../gui.h"
#include "../../game/game.h"
#include "../../net/netgame.h"
#include "../../main.h"

#include <iomanip>

extern UI* pUI;
extern CGame* pGame;
extern CNetGame* pNetGame;

PlayerTabList::PlayerTabList()
{
	m_tabList = nullptr;
	m_header = nullptr;
	m_lServerName = nullptr;
	m_lTotalPlayers = nullptr;
	setVisible(false);
}

void PlayerTabList::show()
{
	if (visible()) return;
	if (!pNetGame) return;

	pNetGame->UpdatePlayerScoresAndPings();

	float w = UISettings::scoreboardSize().x;

	// PC Ratios from samppc/client/scoreboard.cpp
	float x_id = 10.0f;
	float x_name = x_id + (0.085f * w);
	float x_score = x_name + (0.4375f * w);
	float x_ping = x_score + (0.265625f * w);

	if (!m_lServerName) {
		m_lServerName = new Label("", ImColor(0xBE, 0xBE, 0xBE), true, UISettings::smallFontSize());
		this->addChild(m_lServerName);
	}

	if (!m_lTotalPlayers) {
		m_lTotalPlayers = new Label("", ImColor(0xBE, 0xBE, 0xBE), true, UISettings::smallFontSize());
		this->addChild(m_lTotalPlayers);
	}

	if (!m_header) {
		m_header = new TabListWidget::ItemWidget(false);
		this->addChild(m_header);

		// Use PC Ratios for Header
		m_header->add(new Label("id", ImColor(0x95, 0xB0, 0xD0), true, UISettings::smallFontSize()), x_id);
		m_header->add(new Label("name", ImColor(0x95, 0xB0, 0xD0), true, UISettings::smallFontSize()), x_name);
		m_header->add(new Label("score", ImColor(0x95, 0xB0, 0xD0), true, UISettings::smallFontSize()), x_score);
		m_header->add(new Label("ping", ImColor(0x95, 0xB0, 0xD0), true, UISettings::smallFontSize()), x_ping);
	}

	if (!m_tabList) {
		m_tabList = new TabListWidget();
		this->addChild(m_tabList);
	}

	this->setFixedSize(UISettings::scoreboardSize());

	// Assemble using the same offsets
	this->assemble();
	this->performLayout();

	pGame->EnableGameInput(false);
	pUI->keyboard()->setVisible(false);

	setVisible(true);
}

void PlayerTabList::hide()
{
	pGame->EnableGameInput(true);
	setVisible(false);
}

std::string int_to_hex(int val)
{
	char hex[20];
	sprintf(hex, "%02X", val);
	return std::string(hex);
}

void PlayerTabList::assemble()
{
	if (!m_tabList || !pNetGame) return;
	m_tabList->removeAllItems();

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool) return;

	CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();
	if (!pLocalPlayer) return;

	float w = UISettings::scoreboardSize().x;
	float x_id = 10.0f;
	float x_name = x_id + (0.085f * w);
	float x_score = x_name + (0.4375f * w);
	float x_ping = x_score + (0.265625f * w);

	m_lServerName->setText(Encoding::cp2utf(pNetGame->m_szHostName));
	m_lTotalPlayers->setText("Players: " + std::to_string(pPlayerPool->GetTotalPlayers()));

	std::stringstream data;
	/* local player */
	ImColor lc = UI::fixcolor(pLocalPlayer->GetPlayerColorAsRGBA());
	std::string localColorTag = "{" + int_to_hex((int)(lc.Value.x * 255.0f)) + int_to_hex((int)(lc.Value.y * 255.0f)) + int_to_hex((int)(lc.Value.z * 255.0f)) + "}";

	data << localColorTag << pPlayerPool->GetLocalPlayerID() << "\t" << localColorTag << pPlayerPool->GetLocalPlayerName() << "\t" << localColorTag << pPlayerPool->GetLocalPlayerScore() << "\t" << localColorTag << pPlayerPool->GetLocalPlayerPing() << "\n";

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (pPlayerPool->GetAt(i) && !pPlayerPool->IsPlayerNPC(i))
		{
			ImColor rc = UI::fixcolor(pPlayerPool->GetAt(i)->GetPlayerColor());
			std::string remoteColorTag = "{" + int_to_hex((int)(rc.Value.x * 255.0f)) + int_to_hex((int)(rc.Value.y * 255.0f)) + int_to_hex((int)(rc.Value.z * 255.0f)) + "}";

			data << remoteColorTag << i << "\t" << remoteColorTag << pPlayerPool->GetPlayerName(i) << "\t" << remoteColorTag << pPlayerPool->GetPlayerScore(i) << "\t" << remoteColorTag << pPlayerPool->GetPlayerPing(i) << "\n";
		}
	}

	m_tabList->assemble(data.str(), { x_id, x_name, x_score, x_ping }, false, UISettings::smallFontSize(), true);
}

void PlayerTabList::performLayout()
{
	if (!m_tabList || !m_header) return;

	float headerItemHeight = 25.0f; // Height of column titles bar

	m_lServerName->performLayout();
	m_lServerName->setPosition(ImVec2(5.0f, 5.0f));

	m_lTotalPlayers->performLayout();
	m_lTotalPlayers->setPosition(ImVec2(this->width() - m_lTotalPlayers->width() - 5.0f, 5.0f));

	// PC Style: Tightening the gap
	float headerY = 17.0f;
	m_header->setFixedSize(ImVec2(this->width(), headerItemHeight));
	m_header->performLayout();
	m_header->setPosition(ImVec2(0.0f, headerY));

	// Fix: Use setFixedSize so the ListBox has actual height to draw items
	float listY = 57.0f;
	float listH = this->height() - listY - 10.0f;
	m_tabList->setFixedSize(ImVec2(this->width(), listH));
	m_tabList->performLayout();
	m_tabList->setPosition(ImVec2(0.0f, listY));

	this->setPosition(ImVec2((parent()->width() - width()) / 2, (parent()->height() - height()) / 2));
}

void PlayerTabList::draw(ImGuiRenderer* renderer)
{
	// 1. Draw solid dark background for the whole Scoreboard (PC Style)
	renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(10, 10, 10, 150), true);

	// 2. Header (id, name, score, ping) background
	if (m_header) {
		ImVec2 hp = m_header->absolutePosition();
		ImVec2 hs = m_header->size();
		renderer->drawRect(hp, hp + hs, ImColor(15, 15, 15, 220), true);
	}

	Widget::draw(renderer);
}

void PlayerTabList::activateEvent(bool active)
{
	if (active == false) this->hide();
}
