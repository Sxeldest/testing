#include "../../main.h"
#include "../gui.h"
#include "../../game/game.h"
#include "../../log.h"
#include "../settings.h"
#include "../java/jniutil.h"

extern CGame* pGame;
extern UI* pUI;
extern CSettings* pSettings;
extern CJavaWrapper* pJavaWrapper;

InputChat::InputChat()
{
    m_input = "";
    m_caption = "";
    m_cursorPos = 0;
    m_selectionStart = -1;
    m_historyIdx = 0;
    m_caller = nullptr;
}

void InputChat::addCharToInput(char value)
{
    m_input.insert(m_cursorPos, 1, value);
    m_cursorPos++;
    m_caption = Encoding::cp2utf(m_input);
    if (m_caller) {
        m_caller->keyboardEvent(m_input);
        m_caller->cursorEvent(m_cursorPos);
    }
}

void InputChat::popCharFromInput()
{
    if (m_input.empty() || m_cursorPos <= 0) return;
    m_input.erase(m_cursorPos - 1, 1);
    m_cursorPos--;
    m_caption = Encoding::cp2utf(m_input);
    if (m_caller) {
        m_caller->keyboardEvent(m_input);
        m_caller->cursorEvent(m_cursorPos);
    }
}

void InputChat::performLayout()
{
    if (m_caller && m_caller != pUI->chat())
    {
        this->setPosition(m_caller->absolutePosition());
        this->setFixedSize(m_caller->size());
        return;
    }

    if (!pSettings) return;

    float chatX = pSettings->Get().fChatPosX;
    float chatY = pSettings->Get().fChatPosY;

    float inputW = 800.0f;
    float inputH = 45.0f;

    float fontSize = pSettings->Get().fFontSize;
    int maxLines = pSettings->Get().iChatMaxMessages;
    if (maxLines <= 0) maxLines = 10;
    float chatH = fontSize * (float)maxLines;

    this->setPosition(ImVec2(chatX - 5.0f, chatY + chatH + 20.0f));
    this->setFixedSize(ImVec2(inputW, inputH));
}

