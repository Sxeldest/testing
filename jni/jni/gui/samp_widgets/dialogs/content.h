#pragma once

#include "msgbox.h"
#include "inputwidget.h"
#include "listwidget.h"
#include "tablistwidget.h"

enum class DialogStyle
{
	MSGBOX = 0,
	INPUT = 1,
	LIST = 2,
	PASSWORD = 3,
	TABLIST = 4,
	TABLIST_HEADERS = 5,
	NONE = -1
};

class CDialogContent : public Widget
{
public:
	CDialogContent();
	virtual void performLayout() override;

	void SetActive(DialogStyle iStyle, const std::string& szInfo);

	Widget* GetActiveWidget() const { return m_pActiveWidget; }
	DialogStyle GetActiveStyle() const { return m_activeWidgetStyle; }

	const std::string& GetInputString() const;
	int GetListItem() const;

private:
	void RemoveActiveWidget();

private:
	Widget* m_pActiveWidget;
	TabListWidget::ItemWidget* m_pStaticHeader;
	DialogStyle m_activeWidgetStyle;
};