+++
next = "/01-introduction/projet/"
toc = true
date = "2016-12-29T12:24:50+01:00"
title = "Bibliotheques externes"
weight = 3
prev = "/01-introduction/code-template/"

+++

Le template contient plusieurs bibliothèques externes (dans **third-party**) afin de vous simplifier la vie:

- **glfw** (http://www.glfw.org/): bibliothèque de fenetrage et de gestion d'inputs (similaire à la SDL)
- **glm** (http://glm.g-truc.net/): bibliothèque de maths "à la GLSL"
- **glad** (http://glad.dav1d.de/): bibliothèque pour "charger" les fonctions OpenGL 3+ (similaire à GLEW)
- **imgui** (https://github.com/ocornut/imgui): bibliotheque permettant d'afficher une interface utilisateur de manière simple et en "immediate mode"
- **json** (https://github.com/nlohmann/json): bibliotheque pour lire/écrire des fichiers json, pratique pour gérer rapidement des fichiers de config
- **tinyobjloader** (https://github.com/syoyo/tinyobjloader): bibliotheque pour charger des fichiers OBJ (format simple de scenes 3D)

Mon objectif n'étant pas de vous apprendre à utiliser ces libs, je détaillerais assez peu leurs fonctionnalités dans les TPs. Les liens ci-dessus redirige vers des documentations assez complètes pour que vous puissiez vous débrouiller :)

A noter que ImGui fournie une fonction ImGui::ShowTestWindow dont le code montre des exemples d'utilisation d'a peu près tous les widgets de la lib. Le code de cette fonction est dans le fichier *imgui_demo.cpp* du repertoire contenant la lib.