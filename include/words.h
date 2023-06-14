#ifndef WORDS_H
#define WORDS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TAILLE_MOT 30
#define TAILLE_PHRASE 100
void generate_sentence(char sentence[TAILLE_PHRASE][TAILLE_MOT]);
char *get_word(int ligne);

#endif