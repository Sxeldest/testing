#include "main.h"
#include "../game/game.h"
#include "net/netgame.h"
#include "dialog.h"
#include "scoreboard.h"
#include "keyboard.h"
#include "buttons.h"
#include "../settings.h"

extern CKeyBoard *pKeyBoard;
extern CDialogWindow *pDialogWindow;
extern CGame *pGame;
extern CScoreBoard *pScoreBoard;
extern CPlayerPed *pPlayerPed;
extern CNetGame *pNetGame;
extern CGUI *pGUI;
extern CSettings *pSettings;

CButtons::CButtons() {

    m_bIsItemShow = true;
    m_bIsShow = true;
}

void CButtons::Render() 
{
    if (pKeyBoard->IsOpen() || pDialogWindow->m_bIsActive || !m_bIsShow) return;

     m_fButWidth = ImGui::CalcTextSize("QWER").x;
    m_fButHeight = ImGui::CalcTextSize("QWE").x;
		
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.5f, 4.5f));
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
	
	 ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 0.0f, 0.0f, 1.0f).Value);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(255.0f, 255.0f, 255.0f, 1.0f).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(255.0f, 255.0f, 255.0f, 1.0f).Value);

    CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
    if (pPlayerPed) {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("__extra_keyboard", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

        if (m_bIsItemShow) {
           if(ImGui::Button("<<", ImVec2(m_fButWidth, m_fButHeight)))
                m_bIsItemShow = !m_bIsItemShow;
                
         CTextDrawPool *pTextDrawPool = pNetGame->GetTextDrawPool();
        if(pTextDrawPool)
        {
            if(pTextDrawPool->GetState())
            {
                if(ImGui::Button("ESC", ImVec2(m_fButWidth, m_fButHeight)))
                {
                    if(pTextDrawPool->GetState())
                        pTextDrawPool->SetSelectState(true, 0);
                }
                
                ImGui::SameLine(0, 5);
            }
        }
        
        ImGui::SameLine(0, 5);
        if (!pScoreBoard->m_bToggle) {
           if(ImGui::Button("TAB", ImVec2(m_fButWidth, m_fButHeight)))
                pScoreBoard->Toggle();
        }
        else {
            if (ImGui::Button("X", ImVec2(m_fButWidth, m_fButHeight)))
                pScoreBoard->Toggle();

            m_bIsItemShow = true;
        }

            ImGui::SameLine(0, 5);

          if(ImGui::Button("ESC", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
            else
                  LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
                  
                  ImGui::SameLine(0, 5);

          if(ImGui::Button("ALT", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;

             ImGui::SameLine(0, 5);

           if(ImGui::Button("F", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;

            ImGui::SameLine(0, 5);

           if(ImGui::Button("H", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;
                
                ImGui::SameLine(0, 5);
                
                if(ImGui::Button("2", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;

            ImGui::SameLine(0, 5);

          if(ImGui::Button("Y", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;

            ImGui::SameLine(0, 5);

          if(ImGui::Button("N", ImVec2(m_fButWidth, m_fButHeight)))
                LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
            
            ImGui::SameLine(0, 5);
        }
        else {
           if(ImGui::Button(">>", ImVec2(m_fButWidth, m_fButHeight)))
                m_bIsItemShow = !m_bIsItemShow;
                }

        ImGui::SetWindowSize(ImVec2(-1, -1));
        ImGui::SetWindowPos(ImVec2(pGUI->ScaleX(10), pGUI->ScaleY(365)));
        ImGui::End();
        ImGui::PopStyleColor(2);
    }
}

void CButtons::ToggleState() 
{
    m_bIsShow = !m_bIsShow;
}