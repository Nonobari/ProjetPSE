#include "pse.h"
#include <time.h>

#define    CMD      "serveur"
#define GAME_TIME 10
#define NB_CLIENT 2

/*on déclare un mutex statique*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void generateRanking(const int scores[], int ranking[]);
void *timer_thread();
void *sessionClient(void *arg);
void remiseAZeroJournal(void);
void calcul_score(int numero_du_joueur, char mots_ecrits[TAILLE_PHRASE][TAILLE_MOT], char phrase[TAILLE_PHRASE][TAILLE_MOT], int *score_tab);
void *timer();
int trouver_premier_libre(int tab[]);
int fdJournal;
int clients_prets = 0;
char phrase[TAILLE_PHRASE][TAILLE_MOT];
int phrase_flag = FAUX;
int start_chrono = FAUX;
time_t start_time, elapsed_time;
int stop = FAUX;
int score_tab[NB_CLIENT];
int classement[NB_CLIENT];
pthread_t timer_thread_id;

int main(int argc, char *argv[]) {
  short port;
  int ecoute, canal, ret;
  struct sockaddr_in adrEcoute, adrClient;
  unsigned int lgAdrClient;
  DataThread *dataThread;
  int n_client = 0;
  memset(score_tab, -1, sizeof(score_tab));
  fdJournal = open("journal.log", O_WRONLY|O_CREAT|O_APPEND, 0644);
  if (fdJournal == -1)
    erreur_IO("ouverture journal");

  initDataThread();

  if (argc != 2)
    erreur("usage: %s port\n", argv[0]);

  port = (short) atoi(argv[1]);

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
   
    pthread_mutex_lock(&mutex);
    printf("Je suis avat d'appeler la fonction trouver_premier_libre\n");
    printf("score_tab[0] = %d\n",score_tab[0]);
    printf("score_tab[1] = %d\n",score_tab[1]);

    n_client = trouver_premier_libre(score_tab);
    printf("Je viens de trouver un numéro de client pour le thread, n_client = %d\n",n_client);
    dataThread->spec.n_client = n_client; /*On specifie le numéro du joueur*/
    score_tab[n_client] = 0;
    n_client++;
    pthread_mutex_unlock(&mutex);
    
    ret = pthread_create(&dataThread->spec.id, NULL, sessionClient, /*on crée le thread avec la fonction sessionClient */
                          &dataThread->spec);
    if (ret != 0)
      erreur_IO("creation thread");

    joinDataThread(&n_client);
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
  char formatted_ligne[LIGNE_MAX+1];
  char mots_ecrits[TAILLE_PHRASE][TAILLE_MOT];
  int lgLue;
  int i;
  char score[20];
  

  canal = dataTh->canal;
  printf("%s: connexion client\n",CMD);
  while (!fin) {

    /*section critique car un seul thread client va généer la phrase*/
    pthread_mutex_lock(&mutex);
    if (!phrase_flag)
    {
      generate_sentence(phrase);
      printf("%s: Phrase generated\n",CMD);
      phrase_flag = VRAI;
    }
    pthread_mutex_unlock(&mutex);

    ecrireLigne(canal, "serveur: Etes-vous prêts ? o/n\n");
    lgLue = lireLigne(canal, ligne); /*on lit la réponse du client*/
    lgLue++;
    if (strcmp(ligne, "o") == 0)
      {
        /*set client state to ready*/
        score_tab[dataTh->n_client] = 0;
        printf("%s: Client %ld (%d) is ready\n",CMD, dataTh->id,dataTh->n_client);
        clients_prets++;
      }

    else {
      /*set client state to not ready*/
      printf("%s: Client %ld (%d) is not ready\n",CMD, dataTh->id,dataTh->n_client);
      if (close(canal) == -1)
        erreur_IO("fermeture canal");
      dataTh->libre = VRAI;
      printf("scoretab[0] = %d, scoretab[1] = %d\n",score_tab[0],score_tab[1]);
      score_tab[dataTh->n_client] = -1;
      printf("scoretab[0] = %d, scoretab[1] = %d\n",score_tab[0],score_tab[1]);
      pthread_exit(NULL);
    }

    /*wait for all clients to be ready*/
      while (clients_prets < NB_CLIENT)
      {
        printf("%s: Waiting for clients to be ready\n",CMD);
        printf("nb de clients prets %d\n",clients_prets);
        sleep(2);
      }
      
      
      ecrireLigne(canal, "start\n");
      
      /*On lance le thread timer*/
      pthread_mutex_lock(&mutex);
      if (!start_chrono){
        pthread_create(&timer_thread_id,NULL,timer,NULL);
        start_chrono = VRAI;
      }
      pthread_mutex_unlock(&mutex);
      /*On lance le chrono*/
        

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
      printf("%s: envoi du score au client n %d : %s\n",CMD,dataTh->n_client,score);
      ecrireLigne(canal,score);

      /*calcul classement*/
      generateRanking(score_tab,classement);
      printf("%s: annonce les résultats des clients\n",CMD);
      sprintf(ligne, "%d",classement[dataTh->n_client]);
      ecrireLigne(canal,ligne);
      clients_prets--;
      if (clients_prets == 0)
      {

        /*On remet le chrono à 0*/
        start_chrono = FAUX;
        stop = FAUX;

        /*On remet le classement à 0*/
        classement[dataTh->n_client] = 0;
        /*On remet le tableau de mots à 0*/
        for (i = 0; i < 100 ; i++)
        {
          strcpy(mots_ecrits[i],"");
        }
        /*On remet la phrase à 0*/
        for (i = 0; i < 100 ; i++)
        {
          strcpy(phrase[i],"");
        }
        /*On remet le tableau le phrase_flag à FAUX*/
        phrase_flag = FAUX;
        /*On remet à 0 le chrono*/
        start_time = 0;
        elapsed_time = 0;
      }
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


void *timer()
{
    time(&start_time);
    time(&elapsed_time);
  while(difftime(elapsed_time,start_time) < GAME_TIME)
  {
    sleep(0.5);
    time(&elapsed_time);
  }
  stop = VRAI;
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

int trouver_premier_libre(int tab[]) {
  int i;
  for (i = 0; i < NB_CLIENT; i++)
    if (tab[i] == -1)
      return i;
  return -1;
}