void InputChat::draw(ImGuiRenderer* renderer)
{
    if (!m_caller) return;

    this->performLayout();

    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Escape]))
    {
        this->hide(true);
        return;
    }

    bool ctrl = io.KeyCtrl;

    if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_UpArrow]))
    {
        if (m_historyIdx == m_history.size()) m_currentDraft = m_input;
        if (m_historyIdx > 0)
        {
            m_historyIdx--;
            setInputString(m_history[m_historyIdx]);
            m_cursorPos = (int)m_input.length();
            m_selectionStart = -1;
            if (pJavaWrapper) pJavaWrapper->SetKeyboardText(m_input.c_str());
            if (m_caller) {
                m_caller->keyboardEvent(m_input);
                m_caller->cursorEvent(m_cursorPos);
            }
        }
    }

    if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_DownArrow]))
    {
        if (m_historyIdx < m_history.size())
        {
            m_historyIdx++;
            if (m_historyIdx == m_history.size()) setInputString(m_currentDraft);
            else setInputString(m_history[m_historyIdx]);
            m_cursorPos = (int)m_input.length();
            m_selectionStart = -1;
            if (pJavaWrapper) pJavaWrapper->SetKeyboardText(m_input.c_str());
            if (m_caller) {
                m_caller->keyboardEvent(m_input);
                m_caller->cursorEvent(m_cursorPos);
            }
        }
    }

    if (ctrl)
    {
        if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_A]))
        {
            m_selectionStart = 0;
            m_cursorPos = (int)m_input.length();
            if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(0, m_cursorPos);
            if (m_caller) m_caller->cursorEvent(m_cursorPos);
        }
        else if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_C]))
        {
            if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
            {
                int start = std::min(m_selectionStart, m_cursorPos);
                int len = std::abs(m_selectionStart - m_cursorPos);
                std::string selected = m_input.substr(start, len);
                ImGui::SetClipboardText(selected.c_str());
            }
        }
        else if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_V]))
        {
            const char* clipboard = ImGui::GetClipboardText();
            if (clipboard)
            {
                if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
                {
                    int start = std::min(m_selectionStart, m_cursorPos);
                    int len = std::abs(m_selectionStart - m_cursorPos);
                    m_input.erase(start, len);
                    m_cursorPos = start;
                    m_selectionStart = -1;
                }
                m_input.insert(m_cursorPos, clipboard);
                m_cursorPos += strlen(clipboard);
                m_caption = Encoding::cp2utf(m_input);
                if (pJavaWrapper) {
                    pJavaWrapper->SetKeyboardText(m_input.c_str());
                    pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
                }
                if (m_caller) {
                    m_caller->keyboardEvent(m_input);
                    m_caller->cursorEvent(m_cursorPos);
                }
            }
        }
        else if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_X]))
        {
            if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
            {
                int start = std::min(m_selectionStart, m_cursorPos);
                int len = std::abs(m_selectionStart - m_cursorPos);
                std::string selected = m_input.substr(start, len);
                ImGui::SetClipboardText(selected.c_str());
                m_input.erase(start, len);
                m_cursorPos = start;
                m_selectionStart = -1;
                m_caption = Encoding::cp2utf(m_input);
                if (pJavaWrapper) {
                    pJavaWrapper->SetKeyboardText(m_input.c_str());
                    pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
                }
                if (m_caller) {
                    m_caller->keyboardEvent(m_input);
                    m_caller->cursorEvent(m_cursorPos);
                }
            }
        }
    }

    if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_LeftArrow]))
    {
        if (m_cursorPos > 0) m_cursorPos--;
        m_selectionStart = -1;
        if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }

    if (ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_RightArrow]))
    {
        if (m_cursorPos < m_input.length()) m_cursorPos++;
        m_selectionStart = -1;
        if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }

    if (ImGui::IsMouseClicked(0) && contains(io.MousePos))
    {
        float font_sz = UISettings::fontSize() + 2.0f;
        ImVec2 textPos = absolutePosition() + ImVec2(8.0f, (height() - font_sz) / 2);
        float relX = io.MousePos.x - textPos.x;

        float bestDist = 10000.0f;
        int bestIdx = 0;

        for (int i = 0; i <= m_input.length(); i++)
        {
            std::string sub = m_input.substr(0, i);
            std::string cap = Encoding::cp2utf(sub);
            float width = renderer->calculateTextSize(cap, font_sz).x;
            float dist = fabsf(relX - width);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestIdx = i;
            }
            else if (width > relX + 20.0f) break;
        }
        m_cursorPos = bestIdx;
        m_selectionStart = -1;
        if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }

    if (m_caller != pUI->chat()) return;

    if (UI::m_pSampGuiTexture)
    {
        float tw = (float)UI::m_pSampGuiTexture->raster->width;
        float th = (float)UI::m_pSampGuiTexture->raster->height;
        ImVec2 p = absolutePosition();
        p.x = floorf(p.x); p.y = floorf(p.y);
        ImVec2 s = size();

        float v_w = 4.0f;
        ImVec4 rect = UI::rectEditBox;
        float mtx = rect.x; float mty = rect.y;
        float mtw = rect.z - rect.x; float mth = rect.w - rect.y;

        float t_lw = 6.0f; float t_rw = 5.0f;
        float t_th = 8.0f; float t_bh = 8.0f;
        float t_mw = mtw - t_lw - t_rw;
        float t_mh = mth - t_th - t_bh;

        auto drawPart = [&](float stx, float sty, float stw, float sth, float dx, float dy, float dw, float dh) {
            renderer->drawImageUV(p + ImVec2(dx, dy), p + ImVec2(dx + dw, dy + dh),
                                  ImVec2(stx/tw, sty/th), ImVec2((stx+stw)/tw, (sty+sth)/th), (ImTextureID)UI::m_pSampGuiTexture->raster, ImColor(255, 255, 255, 220));
        };

        drawPart(mtx, mty, t_lw, t_th, 0, 0, v_w, v_w);
        drawPart(mtx + t_lw, mty, t_mw, t_th, v_w, 0, s.x - v_w*2, v_w);
        drawPart(mtx + t_lw + t_mw, mty, t_rw, t_th, s.x - v_w, 0, v_w, v_w);

        drawPart(mtx, mty + t_th, t_lw, t_mh, 0, v_w, v_w, s.y - v_w*2);
        drawPart(mtx + t_lw, mty + t_th, t_mw, t_mh, v_w, v_w, s.x - v_w*2, s.y - v_w*2);
        drawPart(mtx + t_lw + t_mw, mty + t_th, t_rw, t_mh, s.x - v_w, v_w, v_w, s.y - v_w*2);

        drawPart(mtx, mty + t_th + t_mh, t_lw, t_bh, 0, s.y - v_w, v_w, v_w);
        drawPart(mtx + t_lw, mty + t_th + t_mh, t_mw, t_bh, v_w, s.y - v_w, s.x - v_w*2, v_w);
        drawPart(mtx + t_lw + t_mw, mty + t_th + t_mh, t_rw, t_bh, s.x - v_w, s.y - v_w, v_w, v_w);
    }

    float font_sz = UISettings::fontSize() + 4.0f;
    ImVec2 textPos = absolutePosition() + ImVec2(8.0f, (height() - font_sz) / 2);

    if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
    {
        int start = std::min(m_selectionStart, m_cursorPos);
        int end = std::max(m_selectionStart, m_cursorPos);

        std::string textBeforeStart = m_input.substr(0, start);
        std::string textSelection = m_input.substr(start, end - start);

        float startOffset = renderer->calculateTextSize(Encoding::cp2utf(textBeforeStart), font_sz).x;
        float selWidth = renderer->calculateTextSize(Encoding::cp2utf(textSelection), font_sz).x;

        renderer->drawRect(ImVec2(textPos.x + startOffset, textPos.y),
                           ImVec2(textPos.x + startOffset + selWidth, textPos.y + font_sz),
                           ImColor(255, 50, 50, 150), true);
    }

    renderer->drawText(textPos, ImColor(1.0f, 1.0f, 1.0f), m_caption, true, font_sz);

    if (pJavaWrapper && pJavaWrapper->IsKeyboardShowing())
    {
        std::string textBeforeCursor = m_input.substr(0, m_cursorPos);
        std::string captionBeforeCursor = Encoding::cp2utf(textBeforeCursor);
        float cursorOffset = renderer->calculateTextSize(captionBeforeCursor, font_sz).x;

        static float cursorAnim = 0.0f;
        cursorAnim += ImGui::GetIO().DeltaTime;
        if (fmodf(cursorAnim, 1.0f) < 0.5f)
        {
            renderer->drawRect(ImVec2(textPos.x + cursorOffset, textPos.y - 2.0f),
                               ImVec2(textPos.x + cursorOffset + 1.0f, textPos.y + font_sz + 2.0f),
                               ImColor(255, 255, 255, 255), true);
        }
    }
}

