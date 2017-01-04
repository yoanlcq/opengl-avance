+++
toc = true
prev = "/02-forward-shading/load-obj"
next = "/03-deferred-shading"
weight = 8
title = "Aller plus loin"
date = "2017-01-04T00:58:16+01:00"

+++

Voici plusieurs choses améliorable pour rendre notre forward renderer plus interessant:

## Modèle de Shading

Améliorer le modèle de shading utilisé pour utiliser le modèle de Blinn-Phong plutot qu'un simple modèle Diffus (voir [cet ancien TD](http://igm.univ-mlv.fr/~lnoel/index.php?section=teaching&teaching=opengl&teaching_section=tds&td=td8#intro) pour les équations).

Cela implique de rajouter des variables uniformes pour le coefficient speculaire, la texture speculaire, l'exposant de shininess et la texture de shininess.

Il faut également charger toutes ces données depuis l'OBJ, ainsi que les textures associées.

## Normal Mapping

Ajouter une texture de normales dans le fragment shader et faire le necessaire pour charger la texture de normales de chaque objet.

L'utiliser à la place de *uViewSpaceNormal* pour calculer l'illumination.

Vous pouvez vous réferer à [ce tutorial](http://ogldev.atspace.co.uk/www/tutorial26/tutorial26.html).

## Plusieurs Lights

Actuellement le shader ne gère que deux lumières, c'est un peu triste.

Utilisez les [Shader Storage Buffer Objects](https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object) pour accéder à des tableau de directions, positions et intensités depuis le fragment shader, correspondant à un nombre arbitraire de directional et point lights (stockez leur nombre dans des uniforms).

Bouclez sur ces lights et accumulez la contribution de chacune avant de l'écrire dans la variable de sortie *fFragColor*.