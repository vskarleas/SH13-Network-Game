# Sherlock 13 comme jeu réseau

###### Manu Guerinel, Vasileios Filippos Skarleas

Un jeu en réseau dont le but est de comprendre les bases du protocole TCP ainsi que la communication serveur-client. Il est écrit en C et le code source a été fourni par M. Pecheux, directeur à Polytech Sorbonne.

## Dépendances

C'est un programme qui est écrit en langage C. Il y a quelques dépendances particuliers pour que le program peut se compiler et tourner.

- SDL
- SDL_image
- SDL_ttf
- netdb
- netinet

Si vous avez pas la librairie SDL, vous pouvez le télécharger en utilisant le package manager Brew:

```bash
brew install sdl2
```

## Comment exécuter ?

> Il faut noter que le program était développé sur un ordinateur Mac. Ça veut dire que toutes les instructions suivantes sont optimisées pour les ordinateurs Mac. Si vous utilisez un autre système d'exploitation, il faut vérifier ou sont installé les différentes librairies que le programme a besoin et qu'ils sont liste ci-dessus.

Le jeu SH13 viens avec un Makefile qui normalise les deux instructions de base suivantes :

```bash
gcc -o sh13 sh13.c -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread
gcc -o server server.c
```

Donc, il suffit sur une terminale de taper `make`. A noter que le compilateur utilisé est clang.

## Le jeu Sherlock 13

Dans Sherlock 13, les joueurs prennent le rôle de détective, essayant de démasquer le célèbre voleur Arsène Lupin, qui est parmi eux, déguisé.

Le jeu se compose de 13 cartes de personnage, avec 2 à 3 caractéristiques. Chaque caractéristique est partagée avec 3 à 5 autres personnages.

## Règles

Les cartes sont mélangées et l'une est mise de côté. Cette carte représente le déguisement choisi par Arsène Lupin, les autres cartes sont réparties parmi les joueurs.

Les joueurs comptent alors le total des caractéristiques qu'ils ont sur leurs cartes (par exemple, 2 détectives, 1 femme, 0 génie, etc.) et vont pouvoir poser une de ces deux questions pour recueillir de nouvelles preuves :

* Soit, à tous les joueurs, "Qui a (au moins un) de (cette caractéristique) ?
* Soit, pour un joueur spécifique, "Combien de (cette caractéristique) avez-vous ?

