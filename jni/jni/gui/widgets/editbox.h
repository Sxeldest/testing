#pragma once

class EditBox : public Widget
{
public:
	EditBox();

	virtual void performLayout() override;
	virtual void touchPopEvent() override;
	virtual void draw(ImGuiRenderer* renderer) override;
	virtual void keyboardEvent(const std::string& input) override;
	virtual void cursorEvent(int pos) override;
	virtual void keyStrokeEvent(int keyCode, int metaState) override;
	virtual void onSubmit() override;

	void setPasswordMode(bool password) { m_passwordMode = password; }
	const std::string& text() const { return m_input; }

private:
	std::string m_input;
	std::string m_caption;
	int m_cursorPos;
	int m_selectionStart;
	bool m_passwordMode;
};
