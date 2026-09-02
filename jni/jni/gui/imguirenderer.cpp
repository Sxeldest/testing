#include <string>
#include "imguirenderer.h"
#include "uisettings.h"

ImGuiRenderer::ImGuiRenderer(ImDrawList* draw_list, ImFont* font)
{
	m_drawList = draw_list;
	m_font = font;
}

void ImGuiRenderer::drawLine(const ImVec2& a, const ImVec2& b, const ImColor& color, float thickness)
{
	m_drawList->AddLine(a, b, color, thickness);
}

void ImGuiRenderer::drawRect(const ImVec2& a, const ImVec2& b, const ImColor& color, bool fill, float thickness)
{
	fill ? m_drawList->AddRectFilled(a, b, color) :
	m_drawList->AddRect(a, b, color, 0.0f, 15, thickness);
}

void ImGuiRenderer::drawRectFilledMulticolor(const ImVec2& a, const ImVec2& b,
											 const ImColor& col_upr_left, const ImColor& col_upr_right,
											 const ImColor& col_bot_right, const ImColor& col_bot_left)
{
	m_drawList->AddRectFilledMultiColor(a, b, col_upr_left, col_upr_right, col_bot_right, col_bot_left);
}

void ImGuiRenderer::drawTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& color, bool fill, float thickness)
{
	fill ? m_drawList->AddTriangleFilled(a, b, c, color) :
	m_drawList->AddTriangle(a, b, c, color, thickness);
}

void ImGuiRenderer::drawConvexPolyFilled(ImVec2* points, int num_points, const ImColor& color)
{
	m_drawList->AddConvexPolyFilled(points, num_points, color);
}

void ImGuiRenderer::drawText(const ImVec2& pos, const ImColor& color,
							 const char* begin, const char* end, bool outlined, float font_size, ImFont* font, bool bold_outline)
{
	float sz_font = font_size == 0.0f ? m_font->FontSize : font_size;
	font = font == NULL ? m_font : font;

	ImVec2 p = ImVec2(floorf(pos.x + 0.5f), floorf(pos.y + 0.5f));

	if (outlined)
	{
		ImColor outlineColor(0.0f, 0.0f, 0.0f, color.Value.w);

		if (bold_outline)
		{
			m_drawList->AddText(font, sz_font, ImVec2(p.x - 2.0f, p.y), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x + 2.0f, p.y), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x, p.y - 2.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x, p.y + 2.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x - 1.0f, p.y - 1.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x + 1.0f, p.y - 1.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x - 1.0f, p.y + 1.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x + 1.0f, p.y + 1.0f), outlineColor, begin, end);
		}
		else
		{
			m_drawList->AddText(font, sz_font, ImVec2(p.x - 1.0f, p.y), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x + 1.0f, p.y), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x, p.y - 1.0f), outlineColor, begin, end);
			m_drawList->AddText(font, sz_font, ImVec2(p.x, p.y + 1.0f), outlineColor, begin, end);
		}
	}

	m_drawList->AddText(font, sz_font, p, color, begin, end);
}

void ImGuiRenderer::drawText(const ImVec2& pos, const ImColor& color, const std::string& text, bool outlined, float font_size, ImFont* font, bool bold_outline)
{
	if (text.empty()) return;

	float sz_font = font_size == 0.0f ? m_font->FontSize : font_size;
	font = font == NULL ? m_font : font;

	const char* text_start = text.c_str();
	const char* text_cur = text.c_str();
	const char* text_end = text.c_str() + text.length();

	ImVec2 pos_cur = pos;
	ImColor color_cur = color;

	while (*text_cur)
	{
		if (*text_cur == '{' && ((&text_cur[7] < text_end) && text_cur[7] == '}'))
		{
			// print accumulated text
			if (text_cur != text_start)
			{
				drawText(pos_cur, color_cur, text_start, text_cur, outlined, sz_font, font, bold_outline);
				ImVec2 sz = calculateTextSize(text_start, text_cur, sz_font);
				pos_cur.x += sz.x;
			}

			// new colorcode
			ImVec4 col;
			if (processInlineHexColor(text_cur + 1, text_cur + 7, col)) {
				color_cur = col;
			}

			text_cur += 7;
			text_start = text_cur + 1;
		}
		else if (*text_cur == '\n')
		{
			// print accumulated text
			if (text_cur != text_start)
			{
				drawText(pos_cur, color_cur, text_start, text_cur, outlined, sz_font, font, bold_outline);
			}

			pos_cur.x = pos.x;
			pos_cur.y += sz_font;
			text_start = text_cur + 1;
		}
		else if (*text_cur == '\t')
		{
			// print accumulated text
			if (text_cur != text_start)
			{
				drawText(pos_cur, color_cur, text_start, text_cur, outlined, sz_font, font, bold_outline);
				ImVec2 sz = calculateTextSize(text_start, text_cur, sz_font);
				pos_cur.x += sz.x;
			}

			pos_cur.x += sz_font;
			text_start = text_cur + 1;
		}

		++text_cur;
	}

	if (text_cur != text_start) {
		drawText(pos_cur, color_cur, text_start, text_cur, outlined, sz_font, font, bold_outline);
	}
}

