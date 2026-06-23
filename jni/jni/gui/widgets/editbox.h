#pragma once

class EditBox : public Widget
{
public:
	EditBox();

	virtual void performLayout() override;
	virtual void touchPopEvent() override;
	virtual void draw(ImGuiRenderer* renderer) override;
	virtual void keyboardEvent(const std::string& input) override;

	void setPasswordMode(bool password) { m_passwordMode = password; }
	const std::string& text() const { return m_input; }

private:
	Label* m_label;
	std::string m_input;
	bool m_passwordMode;
};
