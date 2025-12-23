# 2048 SDL2 Game 🎮

![C](https://img.shields.io/badge/Language-C-blue?style=for-the-badge)
![SDL2](https://img.shields.io/badge/Library-SDL2-green?style=for-the-badge)
![Windows](https://img.shields.io/badge/Platform-Windows-lightgrey?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## Description

**2048 SDL2** is a professional desktop implementation of the classic **2048 puzzle game**, developed in **C** using the **SDL2 library**.  
Designed as part of a university project at the **University of Science and Technology Houari Boumediene (USTHB), Algiers, Algeria**, this project demonstrates modular game development, graphical interface design, and AI-driven gameplay.  

The game offers multiple modes including **Player**, **Machine**, and **Machine vs Player**, providing both interactive and automated gameplay experiences.

---

## Game Overview

The classic **2048 game** is a sliding puzzle where you combine tiles with the same number to reach the **2048 tile**.  

This version adds:

- Full graphical interface  
- Save and load functionality  
- Machine-controlled gameplay  
- Competitive mode  
- Sound effects  

When the player reaches **2048**, a message appears, allowing them to continue or start a new game.

---

## How to Play

- Use the **keyboard** or **mouse** to move tiles  
- Slide tiles in four directions: ↑ ↓ ← →  
- Merge tiles with the same number to increase your score  
- Aim to reach the **2048 tile**  
- After reaching 2048, continue for a higher score or restart  
- Saved games preserve the matrix, score, player name, and elapsed time  
- When no moves are left, a **Game Over** message is displayed

---

## Game Modes

- **Player Mode**: Standard mode; the player controls the game manually  
- **Machine Auto Mode**: The machine plays automatically using a built-in strategy  
- **Machine vs Player Mode**: Both the player and machine play simultaneously on separate boards  
  - First to reach 2048 wins  
  - If no moves remain, the player with the higher score wins  
  - The machine simulates its moves by reading screen pixels — it does not access the game logic directly

---

## Controls

- **Player Mode:**  
  - **Keyboard:** Arrow keys  
  - **Mouse:** Click on the on-screen arrow buttons (↑, ↓, ←, →)  

- **Machine Mode:** Fully automated, no controls needed  

- **Machine vs Player Mode:**  
  - Human player uses the same controls as Player Mode  
  - Machine plays automatically

---

## Authors

**Terki Rayane**  
- GitHub: [trrayane](https://github.com/trrayane)  
- Email: rayaneterki55@gmail.com  
- Discord: tr_rayane

---

## License

This project is licensed under the **MIT License**.
