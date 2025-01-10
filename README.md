# Sherlock 13 comme jeu resseau

###### Manu Guerinel, Vasileios Filippos Skarleas

Un jeu en réseau dont le but est de comprendre les bases du protocole TCP ainsi que la communication serveur-client. Il est écrit en C et le code source a été fourni par M. Pecheux, directeur à Polytech Sorbonne.

## Dependances

C'est un program qui est ecrit en language C. Il y a quelques dependances particuliers pour que le program peut se compiller et tourner.

- SDL
- SDL_image
- SDL_ttf
- netdb
- netinet

Si vous avez pas la librairie SDL, vous pouvez le telecharger en utilisant le package manager Brew:

```bash
brew install sdl2
```

## Comment executer ?

> Il faut noter que le program etait develope sur un ordinateur Mac. Ca veut dire que tou sles instructions suivantes sont optimise pour les ordinateurs Mac. Si vous utilisez un autre systeme d'exploitation, il faut verifier ou sont installe les differents libraries que le program a besoin et qu'ils sont liste ci-dessus.

Le jeu SH13 viens avec un Makefile qui normalise les deux instructions de base suivantes:

```bash
gcc -o sh13 sh13.c -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread
gcc -o server server.c
```

Donc, il suffit sur un terminale de taper `make`. A noter que le compilateur utilisé est clang.

## Le jeu Sherlock 13

Dans Sherlock 13, les joueurs prennent le rôle de détective, essayant de démasquer le célèbre voleur Arsène Lupin, qui est parmi eux, déguisé.

Le jeu se compose de 13 cartes de personnage, avec 2 à 3 caractéristiques. Chaque caractéristique est partagée avec 3 à 5 autres personnages.

## Regles

Les cartes sont mélangées et l'une est mise de côté. Cette carte représente le déguisement choisi par Arsène Lupin, les autres cartes sont réparties parmi les joueurs.

Les joueurs comptent alors le total des caractéristiques qu'ils ont sur leurs cartes (par exemple, 2 détectives, 1 femme, 0 génie, etc.) et vont pouvoir poser une de ces deux questions pour recueillir de nouvelles preuves:

* Soit, à tous les joueurs, "Qui a (au moins un) de (cette caractéristique)?
* Soit, pour un joueur spécifique , "Combien de (cette caractéristique) avez-vous?