ImVec2 ImGuiRenderer::calculateTextSize(const std::string& text, float font_size)
{
	ImVec2 text_size = { 0.0f, 0.0f };
	if (text.empty()) return text_size;

	ImVec2 cur_size = { 0.0f, 0.0f };
	if (font_size == 0.0f) font_size = m_font->FontSize;

	const char* text_start = text.c_str();
	const char* text_cur = text.c_str();
	const char* text_end = text.c_str() + text.length();

	while (*text_cur)
	{
		if (*text_cur == '{' && ((&text_cur[7] < text_end) && text_cur[7] == '}'))
		{
			if (text_cur != text_start)
			{
				// ����� �� �����-����
				ImVec2 sz = calculateTextSize(text_start, text_cur, font_size);
				cur_size.x += sz.x;
				if (cur_size.y == 0.0f) cur_size.y = sz.y;
			}

			text_cur += 7;
			text_start = text_cur + 1;
		}
		else if (*text_cur == '\n')
		{
			if (text_cur != text_start)
			{
				// ����� �� \n
				ImVec2 sz = calculateTextSize(text_start, text_cur, font_size);
				cur_size.x += sz.x;
				if (cur_size.y == 0.0f) cur_size.y = sz.y;
			}

			// ��������� text_size
			text_size.x = ImMax(text_size.x, cur_size.x);
			cur_size.y += font_size;
			cur_size.x = 0.0f;

			text_start = text_cur + 1;
		}
		else if (*text_cur == '\t')
		{
			if (text_cur != text_start)
			{
				// ����� �� \t
				ImVec2 sz = calculateTextSize(text_start, text_cur, font_size);
				cur_size.x += sz.x;
				if (cur_size.y == 0.0f) cur_size.y = sz.y;
			}

			cur_size.x += font_size;
			text_start = text_cur + 1;
		}

		++text_cur;
	}

	if (text_cur != text_start)
	{
		// ����� ��� ��������������
		ImVec2 sz = calculateTextSize(text_start, text_cur, font_size);
		cur_size.x += sz.x;
		if (cur_size.y == 0.0f) cur_size.y = sz.y;
	}

	text_size = ImMax(text_size, cur_size);
	return text_size;
}

ImVec2 ImGuiRenderer::calculateTextSize(const char* begin, const char* end, float font_size)
{
	return m_font->CalcTextSizeA(font_size == 0.0f ? m_font->FontSize : font_size, FLT_MAX, 0.0f, begin, end);
}

bool ImGuiRenderer::processInlineHexColor(const char* start, const char* end, ImVec4& color)
{
	const int hexCount = (int)(end - start);
	if (hexCount == 6)
	{
		char hex[7];
		strncpy(hex, start, hexCount);
		hex[hexCount] = 0;

		unsigned int hexColor = 0;
		if (sscanf(hex, "%x", &hexColor) > 0)
		{
			color.x = static_cast<float>((hexColor & 0x00FF0000) >> 16) / 255.0f;
			color.y = static_cast<float>((hexColor & 0x0000FF00) >> 8) / 255.0f;
			color.z = static_cast<float>((hexColor & 0x000000FF)) / 255.0f;
			color.w = 1.0f;
			return true;
		}
	}

	return false;
}

void ImGuiRenderer::drawImage(const ImVec2& a, const ImVec2& b, ImTextureID texture)
{
	m_drawList->AddImage(texture, a, b);
}

void ImGuiRenderer::drawImageUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImTextureID texture, const ImColor& color)
{
	m_drawList->AddImage(texture, a, b, uv_a, uv_b, color);
}

void ImGuiRenderer::pushClipRect(const ImVec2& min, const ImVec2& max, bool intersect)
{
	m_drawList->PushClipRect(min, max, intersect);
}

void ImGuiRenderer::popClipRect()
{
	m_drawList->PopClipRect();
}