void InputChat::show(Widget* caller)
{
    if (m_caller == pUI->chat()) {
        m_chatDraft = m_input;
    }

    if (m_caller != caller) {
        m_input.clear();
        m_caption.clear();
        m_cursorPos = 0;
    }

    m_caller = caller;

    if (m_caller == pUI->chat()) {
        m_input = m_chatDraft;
        m_caption = Encoding::cp2utf(m_input);
        m_cursorPos = (int)m_input.length();
    }

    this->setVisible(true);

    pJavaWrapper->SetKeyboardText(m_input.c_str());
    pJavaWrapper->ShowKeyboard();
    pGame->EnableGameInput(false);
}

void InputChat::hide(bool deactivate)
{
    if (deactivate)
    {
        pJavaWrapper->SetKeyboardText("");
        pJavaWrapper->HideKeyboard();
        this->setVisible(false);
        if (m_caller) m_caller->setActive(false);
        m_caller = nullptr;
        pGame->EnableGameInput(true);
    }
    else
    {
        pJavaWrapper->HideKeyboard();
    }
}

void InputChat::send()
{
    if (m_caller) {
        m_caller->keyboardEvent(m_input);
        m_caller->onSubmit();
    }

    if (m_caller == pUI->chat()) {
        m_chatDraft.clear();
    }

    pushHistory(m_input);
    this->hide();
}

void InputChat::sendForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
    jsize v8 = pEnv->GetArrayLength(str);
    char* buffer = (char*)malloc(v8 + 1);
    jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
    memcpy(buffer, elements, v8);
    buffer[v8] = 0;
    std::string input = std::string(buffer);

    setInputString(input);

    if (m_caller) {
        m_caller->keyboardEvent(input);
        m_caller->onSubmit();
    }

    if (m_caller == pUI->chat()) {
        m_chatDraft.clear();
    }

    pushHistory(input);
    hide();

    free(buffer);
    pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void InputChat::updateForGB(JNIEnv *pEnv, jobject thiz, jbyteArray str)
{
    jsize v8 = pEnv->GetArrayLength(str);
    char* buffer = (char*)malloc(v8 + 1);
    jbyte* elements = pEnv->GetByteArrayElements(str, NULL);
    memcpy(buffer, elements, v8);
    buffer[v8] = 0;
    std::string input = std::string(buffer);

    m_input = input;
    m_caption = Encoding::cp2utf(m_input);
    if (m_cursorPos > m_input.length()) m_cursorPos = (int)m_input.length();
    m_selectionStart = -1;

    if (m_caller) {
        m_caller->keyboardEvent(input);
        m_caller->cursorEvent(m_cursorPos);
    }

    free(buffer);
    pEnv->ReleaseByteArrayElements(str, elements, JNI_ABORT);
}

void InputChat::updateCursorForGB(int start, int end)
{
    if (start >= 0 && start <= m_input.length() && end >= 0 && end <= m_input.length()) {
        if (start == end) {
            m_cursorPos = start;
            m_selectionStart = -1;
        } else {
            m_selectionStart = start;
            m_cursorPos = end;
        }
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }
}

