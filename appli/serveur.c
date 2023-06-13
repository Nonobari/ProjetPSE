#include "pse.h"


#define    CMD      "serveur"

char phrase[2000][LIGNE_MAX];

void *sessionClient(void *arg);
void remiseAZeroJournal(void);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
int fdJournal;
int clients_prets = 0;

int main(int argc, char *argv[]) {
  short port;
  int ecoute, canal, ret;
  struct sockaddr_in adrEcoute, adrClient;
  unsigned int lgAdrClient;
  DataThread *dataThread;
  
  fdJournal = open("journal.log", O_WRONLY|O_CREAT|O_APPEND, 0644);
  if (fdJournal == -1)
    erreur_IO("ouverture journal");

  initDataThread();

  if (argc != 2)
    erreur("usage: %s port\n", argv[0]);

  port = (short)atoi(argv[1]);

  printf("%s: creating a socket\n", CMD);
  ecoute = socket (AF_INET, SOCK_STREAM, 0);
  if (ecoute < 0)
    erreur_IO("socket");
  
  adrEcoute.sin_family = AF_INET;
  adrEcoute.sin_addr.s_addr = INADDR_ANY;
  adrEcoute.sin_port = htons(port);
  printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
  ret = bind (ecoute,  (struct sockaddr *)&adrEcoute, sizeof(adrEcoute));
  if (ret < 0)
    erreur_IO("bind");
  
  printf("%s: listening to socket\n", CMD);
  ret = listen (ecoute, 5); /*liste d'attente de 5*/
  if (ret < 0)
    erreur_IO("listen");
  
  /*Boucle d'écoute du serveur*/
  while (VRAI) {
    printf("%s: accepting a connection\n", CMD);
    canal = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient);
    if (canal < 0)
      erreur_IO("accept");

    printf("%s: adr %s, port %hu\n", CMD,
	      stringIP(ntohl(adrClient.sin_addr.s_addr)), ntohs(adrClient.sin_port));

    dataThread = ajouterDataThread();
    if (dataThread == NULL)
      erreur_IO("ajouter data thread");

    dataThread->spec.canal = canal; /*on spécifie le canal pour le nouveau thread*/
    
    ret = pthread_create(&dataThread->spec.id, NULL, sessionClient, /*on crée le thread avec la fonction sessionClient */
                          &dataThread->spec);
    if (ret != 0)
      erreur_IO("creation thread");

    joinDataThread();
  }

  if (close(ecoute) == -1)
    erreur_IO("fermeture ecoute");

  if (close(fdJournal) == -1)
    erreur_IO("fermeture journal");

  exit(EXIT_SUCCESS);
}

void *sessionClient(void *arg) {
  DataSpec *dataTh = (DataSpec *)arg;
  int canal;
  int fin = FAUX;
  char ligne[LIGNE_MAX];
  int lgLue;
  char phrase[2000];


  canal = dataTh->canal;
  printf("%s: connexion client\n",CMD);
  ecrireLigne(canal, "serveur: Etes-vous prêts ? o/n\n");
  lgLue = lireLigne(canal, ligne);
  if (strcmp(ligne, "o") == 0)
    {
      /*set client state to ready*/
      dataTh->ready = VRAI;
      printf("Client %ld is ready\n", dataTh->id);
      pthread_mutex_lock(&mutex);
      clients_prets++;
      printf("Sending start to client %ld\n", dataTh->id);
      ecrireLigne(canal, "start");
      pthread_cond_signal(&condition);
      pthread_mutex_unlock(&mutex);
      
    }

  else {
    /*set client state to not ready*/
    dataTh->ready = FAUX;
    printf("Client %ld is not ready\n", dataTh->id);
  }
    generate_sentence(phrase);
    printf("Phrase générée : %s\n", phrase);
  /*on attend que les deux clients soient prêts*/
    pthread_mutex_lock(&mutex);
    while (clients_prets < 2) {
        pthread_cond_wait(&condition, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    /*on envoie la phrase au client*/


  while (!fin) {
    

    /*if (strcmp(ligne, "fin") == 0) {
        printf("%s: fin client\n", CMD);
        fin = VRAI;
    }
    else if (strcmp(ligne, "init") == 0) {
      printf("%s: remise a zero journal\n", CMD);
      remiseAZeroJournal();
    }
    else if (ecrireLigne(fdJournal, ligne) != -1) {
        printf("%s: ligne de %d octets ecrite dans journal\n", CMD, lgLue);
    }
    else {
        erreur_IO("ecriture journal");
    }
     
      printf("ligne> ");
      if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
      erreur("saisie fin de fichier\n");
*/

  }

  if (close(canal) == -1)
    erreur_IO("fermeture canal");

  dataTh->libre = VRAI;

  pthread_exit(NULL);
}

/* le fichier est ferme et rouvert vide */
void remiseAZeroJournal(void) {
  if (close(fdJournal) == -1)
    erreur_IO("raz journal - fermeture");

  fdJournal = open("journal.log", O_WRONLY|O_TRUNC|O_APPEND, 0644);
  if (fdJournal == -1)
    erreur_IO("raz journal - ouverture");
}
