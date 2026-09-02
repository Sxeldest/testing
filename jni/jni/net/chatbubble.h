#pragma once

#include "../game/common.h"
#include <string>

typedef unsigned short PLAYERID;
class ImGuiRenderer;

struct PlayerBubbleStruct
{
    char szText[256];
    float fDistance;
    uint32_t uiExpireTime;
    uint32_t dwLineCount;
    uint32_t uiColor;
};

class ChatBubble
{
public:
    ChatBubble();
    ~ChatBubble();

    void New(PLAYERID playerId, const char *text, uint32_t color, float distance, uint32_t time);
    void Delete(PLAYERID playerId);

    void Render(ImGuiRenderer* renderer);

private:
    PlayerBubbleStruct *m_pPlayerBubble[MAX_PLAYERS];
    bool m_bSlotState[MAX_PLAYERS];
};
