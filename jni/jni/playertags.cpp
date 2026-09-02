#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"

extern CGame* pGame;
extern CNetGame* pNetGame;

CPlayerTags::CPlayerTags() {}
CPlayerTags::~CPlayerTags() {}

void CPlayerTags::Render(ImGuiRenderer* renderer)
{
	if (!pNetGame || !pNetGame->m_pNetSet->bShowNameTags) return;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (!pPlayerPool) return;

	for (PLAYERID i = 0; i < MAX_PLAYERS; i++)
	{
		if (!pPlayerPool->GetSlotState(i)) continue;

		CRemotePlayer* pRemotePlayer = pPlayerPool->GetAt(i);
		if (!pRemotePlayer || !pRemotePlayer->IsActive() || !pRemotePlayer->m_bShowNameTag || pRemotePlayer->IsNPC()) continue;

		CPlayerPed* pPlayerPed = pRemotePlayer->GetPlayerPed();
		if (!pPlayerPed || !pPlayerPed->IsAdded()) continue;

		float fDist = pPlayerPed->GetDistanceFromCamera();
		if (fDist > pNetGame->m_pNetSet->fNameTagDrawDistance) continue;
		if (fDist > 50.0f) continue;

		VECTOR vecPos;
		pPlayerPed->GetBonePosition(8, &vecPos);
		vecPos.Z += 0.25f + (fDist * 0.025f);

		this->Draw(renderer, i, pRemotePlayer, &vecPos, fDist);
	}
}

void CPlayerTags::Draw(ImGuiRenderer* renderer, PLAYERID playerId, CRemotePlayer* pPlayer, VECTOR* pos, float fDist)
{
	if (pNetGame->m_pNetSet->bNameTagLOS)
	{
		VECTOR camPos;
		camPos.X = *(float*)(g_libGTASA + 0x9528D4);
		camPos.Y = *(float*)(g_libGTASA + 0x9528D8);
		camPos.Z = *(float*)(g_libGTASA + 0x9528DC);

		bool bClear = ((bool (*)(VECTOR*, VECTOR*, bool, bool, bool, bool, bool, bool, bool)) (g_libGTASA + 0x423418 + 1))(&camPos, pos, true, true, false, true, true, false, false);

		if (!bClear) return;
	}

	VECTOR Out;
	bool bVisible = ((bool (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(g_libGTASA + 0x5C5798 + 1))(pos, &Out, nullptr, nullptr, false, false);

	if (!bVisible || Out.Z < 1.0f) return;

	Out.X = (float)((int)Out.X);
	Out.Y = (float)((int)Out.Y);

	float fontSize = UISettings::fontSize() * 0.875f;

	char szTag[64];
	sprintf(szTag, "%s (%d)", pNetGame->GetPlayerPool()->GetPlayerName(playerId), playerId);

	ImVec2 textSize = renderer->calculateTextSize(szTag, fontSize);
	ImVec2 textPos = ImVec2(Out.X - (textSize.x * 0.5f), Out.Y - textSize.y);

	renderer->drawText(textPos, UI::fixcolor(pPlayer->GetPlayerColor()), szTag, true, fontSize);

	ImColor colorHealthBar    = ImColor(185, 34, 40, 255);
	ImColor colorHealthBarBG  = ImColor(75, 11, 20, 255);
	ImColor colorArmourBar    = ImColor(200, 200, 200, 255);
	ImColor colorArmourBarBG  = ImColor(40, 40, 40, 255);
	ImColor colorBorder       = ImColor(0, 0, 0, 255);

	float innerWidth   = 38.0f;
	float barHeight    = 4.0f;

	ImVec2 barPos = ImVec2(Out.X - 19.0f, (float)((int)(Out.Y + 3.0f)));

	float health = pPlayer->m_fReportedHealth;
	float armour = pPlayer->m_fReportedArmour;
	float healthOffset = (armour > 0.0f) ? 8.0f : 0.0f;

	if (armour > 0.0f) {
		if (armour > 100.0f) armour = 100.0f;
		float aProgress = (armour / 100.0f) * innerWidth;

		renderer->drawRect(
				ImVec2(barPos.x - 1.0f, barPos.y - 1.0f),
				ImVec2(barPos.x + innerWidth + 2.0f, barPos.y + barHeight + 1.0f),
				colorBorder, true);

		renderer->drawRect(barPos, ImVec2(barPos.x + innerWidth, barPos.y + barHeight), colorArmourBarBG, true);
		renderer->drawRect(barPos, ImVec2(barPos.x + aProgress, barPos.y + barHeight), colorArmourBar, true);
	}

	ImVec2 hBarPos = ImVec2(barPos.x, barPos.y + healthOffset);

	renderer->drawRect(
			ImVec2(hBarPos.x - 1.0f, hBarPos.y - 1.0f),
			ImVec2(hBarPos.x + innerWidth + 2.0f, hBarPos.y + barHeight + 1.0f),
			colorBorder, true);

	renderer->drawRect(hBarPos, ImVec2(hBarPos.x + innerWidth, hBarPos.y + barHeight), colorHealthBarBG, true);

	if (health > 0.0f) {
		if (health > 100.0f) health = 100.0f;
		float hProgress = (health / 100.0f) * innerWidth;
		renderer->drawRect(hBarPos, ImVec2(hBarPos.x + hProgress, hBarPos.y + barHeight), colorHealthBar, true);
	}
}
