#include "../../../main.h"
#include "../../gui.h"

#include <sstream>
#include <algorithm>

/* ListWidget */

TabListWidget::TabListWidget()
	: ListBox()
{
	m_itemSize = { 0.0f, UISettings::dialogListItemHeight() };
}

void TabListWidget::performLayout()
{
	// SAMP PC: Menambahkan padding internal agar selection tidak menempel ke border texture
	m_itemSize.x = ImMax(m_itemSize.x, this->minSize().x - 12.0f);
	this->setItemSize(m_itemSize);
	ListBox::performLayout();
}

void TabListWidget::draw(ImGuiRenderer* renderer)
{
	if (UI::m_pSampGuiTexture)
	{
		float tw = (float)UI::m_pSampGuiTexture->raster->width;
		float th = (float)UI::m_pSampGuiTexture->raster->height;

		// SAMP PC: Background texture digambar sedikit lebih besar (Bounding Box terluar)
		ImVec2 p = absolutePosition() - ImVec2(6.0f, 6.0f);
		ImVec2 s = size() + ImVec2(12.0f, 12.0f);

		// CDXUTListBox - Main
		ImVec4 rect = UI::rectListBoxMain;
		renderer->drawImageUV(p, p + s,
			ImVec2(rect.x / tw, rect.y / th),
			ImVec2(rect.z / tw, rect.w / th),
			(ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 200));
	}
	ListBox::draw(renderer);
}

std::string TabListWidget::getFirstRow(const std::string& data)
{
	size_t pos = data.find('\n');
	if (pos != std::string::npos) return data.substr(0, pos);
	return data;
}

std::string TabListWidget::removeFirstRow(const std::string& data)
{
	size_t pos = data.find('\n');
	if (pos != std::string::npos) return data.substr(pos + 1);
	return "";
}

std::vector<float> TabListWidget::assemble(const std::string& data, const std::vector<float>& offsets, bool skipFirstRow, float fontSize, bool outlined)
{
	std::vector < std::vector<Label*> > vRows;

	std::stringstream ss_data(data);
	std::string s_row;

	float fSize = fontSize == 0.0f ? UISettings::fontSize() : fontSize;
	m_itemSize.y = fSize + 2.0f; // Line spacing dirapatkan (PC Style)

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

	std::vector<float> columnOffsets;
	float max_width = 0.0f;

	if (!offsets.empty())
	{
		columnOffsets = offsets;
		for (int r_idx = (skipFirstRow ? 1 : 0); r_idx < (int)vRows.size(); r_idx++)
		{
			ItemWidget* item = new ItemWidget(true);
			this->addItem(item);

			for (int c_idx = 0; c_idx < (int)vRows[r_idx].size(); c_idx++)
			{
				if (c_idx < (int)offsets.size()) {
					item->add(vRows[r_idx][c_idx], offsets[c_idx]);
					max_width = std::max(max_width, offsets[c_idx] + vRows[r_idx][c_idx]->width());
				}
			}
		}
	}
	else
	{
		std::vector<float> vColumnsWidth;
		for (int r_idx = 0; r_idx < (int)vRows.size(); r_idx++)
		{
			for (int c_idx = 0; c_idx < (int)vRows[r_idx].size(); c_idx++)
			{
				if (vColumnsWidth.size() < (c_idx + 1)) {
					vColumnsWidth.push_back(0.0f);
				}

				vColumnsWidth[c_idx] = std::max(vColumnsWidth[c_idx], vRows[r_idx][c_idx]->width());
			}
		}

		float current_x = 5.0f; // PC Margin (5px)
		for(float w : vColumnsWidth) {
			columnOffsets.push_back(current_x);
			current_x += w + 30.0f; // PC Style Column Gap (Increased)
		}

		for (int r_idx = (skipFirstRow ? 1 : 0); r_idx < (int)vRows.size(); r_idx++)
		{
			ItemWidget* item = new ItemWidget(r_idx == 0 && !skipFirstRow ? false : true);
			this->addItem(item);

			for (int c_idx = 0; c_idx < (int)vRows[r_idx].size(); c_idx++)
			{
				float pos_x = columnOffsets[c_idx];
				Label* l = vRows[r_idx][c_idx];
				item->add(l, pos_x);
				max_width = std::max(max_width, pos_x + l->width() + 30.0f); // Extra right-side padding (PC style)
			}
		}
	}

	m_itemSize.x = max_width;
	return columnOffsets;
}

/* ListWidget::ItemWidget */
TabListWidget::ItemWidget::ItemWidget(bool selectable)
	: ListBoxItem(selectable)
{

}

void TabListWidget::ItemWidget::performLayout()
{
	for (auto& item : m_itemsData)
	{
		Label* l = item.label;
		l->setPosition(ImVec2(
			item.pos_x,
			(height() - l->height()) / 2)
		);
	}
}

void TabListWidget::ItemWidget::draw(ImGuiRenderer* renderer)
{
	if (this->selected())
	{
		// SAMP PC: Selection is Red (B92228), and no texture
		renderer->drawRect(absolutePosition(), absolutePosition() + size(), ImColor(0xB9, 0x22, 0x28, 255), true);
	}
	ListBoxItem::draw(renderer);
}

void TabListWidget::ItemWidget::add(Label* label, float pos_x)
{
	ItemData data;
	data.label = label;
	data.pos_x = pos_x;
	m_itemsData.push_back(data);

	this->addChild(label);
}
