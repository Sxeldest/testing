#include "../gui.h"

ScrollPanel::ScrollPanel()
{
	m_scroll = ImVec2(0.0f, 0.0f);
	m_scrollableX = true;
	m_scrollableY = true;
	m_clipping = true;
	m_showScrollBar = false;
}


void ScrollPanel::setScrollX(float x)
{
	if (children().empty()) return;
	Widget* child = children()[0];

	m_scroll.x = x;
	float posX = -m_scroll.x * (child->width() - width());
	child->setPosition(ImVec2(floorf(posX), child->position().y));
}

void ScrollPanel::setScrollY(float y)
{
	if (children().empty()) return;
	Widget* child = children()[0];

	m_scroll.y = y;
	float posY = -m_scroll.y * (child->height() - height());
	child->setPosition(ImVec2(child->position().x, floorf(posY)));
}

void ScrollPanel::performLayout()
{
	if (children().empty()) return;
	if (children().size() > 1) return;

	Widget* child = children()[0];

	child->performLayout();

	float viewWidth = width();
	if (m_showScrollBar)
	{
		// SAMP PC: Scrollbar (16px) + Padding (12px) gap from list items
		viewWidth -= 28.0f;
	}

	// Force child to be at least the view width to prevent horizontal "jumping"
	if (child->width() < viewWidth) child->setWidth(viewWidth);
}

void ScrollPanel::draw(ImGuiRenderer* renderer)
{
	bool isChat = (dynamic_cast<Chat*>(this) != nullptr);

	if (m_clipping) {
		if (isChat) {
			// Vertical clipping only for Chat, no horizontal clipping
			// Add 5px padding to bottom to prevent outline cutting
			renderer->pushClipRect(ImVec2(0.0f, absolutePosition().y), ImVec2(ImGui::GetIO().DisplaySize.x, absolutePosition().y + height() + 5.0f), true);
		} else {
			renderer->pushClipRect(absolutePosition(), absolutePosition() + size(), true);
		}
	}
	Widget::draw(renderer);
	if (m_clipping) renderer->popClipRect();

	if (children().empty()) return;
	Widget* child = children()[0];

	// SAMP PC: Scrollbar (Only for Dialog Lists/Tablists)
	if (m_showScrollBar && m_scrollableY && UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		float sbWidth = isChat ? 20.0f : 16.0f;

		ImVec2 sbPos;
		if (isChat) {
			// Left side for Chat (aligned with screen left edge)
			// PC Style: Location(10, 40)
			sbPos = ImVec2(10.0f, absolutePosition().y + 40.0f);
		} else {
			// Right side for Dialogs
			sbPos = absolutePosition() + ImVec2(width() + 4.0f - sbWidth, -6.0f);
		}

		// Height: PC Style uses Size(20, (lines * font) - 60)
		float sbHeightOffset = isChat ? -60.0f : 12.0f;
		ImVec2 sbSize = ImVec2(sbWidth, height() + sbHeightOffset);

		// 1. Draw Track (Always displayed in Dialog Lists)
		ImVec4 rectTrack = UI::rectScrollTrack;
		renderer->drawImageUV(sbPos, sbPos + sbSize,
			ImVec2(rectTrack.x / tw, rectTrack.y / th),
			ImVec2(rectTrack.z / tw, rectTrack.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		// 2. Draw Arrows (Always displayed in Dialog Lists)
		float arrowHeight = 16.0f;
		ImVec4 rectUp = UI::rectScrollUpArrow;
		renderer->drawImageUV(sbPos, sbPos + ImVec2(sbWidth, arrowHeight),
			ImVec2(rectUp.x / tw, rectUp.y / th),
			ImVec2(rectUp.z / tw, rectUp.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		ImVec4 rectDown = UI::rectScrollDownArrow;
		renderer->drawImageUV(sbPos + ImVec2(0.0f, sbSize.y - arrowHeight), sbPos + sbSize,
			ImVec2(rectDown.x / tw, rectDown.y / th),
			ImVec2(rectDown.z / tw, rectDown.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 255));

		// 3. Draw Thumb (Only if content is actually scrollable)
		if (child->height() > height())
		{
			float totalHeight = child->height();
			float windowHeight = height();
			float trackHeight = sbSize.y - (arrowHeight * 2.0f);

			float thumbHeight = ImMax(20.0f, (windowHeight / totalHeight) * trackHeight);

			float scrollableHeight = totalHeight - windowHeight;
			// Child position is negative as it moves up
			float scrollRatio = -child->position().y / scrollableHeight;
			scrollRatio = ImSaturate(scrollRatio);

			float thumbY = arrowHeight + (scrollRatio * (trackHeight - thumbHeight));

			ImVec4 rectThumb = UI::rectScrollThumb;
			renderer->drawImageUV(sbPos + ImVec2(0.0f, thumbY), sbPos + ImVec2(sbWidth, thumbY + thumbHeight),
				ImVec2(rectThumb.x / tw, rectThumb.y / th),
				ImVec2(rectThumb.z / tw, rectThumb.w / th),
				(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(0xB9, 0x22, 0x28, 255));
		}
	}
}

void ScrollPanel::touchMoveEvent(const ImVec2& delta)
{
	if (children().empty()) return;
	Widget* child = children()[0];

	/*if (m_scrollableX && child->width() > width())
	{
		float scrollX = width() * ImMin(1.0f, width() / child->width());
		m_scroll.x = ImMax(0.0f, ImMin(1.0f, m_scroll.x - delta.x / (child->width() + scrollX)));
	}

	if (m_scrollableY && child->height() > height())
	{
		float scrollY = height() * ImMin(1.0f, height() / child->height());
		m_scroll.y = ImMax(0.0f, ImMin(1.0f, m_scroll.y - delta.y / (child->height() + scrollY)));
	}

	ImVec2 deltaPos = ImVec2(
		-m_scroll.x * (child->width() - width()),
		-m_scroll.y * (child->height() - height())
	);*/

	ImVec2 deltaPos = child->position();
	ImVec2 deltaMinPos = { width() - child->width(), height() - child->height() };

	if (m_scrollableX)
	{
		deltaPos.x += delta.x;
	}

	if (m_scrollableY)
	{
		deltaPos.y += delta.y;
	}

	bool isChat = (dynamic_cast<Chat*>(this) != nullptr);
	if (isChat) {
		float step = UISettings::fontSize();
		if (step > 0.1f) {
			deltaPos.y = roundf(deltaPos.y / step) * step;
		}
	}

	if (deltaPos.x < deltaMinPos.x) deltaPos.x = deltaMinPos.x;
	if (deltaPos.y < deltaMinPos.y) deltaPos.y = deltaMinPos.y;
	if (deltaPos.x > 0) deltaPos.x = 0.0f;
	if (deltaPos.y > 0) deltaPos.y = 0.0f;
	
	deltaPos.x = floorf(deltaPos.x);
	deltaPos.y = floorf(deltaPos.y);

	child->setPosition(deltaPos);
}
