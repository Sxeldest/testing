#pragma once

#include "game/common.h"

class CRemotePlayer;
class ImGuiRenderer;

class CPlayerTags
{
public:
	CPlayerTags();
	~CPlayerTags();

	void Render(ImGuiRenderer* renderer);

private:
	void Draw(ImGuiRenderer* renderer, PLAYERID playerId, CRemotePlayer* pPlayer, VECTOR* pos, float fDist);
};
