+++
date = "2016-12-29T12:22:16+01:00"
title = "Chargement de modèles OBJ"
weight = 7
prev = "/02-forward-shading/textures"
next = "/02-forward-shading/plus-loin"
toc = true

+++

Utiliser la lib *tinyobjloader* (déjà include dans le template) pour remplacer votre sphere et cube par une liste d'objets chargés depuis un OBJ.

Utiliser la doc et exemples de [la page github de la lib](http://syoyo.github.io/tinyobjloader/).

En iterant sur les materiaux, charger uniquement la texture diffuse et le coefficient diffus de chaque objet.

Faites en sorte que chaque objet soit affiché avec la bonne couleur diffuse.

{{% notice note %}}
Un OBJ ne stocke pas directement les textures mais seulement le chemin (relatif à l'OBJ) de chaque texture. Il faut donc les charger ensuite en utilisant la fonction glmlv::readImage.
{{% /notice %}}

Vous pouvez obtenir différents modèles OBJ sur [cette page](http://graphics.cs.williams.edu/data/meshes.xml) (je vous conseille de commencer par essayer avec Crytek-Sponza).