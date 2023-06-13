# ProjetPSE
Multithreading client server


## Fonctionnement

### Serveur 

1 thread principal
1 thread par client (serveur dynamique)

-> envoie un message d'accueil aux clients lors de leurs conenxions
-> demande leur pseudo
-> check si au moins deux clients sont connectés et prêts à lancer la partie


### Client

1 thread principal

-> accueil : demande le pseudo du joueur, l'envoi au serveur
-> entrer "p" pour indiquer au serveur que le joueur est prêt
-> attendre que le serveur lance la partie
-> entrer "q" pour quitter la partie
-> entrer "r" pour relancer une partie
-> entrer "s" pour afficher le score
-> entrer "h" pour afficher l'historique des parties
-> entrer "c" pour afficher le classement des joueurs

