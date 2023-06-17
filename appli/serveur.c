#include "pse.h"
#include <time.h>

#define    CMD      "serveur"
#define GAME_TIME 10
#define NB_CLIENT 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void generateRanking(const int scores[], int ranking[]);
void *timer_thread();
void *sessionClient(void *arg);
void remiseAZeroJournal(void);
void calcul_score(int numero_du_joueur, char mots_ecrits[TAILLE_PHRASE][TAILLE_MOT], char phrase[TAILLE_PHRASE][TAILLE_MOT], int *score_tab);


int fdJournal;
int clients_prets = 0;
char phrase[TAILLE_PHRASE][TAILLE_MOT];
int start_chrono = FAUX;
time_t start_time, elapsed_time;
int stop = FAUX;
int score_tab[NB_CLIENT];
int classement[NB_CLIENT];
int partie_start = FAUX;

int main(int argc, char *argv[]) {
  short port;
  int ecoute, canal, ret;
  struct sockaddr_in adrEcoute, adrClient;
  unsigned int lgAdrClient;
  DataThread *dataThread;
  pthread_t timer_thread_id;
  int n_client = 0;
  generate_sentence(phrase);


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
  pthread_create(&timer_thread_id,NULL,timer_thread,NULL);
  /*Boucle d'écoute du serveur*/
  while (VRAI) {
    /**si la partie n'a pas commencé on est en écoute*/
    if (!partie_start) {
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
      dataThread->spec.n_client = n_client; /*On specifie le numéro du joeur*/
      n_client++;
      if (n_client == NB_CLIENT) {
        partie_start = VRAI;
      }

      ret = pthread_create(&dataThread->spec.id, NULL, sessionClient, /*on crée le thread avec la fonction sessionClient */
                            &dataThread->spec);
      if (ret != 0)
        erreur_IO("creation thread");

      joinDataThread();
    }

    /**si la partie a commencé on attend la fin de la partie*/
    else {
       /*Attente de la fin des threads en parcourant la liste*/
        DataThread *current = dataThread;
        while (current != NULL) {
          pthread_join(current->spec.id, NULL);
          printf("%s: thread %lu est termine\n", CMD, current->spec.id);
          n_client --;
          current = current->next;
        }
        partie_start = FAUX;

      /*On reinitialise les données*/

      pthread_mutex_lock(&mutex);
      memset(phrase,0,sizeof(phrase));
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      memset(score_tab,0,sizeof(score_tab));
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      memset(classement,0,sizeof(classement));
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      clients_prets = 0;
      pthread_mutex_unlock(&mutex);

    }
     
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
  char ligne[LIGNE_MAX];
  char formatted_ligne[LIGNE_MAX+1];
  char mots_ecrits[TAILLE_PHRASE][TAILLE_MOT];
  int lgLue;
  int i;
  char score[20];
  

  canal = dataTh->canal;
  printf("%s: connexion client\n",CMD);
  ecrireLigne(canal, "serveur: Etes-vous prêts ? o/n\n");
  lgLue = lireLigne(canal, ligne); /*on lit la réponse du client*/
  lgLue++;
  if (strcmp(ligne, "o") == 0)
    {
      /*set client state to ready*/
      printf("%s: Client %d is ready\n",CMD, dataTh->n_client);
      
      pthread_mutex_lock(&mutex);
      clients_prets++;
      pthread_mutex_unlock(&mutex);
    }

  else {
    /*set client state to not ready*/
    printf("%s: Client %d is not ready\n",CMD, dataTh->n_client);
  }

  /*wait for all clients to be ready*/
    while (clients_prets < NB_CLIENT)
    {
      printf("%s: Waiting for clients to be ready\n",CMD);
      sleep(2);
    }
    
    
    ecrireLigne(canal, "start\n");
    
    /*On lance le chrono*/
    pthread_mutex_lock(&mutex);
    start_chrono = VRAI;
    pthread_mutex_unlock(&mutex);

    for (i = 0; i < 100 ; i++)
    {
      if (!stop)
      {
      ecrireLigne(canal,phrase[i]);
      lgLue = lireLigne(canal,ligne);
      sprintf(formatted_ligne,"%s\n",ligne);
      strcpy(mots_ecrits[i],formatted_ligne);
      }
    }

  
    /*On arrête le chrono*/
    ecrireLigne(canal, "stop\n");
    /*on envoie le score convertit*/
    calcul_score(dataTh->n_client,mots_ecrits,phrase,score_tab);
    sprintf(score, "%d",score_tab[dataTh->n_client]);
    printf("%s: envoi du score aux clients%s\n",CMD,score);
    ecrireLigne(canal,score);

    /*calcul classement*/
    generateRanking(score_tab,classement);
    printf("%s: annonce les résultats des clients\n",CMD);
    sprintf(ligne, "%d",classement[dataTh->n_client]);
    ecrireLigne(canal,ligne);
    
    
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


void *timer_thread()
{
   while(!start_chrono){};
    pthread_mutex_lock(&mutex);
    start_chrono = FAUX;
    pthread_mutex_unlock(&mutex);

    printf("passage time");
    time(&start_time);
    time(&elapsed_time);
    /*printf("difftime = %.2f\n",difftime(elapsed_time,start_time));*/
  while(difftime(elapsed_time,start_time) < GAME_TIME)
  {
    /*printf("elapsed_time : %.2f\n",difftime(elapsed_time,start_time));*/
    sleep(0.5);
    time(&elapsed_time);
  }
  printf("Stop temps écoulé");

  pthread_mutex_lock(&mutex);
  stop = VRAI;
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}


void calcul_score(int numero_du_joueur, char mots_ecrits[TAILLE_PHRASE][TAILLE_MOT], char phrase[TAILLE_PHRASE][TAILLE_MOT], int *score_tab)
{
  int i;
  for (i = 0; i < TAILLE_PHRASE; i++)
  {
    if (strcmp(mots_ecrits[i],phrase[i]) == 0)
    {
      score_tab[numero_du_joueur]++;
    }
  }
}

void generateRanking(const int scores[], int ranking[]) {
    int i, j;

    for (i = 0; i < NB_CLIENT; i++) {
        int currentScore = scores[i];
        int currentRank = 1;

        for (j = 0; j < NB_CLIENT; j++) {
            if (scores[j] > currentScore) {
                currentRank++;
            }
        }

        ranking[i] = currentRank;
    }
}