#include "../../gui.h"

Content::Content()
{
    m_activeWidgetStyle = DialogStyle::NONE;
    m_activeWidget = nullptr;
    m_staticHeader = nullptr;
}

void Content::performLayout()
{
    if (m_activeWidget == nullptr) return;

    float headerOffset = 0.0f;
    if (m_staticHeader && m_staticHeader->visible()) {
        m_staticHeader->setFixedSize(ImVec2(this->width(), UISettings::dialogListItemHeight()));
        m_staticHeader->performLayout();
        m_staticHeader->setPosition(ImVec2(0.0f, 0.0f));
        headerOffset = m_staticHeader->height() + 15.0f; // Gap between header and list (Identical to columnGap)
    }

    m_activeWidget->setFixedSize(ImVec2(this->width(), this->height() - headerOffset));
    m_activeWidget->performLayout();
    m_activeWidget->setPosition(ImVec2(0.0f, headerOffset));
}

void Content::setActive(DialogStyle style, const std::string& data)
{
    removeActiveWidget();

    switch (style)
    {
    case DialogStyle::MSGBOX:
    {
        MsgBoxWidget* msgbox = new MsgBoxWidget();
        this->addChild(msgbox);
        msgbox->setMessage(data);
        m_activeWidget = msgbox;
        break;
    }
    case DialogStyle::PASSWORD:
    case DialogStyle::INPUT:
    {
        InputWidget* input = new InputWidget();
        this->addChild(input);
        input->setMessage(data);
        if (style == DialogStyle::PASSWORD) input->setPasswordMode(true);
        m_activeWidget = input;
        break;
    }
    case DialogStyle::LIST:
    case DialogStyle::TABLIST:
    {
        ListWidget* list = new ListWidget();
        this->addChild(list);
        // PC Style: Outlined, Bold, Besar (+2)
        list->assemble(data, UISettings::fontSize() + 2.0f, true);
        m_activeWidget = list;
        break;
    }
    case DialogStyle::TABLIST_HEADERS:
    {
        TabListWidget* list = new TabListWidget();
        this->addChild(list);

        // TABLIST ITEMS: Tetap Besar & Bold (+2)
        std::vector<float> offsets = list->assemble(data, {}, true, UISettings::fontSize() + 2.0f, true);
        m_activeWidget = list;

        // Create Static Header (Column Titles) using the first row
        std::string headerStr = TabListWidget::getFirstRow(data);
        m_staticHeader = new TabListWidget::ItemWidget(false);
        this->addChild(m_staticHeader);

        std::stringstream ss(headerStr);
        std::string item;
        int col = 0;
        while (std::getline(ss, item, '\t')) {
            if (col < (int)offsets.size()) {
                // Teks Kolom/Title di bawah header: Menggunakan Small Font & Normal (PC Style)
                m_staticHeader->add(new Label(item, ImColor(0x95, 0xB0, 0xD0), false, UISettings::smallFontSize()), offsets[col]);
            }
            col++;
        }
        break;
    }
	}

    m_activeWidgetStyle = style;
    m_activeWidget->setVisible(true);
}

void Content::removeActiveWidget()
{
    if (m_activeWidget != nullptr) this->removeChild(m_activeWidget);
    if (m_staticHeader != nullptr) this->removeChild(m_staticHeader);

    m_activeWidget = nullptr;
    m_staticHeader = nullptr;
    m_activeWidgetStyle = DialogStyle::NONE;
}

const std::string& Content::inputString() const
{
    if (m_activeWidgetStyle == DialogStyle::INPUT || m_activeWidgetStyle == DialogStyle::PASSWORD)
        return dynamic_cast<InputWidget*>(m_activeWidget)->inputString();
    return "";
}

int Content::listItem() const
{
    if (m_activeWidgetStyle == DialogStyle::LIST || m_activeWidgetStyle == DialogStyle::TABLIST || m_activeWidgetStyle == DialogStyle::TABLIST_HEADERS)
        return dynamic_cast<ListBox*>(m_activeWidget)->activeItemIndex();
    return -1;
}
