#pragma once

class TabListWidget : public ListBox
{
public:
	TabListWidget();

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;

	std::vector<float> assemble(const std::string& data, const std::vector<float>& offsets = {}, bool skipFirstRow = false, float fontSize = 0.0f, bool outlined = false);

	// Helper to get only the first row as a string (for static headers)
	static std::string getFirstRow(const std::string& data);
	// Helper to get data without the first row
	static std::string removeFirstRow(const std::string& data);

public:
	class ItemWidget : public ListBoxItem
	{
	public:
		struct ItemData
		{
			Label* label;
			float pos_x;
		};

	public:
		ItemWidget(bool selectable = true);

		void add(Label* label, float pos_x);

		void performLayout() override;
		virtual void draw(ImGuiRenderer* renderer) override;

	private:
		std::vector<ItemData> m_itemsData;
	};

private:
	ImVec2 m_itemSize;
};