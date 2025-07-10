# Obsidian

Moteur de jeu basé sur l'API graphique Vulkan

## Prérequis

- **Vulkan SDK** : Téléchargez et installez depuis [LunarG](https://vulkan.lunarg.com/)
- **CMake** : Version 3.16 ou supérieure
- **Visual Studio 2022** (ou compatible C++23)
- **Git** pour cloner les dépendances

## Installation des dépendances

### 1. Créer le dossier Libraries
```bash
mkdir Libraries
cd Libraries
```

### 2. Télécharger GLFW
```bash
# Télécharger GLFW 3.4 depuis https://github.com/glfw/glfw/releases
# Extraire dans Libraries/glfw-3.4/
```

### 3. Télécharger GLM
```bash
# Télécharger GLM 1.0.1 depuis https://github.com/g-truc/glm/releases
# Extraire dans Libraries/glm-1.0.1/
```

### 4. Configurer Vulkan SDK
Créer un fichier `.env` à la racine du projet :
```properties
VULKAN_SDK=C:/dev/VulkanSDK/1.4.313.2
```
> Remplacez par votre chemin d'installation Vulkan SDK

## Compilation

### Avec VS Code
1. Ouvrir le projet dans VS Code
2. Installer les extensions recommandées :
   - C/C++ Extension Pack
   - CMake Tools
3. Appuyer sur `Ctrl+Shift+B` pour compiler
4. Appuyer sur `F5` pour déboguer

### Avec la ligne de commande
```bash
# Configuration
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Compilation
cmake --build build --config Debug

# Exécution
.\build\bin\Debug\Obsidian.exe
```

## Structure du projet

```
Obsidian/
├── engine/           # Code source du moteur
│   └── main.cpp
├── Libraries/        # Bibliothèques externes (non versionnées)
│   ├── glfw-3.4/
│   └── glm-1.0.1/
├── .vscode/          # Configuration VS Code
├── CMakeLists.txt    # Configuration CMake
└── README.md
```

## Fonctionnalités

- [x] Initialisation Vulkan
- [x] Création de fenêtre avec GLFW
- [x] Intégration GLM pour les mathématiques
- [ ] Rendu de base
- [ ] Système d'entités
- [ ] Chargement de modèles

## Licence

Ce projet est sous licence MIT.
