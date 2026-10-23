# RPG ENHANCED
This is fork of rpg game my friend Mikołaj made.
My goal was to enhance his idea to the fullest, by adding various new ideas.

This project is 50% vibe-coded. Not because I like doing that, but because I want to test limits of current Copilot level of programming in more complicated ideas. I'm strongly against using AI in any sort of creative projects, but because RPG Enhanced is something I like to work on in my free time between college and other, more important projects (wishlist Yavalanche on steam!).
I started this project as a way to learn c++, but I realised I won't be using this language too soon, I decided to create this fork by vibe coding. Not an optimal strategy, but I don't care :D

Project structure is a mess and it's spaghetti code all over the place, be aware of that.

I used [FTXUI](https://github.com/ArthurSonzogni/FTXUI) to make my console game look pleasing, with additional controls over UI.

# Gameplay
Choose your class, fight with monsters on different arenas, gain experience and level up.
You can attack directly, or use spells that can damage and debuff your enemies.
Direct attacking has option to select body part to hit. Each body part has chance to success with damage multiplier.

# Compilation

This is currently only method to compile the game:

* `mkdir build`
* `cd build`
* `cmake ..`
* `cmake --build .`
* `./rpg`

