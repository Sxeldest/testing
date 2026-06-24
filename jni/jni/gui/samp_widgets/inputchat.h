#pragma once

#include <jni.h>
#include "../widget.h"

class InputChat : public Widget
{
public:
	InputChat();

	void clear() { m_input.clear(); m_caption.clear(); m_cursorPos = 0; }

	void addCharToInput(char value);
	void popCharFromInput();

	void setInputString(const std::string& string) { m_input = string; m_caption = Encoding::cp2utf(string); m_cursorPos = m_input.length(); }
	const std::string& inputString() const { return m_input; }

	void show(Widget* caller);
	void hide(bool deactivate = true);
	void send();
	void pushHistory(const std::string& str);

	void sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);
	void updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);
	void updateCursorForGB(int pos);
	void handleKeyStrokeForGB(int keyCode, int metaState);

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;
	virtual void activateEvent(bool active) override;

	Widget* getCaller() const { return m_caller; }

private:
	std::string m_caption;
	std::string m_input;
	std::string m_chatDraft; // Draft for chat persistence
	int m_cursorPos;
	int m_selectionStart; // -1 if no selection

	std::vector<std::string> m_history;
	int m_historyIdx;
	std::string m_currentDraft;

	Widget* m_caller;
};
