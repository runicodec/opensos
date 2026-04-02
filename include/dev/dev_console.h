#pragma once
#include "core/common.h"
#include "core/input.h"
#include "dev/command_registry.h"

// Quake-style in-game developer console.
// Owns all visual and input state; has no game-state dependencies.
// GameScreen wires it to a CommandRegistry and calls the three public methods.
class DevConsole {
public:
    explicit DevConsole(CommandRegistry* registry);

    bool isOpen() const { return open_; }

    // Close without user interaction (e.g. on session reset).
    void forceClose();

    // Call first in GameScreen::handleInput().
    // Handles the ~ toggle, all text input, and command submission.
    // Returns true if input was consumed — caller must return early.
    bool handleInput(const InputState& input);

    // Call last in GameScreen::render() so it overlays everything.
    void render(SDL_Renderer* r, int screenW, int screenH);

private:
    CommandRegistry* registry_;
    bool open_ = false;

    std::string inputBuffer_;
    std::vector<std::string> outputHistory_;  // displayed lines, capped at kMaxOutput
    std::vector<std::string> inputHistory_;   // submitted commands for up/down nav
    int historyIndex_ = -1;                   // -1 means not navigating history
    int scrollOffset_ = 0;                    // lines scrolled up from bottom
    int cursorPos_   = 0;                     // insertion point within inputBuffer_

    static constexpr int kMaxOutput  = 200;
    static constexpr int kMaxHistory = 50;

    void open();
    void close();
    void submit();
};
