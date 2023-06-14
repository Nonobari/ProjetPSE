#include "words.h"



void generate_sentence(char sentence[TAILLE_PHRASE][TAILLE_MOT]) {

    char new_word[TAILLE_MOT];
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    for (int i = 0; i < 100 ; i++)
    {
        //On génère un nombre aléaoire entre 0 et 21074 100 fois pour avoir 100 mots
        int random = rand() % 21073;
        strcpy(new_word,get_word(random));
        strcpy(sentence[i],new_word);
    }
}


char *get_word(int ligne)
/*retourne le mot de la ligne ligne du fichier texte file */
{
    FILE *file = fopen("liste_francais.txt", "r");
    if (file == NULL)
    {
        printf("Impossible d'ouvrir le fichier\n");
    }
    //test si le fichier a bien été ouvert
    char *mot = malloc(TAILLE_MOT*sizeof(char));
    int i = 0;
    char c;
    while (i < ligne)
    {
        c = fgetc(file);
        if (c == '\n')
        {
            i++;
        }
    }
    fscanf(file, "%s", mot);
    fclose(file);
    sprintf(mot,"%s\n",mot);
    return mot;
}