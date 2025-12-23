# 2048 SDL2 Game 🎮

![C](https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c&logoColor=white)
![SDL2](https://img.shields.io/badge/Library-SDL2-green?style=for-the-badge)
![Windows](https://img.shields.io/badge/Platform-Windows-lightgrey?style=for-the-badge&logo=windows&logoColor=white)

<!-- GitHub dynamic badges -->
![GitHub stars](https://img.shields.io/github/stars/trrayane/2048-SDL?style=for-the-badge)
![GitHub forks](https://img.shields.io/github/forks/trrayane/2048-SDL?style=for-the-badge)
![GitHub issues](https://img.shields.io/github/issues/trrayane/2048-SDL?style=for-the-badge)
![License](https://img.shields.io/github/license/trrayane/2048-SDL?style=for-the-badge&color=yellow)

---

## Description 🎯

**2048 SDL2** is a professional desktop implementation of the classic **2048 puzzle game**, developed in **C** using the **SDL2 library**.  
Created as part of a university project at the **University of Science and Technology Houari Boumediene (USTHB), Algiers, Algeria**, this project highlights **modular game development**, **graphical interface design**, and **AI-driven gameplay mechanics**.  

The game offers multiple modes including **Player**, **Machine**, and **Machine vs Player**, providing both interactive and automated gameplay experiences.

---

## Features ✨

- 🖼 Full **graphical interface** with SDL2  
- 🎮 **Keyboard and mouse input** support  
- 🔢 **Tile merging and scoring logic**  
- 💾 **Game save/load** functionality  
- 🤖 **AI-driven machine player**  
- ⚔️ **Competitive mode** (Machine vs Player)  
- ❌ **Game over and win detection**  
- 🔊 Sound effects and basic animations  
- 🧩 Modular and maintainable **C codebase** suitable for game development practice  

---

## Game Overview 🕹️

The classic **2048 game** is a sliding puzzle where tiles of the same value combine to reach the **2048 tile**.  

Enhancements in this version:

- Separate **game modes** for player and machine  
- Machine simulates gameplay by reading **screen pixels** instead of accessing internal logic  
- Saves **matrix, score, player name, and elapsed time**  
- Supports continued gameplay after reaching **2048**  

---

## How to Play 📝

- Use **keyboard** (arrow keys) or **mouse** (on-screen arrow buttons)  
- Slide tiles in four directions: ↑ ↓ ← →  
- Merge tiles to increase your score  
- Aim to reach the **2048 tile**  
- Saved games can be loaded at any time  
- When no moves are possible, **Game Over** is displayed  

---

## Game Modes 🎮

- **Player Mode:** Play manually, start new game or continue saved game  
- **Machine Auto Mode:** Machine plays automatically with built-in strategy  
- **Machine vs Player Mode:** Player vs Machine on separate boards  
  - First to 2048 wins  
  - If no moves left, highest score wins  
  - Machine simulates moves using screen analysis  

---

## Controls 🎮

- **Player Mode:**  
  - ⌨️ **Keyboard:** Arrow keys  
  - 🖱️ **Mouse:** On-screen arrow buttons (↑, ↓, ←, →)  

- **Machine Mode:** Fully automated  

- **Machine vs Player Mode:**  
  - Human player uses standard controls  
  - Machine plays automatically  

---

## Tech Stack 🛠️

- 🖥️ **Language:** C  
- 🖼️ **Graphics & UI:** SDL2 (SDL_ttf, SDL_image, SDL_mixer)  
- 💻 **Platform:** Windows (tested)  
- ⚙️ **Build System:** Makefile  
- 🌐 **Version Control:** Git/GitHub  

---

## Authors 👤

**Terki Rayane**  
- GitHub: [trrayane](https://github.com/trrayane)  
- Email: rayaneterki55@gmail.com  
- Discord: tr_rayane

---

## License 📜

This project is licensed under the **MIT License**.