En utilisant ces indices, les joueurs tentent de déterminer quel déguisement Arsène a choisi (c'est-à-dire quelle carte est manquante).

Au lieu d'une question, un joueur peut annoncer quel personnage il soupçonnera d'être Arsène déguisé. Ce joueur vérifie en secret la carte cachée. Si le soupçon était correct, ce joueur gagne le jeu, sinon le joueur est éliminé et les autres joueurs continuent.

## Architecture

### Relation serveur/clients

1. Il y a un server central qui connait toutes les informations. C'est lui qui va mélanger les cartes et les distribuer aux joueurs une fois que tout le monde est connecté sur la session du jeu. En plus, c'est lui qui est responsable de répondre aux questions que les joueurs posent aux autres joueurs. Par exemple, selon le type de la question, il faut donner les corrects informations et faire attentions de pas transmettre des informations que les joueurs n'ont pas demande ou ils sont hors des règles du jeu. Il s'agit du fichier server.c
2. Il y a 4 clients qui vont joueurs. Tous les 4 clients partage le même source code qu'il s'agit du fichier sh13.c. Chaque personne qui essaye de se connecter au server central faut donner les informations suivantes

   * Son adresse IP
   * La port utilise pour communiquer avec le client en question
   * Son nom

   Le serveur envoie un message de confirmation une fois qu'il a accepté le client. Si tous les 4 clients sont connectés alors la boucle principale du jeu peu commencer (poser des questions et faire de Guess pour la carte caché). Voici un schéma UML de l'architecture réseau du jeu Sherlock 13.

![Architecture Réseau du jeu](UML/network.png)

### La boucle du jeu

Comme dans chaque jeu, on joue dans une boucle infinie jusque que quelqu'un gagne (ou si on abandonne parce qu'il y a plus d'intérêt :]). On peut observer le même comportement au cœur de notre program.

Une fois que tous les initialisations nécessaires sont fait comme ceux ci-dessous, on peut commencer le jeu :

* Connexion des tous les 4 clients
* Mélange des cartes
* Distribution de 3 cartes à chaque client
* Envoi des informations aux clients telles que les détails de leurs cartes et les noms des autres joueurs.

Vous allez trouver à la ligne 260 du source code des clients (sh13.c) un while qui va se terminer si et seulement si l'interface graphique SDL est quitté `while (!quit)`. C'est dans cette boucle while que chaque utilisateur peut poser ses questions :

* Soit, à tous les joueurs, "Qui a (au moins un) de (cette caractéristique) ?
* Soit, pour un joueur spécifique, "Combien de (cette caractéristique) avez-vous ?

Chaque jouer à une tentative de deviner la carte cache comme explique aux règles du jeu. Si jamais quelqu'un trouve cette carte, tous les autres perdent. En termes de réseau, on communique qui a gagné et qui a perdu jusque ce moment ou pas, ainsi que qui est le gagnant. Voici un schéma UML qui montre les messages envoyés par le serveur vers les clients dans les différents scenarios qu’un client x a deviné ou pas la bonne carte :

![Messages serveur en cas de Guess par les clients](UML/game.png)

## Explications

ICI IL FAUT EXPLIQUER TOUT LE CODE

## Changements essentiels

Dans cette partie vous allez trouver quelques changements et modifications qui était apporte sur le code et qui étaient essentiels pour le bon fonctionnement du programme:

1. `bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); `

   La fonction bcopy a la ligne 116 du fichier sh13.c est utilisée pour copier l'adresse IP de la structure du serveur vers la structure serv_addr. Cela permet de définir l'adresse IP du serveur auquel le client se connectera. Il s'agit d'une connexion TCP entre le client et le serveur.

   Le client crée d'abord un socket, puis il lie le socket à un port local, puis il tente de se connecter à l'adresse IP et au numéro de port du serveur.

   Cependant, il est important de noter que l'utilisation de bcopy a été aboli. Voici comment on obtiens le même comportement avec la fonction memcpy: `memcpy((char *)&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);`
2. Le server n'était pas capable de mélanger les cartes parce qu'il n'y avait un endroit ou le générateur aléatoire était initialise avec l'heur actuel de la machine. Donc la commande `srand(time(NULL));` était ajoute au fichier server.c a la ligne 278.

## Ameliorations

Il y a toujours des améliorations qu'on pourrait apporter au projet. Voici nos idées:

* [ ] Détecter si un utilisateur déconnecté par la session et informer les autres. Si il est reconnecte, il puissent continuer le jeu
* [ ] Développer un codec de sauvegarde de l'état du jeu et donner la capacite aux jouers de sauvegarder leur jeu.
* [ ] Possibilité de mettre à jour le username d'un client après la connexion pour offrir encore plus des possibilités de customisation

## Versions

Le versioning est un élément clé en programmation, assurant la cohérence des modifications et facilitant la collaboration. Il est aussi primordial pour la récupération de données en cas de perte ou corruption. Au fil du projet, nous avons créé différentes versions de notre code, chacune marquant une étape importante de son évolution. Cela nous a permis de suivre les progrès, d'intégrer de nouvelles fonctionnalités et d'effectuer des corrections de manière structurée.

* V1.0.0: Récupérer les fichiers source du jeu
* V1.2.1: Création du git de la structure de base [initial commit]
* V1.2.3: sh13.c était complété
* V2.0.1: server.c était complété [added server]
* V3.0.1: Reconstruction du répertoire, changement de la police, nettoyage, création du makefile, premier version du compte rendu, changement du background vers une image.
