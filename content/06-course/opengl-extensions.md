+++
title = "OpenGL Extensions"
date = "2017-01-20T00:05:53+01:00"
toc = true
weight = 2

+++

Le méchanisme d'extensions OpenGL permet aux constructeurs de carte graphique d'ajouter à l'API des fonctionnalités avancées qui ne sont pas encore présentes dans le Core profile.

Il est assez standard d'utiliser des extensions OpenGL et d'adapter son moteur en fonction des extensions disponibles sur la carte graphique de l'utilisateur.

Le nom des extensions suit une nomenclature assez précises. Il y a 3 types d'extensions:

- Propriétaires: Spécifiques à un seul constructeur. Par exemple les extensions préfixées par GL_NV sont spécifiques aux GPU de NVidia.
- Génériques: Généralement implémentées par un grand nombre de constructeurs. Préfixées par GL_EXT
- ARB: Extensions génériques approuvées par l'[OpenGL ARB](https://www.opengl.org/archives/about/arb/), destinées à entrer dans le Core profile d'une future version d'OpenGL.

Les extensions sont détaillées dans un fichier texte dedié assez difficile à lire (exemple: https://www.opengl.org/registry/specs/ARB/vertex_array_object.txt) car il mentionne des additions, suppressions ou modifications dans la spécification générale d'OpenGL.

Plus d'infos sur le Wiki: https://www.khronos.org/opengl/wiki/OpenGL_Extension

Pour savoir si une extension OpenGL est proposée par votre driver, vous pouvez utiliser la commande *glxinfo sous Linux (*nvidia-config --glxinfo* sous certains système) et piper le résultat dans un grep du nom de l'extension recherchée. Sous windows vous pouvez passer par un utilitaire tel que [GLview](http://realtech-vr.com/admin/glview).

Quelques extensions très interessantes:

## Direct State Access

L'extension Direct State Access (DSA) ([GL_EXT_direct_state_access](https://www.opengl.org/registry/specs/EXT/direct_state_access.txt), puis [GL_ARB_direct_state_access](https://www.opengl.org/registry/specs/ARB/direct_state_access.txt), puis introduite en Core 4.5) propose une nouvelle manière de modifier/accéder aux objets OpenGL, sans avoir à les binder.

Cela permet d'éviter de se préoccuper de l'état global du contexte OpenGL lorsque l'on veut manipuler les objets OpenGL et ainsi d'éviter un grand nombre d'erreurs de programmation.

## Debug Output

L'extension Debug Output ([GL_ARB_debug_output](https://www.opengl.org/registry/specs/ARB/debug_output.txt), puis introduite en Core 4.3) à fait beaucoup de bien aux développeurs OpenGL puisqu'elle fournit un méchanisme de messages d'erreurs bien plus efficace que l'ignoble fonction *glGetError*.

L'idée est de donner une fonction de callback à OpenGL qui sera appelée par l'implémentation dès qu'une erreur est rencontrée. Il est de plus possible de filtrer de manière assez fine les erreurs/warning à ignorer.