#include "pse.h"

#define CMD   "client"

int main(int argc, char *argv[]) {
  int sock, ret;
  struct sockaddr_in *adrServ;
  int fin = FAUX;
  int partie_en_cours = VRAI;
  char ligne[LIGNE_MAX];
  char mot[TAILLE_MOT];
  char mot_reponse[TAILLE_MOT];
  int lg;
  int score;

  if (argc != 3)
    erreur("usage: %s machine port\n", argv[0]);

  /*printf("%s: creating a socket\n", CMD);*/
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    erreur_IO("socket");

  /*printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);*/
  adrServ = resolv(argv[1], argv[2]);
  if (adrServ == NULL)
    erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

  /*printf("%s: adr %s, port %hu\n", CMD,
	        stringIP(ntohl(adrServ->sin_addr.s_addr)),
	        ntohs(adrServ->sin_port));*/

  /*printf("%s: connecting the socket\n", CMD);*/
  ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
  if (ret < 0)
    erreur_IO("connect");
  
  printf("-----------------------------------------\n");
  printf("\n");
  printf(" TTTTT  Y   Y  PPPP    I    N   N   GGG         TTTTT   EEEEE  SSSS  TTTTT \n");  
  printf("   T     Y Y   P   P   I    NN  N  G              T     E     S        T   \n"); 
  printf("   T      Y    PPPP    I    N N N  G  GG          T     EEE    SSS     T   \n");
  printf("   T      Y    P       I    N  NN  G   G          T     E         S    T   \n");
  printf("   T      Y    P       I    N   N   GGG           T     EEEEE  SSSS    T   \n");
  printf("\n");
  printf("Made by Banchet Antoine and Backert Noé\n");
  printf("\n");
  

  /*boucle d'envoi de lignes de texte*/
  while (!fin) {

    /*on recoit le message d'acceuil du serveur */
    lg = lireLigne(sock, ligne);
    if (lg == -1)
      erreur_IO("lireLigne");

    printf("%s\n", ligne);

    /*On lit la réponse de l'utilisateur */
    if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
        erreur("saisie fin de fichier\n");

    /*On envoie la réponse au serveur*/
    /*send_to_server(lgLue, sock, ligneLus);*/
    lg = ecrireLigne(sock, ligne);
    if (lg == -1)
      erreur_IO("ecrire ligne");
  
    /*On lit la réponse de l'utilisateur savoir s'il est pret */
    if (strcmp(ligne,"o\n")==0) {
      fin = FAUX;
      }
    else {
      fin = VRAI;
    }
    /*Fonction bloquante tant qu'on a pas de start*/
    printf("Veuillez patientez, en attente de joueurs... \n");
    lg = lireLigne(sock, ligne);
    if (lg == -1)
        erreur_IO("lireLigne");

    /*On affiche le message de start*/
    printf("J'ai reçu le start du serveur %s\n",ligne);  
    printf("La partie va commencer !\n");
    printf("Vous avez 10 secondes pour écrire le plus de mots possible !\n");
    printf("C'est parti !\n");
    partie_en_cours = VRAI;
    while(partie_en_cours)
    {
        /*On reçois le mot*/
        lg = lireLigne(sock, mot);
        if (lg == -1)
          erreur_IO("lireLigne");
        
        /*Si le serveur envoie stop c'est la fin*/
        if (strcmp(mot,"stop") != 0) {
          printf("Veuillez saisir le mot : %s\n", mot);
       
        /*on lit le premier mot*/
        if (fgets(mot_reponse, TAILLE_MOT, stdin) == NULL)
          erreur("saisie fin de fichier\n");
        
        /*on l'envoie au serveur */
        lg = ecrireLigne(sock, mot_reponse);
        if (lg == -1)
          erreur_IO("ecrire ligne");
        }
        
        else {
          partie_en_cours = FAUX;
        }
    }
    printf("Bravo ! Vous avez fini la partie !\n");

    /*Lire le score calculé par le serveur*/
    printf("Jattend le score du serveur...\n");
    lg = lireLigne(sock, ligne);
    score = atoi(ligne);
    printf("Votre score est de : %d\n", score);

    /*On lit le résultat */
    printf("J'attend le résultat du serveur...\n");
    lg = lireLigne(sock, ligne);
    if (lg == -1)
      erreur_IO("lireLigne");
    
    printf("Vous êtes %s\n", ligne);
  }
  
  /*on appuie pas sur 'o' pour jouer*/
  printf("Partie terminée !\n");
  if (close(sock) == -1)
    erreur_IO("close socket");
  exit(EXIT_SUCCESS);
}
