#pragma once
#include <vector>

enum class ButtonState : unsigned char {
    Up,
    Released,
    Pressed,
    Down
};

struct ButtonStates {
    ButtonStates(size_t count) {
        states.resize(count, ButtonState::Up);
    }

    inline ButtonState GetEntry(int index) const {
        return states[index];
    }

    inline bool IsPressed(int index) const {
        return GetEntry(index) == ButtonState::Pressed;
    }
    
    void UpdateEntry(int index, bool is_up) {
        if (is_up) {
            if (states[index] == ButtonState::Down || states[index] == ButtonState::Pressed) {
                states[index] = ButtonState::Released;
            }
            else {
                states[index] = ButtonState::Up;
            }
        }
        else {
            if (states[index] == ButtonState::Released || states[index] == ButtonState::Up) {
                states[index] = ButtonState::Pressed;
            }
            else {
                states[index] = ButtonState::Down;
            }
        }
    }

    std::vector<ButtonState> states;
};
