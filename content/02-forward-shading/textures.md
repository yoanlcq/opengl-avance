+++
title = "Textures"
weight = 6
prev = "/02-forward-shading/lighting"
next = "/02-forward-shading/load-obj"
toc = true
date = "2016-12-29T12:21:29+01:00"

+++

## Fragment Shader

- Ajouter une variable uniform *sampler2D uKdSampler*, destiné à pointer sur une texture des couleurs diffuses pour l'objet en cours de rendu.
- Dans le main, utiliser la fonction GLSL [*texture*](http://docs.gl/sl4/texture) afin de lire le sampler en utiliser les tex coords du fragment.
- Multiplier la valeur lue avec la variable uKd pour obtenir le coefficient diffus final de l'objet, à utiliser pour l'illumination

## Application

A l'initialisation

- Charger deux images de votre choix à utiliser en temps que texture diffuse de chacun de nos objets
- Construire deux texture objects OpenGL et envoyer les pixel des deux images chargés dans ces texture objects
- Construire un sampler object OpenGL
- Récuperer la location de l'uniform *sampler2D uKdSampler*

Au rendu

- Binder la texture sur la texture unit 0
- Binder le sampler sur la sampler unit 0

Fonctions GL à utiliser:

A l'initialisation:

- glCreateTextures
- glTextureParameteri
- glTextureStorage2D
- glCreateSamplers
- glSamplerParameteri

Au rendu:

- glBindSamplers
- glBindTextureUnit