void InputChat::handleKeyStrokeForGB(int keyCode, int metaState)
{
    ImGuiIO& io = ImGui::GetIO();

    if (m_caller) m_caller->keyStrokeEvent(keyCode, metaState);

    if (keyCode == 19)
    {
        if (m_historyIdx == m_history.size()) m_currentDraft = m_input;
        if (m_historyIdx > 0)
        {
            m_historyIdx--;
            setInputString(m_history[m_historyIdx]);
            m_cursorPos = (int)m_input.length();
            m_selectionStart = -1;
            if (pJavaWrapper) pJavaWrapper->SetKeyboardText(m_input.c_str());
            if (m_caller) {
                m_caller->keyboardEvent(m_input);
                m_caller->cursorEvent(m_cursorPos);
            }
        }
    }
    else if (keyCode == 20)
    {
        if (m_historyIdx < m_history.size())
        {
            m_historyIdx++;
            if (m_historyIdx == m_history.size()) setInputString(m_currentDraft);
            else setInputString(m_history[m_historyIdx]);
            m_cursorPos = (int)m_input.length();
            m_selectionStart = -1;
            if (pJavaWrapper) pJavaWrapper->SetKeyboardText(m_input.c_str());
            if (m_caller) {
                m_caller->keyboardEvent(m_input);
                m_caller->cursorEvent(m_cursorPos);
            }
        }
    }
    else if (keyCode == 21)
    {
        if (m_cursorPos > 0) m_cursorPos--;
        m_selectionStart = -1;
        if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }
    else if (keyCode == 22)
    {
        if (m_cursorPos < m_input.length()) m_cursorPos++;
        m_selectionStart = -1;
        if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
        if (m_caller) m_caller->cursorEvent(m_cursorPos);
    }

    bool ctrl = (metaState & 0x1000) != 0 || (metaState & 0x2000) != 0;
    if (ctrl)
    {
        if (keyCode == 29)
        {
            m_selectionStart = 0;
            m_cursorPos = (int)m_input.length();
            if (pJavaWrapper) pJavaWrapper->SetKeyboardSelection(0, m_cursorPos);
            if (m_caller) m_caller->cursorEvent(m_cursorPos);
        }
        else if (keyCode == 31)
        {
            if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
            {
                int start = std::min(m_selectionStart, m_cursorPos);
                int len = std::abs(m_selectionStart - m_cursorPos);
                std::string selected = m_input.substr(start, len);
                ImGui::SetClipboardText(selected.c_str());
            }
        }
        else if (keyCode == 50)
        {
            const char* clipboard = ImGui::GetClipboardText();
            if (clipboard)
            {
                if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
                {
                    int start = std::min(m_selectionStart, m_cursorPos);
                    int len = std::abs(m_selectionStart - m_cursorPos);
                    m_input.erase(start, len);
                    m_cursorPos = start;
                    m_selectionStart = -1;
                }
                m_input.insert(m_cursorPos, clipboard);
                m_cursorPos += strlen(clipboard);
                m_caption = Encoding::cp2utf(m_input);
                if (pJavaWrapper) {
                    pJavaWrapper->SetKeyboardText(m_input.c_str());
                    pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
                }
                if (m_caller) {
                    m_caller->keyboardEvent(m_input);
                    m_caller->cursorEvent(m_cursorPos);
                }
            }
        }
        else if (keyCode == 52)
        {
            if (m_selectionStart != -1 && m_selectionStart != m_cursorPos)
            {
                int start = std::min(m_selectionStart, m_cursorPos);
                int len = std::abs(m_selectionStart - m_cursorPos);
                std::string selected = m_input.substr(start, len);
                ImGui::SetClipboardText(selected.c_str());
                m_input.erase(start, len);
                m_cursorPos = start;
                m_selectionStart = -1;
                m_caption = Encoding::cp2utf(m_input);
                if (pJavaWrapper) {
                    pJavaWrapper->SetKeyboardText(m_input.c_str());
                    pJavaWrapper->SetKeyboardSelection(m_cursorPos, m_cursorPos);
                }
                if (m_caller) {
                    m_caller->keyboardEvent(m_input);
                    m_caller->cursorEvent(m_cursorPos);
                }
            }
        }
    }
}

void InputChat::pushHistory(const std::string& str)
{
    if (str.empty()) return;
    if (!m_history.empty() && m_history.back() == str)
    {
        m_historyIdx = (int)m_history.size();
        return;
    }

    m_history.push_back(str);
    if (m_history.size() > 50) m_history.erase(m_history.begin());
    m_historyIdx = (int)m_history.size();
}

void InputChat::activateEvent(bool active)
{
}
