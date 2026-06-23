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

/* ChatInput (PC Style) */

ChatInput::ChatInput()
{
	m_input = "";
	m_caption = "";
}

void ChatInput::addCharToInput(char value)
{
	m_input += value;
	m_caption = Encoding::cp2utf(m_input);
}

void ChatInput::popCharFromInput()
{
	if (m_input.empty()) return;
	m_input.pop_back();
	m_caption = Encoding::cp2utf(m_input);
}

void ChatInput::performLayout()
{
	// Fixed height for PC chat input
	this->setHeight(30.0f);
}

void ChatInput::draw(ImGuiRenderer* renderer)
{
	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;
		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

		// Visual border width
		float v_w = 4.0f;

		// Texture coordinates (PC DXUT EditBox)
		ImVec4 rect = UI::rectEditBox;
		float mtx = rect.x; float mty = rect.y;
		float mtw = rect.z - rect.x; float mth = rect.w - rect.y;

		// Slices
		float t_lw = 6.0f; float t_rw = 5.0f;
		float t_th = 8.0f; float t_bh = 8.0f;
		float t_mw = mtw - t_lw - t_rw;
		float t_mh = mth - t_th - t_bh;

		auto drawPart = [&](float stx, float sty, float stw, float sth, float dx, float dy, float dw, float dh) {
			renderer->drawImageUV(p + ImVec2(dx, dy), p + ImVec2(dx + dw, dy + dh),
				ImVec2(stx/tw, sty/th), ImVec2((stx+stw)/tw, (sty+sth)/th), (ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 220));
		};

		// 9-Slice
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

	float font_sz = UISettings::fontSize();
	std::string display_text = m_caption + "|"; // Removed "> " prefix
	renderer->drawText(absolutePosition() + ImVec2(8.0f, (height() - font_sz) / 2),
					   ImColor(1.0f, 1.0f, 1.0f), display_text, true, font_sz);

	Widget::draw(renderer);
}

/* Keyboard Wrapper (Now acts as Chat Input Manager) */

Keyboard::Keyboard()
{
	m_chatInput = new ChatInput();
	this->addChild(m_chatInput);
	m_caller = nullptr;
}

void Keyboard::performLayout()
{
	if (!pSettings) return;

	// Position right below the chat
	float chatX = pSettings->Get().fChatPosX;
	float chatY = pSettings->Get().fChatPosY;

	// PC Style: Long input bar (80% of screen width)
	float screenW = parent() ? parent()->width() : 1920.0f;
	float inputW = screenW * 0.8f;

	float fontSize = pSettings->Get().fFontSize;
	int maxLines = pSettings->Get().iChatMaxMessages;
	if (maxLines <= 0) maxLines = 10;
	float chatH = fontSize * (float)maxLines;

	this->setPosition(ImVec2(chatX, chatY + chatH + 5.0f));
	this->setFixedSize(ImVec2(inputW, 35.0f));

	m_chatInput->setFixedSize(ImVec2(inputW, 30.0f));
	m_chatInput->setPosition(ImVec2(0.0f, 0.0f));

	Widget::performLayout();
}

void Keyboard::draw(ImGuiRenderer* renderer)
{
	if (!m_caller) return;

	// PC Style Input Bar only visible when typing for Chat
	// If caller is NOT a dialog EditBox, we assume it's Chat
	if (m_caller->childCount() == 0 || m_caller->childAt(0)->childCount() > 0)
	{
		m_chatInput->setVisible(true);
		this->performLayout();
		Widget::draw(renderer);
	}
	else
	{
		m_chatInput->setVisible(false);
	}
}

void Keyboard::show(Widget* caller)
{
	m_chatInput->clear();
	this->setVisible(true);
	m_caller = caller;

	// Trigger Android keyboard for input
	pJavaWrapper->ShowKeyboard();
	pGame->EnableGameInput(false);
}

void Keyboard::hide()
{
	this->setVisible(false);
	pJavaWrapper->HideKeyboard();
	pGame->EnableGameInput(true);
	m_caller = nullptr;
}

void Keyboard::send()
{
	const std::string input = m_chatInput->inputString();
	if (m_caller) m_caller->keyboardEvent(input);
	this->hide();
}

void Keyboard::sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
	std::string::size_type v8 = pEnv->functions->GetArrayLength(pEnv, (jarray)str);
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

void Keyboard::updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
	std::string::size_type v8 = pEnv->functions->GetArrayLength(pEnv, (jarray)str);
	char* buffer = (char*)malloc(v8 + 1);
	jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
	memcpy(buffer, elements, v8);
	buffer[v8] = 0;
	std::string input = std::string(buffer);

	m_chatInput->setInputString(input);

	// ONLY send keyboardEvent (real-time update) if NOT chat.
	// We distinguish Chat by checking if it's the specific Chat widget.
	if (m_caller && m_caller != pUI->chat()) {
		m_caller->keyboardEvent(input);
	}

	free(buffer);
	pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void Keyboard::activateEvent(bool active)
{
	if (!active) this->hide();
}
