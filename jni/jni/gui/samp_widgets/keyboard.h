#pragma once

#include <jni.h>

class ChatInput : public Widget
{
public:
	ChatInput();

	void clear() { m_input.clear(); m_caption.clear(); }

	void addCharToInput(char value);
	void popCharFromInput();

	void setInputString(const std::string& string) { m_input = string; m_caption = Encoding::cp2utf(string); }
	const std::string& inputString() const { return m_input; }

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;

private:
	std::string m_caption;
	std::string m_input;
};

class Keyboard : public Widget
{
public:
	Keyboard();

	void show(Widget* caller);
	void hide();
	void send();
	void sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);
	void updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str);

	virtual void performLayout() override;
	virtual void draw(ImGuiRenderer* renderer) override;

	ChatInput* chatInput() const { return m_chatInput; }

	void activateEvent(bool active) override;

private:
	ChatInput* m_chatInput;
	Widget* m_caller;
};
