#include "../gui.h"

extern UI* pUI;

EditBox::EditBox()
{
	m_label = new Label(" ", ImColor(1.0f, 1.0f, 1.0f), true, UISettings::fontSize() + 2.0f); // PC Style: Outlined/Bold + Large
	this->addChild(m_label);
	m_passwordMode = false;
}

void EditBox::performLayout()
{
	m_label->performLayout();

	m_label->setPosition(ImVec2(
		UISettings::padding(),

		(height() - m_label->height()) / 2
	));
}

void EditBox::draw(ImGuiRenderer* renderer)
{
	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;
		ImVec2 p = absolutePosition();
		p.x = floorf(p.x); p.y = floorf(p.y);
		ImVec2 s = size();

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
		// Row 1: Top
		drawPart(mtx, mty, t_lw, t_th, 0, 0, v_lw, v_th);
		drawPart(mtx + t_lw, mty, t_mw, t_th, v_lw, 0, s.x - v_lw - v_rw, v_th);
		drawPart(mtx + t_lw + t_mw, mty, t_rw, t_th, s.x - v_rw, 0, v_rw, v_th);

		// Row 2: Middle
		drawPart(mtx, mty + t_th, t_lw, t_mh, 0, v_th, v_lw, s.y - v_th - v_bh);
		drawPart(mtx + t_lw, mty + t_th, t_mw, t_mh, v_lw, v_th, s.x - v_lw - v_rw, s.y - v_th - v_bh);
		drawPart(mtx + t_lw + t_mw, mty + t_th, t_rw, t_mh, s.x - v_rw, v_th, v_rw, s.y - v_th - v_bh);

		// Row 3: Bottom
		drawPart(mtx, mty + t_th + t_mh, t_lw, t_bh, 0, s.y - v_bh, v_lw, v_bh);
		drawPart(mtx + t_lw, mty + t_th + t_mh, t_mw, t_bh, v_lw, s.y - v_bh, s.x - v_lw - v_rw, v_bh);
		drawPart(mtx + t_lw + t_mw, mty + t_th + t_mh, t_rw, t_bh, s.x - v_rw, s.y - v_bh, v_rw, v_bh);
	}

	Widget::draw(renderer);
}

void EditBox::touchPopEvent()
{
	pUI->inputChat()->show(this);
}

void EditBox::keyboardEvent(const std::string& input)
{
	m_input = input;
	if (m_passwordMode) {
		std::string masked(input.length(), '*');
		m_label->setText(masked);
	} else {
		m_label->setText(Encoding::cp2utf(input));
	}
}
