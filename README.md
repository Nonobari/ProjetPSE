# ProjetPSE
Multithreading client server


## Fonctionnement

Comment utiliser notre programme:

1. Il faut compiler les modules dans le dossier `modules`:

        cd modules
        make

2. Il faut compiler le serveur et le client dans le dossier `appli` :

        cd ../appli
        make

3. Il faut lancer le serveur, dossier `appli`:

        ./server <port>

4. Il faut lancer le/les client, dosser `appli` :

        ./client <ip> <port>

## Principe du jeu

- Jeu multijoueur : on peut connecter plusieurs clients au serveur (jusqu'à 10)
- Le serveur attend que tous les clients soient prêts pour lancer la partie (appui sur la touche "o" puis "enter")

## Architecture du projet

Voici l'arbre du projet :



### Serveur


