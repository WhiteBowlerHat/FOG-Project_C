# Projet Fog
## Objectif

## Pré-requis

## Fonctionnement
### 1. Chiffrement
1.1 Définir le dossier de banque d'images
1.2 Créér un GNPA pour ordonner les images 
1.3 Définir l'ordre des images à l'aide du GNPA
1.4 Définir l'en-tête du protocole
1.5 Créer un GNPA de distribution de position
1.6 Distribuer l'en-tête
1.7 Distribuer le contenu du fichier bite par bit

### 2. Déchiffrement
2.1 Définir le dossier de banque d'images
2.2 Créér un GNPA pour ordonner les images 
2.3 Définir l'ordre des images à l'aide du GNPA
2.4 Créer un GNPA de distribution de position
2.5 Obtenir les informations de l'en-tête
2.6 Obtenir le contenu du fichier en binaire
2.7 Réassemblage du fichier et ajout de l'extension


## Annexe : Fonctionnement XChacha20
Here is a simple flow chart:

```mermaid
flowchart LR;
 subgraph s1["GNPA (SC)"];
    direction LR;
        n3["Constante"];
        n1["Clef secrète"];
        n2["Nonce"];
  end
    A["0,0,0,0,..."] --0,0,0...--> n4;
    s1 --f1,f2,f3,...--> n4;
    n4 --> B["0,2,9,1,..."];
    n4["Summary"];
    n4@{ shape: summary};

```
