#include "../main.h"
#include "../game/RW/RenderWare.h"
#include "../game/game.h"
#include "../gui/gui.h"
#include "../game/audiostream.h"
#include "loadingscreen.h"

#define COLOR_WHITE		0xFFFFFFFF
#define COLOR_BLACK 	0xFF000000

extern CGUI *pGUI;
extern CAudioStream *pAudioStream;

void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
void ImGui_ImplRenderWare_NewFrame();

CLoadingScreen::CLoadingScreen()
{
	dwLastUpdateTick = 0;
	textID = 0;
	text = "";
}

void CLoadingScreen::LoadSplashTexture()
{
	Log("Loading splash texture..");
	splashTexture = (RwTexture*)LoadTextureFromDB("samp", "mylogo");
}

void CLoadingScreen::Draw(stRect *rect, uint32_t color, RwRaster *raster, stfRect *uv)
{
	static RwIm2DVertex vert[4];
	const RwReal nearScreenZ 	= *(RwReal*)(g_libGTASA+0x9DAA60);	// CSprite2d::NearScreenZ
	const RwReal recipNearClip 	= *(RwReal*)(g_libGTASA+0x9DAA64);	// CSprite2d::RecipNearClip

	RwIm2DVertexSetScreenX(&vert[0], rect->x1);
	RwIm2DVertexSetScreenY(&vert[0], rect->y2);
	RwIm2DVertexSetScreenZ(&vert[0], nearScreenZ);
	RwIm2DVertexSetRecipCameraZ(&vert[0], recipNearClip);
	vert[0].emissiveColor = color;
	RwIm2DVertexSetU(&vert[0], uv ? uv->x1 : 0.0f, recipNearClip);
	RwIm2DVertexSetV(&vert[0], uv ? uv->y2 : 0.0f, recipNearClip);

	RwIm2DVertexSetScreenX(&vert[1], rect->x2);
	RwIm2DVertexSetScreenY(&vert[1], rect->y2);
	RwIm2DVertexSetScreenZ(&vert[1], nearScreenZ);
	RwIm2DVertexSetRecipCameraZ(&vert[1], recipNearClip);
	vert[1].emissiveColor = color;
	RwIm2DVertexSetU(&vert[1], uv ? uv->x2 : 0.0f, recipNearClip);
	RwIm2DVertexSetV(&vert[1], uv ? uv->y2 : 0.0f, recipNearClip);

	RwIm2DVertexSetScreenX(&vert[2], rect->x1);
	RwIm2DVertexSetScreenY(&vert[2], rect->y1);
	RwIm2DVertexSetScreenZ(&vert[2], nearScreenZ);
	RwIm2DVertexSetRecipCameraZ(&vert[2], recipNearClip);
	vert[2].emissiveColor = color;
	RwIm2DVertexSetU(&vert[2], uv ? uv->x1 : 0.0f, recipNearClip);
	RwIm2DVertexSetV(&vert[2], uv ? uv->y1 : 0.0f, recipNearClip);

	RwIm2DVertexSetScreenX(&vert[3], rect->x2);
	RwIm2DVertexSetScreenY(&vert[3], rect->y1);
	RwIm2DVertexSetScreenZ(&vert[3], nearScreenZ);
	RwIm2DVertexSetRecipCameraZ(&vert[3], recipNearClip);
	vert[3].emissiveColor = color;
	RwIm2DVertexSetU(&vert[3], uv ? uv->x2 : 0.0f, recipNearClip);
	RwIm2DVertexSetV(&vert[3], uv ? uv->y1 : 0.0f, recipNearClip);

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)raster);
	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vert, 4);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
}

void CLoadingScreen::RenderSplash()
{
	if (!splashTexture) return;

	stRect rect;
	stfRect uv;
	stfRect sRect;

	// background
	rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = RsGlobal->maximumWidth;
    rect.y2 = RsGlobal->maximumHeight;
	Draw(&rect, COLOR_WHITE);

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	
	// splash
	rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = RsGlobal->maximumWidth;
    rect.y2 = RsGlobal->maximumWidth / 16 * 9;
    uv.x1 = 0.0f;
    uv.y1 = 0.0f;
    uv.x2 = 1.0f;
    uv.y2 = 1.0f;
	Draw(&rect, COLOR_WHITE, splashTexture->raster, &uv);
}

#include "../vendor/imgui/stb_image.h"
void CLoadingScreen::Render()
{
	RenderSplash();

	/*if (!pGUI) return; 
	const float percent = *(float*)(g_libGTASA + 0x8F08C0);
	if (percent <= 0.0f) return;
	float mult = percent / 100.0f;
	// offset
    float newX = pGUI->ScaleX(0.0f) + (mult * 3.0 * RsGlobal->maximumWidth) + pGUI->ScaleX(0.0f);*/

	ImGui_ImplRenderWare_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();
	
	ImColor colorWhite(0xFF, 0xFF, 0xFF, 0xFF);

    // -> Animation text --> --> --> --> -->
    if((GetTickCount() - dwLastUpdateTick) >= 500)
    {
        dwLastUpdateTick = GetTickCount();

        textID++;
        if (textID == 3) textID = 0;

        switch (textID) {
            case 0: text = "DEV LUFZY"; break;
            case 1: text = "DEV LUFZY"; break;
            case 2: text = "DEV LUFZY"; break;
        }
    }

    ImVec2 posText(pGUI->ScaleX(70), io.DisplaySize.y - pGUI->ScaleY(70));
    pGUI->RenderOverlayText(posText, colorWhite, false, text);

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());
}
