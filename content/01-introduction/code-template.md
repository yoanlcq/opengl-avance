+++
title = "Code template"
weight = 2
prev = "/01-introduction/introduction/"
next = "/01-introduction/lib-externes/"
toc = true
date = "2016-12-29T11:36:02+01:00"

+++

## Téléchargement

Clonez, forkez sur votre compte Github ou téléchargez le [repository sur github](https://github.com/Celeborn2BeAlive/opengl-avance) (fork de préférence, plus facile de partager le code avec moi ensuite)

## Branches du repo

Principales:

- master *Code de base + apps d'exemple en OpenGL 3*
- master-direct-state-access *Code de base + apps d'exemple en OpenGL 4.5 (avec DSA)*

Corrections:

- cheat *Correction des TDs en OpenGL 3*
- cheat-direct-state-access *Correction des TDs en OpenGL OpenGL 4.5 (avec DSA)*

## Compilation

- Avec CMake
- Support de GCC 5+ (sans boost), GCC 4.9.2 (boost requis) et Visual Studio 2015 (sans boost)

### Linux

En console:

1. Créer un repertoire *opengl-avance-build* à coté du repertoire *opengl-avance* (correspondant au clone du repo)
2. Se placer dans *opengl-avance-build*
3. Sur un PC ou GCC 5+ est installé, taper la commande **cmake ../opengl-avance**. Si uniquement GCC 4.9.2 est disponible (machines de la FAC), taper la commande **cmake -DGLMLV_USE_BOOST_FILESYSTEM=ON ../opengl-avance**
4. Compiler avec **make -j**

Les executables sont compilés dans le repertoire *bin* du dossier de build

### Windows

1. Télécharger et installer [CMake GUI](https://cmake.org/download/) (Windows win64-x64 Installer ou Windows win32-x86 Installer selon votre CPU) et [Visual Studio Community Edition 2015](https://www.visualstudio.com/fr/vs/community/).
2. Créer un repertoire *opengl-avance-build* à coté du repertoire *opengl-avance* (correspondant au clone du repo)
3. Lancer CMake GUI, choisir comme repertoire source le clone du repo et comme repertoire de build le dossier que vous avez créé
4. Cliquer sur **Configure** et choisir comme generateur: *Visual Studio 14 2015 Win64* pour un CPU 64-bits, *Visual Studio 14 2015* pour un CPU 32-bits (ça existe encore ?)
5. Cliquer sur **Generate**
6. Ouvrir la solution Visual Studio *opengl.sln* qui a été créée par CMake dans le repertoire de build
7. Dans le panel *Explorateur de solutions*, choisir le projet *template* comme projet de démarage (clic droit dessus, puis trouver l'option)
8. Menu *Générer* -> *Générer la solution*
9. Lancer l'application avec F5 (debug) ou Ctrl+F5 (non debug)

Les executables sont compilés dans les repertoire *bin/Debug* et *bin/Release* selon la configuration.

Il est possible de compiler le projet *INSTALL* de la solution avec que les executables compilés soit recopiés dans un repertoire d'installation. Par défault ce repertoire est *C:/Program Files/opengl* mais il est possible de le changer depuis CMake GUI en recherchant la variable *CMAKE_INSTALL_PREFIX* et en la modifiant.

## Arborescence

- apps *Contient le code des executables à compiler*
- cmake *Contient des modules cmake*
- lib *Contient le code partagé entre les executables*
- third-party *Contient des bibliothèques externes*

Chaque sous-repertoire de *apps* est compilé en tant qu'application indépendant. Ainsi le code du repertoire *apps/template* sera compilé en un executable *template* (Linux) ou *template.exe* (Windows), et le code du repertoire *apps/triangle* sera compilé en un executable *triangle* (Linux) ou *triangle.exe* (Windows).

Le répertoire *lib* est destiné à contenir du code d'une lib (nommée *glmlv*) partagée entre les executables.

Afin de garder un code clair, je vous conseille de créer une application différente par theme de TP. Il faudra également créer une application pour le projet.

## Shaders

Concernant les shaders GLSL, le processus de compilation se charge de les copier à coté des executables afin qu'il soient facilement accessible (de manière relative) par ces derniers. Leur extension doit être ".glsl" afin qu'ils soient reconnu par le CMakeLists.txt, et ils doivent être placés dans un sous repertoire *shaders* de l'application qui les utilise. Par convention, on suffixera ".vs.glsl" les vertex shader, ".fs.glsl" les fragments shaders, ".gs.glsl" les geometry shaders et ".cs.glsl" les compute shaders.

La lib *glmlv* peut également contenir des shaders partagés entre les applications, en les plaçant dans le repertoire *lib/shaders*.

Par exemple, supposons l'arborescence suivante pour les shaders de *apps* et *libs*:

- apps
    - triangle
        - shaders
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

On a ici deux applications *triangle* et *smab*, contenant chacune des shaders différents et partageant deux shaders de *glmlv*.

Cette arborescence sera reproduite de la manière suivante dans le repertoire de build:

- opengl-avance-build
    - bin
        - triangle.exe
        - smab.exe
        - shaders
            - glmlv
                - post-processing
                    - blur.cs.glsl
                    - dof.cs.glsl
            - triangle
                - triangle.vs.glsl
                - triangle.fs.glsl
            - smab
                - shadow_mapping
                    - sm.vs.glsl
                    - sm.fs.glsl
                - deferred
                    - gbuffer.vs.glsl
                    - gbuffer.fs.glsl
                    - shading.vs.glsl
                    - shading.fs.glsl

De cette manière, un code comme le suivant accède facilement aux shaders **sans dépendre de l'emplacement de ces derniers sur la machine de l'utilisateur**.

```cpp
#include <glmlv/filesystem.hpp>

// Supposons que le code est celui de smab.exe, dont le chemin est C:/toto/tata/bin/smab.exe
int main(int argc, char** argv)
{
    // argv[0] contient toujours le chemin vers l'executable, ici "C:/toto/tata/bin/smab.exe"
    // On le récupère dans une variable de type glmlv::fs::path permettant de gérer facilement les chemins
    const auto applicationPath = glmlv::fs::path{ argv[0] };

    // stem() renvoit le nom du fichier sans son extension, i.e. "smab":
    const auto appName = applicationPath.stem().string();

    // L'opérateur '/' permet de concatener les chemins de fichier
    const auto shadersRootPath = applicationPath.parent_path() / "shaders";

    // Pour les shaders de l'application, on utilise la variable contenant le nom de l'application pour accéder au dossier contenant ses shaders
    const auto pathToSMVS = shadersRootPath / appName / "shadow_mapping" / "sm.vs.glsl";
    const auto pathToSMFS = shadersRootPath / appName / "shadow_mapping" / "sm.fs.glsl";

    // Pour les shaders de la lib, on cible directement le dossier "glmlv"
    const auto pathToBlurCS = shadersRootPath / "glmlv" / "post-processing" / "blur.cs.glsl";

    [...]

    return 0;
}
```

## Applications d'exemple

Le repertoire *apps* contient déjà du code pour plusieurs applications simples. Toutes sont basées sur le template de base *template* qui se contente d'ouvrir une fenêtre contenant une GUI affichant le framerate et permettant de changer la couleur de fond.

Ces codes simples sont destinés à vous donner quelques exemple d'appels aux fonctions OpenGL, en particulier l'utilisation des fonction de l'extension direct_state_access.

Une ressource supplémentaire est [ce repository GIT](https://github.com/g-truc/ogl-samples/tree/master/tests) contenant un grand nombre d'exemples de code pour l'ensemble des features OpenGL.

Voici une description des apps d'exemple:

- triangle *Dessine un triangle coloré* (création de VBO, création de VAO, attributs entrelacés, chargement de shaders)
- triangle_2vbos *Pareil mais en utilisant 2 VBOs, un pour les positions, l'autre pour les couleurs* (plusieurs VBOs pour un objet, attributs non entrelacés)
- quad *Dessine un quad coloré* (création d'IBO, i.e. buffer d'index)