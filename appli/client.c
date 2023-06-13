#include "pse.h"


#define CMD   "client"

int main(int argc, char *argv[]) {
  int sock, ret;
  struct sockaddr_in *adrServ;
  int fin = VRAI;
  char ligne[LIGNE_MAX];
  int lg;

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

  printf(" TTTTT  Y   Y  PPPP    I    N   N   GGG         TTTTT   EEEEE  SSSS  TTTTT \n");  
  printf("   T     Y Y   P   P   I    NN  N  G              T     E     S        T   \n"); 
  printf("   T      Y    PPPP    I    N N N  G  GG          T     EEE    SSS     T   \n");
  printf("   T      Y    P       I    N  NN  G   G          T     E         S    T   \n");
  printf("   T      Y    P       I    N   N   GGG           T     EEEEE  SSSS    T   \n");
  printf("\n");
  printf("Made by Banchet Antoine and Backert Noé\n");
  printf("\n");
  
  /*on recoit le message d'acceuil du serveru */
  /*receve_from_server(lgLue, sock, ligneLus);*/
  lg = lireLigne(sock, ligne);
    if (lg == -1)
      erreur_IO("lireLigne");

    printf("%s\n", ligne);

  /*On lit la réponse */
  if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
      erreur("saisie fin de fichier\n");

  /*On envoie la réponse au serveur*/
  /*send_to_server(lgLue, sock, ligneLus);*/
  lg = ecrireLigne(sock, ligne);
  if (lg == -1)
    erreur_IO("ecrire ligne");
 
  /*On lit la réponse du client savoir s'il est pret */
  if (strcmp(ligne,"o")) {
    fin = FAUX;
    }

  /*boucle d'envoi de lignes de texte*/
  while (!fin) {

    printf("Vous êtes dans la salle d'attente, veuillez patienter...\n");
    if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
      erreur("saisie fin de fichier\n");

    
    /*Tant que le serveur envoie pas le message de start*/
    while (lg = lireLigne(sock, ligne) && strcmp(ligne, "start\n") != 0) {
      lg = lireLigne(sock, ligne);
      if (lg == -1)
        erreur_IO("lireLigne");
      }

      /*On affiche le message de start*/
      printf("J'ai reçu le start du serveur %s\n",ligne);  }

  printf("Partie terminée !\n");
  if (close(sock) == -1)
    erreur_IO("close socket");

  exit(EXIT_SUCCESS);
}
