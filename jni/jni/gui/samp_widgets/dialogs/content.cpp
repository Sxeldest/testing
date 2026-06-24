#include "../../gui.h"

CDialogContent::CDialogContent()
{
    m_activeWidgetStyle = DialogStyle::NONE;
    m_pActiveWidget = nullptr;
    m_pStaticHeader = nullptr;
}

void CDialogContent::performLayout()
{
    if (m_pActiveWidget == nullptr) return;

    float fHeaderOffset = 0.0f;
    if (m_pStaticHeader && m_pStaticHeader->visible()) {
        m_pStaticHeader->setFixedSize(ImVec2(this->width(), UISettings::dialogListItemHeight()));
        m_pStaticHeader->performLayout();
        m_pStaticHeader->setPosition(ImVec2(0.0f, 0.0f));
        fHeaderOffset = m_pStaticHeader->height() + 15.0f;
    }

    m_pActiveWidget->setFixedSize(ImVec2(this->width(), this->height() - fHeaderOffset));
    m_pActiveWidget->performLayout();
    m_pActiveWidget->setPosition(ImVec2(0.0f, fHeaderOffset));
}

void CDialogContent::SetActive(DialogStyle iStyle, const std::string& szInfo)
{
    RemoveActiveWidget();

    switch (iStyle)
    {
    case DialogStyle::MSGBOX:
    {
        MsgBoxWidget* pMsgBox = new MsgBoxWidget();
        this->addChild(pMsgBox);
        pMsgBox->setMessage(szInfo);
        m_pActiveWidget = pMsgBox;
        break;
    }
    case DialogStyle::PASSWORD:
    case DialogStyle::INPUT:
    {
        InputWidget* pInput = new InputWidget();
        this->addChild(pInput);
        pInput->setMessage(szInfo);
        if (iStyle == DialogStyle::PASSWORD) pInput->setPasswordMode(true);
        m_pActiveWidget = pInput;
        break;
    }
    case DialogStyle::LIST:
    case DialogStyle::TABLIST:
    {
        ListWidget* pList = new ListWidget();
        this->addChild(pList);
        pList->assemble(szInfo, UISettings::fontSize() + 2.0f, false);
        m_pActiveWidget = pList;
        break;
    }
    case DialogStyle::TABLIST_HEADERS:
    {
        TabListWidget* pTabList = new TabListWidget();
        this->addChild(pTabList);

        // Tablist items
        std::vector<float> fOffsets = pTabList->assemble(szInfo, {}, true, UISettings::fontSize() + 2.0f, false);
        m_pActiveWidget = pTabList;

        std::string szHeaderStr = TabListWidget::getFirstRow(szInfo);
        m_pStaticHeader = new TabListWidget::ItemWidget(false);
        this->addChild(m_pStaticHeader);

        std::stringstream ss(szHeaderStr);
        std::string szItem;
        int iCol = 0;
        while (std::getline(ss, szItem, '\t')) {
            if (iCol < (int)fOffsets.size()) {
                m_pStaticHeader->add(new Label(szItem, ImColor(0x95, 0xB0, 0xD0), false, UISettings::smallFontSize()), fOffsets[iCol]);
            }
            iCol++;
        }
        break;
    }
	}

    m_activeWidgetStyle = iStyle;
    m_pActiveWidget->setVisible(true);
}

void CDialogContent::RemoveActiveWidget()
{
    if (m_pActiveWidget != nullptr) this->removeChild(m_pActiveWidget);
    if (m_pStaticHeader != nullptr) this->removeChild(m_pStaticHeader);

    m_pActiveWidget = nullptr;
    m_pStaticHeader = nullptr;
    m_activeWidgetStyle = DialogStyle::NONE;
}

const std::string& CDialogContent::GetInputString() const
{
    if (m_activeWidgetStyle == DialogStyle::INPUT || m_activeWidgetStyle == DialogStyle::PASSWORD)
        return dynamic_cast<InputWidget*>(m_pActiveWidget)->inputString();
    return "";
}

int CDialogContent::GetListItem() const
{
    if (m_activeWidgetStyle == DialogStyle::LIST || m_activeWidgetStyle == DialogStyle::TABLIST || m_activeWidgetStyle == DialogStyle::TABLIST_HEADERS)
        return dynamic_cast<ListBox*>(m_pActiveWidget)->activeItemIndex();
    return -1;
}
