# 🎮 2048-SDL

![C](https://img.shields.io/badge/Language-C-blue)
![SDL2](https://img.shields.io/badge/Library-SDL2-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)
![Status](https://img.shields.io/badge/Status-Completed-success)
![License](https://img.shields.io/badge/License-MIT-yellow)

Jeu **2048** développé en **langage C** avec la bibliothèque **SDL2**.  
Projet réalisé dans un cadre **académique (L3 Informatique)** mettant en pratique la programmation graphique et la logique algorithmique.

---

## 🧩 Description

Le jeu 2048 consiste à déplacer des tuiles sur une grille **4x4** afin de fusionner celles ayant la même valeur.  
L’objectif est d’atteindre la tuile **2048** tout en maximisant le score.

---

## ✨ Fonctionnalités

- Interface graphique avec **SDL2**
- Déplacements via les **flèches du clavier**
- Fusion automatique des tuiles
- Génération aléatoire des tuiles
- Gestion et sauvegarde du score
- Détection de fin de partie (Game Over)

---

## 🛠️ Technologies utilisées

- **Langage** : C  
- **Bibliothèque graphique** : SDL2  
- **Compilation** : Makefile  
- **Système** : Windows  

---

## 📁 Structure du projet

mini-projet-main/
│── src/
│ ├── main.c
│ ├── game.c
│ └── game.h
│
│── assets/
│ ├── images/
│ └── fonts/
│
│── makefile
│── README.md

yaml
Copier le code

---

## ⚙️ Installation et exécution (Windows)

### 🔹 Prérequis
- Compilateur **GCC**
- Bibliothèque **SDL2** installée

### 🔹 Installation de SDL2
- Télécharger SDL2 depuis : https://www.libsdl.org/
- Extraire la bibliothèque
- Configurer les chemins `include` et `lib` dans votre environnement ou Makefile

---

## 🚀 Compilation et lancement

Dans le dossier du projet :

```bash
make
Puis :

bash
Copier le code
./2048
📥 Cloner le projet (pour les autres utilisateurs)
Les utilisateurs peuvent cloner le projet avec la commande suivante :

bash
Copier le code
git clone https://github.com/trrayane/2048-SDL.git
Puis accéder au dossier :

bash
Copier le code
cd 2048-SDL
🎯 Objectifs pédagogiques
Utilisation d’une bibliothèque graphique en C

Gestion des événements clavier

Implémentation de la logique du jeu 2048

Organisation et structuration d’un projet en C

Utilisation de Git et GitHub

Respect des bonnes pratiques de développement

👤 Auteur
Rayane Terki
Étudiant en L3 Informatique
📧 Email : rayaneterki55@gmail.com
💻 GitHub : https://github.com/trrayane

📜 Licence
Ce projet est sous licence MIT.
Libre d’utilisation à des fins pédagogiques et éducatives.

yaml
Copier le code

---

## ✅ Dernière commande à faire

```bash
git add README.md
git commit -m "Add README with badges and clone instructions"
git push
