#include "../../../main.h"
#include "../../gui.h"

#include <sstream>

/* ListWidget */

RwTexture* ListWidget::m_guiTexture = nullptr;

ListWidget::ListWidget()
	: ListBox()
{
	m_itemSize = { 0.0f, UISettings::dialogListItemHeight() };

	if (!m_guiTexture)
		m_guiTexture = (RwTexture*)LoadTextureFromDB("samp", "sampgui");
}

void ListWidget::performLayout()
{
	m_itemSize.x = ImMax(m_itemSize.x, this->minSize().x);
	this->setItemSize(m_itemSize);
	ListBox::performLayout();
}

void ListWidget::draw(ImGuiRenderer* renderer)
{
	if (m_guiTexture)
	{
		float tw = (float)m_guiTexture->raster->width;
		float th = (float)m_guiTexture->raster->height;

		ImVec2 p = absolutePosition();
		ImVec2 s = size();

		// CDXUTListBox - Main (13, 124, 241, 265)
		// We use 9-slice or simple stretch depending on how it looks
		// PC uses simple stretch for the background usually
		renderer->drawImageUV(p, p + s,
			ImVec2(13.0f / tw, 124.0f / th),
			ImVec2(241.0f / tw, 265.0f / th),
			(ImTextureID)m_guiTexture->raster, ImColor(255, 255, 255, 200));
	}

	ListBox::draw(renderer);
}

void ListWidget::assemble(const std::string& data, float fontSize, bool outlined)
{
	std::vector < std::vector<Label*> > vRows;

	std::stringstream ss_data(data);
	std::string s_row;

	float fSize = fontSize == 0.0f ? UISettings::fontSize() : fontSize;
	m_itemSize.y = fSize + 4.0f; // Dinamis: Mengikuti ukuran font agar spacing pas

	while (std::getline(ss_data, s_row, '\n'))
	{
		if (s_row.length() == 0) continue;

		std::vector<Label*> vRowItems;

		std::stringstream ss_row(s_row);
		std::string s_item;
		while (std::getline(ss_row, s_item, '\t'))
		{
			Label* label = new Label(s_item, ImColor(1.0f, 1.0f, 1.0f), false, fSize);
			label->performLayout();
			vRowItems.push_back(label);
		}

		vRows.push_back(vRowItems);
	}

	std::vector<float> vColumnsWidth;
	for (int r_idx = 0; r_idx < vRows.size(); r_idx++)
	{
		for (int c_idx = 0; c_idx < vRows[r_idx].size(); c_idx++)
		{
			if (vColumnsWidth.size() < (c_idx + 1)) {
				vColumnsWidth.push_back(0.0f);
			}

			vColumnsWidth[c_idx] = std::max(vColumnsWidth[c_idx], vRows[r_idx][c_idx]->width());
		}
	}

	float max_width = 0.0f;
	for (int r_idx = 0; r_idx < vRows.size(); r_idx++)
	{
		ItemWidget* item = new ItemWidget();
		this->addItem(item);

		float pos_x = UISettings::padding();
		for (int c_idx = 0; c_idx < vRows[r_idx].size(); c_idx++)
		{
			Label* l = vRows[r_idx][c_idx];
			item->add(l, pos_x);
			pos_x += vColumnsWidth[c_idx] + 30.0f; // PC Style Column Gap
		}
		max_width = std::max(max_width, pos_x + 20.0f); // Extra right-side margin
	}

	m_itemSize.x = max_width;
}

/* ListWidget::ItemWidget */
ListWidget::ItemWidget::ItemWidget()
{

}

void ListWidget::ItemWidget::performLayout()
{
	for (auto item : m_itemsData)
	{
		Label* l = item.label;
		l->setPosition(ImVec2(
			item.pos_x,
			(height() - l->height()) / 2)
		);
	}
}

void ListWidget::ItemWidget::draw(ImGuiRenderer* renderer)
{
	if (this->selected())
	{
		if (ListWidget::m_guiTexture)
		{
			float tw = (float)ListWidget::m_guiTexture->raster->width;
			float th = (float)ListWidget::m_guiTexture->raster->height;

			// CDXUTListBox - Selection (17, 269, 241, 287)
			renderer->drawImageUV(absolutePosition(), absolutePosition() + size(),
				ImVec2(17.0f / tw, 269.0f / th),
				ImVec2(241.0f / tw, 287.0f / th),
				(ImTextureID)ListWidget::m_guiTexture->raster, ImColor(255, 255, 255, 255));
		}
		else
		{
			renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(0xB9, 0x22, 0x28, 255), true);
		}
	}
	ListBoxItem::draw(renderer);
}

void ListWidget::ItemWidget::add(Label* label, float pos_x)
{
	ItemData data;
	data.label = label;
	data.pos_x = pos_x;
	m_itemsData.push_back(data);

	this->addChild(label);
}
