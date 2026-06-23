#pragma once

#include <jni.h>
#include "../widget.h"

class InputChat : public Widget
{
public:
	InputChat();

	void clear() { m_input.clear(); m_caption.clear(); }

	void addCharToInput(char value);
	void popCharFromInput();

	void setInputString(const std::string& string) { m_input = string; m_caption = Encoding::cp2utf(string); }
	const std::string& inputString() const { return m_input; }

	void show(Widget* caller);
	void hide(bool deactivate = true);
	void send();

	void sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);
	void updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;
	virtual void activateEvent(bool active) override;

private:
	std::string m_caption;
	std::string m_input;
	Widget* m_caller;
};
