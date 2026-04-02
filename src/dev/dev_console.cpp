#include "dev/dev_console.h"
#include "core/engine.h"
#include "ui/primitives.h"
#include "ui/theme.h"
#include <algorithm>
#include <cstring>

DevConsole::DevConsole(CommandRegistry* registry)
    : registry_(registry) {}

void DevConsole::open() {
    open_ = true;
    SDL_StartTextInput();
}

void DevConsole::close() {
    open_ = false;
    SDL_StopTextInput();
    inputBuffer_.clear();
    historyIndex_ = -1;
    cursorPos_ = 0;
}

void DevConsole::forceClose() {
    if (open_) close();
}

bool DevConsole::handleInput(const InputState& input) {
    // ~ toggles the console regardless of current state.
    if (input.isKeyDown(SDL_SCANCODE_GRAVE)) {
        if (open_) close(); else open();
        return true;
    }

    if (!open_) return false;

    // Escape closes.
    if (input.isKeyDown(SDL_SCANCODE_ESCAPE)) {
        close();
        return true;
    }

    // Text input — filter out the grave character that SDL emits on the same
    // frame as SDL_KEYDOWN(GRAVE), since we already handled the toggle above.
    if (input.hasTextInput) {
        const bool graveThisFrame = input.isKeyDown(SDL_SCANCODE_GRAVE);
        for (char c : input.textInput) {
            if (graveThisFrame && (c == '`' || c == '~')) continue;
            inputBuffer_.insert(inputBuffer_.begin() + cursorPos_, c);
            cursorPos_++;
        }
    }

    // Backspace deletes the character to the left of the cursor.
    if (input.isKeyDown(SDL_SCANCODE_BACKSPACE) && cursorPos_ > 0) {
        inputBuffer_.erase(cursorPos_ - 1, 1);
        cursorPos_--;
        historyIndex_ = -1;
    }

    // Left/right arrow keys move the cursor.
    if (input.isKeyDown(SDL_SCANCODE_LEFT) && cursorPos_ > 0) {
        cursorPos_--;
        return true;
    }
    if (input.isKeyDown(SDL_SCANCODE_RIGHT) && cursorPos_ < static_cast<int>(inputBuffer_.size())) {
        cursorPos_++;
        return true;
    }

    // Enter submits the current input.
    if (input.isKeyDown(SDL_SCANCODE_RETURN)) {
        submit();
        return true;
    }

    // Up arrow navigates backwards through submitted command history.
    if (input.isKeyDown(SDL_SCANCODE_UP)) {
        if (!inputHistory_.empty()) {
            if (historyIndex_ < 0)
                historyIndex_ = static_cast<int>(inputHistory_.size()) - 1;
            else if (historyIndex_ > 0)
                historyIndex_--;
            inputBuffer_ = inputHistory_[historyIndex_];
            cursorPos_ = static_cast<int>(inputBuffer_.size());
        }
        return true;
    }

    // Down arrow navigates forwards; past the end clears the buffer.
    if (input.isKeyDown(SDL_SCANCODE_DOWN)) {
        if (historyIndex_ >= 0) {
            historyIndex_++;
            if (historyIndex_ >= static_cast<int>(inputHistory_.size())) {
                historyIndex_ = -1;
                inputBuffer_.clear();
            } else {
                inputBuffer_ = inputHistory_[historyIndex_];
            }
            cursorPos_ = static_cast<int>(inputBuffer_.size());
        }
        return true;
    }

    // Mouse wheel scrolls the output area.
    if (input.scrollY != 0) {
        scrollOffset_ -= input.scrollY;
        if (scrollOffset_ < 0) scrollOffset_ = 0;
        return true;
    }

    // Console is open: consume all remaining input.
    return true;
}

