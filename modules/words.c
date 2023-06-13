//#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


char *get_word(int ligne);

int main() {

    FILE *file = fopen("liste_francais.txt", "r");
    if (file == NULL) {
        printf("Impossible d'ouvrir le fichier liste_francais.txt\n");
        exit(1);
    }
    char sentence[1000];
    char new_word[20];
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));
    for (int i = 0; i < 100 ; i++)
    {
        //On génère un nombre aléaoire entre 0 et 21074 100 fois pour avoir 100 mots
        int random = rand() % 21073;
        strcpy(new_word,get_word(random));
        sprintf(sentence, "%s %s",sentence,new_word);
    }
    
    printf("phrase :\n%s",sentence);
    
    return 0;
}

char *get_word(int ligne)
/*retourne le mot de la ligne ligne du fichier texte file */
{
    FILE *file = fopen("liste_francais.txt", "r");
    char *mot = malloc(20*sizeof(char));
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
    return mot;
}