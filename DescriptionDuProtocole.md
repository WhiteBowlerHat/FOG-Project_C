# Protocole FOG
Lors de mes recherches sur quel genre de messages je voulais cacher dans les images, je voulais faire en sorte de démocratiser le programme un maximum, c'est à dire, transmettre le plus d'informations possible et n'importe quel type d'information. 

## _Pourquoi un protocole est nécessaire ?_
Afin de démocratiser le programme et de cacher de l'information au sein d'une image en vu de la retrouver. Il faut pouvoir transmettre différents éléments. Ceci étant statué, voici la liste de ces éléments:

## _Elements à transmettre:_
- Le type de fichier (.mp4, .mpeg, .txt, ...)
- La taille du message (En octets, mega-octets, giga-octets,  ...)
- Le nonce (number used once)
- L'identificateur d'ordre de l'image

### 1. Le type de fichier
Dans une première version en python du programme j'ai fait le choix de transmettre comme type de message, un fichier ZIP. Dans l'optique d'élargir l'usage du programme et de ne pas le limiter à un fichier ZIP, je souhaite ajouter au protocole. Un type de fichier, dans ma vision des choses est associé aux caractères après le point dans le nom du fichier. Notons toutefois qu'un fichier sans point est aussi un type de fichier et qu'il faudra également le prendre en compte.

Afin de répondre à ce besoin j'ai pris la décision de définir le type de fichier comme suit:
| Indicateur de standard | Caractère de scission | Taille | Caractère de scission | Contenu du type de fichier | 
|-|-|-|-|-|
|0 (Standard) (1 bit)| None (0 bit)| None (0 bit)| None (0 bit)|Extension du fichier (3 octets) 
|1 (Non-standard) (1 bit)| Lettre (1 octet) | Taille (n-octets) | Lettre (1 octet) | Extension du fichier (k-octets)

L'indicateur de standard permettra de savoir si on a affaire à un fichier dont la taille de la chaine de caractère de l'extension est différente de 3 octets. Dans le cas non-standard, j'ai ajouté deux caractère de scission. Leur utilité est de délimiter la taille de la taille de l'extension. C'est un peu une mise en abyme cependant si un utilisateur décide de transmettre un fichier avec très long nom d'extension, il doit être en mesure de pouvoir effectuer ce genre de chose. Les carctère de scission seront alphabétique [a-z] ceci les différenciera de la taille qui n'est composée que d'une suite d'octets uniquement de [0-9]. On pourra, pour un cas non standard avoir une taille variable qui définira le nombre k d'octets à lire pour l'extension du fichier.

**_Exemple:_**
_Pour décrire un fichier .mp4:_
| Indicateur de standard | Caractère de scission | Taille | Caractère de scission | Contenu du type de fichier | 
|-|-|-|-|-|
|0 |-|-|-|01101101 01110000 00110100 ( = mp4)|
_Pour définir un fichier de type .supertypedefichier:_
| Indicateur de standard | Caractère de scission | Taille | Caractère de scission | Contenu du type de fichier | 
|-|-|-|-|-|
|1|01100001 ( = a)| 00110001 00111000 ( = 18)| 01100001 ( = a)| 01110011 01110101 01110000 01100101 01110010 01110100 01111001 01110000 01100101 01100100 01100101 01100110 01101001 01100011 01101000 01101001 01100101 01110010 ( = supertypedefichier)|

### 2. La taille du message
La taille du message (ou de l'information transmise) est un point clé pour le programme, elle permet de définir jusqu'où le programme s'arrête. Pour la définir nous allons définir, de la même manière que pour l'extension de nom de fichier, des chaines de scission.
| Caractère de scission | Taille | Caractère de scission | 
|-|-|-|
|Lettre (1 octet) | Taille (n-octets) | Lettre (1 octet) 

### 3. Le nonce
Le nonce (ou _number used once_), est une valeur clé pour la mise en place du générateur de nombres pseudo-aléatoire. De ce fait, on ne peut le transmettre avec le message directement puisqu'il permet de le déchiffrer. Une première proposition était de l'inclure dans les meta-données de l'image. Cependant ce serait "visible" assez facilement. La seconde option est de le placer selon la clef mais cela causerait des vulnérabilité sur la clef elle-même. Il est de 192 bits (soit 24 octets). Nous conserverons la première option dans un premier temps.

### 4. L'indentificateur d'image
Afin de définir quelle image traiter en premier, puis en second, etc... Il faut définir un identificateur d'image (1,2,3,..). 
Pour cela: 
1. Determiner le nombre d'images reçues (n).
2. Collecter les 3 premiers octets de chaque image.
3. Effectuer un XOR sur ces octets pour chaque image.
4. Générer un seed combinant la clé symétrique et le résultat du XOR.
5. Utiliser XChaCha20 comme générateur pseudo-aléatoire (PRNG) pour produire un ordre aléatoire.
6. Générer une permutation décroissante : on tire des indices entre 0 et n − 1, n−2, etc., jusqu'à 1.

Ce protocole garantit que le destinataire pourra reconstruire l'ordre en utilisant la même clé et les mêmes images.

