#include "../../main.h"
#include "../gui.h"
#include "../../game/game.h"
#include "../../log.h"
#include "../settings.h"
#include "../java/jniutil.h"

extern CGame* pGame;
extern UI* pUI;
extern CSettings* pSettings;
extern CJavaWrapper* pJavaWrapper;

InputChat::InputChat()
{
	m_input = "";
	m_caption = "";
	m_caller = nullptr;
}

void InputChat::addCharToInput(char value)
{
	m_input += value;
	m_caption = Encoding::cp2utf(m_input);
}

void InputChat::popCharFromInput()
{
	if (m_input.empty()) return;
	m_input.pop_back();
	m_caption = Encoding::cp2utf(m_input);
}

void InputChat::performLayout()
{
	if (!pSettings) return;

	float chatX = pSettings->Get().fChatPosX;
	float chatY = pSettings->Get().fChatPosY;

	float inputW = 800.0f;
	float inputH = 45.0f;

	float fontSize = pSettings->Get().fFontSize;
	int maxLines = pSettings->Get().iChatMaxMessages;
	if (maxLines <= 0) maxLines = 10;
	float chatH = fontSize * (float)maxLines;

	this->setPosition(ImVec2(chatX - 5.0f, chatY + chatH + 20.0f));
	this->setFixedSize(ImVec2(inputW, inputH));
}

void InputChat::draw(ImGuiRenderer* renderer)
{
	if (!m_caller) return;

	if (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Escape]))
	{
		this->hide(true);
		return;
	}

	if (!(m_caller->childCount() == 0 || m_caller->childAt(0)->childCount() > 0))
	{
		return;
	}

	this->performLayout();

	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;
		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		float v_w = 4.0f;
		ImVec4 rect = UI::rectEditBox;
		float mtx = rect.x; float mty = rect.y;
		float mtw = rect.z - rect.x; float mth = rect.w - rect.y;

		float t_lw = 6.0f; float t_rw = 5.0f;
		float t_th = 8.0f; float t_bh = 8.0f;
		float t_mw = mtw - t_lw - t_rw;
		float t_mh = mth - t_th - t_bh;

		auto drawPart = [&](float stx, float sty, float stw, float sth, float dx, float dy, float dw, float dh) {
			renderer->drawImageUV(p + ImVec2(dx, dy), p + ImVec2(dx + dw, dy + dh),
				ImVec2(stx/tw, sty/th), ImVec2((stx+stw)/tw, (sty+sth)/th), (ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 220));
		};

		drawPart(mtx, mty, t_lw, t_th, 0, 0, v_w, v_w);
		drawPart(mtx + t_lw, mty, t_mw, t_th, v_w, 0, s.x - v_w*2, v_w);
		drawPart(mtx + t_lw + t_mw, mty, t_rw, t_th, s.x - v_w, 0, v_w, v_w);

		drawPart(mtx, mty + t_th, t_lw, t_mh, 0, v_w, v_w, s.y - v_w*2);
		drawPart(mtx + t_lw, mty + t_th, t_mw, t_mh, v_w, v_w, s.x - v_w*2, s.y - v_w*2);
		drawPart(mtx + t_lw + t_mw, mty + t_th, t_rw, t_mh, s.x - v_w, v_w, v_w, s.y - v_w*2);

		drawPart(mtx, mty + t_th + t_mh, t_lw, t_bh, 0, s.y - v_w, v_w, v_w);
		drawPart(mtx + t_lw, mty + t_th + t_mh, t_mw, t_bh, v_w, s.y - v_w, s.x - v_w*2, v_w);
		drawPart(mtx + t_lw + t_mw, mty + t_th + t_mh, t_rw, t_bh, s.x - v_w, s.y - v_w, v_w, v_w);
	}

	float font_sz = UISettings::fontSize() + 2.0f;
	std::string display_text = m_caption + "|";
	renderer->drawText(absolutePosition() + ImVec2(8.0f, (height() - font_sz) / 2),
					   ImColor(1.0f, 1.0f, 1.0f), display_text, true, font_sz);
}

void InputChat::show(Widget* caller)
{
	if (m_caller != caller) clear();
	this->setVisible(true);
	m_caller = caller;

	pJavaWrapper->ShowKeyboard();
	pGame->EnableGameInput(false);
}

void InputChat::hide(bool deactivate)
{
	pJavaWrapper->HideKeyboard();
	if (deactivate)
	{
		this->setVisible(false);
		pGame->EnableGameInput(true);
		if (m_caller) m_caller->setActive(false);
		m_caller = nullptr;
	}
}

void InputChat::send()
{
	if (m_caller) m_caller->keyboardEvent(m_input);
	this->hide();
}

void InputChat::sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
	jsize v8 = pEnv->GetArrayLength(str);
	char* buffer = (char*)malloc(v8 + 1);
	jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
	memcpy(buffer, elements, v8);
	buffer[v8] = 0;
	std::string input = std::string(buffer);

	if (m_caller) m_caller->keyboardEvent(input);
	hide();

	free(buffer);
	pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void InputChat::updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
	jsize v8 = pEnv->GetArrayLength(str);
	char* buffer = (char*)malloc(v8 + 1);
	jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
	memcpy(buffer, elements, v8);
	buffer[v8] = 0;
	std::string input = std::string(buffer);

	setInputString(input);

	if (m_caller && m_caller != pUI->chat()) {
		m_caller->keyboardEvent(input);
	}

	free(buffer);
	pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void InputChat::activateEvent(bool active)
{
}