En utilisant ces indices, les joueurs tentent de déterminer quel déguisement Arsène a choisi (c'est-à-dire quelle carte est manquante).

Au lieu d'une question, un joueur peut annoncer quel personnage il soupçonnera d'être Arsène déguisé. Ce joueur vérifie en secret la carte cachée. Si le soupçon était correct, ce joueur gagne le jeu, sinon le joueur est éliminé et les autres joueurs continuent.

## Architecture

### Relation serveur/clients

1. Il y a un server central qui connait tous les informations. C'est lui qui va melanger les cartes et les distribuer aux joueurs une fois que tout le monde est connecte sur la session du jeu. En plus, c'est lui qui est responsable de repondre aux questiosn que les jouers posent aux autres joeurs. Par exemple, selon le type de la question, il faut donner les corrects informatiosn et faire attentions de pas transmettre des informations que les jouers n'ont pas demande ou ils sont hors des regles du jeu. Il s'agit du fichier server.c
2. Il y a 4 clients qui vont jouers. Tous les 4 cleints partage le meme source code qu'il s'agit du fichier sh13.c. Chaque personne qui essaye de se connecter au server central faut donner les informations suivantes

   * Son adresse IP
   * La port utilise pour communiquer avec le client en question
   * Son nom

   Le serveur envois un message de confirmation une fois qu'il a accepte le client. Si tous les 4 clients sont connecté alors la boucle principale du jeu peu commencer (poser des question et faire de guess pour la carte caché). Voici un schema UML de l'architecture reseau du jeu Sherlock 13.

![Architecture Resseau du jeu](UML/network.png)

### La boucle du jeu

Comme dans chaque jeu, on joue dans une boucle infinie jusque que quelqu'un gagne (ou si on abandone parce qu'il y a plus d'interet :] ). On peut observer le meme comportment au coeur de notre program.

Une fois que tous les initialisations necesaires sont fait comme ceux ci-dessous, on peut commencer le jeu:

* Connexion des tous les 4 clients
* Melange des cartes
* Distribution de 3 coartes a chaque client
* Envoi des informations aux clients telles que les details de leurs cartes et les noms des autres joeurs.

Vous allez trouver a la ligne 260 du source code des clients (sh13.c) un while qui va se temriner si et seulment si l'interface graphique SDL est quite `while (!quit)`. C'est dans cette boucle while que chaque utilisateur peut posers ses questions:

* Soit, à tous les joueurs, "Qui a (au moins un) de (cette caractéristique)?
* Soit, pour un joueur spécifique , "Combien de (cette caractéristique) avez-vous?

Chaque joeur a une tentative de deviner la carte cache comme explique aux regles du jeu. Si jamais quelqu'un trouve cette carte, tous les autres perdent. En termes de resseau, on comminique qui a gagne et qui a perdu jusque ce moment ou pas, ainsi que qui est le gagnant. Voici un schema UML qui montre les messages envoye par le serveur vers les clients dans les differents scenarios que un client x a devine ou pas la bonne carte:

![Messages serveur en cas de Guess par les clients](UML/game.png)

## Explications

ICI IL FAUT EXPLIQUER TOUT LE CODE

## Changements essentiels

Dasn cette partie vous allez trouver quelques changements et modifications qui etait apporte sur le code et qui etaient essentiels pour le bon fonctionement du program:

1. `bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); `

   La fonction bcopy a la ligne 116 du fichier sh13.c est utilisée pour copier l'adresse IP de la structure du serveur vers la structure serv_addr. Cela permet de définir l'adresse IP du serveur auquel le client se connectera. Il s'agit d'une connexion TCP entre le client et le serveur.

   Le client crée d'abord un socket, puis il lie le socket à un port local, puis il tente de se connecter à l'adresse IP et au numéro de port du serveur.

   Cependant, il est important de noter que l'utilisation de bcopy a été aboli. Voici comment on obtiens le meme comportement avec la fonction memcpy: `memcpy((char *)&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);`
2. Le server n'etait pas capable de melanger les cartes parce qu'il n'y avait un endroit ou le generateur aleartoir etait initialise avec l'eheur actuel de la machine. Donc la commande `srand(time(NULL));` etait ajoute au fichier server.c a la ligne 278.

## Ameliorations

Il y a toujours des ameliorations qu'on pourraut apporter au projet. Voici nos idees:

* [ ] Detecter si un utilisateur deconecte par la session et informer les autres. Si il est reconnecte, il puissent continuer le jeu
* [ ] Developer un codec de sauvegarde de l'etat du jeu et donner la capacite aux jouers de sauvegarder leur jeu.
* [ ] Possibilite de mettre a jour le username d'un client apres la connexion pour offrir encore plus des possibilites de customisation

## Versions

Le versioning est un élément clé en programmation, assurant la cohérence des modifications et facilitant la collaboration. Il est aussi primordial pour la récupération de données en cas de perte ou corruption. Au fil du projet, nous avons créé différentes versions de notre code, chacune marquant une étape importante de son évolution. Cela nous a permis de suivre les progrès, d'intégrer de nouvelles fonctionnalités et d'effectuer des corrections de manière structurée.

* V1.0.0: Recuperer les fichiers source du jeu
* V1.2.1: Creation du git de la structure de base [initial commit]
* V1.2.3: sh13.c etait complete
* V2.0.1: server.c etait complete [added server]
* V3.0.1: Reconstruction du reperoire, changement de la police, netoyage, creation du makefile, premier version du compte rendu, changement du background vers une image.
