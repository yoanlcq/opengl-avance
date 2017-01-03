+++
title = "Code template"
weight = 2
prev = "/01-introduction/objectifs/"
next = "/01-introduction/lib-externes/"
toc = true
date = "2016-12-29T11:36:02+01:00"

+++

## Téléchargement

Cloner, forker ou téléchargez le repository sur github: https://github.com/Celeborn2BeAlive/opengl-avance (fork de préférence, plus facile de partager le code avec moi ensuite)

## Compilation

- Avec CMake
- Support de GCC 5+ et Visual Studio 2015

### Linux

En console:

1. Créer un repertoire *opengl-avance-build* à coté du repertoire *opengl-avance* (correspondant au clone du repo)
2. Se placer dans *opengl-avance-build*
3. Taper la commande **cmake ../opengl-avance**
4. Compiler avec **make -j**

Les executables sont compilés dans le repertoire **bin** du dossier de build

### Windows

1. Téléchargez et installez CMake GUI (https://cmake.org/download/ le Windows win64-x64 Installer ou Windows win32-x86 Installer selon votre CPU) et Visual Studio Community Edition 2015 (https://www.visualstudio.com/fr/vs/community/).
2. Créer un repertoire *opengl-avance-build* à coté du repertoire *opengl-avance* (correspondant au clone du repo)
3. Lancer CMake GUI, choisir comme repertoire source le clone du repo et comme repertoire de build le dossier que vous avez créé
4. Cliquer sur Configure et choisir comme generateur: "Visual Studio 14 2015 Win64" pour un CPU 64-bits, "Visual Studio 14 2015" pour un CPU 32-bits
5. Cliquer sur Generate
6. Ouvrir la solution Visual Studio **opengl.sln** qui a été créée par CMake dans le repertoire de build
7. Dans le panel "Explorateur de solutions", choisir le projet "template" comme projet de démarage (clic droit dessus, puis trouver l'option)
8. Menu "Générer" -> "Générer la solution"
9. Lancer l'application avec F5 (debug) ou Ctrl+F5 (non debug)

Les executables sont compilés dans les repertoire **bin**/Debug et **bin**/Release selon la configuration.

Il est possible de compiler le projet "INSTALL" de la solution avec que les executables compilés soit recopiés dans un repertoire d'installation. Par défault ce repertoire est **C:/Program Files/opengl** mais il est possible de le changer depuis CMake GUI en recherchant la variable **CMAKE_INSTALL_PREFIX** et en la modifiant.

## Structure

- apps *Contiendra le code des executables qui seront créés*
- cmake *Contient des modules cmake*
- lib *Contiendra le code partagé entre les executables*
- third-party *Contient des bibliothèques externes*

Chaque sous-repertoire de **apps** est compilé en tant qu'application differente. Ainsi le repertoire **apps/template** actuellement présent sera compilé en l'executable **template** (Linux) ou **template.exe** (Windows).

**lib** est destiné à contenir du code d'une lib (nommée **glmlv**) partagée entre les executables.

Afin de garder un code clair, je vous conseille de créer une application différente par theme de TP. Il faudra également créer une application pour le projet.

## Shaders

Pour ce qui concerne les shaders, le processus de compilation se charge de les copier à coté des executables afin qu'il soient facilement accessible de manière relative par ces dernier. Leur extension doit être ".glsl" afin qu'ils soient reconnu, et ils doivent être placés dans un sous repertoire **shaders** de l'application qui les utilise. Par convention, on suffixera ".vs.glsl" les vertex shader, ".fs.glsl" les fragments shaders, ".gs.glsl" les geometry shaders et ".cs.glsl" les compute shaders.

La lib **glmlv** peut également contenir des shaders partagés entre les applications, en les plaçant dans le repertoire **lib/shaders**.

Supposons l'arborescence suivante pour les shaders de **apps** et **libs**:

- apps
    - template
        -shaders
            - triangle.vs.glsl
            - triangle.fs.glsl
    - smab
        - shaders
            - shadow_mapping
                - sm.vs.glsl
                - sm.fs.glsl
            - deferred
                - gbuffer.vs.glsl
                - gbuffer.fs.glsl
                - shading.vs.glsl
                - shading.fs.glsl
- lib
    - shaders
        - post-processing
            - blur.cs.glsl
            - dof.cs.glsl

On a ici deux applications **template** et **smab**, contenant chacune des shaders différents et partageant deux shaders de **glmlv**.

Cette arborescence sera reproduite de la manière suivante dans le repertoire de build:

- opengl-avance-build
    - bin
        - template.exe
        - smab.exe
        - shaders
            - glmlv
                - post-processing
                    - blur.cs.glsl
                    - dof.cs.glsl
            - template
                - triangle.vs.glsl
                - triangle.fs.glsl
            - deferred
                - gbuffer.vs.glsl
                - gbuffer.fs.glsl
                - shading.vs.glsl
                - shading.fs.glsl