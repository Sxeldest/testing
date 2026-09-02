#pragma once

#include <string>
#include <deque>
#include <mutex>
#include "../widget.h"

#define MAX_MESSAGES 100

struct ChatEntry {
	std::string message;
	ImColor color;
};

class Chat : public Widget
{
public:
	Chat();

	void addChatMessage(const std::string& message, const std::string& nick, const ImColor& nick_color);
	void addInfoMessage(const std::string& format, ...);
	void addDebugMessage(const std::string& format, ...);
	void addClientMessage(const std::string& message, const ImColor& color);

	virtual void draw(ImGuiRenderer* renderer) override;
	virtual void performLayout() override;

	virtual void activateEvent(bool active) override;
	virtual void touchEvent(const ImVec2& pos, TouchType type) override;
	virtual void touchPopEvent() override;
	virtual void keyboardEvent(const std::string& input) override;
	virtual void onSubmit() override;

private:
	void addMessage(const std::string& message, const ImColor& color = ImColor(1.0f, 1.0f, 1.0f));

private:
	std::deque<ChatEntry> m_messages;
	std::mutex m_mutex;
	bool m_active;
	int m_scrollIdx;
};
