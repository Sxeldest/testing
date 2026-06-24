#include "../gui.h"
#include "../../net/netgame.h"

extern UI* pUI;
extern CNetGame* pNetGame;

EditBox::EditBox()
{
	m_input = "";
	m_caption = "";
	m_cursorPos = 0;
	m_selectionStart = -1;
	m_passwordMode = false;
}

void EditBox::performLayout()
{
}

void EditBox::draw(ImGuiRenderer* renderer)
{
	float font_sz = UISettings::fontSize() + 4.0f;
	ImVec2 p = absolutePosition();
	ImVec2 s = size();

	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;
		p.x = floorf(p.x); p.y = floorf(p.y);

		// UKURAN VISUAL DI LAYAR (Dibuat tipis: 2.5 pixel)
		float v_lw = 5.5; float v_rw = 5.5f;
		float v_th = 5.5; float v_bh = 5.5f;

		// KOORDINAT MENTAH DI TEXTURE (PC DXUT)
		ImVec4 rect = UI::rectEditBox;
		float mtx = rect.x; float mty = rect.y;
		float mtw = rect.z - rect.x; float mth = rect.w - rect.y;

		// Slice asli texture
		float t_lw = 6.0f; float t_rw = 5.0f;
		float t_th = 8.0f; float t_bh = 8.0f;
		float t_mw = mtw - t_lw - t_rw;
		float t_mh = mth - t_th - t_bh;

		auto drawPart = [&](float stx, float sty, float stw, float sth, float dx, float dy, float dw, float dh) {
			renderer->drawImageUV(p + ImVec2(dx, dy), p + ImVec2(dx + dw, dy + dh),
				ImVec2(stx/tw, sty/th), ImVec2((stx+stw)/tw, (sty+sth)/th), (ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 200));
		};

		// 9-Slice Rendering dengan Border Tipis (2.5px)
		drawPart(mtx, mty, t_lw, t_th, 0, 0, v_lw, v_th);
		drawPart(mtx + t_lw, mty, t_mw, t_th, v_lw, 0, s.x - v_lw - v_rw, v_th);
		drawPart(mtx + t_lw + t_mw, mty, t_rw, t_th, s.x - v_rw, 0, v_rw, v_th);

		drawPart(mtx, mty + t_th, t_lw, t_mh, 0, v_th, v_lw, s.y - v_th - v_bh);
		drawPart(mtx + t_lw, mty + t_th, t_mw, t_mh, v_lw, v_th, s.x - v_lw - v_rw, s.y - v_th - v_bh);
		drawPart(mtx + t_lw + t_mw, mty + t_th, t_rw, t_mh, s.x - v_rw, v_th, v_rw, s.y - v_th - v_bh);

		drawPart(mtx, mty + t_th + t_mh, t_lw, t_bh, 0, s.y - v_bh, v_lw, v_bh);
		drawPart(mtx + t_lw, mty + t_th + t_mh, t_mw, t_bh, v_lw, s.y - v_bh, s.x - v_lw - v_rw, v_bh);
		drawPart(mtx + t_lw + t_mw, mty + t_th + t_mh, t_rw, t_bh, s.x - v_rw, s.y - v_bh, v_rw, v_bh);
	}

	ImVec2 textPos = p + ImVec2(8.0f, (height() - font_sz) / 2);

	// Draw selection
	if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
	{
		int start = std::min(m_selectionStart, m_cursorPos);
		int end = std::max(m_selectionStart, m_cursorPos);

		std::string textBeforeStart = m_input.substr(0, start);
		std::string textSelection = m_input.substr(start, end - start);

		float startOffset = renderer->calculateTextSize(Encoding::cp2utf(textBeforeStart), font_sz).x;
		float selWidth = renderer->calculateTextSize(Encoding::cp2utf(textSelection), font_sz).x;

		renderer->drawRect(ImVec2(textPos.x + startOffset, textPos.y),
			ImVec2(textPos.x + startOffset + selWidth, textPos.y + font_sz),
			ImColor(255, 50, 50, 150), true);
	}

	renderer->drawText(textPos, ImColor(1.0f, 1.0f, 1.0f), m_caption, true, font_sz);

	// Draw cursor
	if (pUI->inputChat()->visible() && pUI->inputChat()->getCaller() == this)
	{
		std::string textBeforeCursor = m_input.substr(0, m_cursorPos);
		std::string captionBeforeCursor = Encoding::cp2utf(textBeforeCursor);
		float cursorOffset = renderer->calculateTextSize(captionBeforeCursor, font_sz).x;

		static float cursorAnim = 0.0f;
		cursorAnim += ImGui::GetIO().DeltaTime;
		if (fmodf(cursorAnim, 1.0f) < 0.5f)
		{
			renderer->drawRect(ImVec2(textPos.x + cursorOffset, textPos.y - 2.0f),
				ImVec2(textPos.x + cursorOffset + 1.0f, textPos.y + font_sz + 2.0f),
				ImColor(255, 255, 255, 255), true);
		}
	}
}

void EditBox::touchPopEvent()
{
	pUI->inputChat()->show(this);
	pUI->inputChat()->setInputString(m_input);
	this->keyboardEvent(m_input);
}

void EditBox::keyboardEvent(const std::string& input)
{
	m_input = input;
	if (m_passwordMode) {
		m_caption = std::string(input.length(), '*');
	} else {
		m_caption = Encoding::cp2utf(input);
	}
}

void EditBox::cursorEvent(int pos)
{
	m_cursorPos = pos;
}

void EditBox::keyStrokeEvent(int keyCode, int metaState)
{
	bool ctrl = (metaState & 0x1000) != 0 || (metaState & 0x2000) != 0;
	if (ctrl && keyCode == 29) // KEYCODE_A
	{
		m_selectionStart = 0;
		m_cursorPos = (int)m_input.length();
	}
	else if (keyCode == 21 || keyCode == 22 || keyCode == 19 || keyCode == 20) // Arrows
	{
		m_selectionStart = -1;
	}
}

void EditBox::onSubmit()
{
	// Hierarchy: EditBox -> InputWidget -> CDialogContent -> CDialog
	Widget* parent1 = parent();
	if (!parent1) return;
	Widget* parent2 = parent1->parent();
	if (!parent2) return;
	Widget* parent3 = parent2->parent();
	if (!parent3) return;

	CDialog* pDialog = dynamic_cast<CDialog*>(parent3);
	if (pDialog && pNetGame) {
		pNetGame->SendDialogResponse(pDialog->GetID(), 1, pDialog->GetContent()->GetListItem(), m_input.c_str());
		pDialog->Hide();
	}
}
