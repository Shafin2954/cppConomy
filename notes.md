- header file incuding
- CLI class definition

- cls for windows and clear for linux/mac
- ifdef _WIN32 checks if in windows

- stringstream --> saves a formatted complex string in a variable, we are doing it for styling

- format two column box:
    splits all lines into line and stores in a vector

- styled()!
- size_t


## Command Suggestion:

\033[s (or ESC[s) — Save cursor position. Stores the current cursor location so you can return to it.
\033[u (or ESC[u) — Restore cursor position. Jumps back to the last saved cursor location.
\033[K (or ESC[K) — Clear line from cursor to end. Erases everything to the right of the cursor on the current line.
\b \b — Backspace + erase + backspace.
\b moves the cursor left one cell.
' ' overwrites the character with a space.
another \b moves left again, so the cursor stays in the original position.
These are used together to show “ghost” autocomplete text without permanently moving the cursor or leaving artifacts.


1. main.cpp
2. cli.h
3. style.h
4. world.h



## todo:
1. BoxedText
2. tech level - supply curve curve shift