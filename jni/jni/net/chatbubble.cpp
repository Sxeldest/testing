#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../game/RW/RenderWare.h"
#include "../gui/gui.h"
#include "chatbubble.h"
#include <vector>

extern CNetGame *pNetGame;

int WrapText(char* szText, int iWidth, int iMaxWord)
{
    int s, i, r, w, c;
    char buf[512];

    r = strlen(szText);
    w = iWidth;
    i = 0;
    c = 1;

    memset(buf, 0, sizeof(buf));

    if(r <= (int)sizeof(buf))
    {
        while(r > iWidth)
        {
            s = iWidth;

            while(s != 0 && szText[i + s] != ' ')
                --s;

            if(w - i - s <= iMaxWord)
            {
                i += s;
                szText[i] = '\n';
                r -= s;
                w = i + iWidth;
            }
            else
            {
                strcpy(buf, szText + w);
                szText[w + 1] = 0;
                szText[w] = '\n';
                memcpy(szText + strlen(szText), buf, strlen(buf) + 1);
                i = w + 1;
                r -= iWidth;
                w += 1 + iWidth;
            }

            c++;
        }

        return c;
    }

    return -1;
}

ChatBubble::ChatBubble()
{
    for(int i = 0; i < MAX_PLAYERS; i++)
    {
        m_pPlayerBubble[i] = nullptr;
        m_bSlotState[i] = false;
    }
}

ChatBubble::~ChatBubble()
{
    for(int i = 0; i < MAX_PLAYERS; i++)
    {
        if(m_bSlotState[i] && m_pPlayerBubble[i])
        {
            delete m_pPlayerBubble[i];
        }

        m_pPlayerBubble[i] = nullptr;
        m_bSlotState[i] = false;
    }
}

void ChatBubble::New(PLAYERID playerId, const char *text, uint32_t color, float distance, uint32_t time)
{
    if (playerId >= MAX_PLAYERS) return;

    if(m_bSlotState[playerId] && m_pPlayerBubble[playerId])
    {
        delete m_pPlayerBubble[playerId];
    }

    PlayerBubbleStruct* playerBubbleStruct = new PlayerBubbleStruct;
    std::string GText = Encoding::cp2utf(text);
    strncpy(playerBubbleStruct->szText, GText.c_str(), 255);
    playerBubbleStruct->szText[255] = '\0';

    playerBubbleStruct->fDistance = distance;
    playerBubbleStruct->uiExpireTime = GetTickCount() + time;
    playerBubbleStruct->uiColor = color;
    WrapText(playerBubbleStruct->szText, 36, 12);

    m_pPlayerBubble[playerId] = playerBubbleStruct;
    m_bSlotState[playerId] = true;
}

void ChatBubble::Delete(PLAYERID playerId)
{
    if (playerId >= MAX_PLAYERS) return;

    if(m_bSlotState[playerId] && m_pPlayerBubble[playerId])
    {
        delete m_pPlayerBubble[playerId];
        m_pPlayerBubble[playerId] = nullptr;
        m_bSlotState[playerId] = false;
    }
}

void ChatBubble::Render(ImGuiRenderer* renderer)
{
    if (!pNetGame) return;

    CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
    if (!pPlayerPool) return;

    uint32_t now = GetTickCount();

    for(PLAYERID i = 0; i < MAX_PLAYERS; i++)
    {
        if(!m_bSlotState[i] || !m_pPlayerBubble[i]) continue;

        if(now >= m_pPlayerBubble[i]->uiExpireTime)
        {
            Delete(i);
            continue;
        }

        CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(i);
        if (!pRemotePlayer || !pRemotePlayer->IsActive()) continue;

        CPlayerPed *pPlayerPed = pRemotePlayer->GetPlayerPed();
        if (!pPlayerPed || !pPlayerPed->IsAdded()) continue;

        float fDist = pPlayerPed->GetDistanceFromCamera();
        if (fDist > m_pPlayerBubble[i]->fDistance || fDist > 50.0f) continue;

        VECTOR headPos;
        pPlayerPed->GetBonePosition(8, &headPos);

        float fVerticalOffset = 0.20f + (fDist * 0.055f);
        headPos.Z += fVerticalOffset;

        VECTOR out;
        bool bVisible = ((bool (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(g_libGTASA + 0x5C5798 + 1))(&headPos, &out, nullptr, nullptr, false, false);

        if (bVisible && out.Z >= 1.0f) {
            out.X = (float)((int)out.X);
            out.Y = (float)((int)out.Y);

            std::string text = m_pPlayerBubble[i]->szText;
            uint32_t dwColor = __builtin_bswap32(m_pPlayerBubble[i]->uiColor | 0x000000FF);
            ImColor finalColor = dwColor;

            float fontSize = UISettings::fontSize() * 0.875f;

            std::vector<std::string> lines;
            size_t start_pos = 0, end_pos;
            while ((end_pos = text.find('\n', start_pos)) != std::string::npos) {
                lines.push_back(text.substr(start_pos, end_pos - start_pos));
                start_pos = end_pos + 1;
            }
            lines.push_back(text.substr(start_pos));
            ImVec2 totalSize = renderer->calculateTextSize(text, fontSize);
            float currentY = out.Y - totalSize.y;

            for (const auto& line : lines) {
                if (line.empty()) {
                    currentY += fontSize;
                    continue;
                }
                ImVec2 lineSize = renderer->calculateTextSize(line, fontSize);
                ImVec2 linePos = ImVec2(out.X - (lineSize.x * 0.5f), (float)((int)currentY));
                renderer->drawText(linePos, finalColor, line, true, fontSize);
                currentY += fontSize;
            }
        }
    }
}
