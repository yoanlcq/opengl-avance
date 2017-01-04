+++
title = "Pipeline de rendu"
weight = 1
prev = "/02-forward-shading/"
next = "/02-forward-shading/geometrie/"
toc = true
date = "2016-12-29T12:15:45+01:00"

+++

Le [**forward shading**](https://www.wikiwand.com/fr/Forward_Rendering) (ou **forward rendering**) est le rendu "classique" OpenGL que vous avez normalement vu l'année dernière. Cette partie est donc une série d'exercices pour implémenter rapidement un forward renderer afin de se remettre tranquilement dans le bain.

Pour chaque objet de la scène, un forward renderer fait passer la géométrie de l'objet dans un Vertex Shader afin de projeter les sommets à l'écran, la carte graphique rasterise les triangles projetés pour produire des fragments qui sont illuminés dans le fragment shader afin de calculer leur couleur.

Cette approche à le mérite d'être simple mais introduit un coup supplémentaire lorsque beaucoup de fragments sont occultés: on paye le coup d'illumination de ces derniers alors qu'ils sont finalement discardés par le test de profondeur. Le TD suivant vous fera implémenter un **deferred renderer**, qui palie à ce problème.

[Voir également cet article] (https://gamedevelopment.tutsplus.com/articles/forward-rendering-vs-deferred-rendering--gamedev-12342) qui explique le principe des deux approches.

L'objectif à la fin de ce premier TP est de pouvoir charger une scène 3D en OBJ constituée de modèles texturées, et pouvoir naviguer dedans en vue FPS.

Commencez par dupliquer le repertoire *apps/template* et renommez le *forward-renderer*. Vous partirez du code de ce dossier. N'oubliez pas de **relancer CMake** pour prendre en compte la nouvelle application. Il faudra églament relancer CMake à chaque ajout de fichier source ou shader. Pour faire cela rapidement, se placer dans le dossier de build et lancer simplement la commande **"cmake ."**.

{{% notice warning %}}
Les instructions de ce premier TP seront concises car ce sont des exercices de rappel.
Attachez vous à utiliser des fonctions de l'extension direct_state_access afin de moderniser votre code.
N'hésitez pas à prendre exemple sur les applications déjà présente dans le template, et à vous aidez [des liens] (/01-introduction/liens/).
{{% /notice %}}