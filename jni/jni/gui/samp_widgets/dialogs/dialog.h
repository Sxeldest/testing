#pragma once

#include "content.h"

class CDialogButton : public Layout
{
public:
	CDialogButton();

	void SetButton1Caption(const std::string& szCaption);
	void SetButton2Caption(const std::string& szCaption);

	virtual void performLayout() override;

private:
	class CDialogButton1 : public Button
	{
	public:
		CDialogButton1();

		virtual void touchPopEvent() override;
	};

	class CDialogButton2 : public Button
	{
	public:
		CDialogButton2();

		virtual void touchPopEvent() override;
	};

	CDialogButton1* m_pButton1;
	CDialogButton2* m_pButton2;
	friend class CDialog;
};

class CDialogTitle : public Widget
{
public:
	CDialogTitle();

	void SetTitle(const std::string& szTitle);

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;

private:
	Label* m_pLabel;
};

class CDialog : public Widget
{
public:
	CDialog();

	void Show(int16_t iID, DialogStyle iStyle, const std::string& szTitle, const std::string& szInfo, const std::string& szButton1, const std::string& szButton2);
	void Hide();

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;

	virtual void touchEvent(const ImVec2& pos, TouchType type) override;

	CDialogContent* GetContent() const { return m_pContent; }
	int16_t GetID() const { return m_iID; }

private:
	void SetTitleCaption(const std::string& szCaption);
	void SetButton1Caption(const std::string& szCaption);
	void SetButton2Caption(const std::string& szCaption);

private:
	CDialogTitle* m_pTitle;
	CDialogContent* m_pContent;
	CDialogButton* m_pButton;

	int16_t m_iID;
	float m_fWidth;
	float m_fHeight;
};