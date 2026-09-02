#pragma once

class Label : public Widget
{
public:
	Label(const std::string& text, const ImColor& color = ImColor(1.0f, 1.0f, 1.0f), bool outlined = false, float font_size = 0.0f, bool bold_outline = false);

	void setText(const std::string& text);
	const std::string& text() const { return m_text; }
	void setBoldOutline(bool bold) { m_boldOutline = bold; }

	void setColor(const ImColor& color) { m_color = color; }
	const ImColor& color() const { return m_color; }

	virtual void performLayout() override;

	virtual void draw(ImGuiRenderer* renderer) override;

private:
	ImVec2 calculateTextSize(const std::string& text) const;
	ImVec2 calculateTextSize(const char* begin, const char* end) const;

private:
	std::string m_text;
	ImColor m_color;
	bool m_outlined;
	bool m_boldOutline;
	float m_fontSize;
};




class LabelImage : public Widget
{
public:
	LabelImage(const std::string& text, const ImColor& color = ImColor(1.0f, 1.0f, 1.0f), bool outlined = false, float font_size = 0.0f, bool bold_outline = false);

	void setText(const std::string& text);
	const std::string& text() const { return m_text; }
	void setBoldOutline(bool bold) { m_boldOutline = bold; }

	void setColor(const ImColor& color) { m_color = color; }
	const ImColor& color() const { return m_color; }

	virtual void performLayout() override;

	virtual void draw(ImGuiRenderer* renderer) override;

private:
	ImVec2 calculateTextSize(const std::string& text) const;
	ImVec2 calculateTextSize(const char* begin, const char* end) const;

private:
	std::string m_text;
	ImColor m_color;
	bool m_outlined;
	bool m_boldOutline;
	float m_fontSize;
};