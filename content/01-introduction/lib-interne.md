+++
next = "/01-introduction/projet/"
prev = "/01-introduction/lib-externes/"
weight = 4
title = "Bibliotheque interne (glmlv)"
date = "2017-01-03T22:29:22+01:00"
toc = true

+++

La bibliothèque interne, dont le code est situé dans le repertoire *lib* du template, est destiné à contenir le code partagé entre les applications. **Les classes et fonctions réutilisables doivent donc y être placées**.

Le template de base contient déjà plusieurs fichiers dans la lib:

- filesystem.hpp *Inclut une bibliothèque de manipulation du système de fichier (soit la lib experimentale de la std, soit la lib de boost si cmake a été appelé avec l'option -DGLMLV_USE_BOOST_FILESYSTEM)*
- gl_debug_output.cpp/hpp *Définit la fonction initGLDebugOutput() permettant d'activer les message de debug d'OpenGL*
- glfw.hpp *Inclut simplement les header de GLFW et glad dans le bon ordre afin d'éviter des erreurs de compil*
- GLFWHandle.hpp *Définit la classe GLFWHandle qui initialise GLFW, ouvre une fenetre, initialise OpenGL avec glad et initialise ImGUI dans son constructeur*
- GLProgram.hpp, GLShader.hpp *Définissent des classes et fonction helpers pour charger, compiler et linker des shaders GLSL*
- Image2DRGBA.hpp *Définit la classe Image2DRGBA et des fonctions de lecture/écriture d'image en surcouche de la lib stb*
- imgui_impl_glfw_gl3.hpp/.cpp *Définit les fonctions de dessin et d'interaction avec l'utilisateurs nécessaires au bon fonctionnement de ImGui*