#pragma once

class InputWidget : public Widget
{
public:
	InputWidget();
	virtual void performLayout() override;

	void setMessage(const std::string& message);
	void setPasswordMode(bool password) { m_editbox->setPasswordMode(password); }
	const std::string& inputString() const;

private:
	MsgBoxWidget* m_msgbox;
	EditBox* m_editbox;
};