void DevConsole::submit() {
    if (inputBuffer_.empty()) return;

    outputHistory_.push_back("> " + inputBuffer_);

    // Append to navigation history, skipping consecutive duplicates.
    if (inputHistory_.empty() || inputHistory_.back() != inputBuffer_) {
        inputHistory_.push_back(inputBuffer_);
        if (static_cast<int>(inputHistory_.size()) > kMaxHistory)
            inputHistory_.erase(inputHistory_.begin());
    }

    // Dispatch and record output.
    auto result = registry_->dispatch(inputBuffer_);
    for (auto& line : result) {
        outputHistory_.push_back(line);
    }

    // Trim output history to cap.
    while (static_cast<int>(outputHistory_.size()) > kMaxOutput)
        outputHistory_.erase(outputHistory_.begin());

    inputBuffer_.clear();
    historyIndex_ = -1;
    scrollOffset_ = 0;
    cursorPos_ = 0;
}

void DevConsole::render(SDL_Renderer* r, int screenW, int screenH) {
    if (!open_) return;

    const float consoleH    = screenH * 0.40f;
    const float inputBarH   = Theme::s(20.0f);
    const float outputAreaH = consoleH - inputBarH;
    const float padX        = Theme::s(6.0f);

    const int outputFontSize = std::max(11, Theme::si(12.0f));
    const int inputFontSize  = std::max(11, Theme::si(13.0f));
    const float lineH        = static_cast<float>(outputFontSize) * 1.4f;

    // Semi-transparent background.
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    UIPrim::drawRectFilled(r, Color{10, 12, 14, 210},
                           0.0f, 0.0f,
                           static_cast<float>(screenW), consoleH);

    // Gold border at the bottom of the console.
    UIPrim::drawHLine(r, Theme::gold_dim,
                      0.0f, static_cast<float>(screenW), consoleH - 1.0f, 2);

    // --- Output area ---
    const int totalLines    = static_cast<int>(outputHistory_.size());
    const int maxVisible    = static_cast<int>(outputAreaH / lineH);
    const int maxScroll     = std::max(0, totalLines - maxVisible);
    scrollOffset_           = std::clamp(scrollOffset_, 0, maxScroll);

    const int lastLine  = std::max(0, totalLines - scrollOffset_);
    const int firstLine = std::max(0, lastLine - maxVisible);

    float lineY = outputAreaH - lineH;
    for (int i = lastLine - 1; i >= firstLine && lineY >= 0.0f; --i) {
        UIPrim::drawText(r, outputHistory_[i], outputFontSize,
                         padX, lineY, "topleft", Theme::cream);
        lineY -= lineH;
    }

    // Input bar background.
    UIPrim::drawRectFilled(r, Color{28, 30, 35, 230},
                           0.0f, outputAreaH,
                           static_cast<float>(screenW), inputBarH);

    // Separator above input bar.
    UIPrim::drawHLine(r, Theme::border,
                      0.0f, static_cast<float>(screenW), outputAreaH, 1);

    // Prompt and input text.
    const std::string promptedText = "> " + inputBuffer_;
    const float inputTextY = outputAreaH + (inputBarH - static_cast<float>(inputFontSize)) * 0.5f;
    UIPrim::drawText(r, promptedText, inputFontSize,
                     padX, inputTextY, "topleft", Theme::gold_bright);

    // Blinking cursor — positioned after the text up to cursorPos_.
    if ((SDL_GetTicks() % 1000) < 500) {
        const std::string beforeCursor = "> " + inputBuffer_.substr(0, cursorPos_);
        const int textW   = UIPrim::textWidth(beforeCursor, inputFontSize);
        const float curX  = padX + static_cast<float>(textW);
        SDL_SetRenderDrawColor(r,
            Theme::gold_bright.r, Theme::gold_bright.g, Theme::gold_bright.b, 255);
        SDL_Rect cursorRect = {
            static_cast<int>(curX),
            static_cast<int>(inputTextY),
            2,
            inputFontSize
        };
        SDL_RenderFillRect(r, &cursorRect);
    }
}
