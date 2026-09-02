# Implementation Plan - Synchronize Chat Window with Nezuko SA-MP

This plan aims to update the Chat window implementation in the current project to match the logic, UI, and style of the `nezuko-samp` version. This involves replacing the complex `ListBox`-based system with a simpler `std::deque` and direct text rendering.

## Proposed Changes

### GUI Component

#### [MODIFY] [chatwindow.h](file:///C:/Users/runneradmin/Downloads/testing/jni/jni/gui/samp_widgets/chatwindow.h)
- Replace `ListBox` inheritance with `Widget` (or keep it but override all behavior).
- Add `ChatEntry` struct.
- Add `std::deque<ChatEntry>` for message storage.
- Add `std::mutex` for thread safety.
- Keep existing public methods (`addChatMessage`, etc.) for compatibility but update their internal implementation.
- Remove `MessageItem` and `PlayerMessageItem` inner classes.

#### [MODIFY] [chatwindow.cpp](file:///C:/Users/runneradmin/Downloads/testing/jni/jni/gui/samp_widgets/chatwindow.cpp)
- Rewrite `addMessage` to store messages in the deque.
- Update `addChatMessage`, `addInfoMessage`, `addDebugMessage`, and `addClientMessage` to use the new `addMessage` logic.
- Rewrite `draw` to use `ImGuiRenderer::drawText` directly, mimicking `nezuko-samp`'s rendering logic (fixed X/Y, line height based on font size).
- Update `performLayout` if needed, though `nezuko-samp` uses hardcoded offsets for rendering.
- Remove old `ListBox` related logic.

## Verification Plan

### Manual Verification
- Deploy the app to a device/emulator.
- Check the chat window appearance: it should now match the style of `nezuko-samp` (likely simpler text overlay).
- Verify that messages from different sources (chat, info, debug) are displayed correctly.
- Verify that nicknames in chat messages are correctly formatted (e.g., "Nick: Message").
- Verify that the message limit is respected and scrolling (or showing only N last messages) works as intended.
