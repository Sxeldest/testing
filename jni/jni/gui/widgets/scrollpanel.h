#pragma once

class ScrollPanel : public Widget
{
public:
	ScrollPanel();

	void setScrollableX(bool scrollableX) { m_scrollableX = scrollableX; }
	void setScrollableY(bool scrollableY) { m_scrollableY = scrollableY; }
	void setScrollable(bool scrollable) { m_scrollableX = m_scrollableY = scrollable; }
	void setClipping(bool clipping) { m_clipping = clipping; }
	void setScrollX(float x);
	void setScrollY(float y);
	void setShowScrollBar(bool show) { m_showScrollBar = show; }
	bool showScrollBar() const { return m_showScrollBar; }

	virtual void performLayout() override;

	virtual void draw(ImGuiRenderer* renderer) override;

	virtual void touchMoveEvent(const ImVec2& delta) override;


private:
	ImVec2 m_scroll;
	bool m_scrollableX;
	bool m_scrollableY;
	bool m_clipping;
	bool m_showScrollBar;
};