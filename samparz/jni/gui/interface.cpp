#include "../main.h"
#include "../settings.h"
#include "../game/game.h"
#include "../net/netgame.h"
#include "../deathmessage.h"
#include "../util/patch.h"
#include "../util/util.h"
#include "../game/vehicle.h"
#include "interface.h"
#include "gamescreen.h"
#include "../dialog.h"
#include "../vendor/imgui/imgui_internal.h"

#define IM_PI           3.14159265358979323846f
#define COLOR_ROSA		0xFFFF99FF

extern CGUI *pGUI;
extern CGame *pGame;
extern CVehicle *pVehicle;
extern CDeathMessage *pDeathMessage;
extern CNetGame *pNetGame;
extern CSettings *pSettings;
extern CGameScreen *pGameScreen;
extern CDialogWindow *pDialogWindow;

static int m_iContentId;

CInterface::CInterface() {
	
	Log("Interface");
    m_bRenderMenu = false;
	iMileage = 0;
    speedFont = pGUI->LoadFont("Nogspeed.ttf", 30);
	
    engineTex0 		= (RwTexture*)LoadTextureFromDB("samp","speed_engine_off");
    engineTex1		= (RwTexture*)LoadTextureFromDB("samp","speed_engine_on");
	btt				= (RwTexture*)LoadTextureFromDB("samp","btt");

    lockTex0		= (RwTexture*)LoadTextureFromDB("samp", "speed_key_off");
    lockTex1 		= (RwTexture*)LoadTextureFromDB("samp", "speed_key_on");

    lightTex0		= (RwTexture*)LoadTextureFromDB("samp", "speed_light_off");
    lightTex1 		= (RwTexture*)LoadTextureFromDB("samp", "speed_light_on");

    beltTex0 		= (RwTexture*)LoadTextureFromDB("samp", "speed_belt_off");
    beltTex1 		= (RwTexture*)LoadTextureFromDB("samp", "speed_belt_on");

    //arrowLTex   [OFF]       = (RwTexture*)LoadTextureFromDB("flame", "speed_arrow_l_off");
    //arrowLTex   [ON]        = (RwTexture*)LoadTextureFromDB("flame", "speed_arrow_l_on");

    //arrowRTex   [OFF]       = (RwTexture*)LoadTextureFromDB("flame", "speed_arrow_r_off");
    //arrowLTex   [ON]        = (RwTexture*)LoadTextureFromDB("speed_arrow_l_on");

    fuelTex   		        = (RwTexture*)LoadTextureFromDB("samp", "speed_fuel");
    healthTex               = (RwTexture*)LoadTextureFromDB("samp", "speed_health");   
}

CInterface::~CInterface() = default;

void CInterface::RenderMenu() {
    if (!m_bRenderMenu) return;
	
    SaveMenuSettings();
    pGUI->SetupDefaultStyle();

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("#menu", nullptr,
                 ImGuiWindowFlags_NoTitleBar
                 | ImGuiWindowFlags_NoResize
                 | ImGuiWindowFlags_NoSavedSettings
                 | ImGuiWindowFlags_NoScrollbar
                 | ImGuiWindowFlags_NoMove
    );

    ImGui::GetBackgroundDrawList()->AddRectFilled(
            ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y),
            ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetFontSize() * 2),
            COLOR_ROSA
    );

    ImGui::Text("Menu de personalizacao do apk");
	
	ImGui::Separator();

    ImGui::BeginChild("#__menu", ImVec2(-1, ImGui::GetWindowSize().y - ImGui::GetFontSize() * 4.7f), false);

    ImGui::Separator();


    /*ImGui::Text("Hud Botoes:");
	
    const char* s_items[] = { " Mostrar "," Ocultar " };
    static const char* s_item_current = s_items[0];
	
	if(s_item_current == s_items[0]) pExtraKeyBoard->Show(true);
    else if(s_item_current == s_items[1]) pExtraKeyBoard->Show(false);
	
	ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("#__hud_c", s_item_current))
    {
        for (auto & s_item : s_items)
        {
            bool is_selected = (s_item_current == s_item);
            if (ImGui::Selectable(s_item, is_selected))
                s_item_current = s_item;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }*/

    // --> HUD Selector --> --> --> --> -->
	/*,ImGui::Text("Botao cor");
	
    const char* items[] = { "Roxo escuro", "Marrom", "Vermelho", "Verde", "Azul", "Preto", "Roxo" };
    static const char* item_current = items[0];

    if(item_current == items[0]) pSettings->Get().iBtnStyle = 0;
    else if(item_current == items[1]) pSettings->Get().iBtnStyle = 1;
    else if(item_current == items[2]) pSettings->Get().iBtnStyle = 2;
    else if(item_current == items[3]) pSettings->Get().iBtnStyle = 3;
    else if(item_current == items[4]) pSettings->Get().iBtnStyle = 4;
    else if(item_current == items[5]) pSettings->Get().iBtnStyle = 5;
    else pSettings->Get().iBtnStyle = item_current == items[6] ? 6 : 0;

    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("#__keys_c", item_current))
    {
        for (auto & item : items)
        {
            bool is_selected = (item_current == item);
            if (ImGui::Selectable(item, is_selected))
                item_current = item;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }*/
    // --> Checkbox`s --> --> --> --> -->
    ImGui::Checkbox("Hora no Chat", &pGUI->timestamp);
	ImGui::Checkbox("Voice", &pSettings->Get().bVoiceChatEnable);
	
    ImGui::Separator();
    ImGui::EndChild();

    if (ImGui::Button("Ok", ImVec2(-1, -1)))
        m_bRenderMenu = false;

    ImGui::SetWindowSize(ImVec2(pGUI->ScaleY(1400), pGUI->ScaleY(700)));
    ImGui::SetWindowPos(ImVec2(((io.DisplaySize.x - ImGui::GetWindowSize().x) / 2), ((io.DisplaySize.y - ImGui::GetWindowSize().y) / 2)));

    ImGui::End();
}