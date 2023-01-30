# BirbType
Minimalist typing game inspired by [Monkeytype](https://monkeytype.com/). The game is made using the [Birb2D](https://github.com/Toasterbirb/Birb2D) game engine

## Keybindings
- `Left click` and `Right click` to change the background color
- `Backspace` to go back by one letter (kinda expected :P)
- `LCTRL + Backspace` to go back by one word
- `LCTRL + U` clears the entire line
- `LCTRL + R` restarts the WPM counter and randomizes the words to type
- `Page Up` and `Page down` changes the text size

## Building the project
```sh
git clone --recursive https://github.com/Toasterbirb/BirbType
cd BirbType
mkdir build ; cd build
cmake .. && cmake -j$(nproc)
```
