#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>    // [cite: 1915]
#include <semaphore.h>  
#include <string.h>

#define N 10            
#define NB_ITEMS 20     
#define TAILLE_MSG 64

// Structure enveloppe
typedef struct {
    char texte[TAILLE_MSG];
} Donnee;

// --- VARIABLES GLOBALES ---
Donnee tab[N]; // Tableau global de structures
int in = 0;  
int out = 0; 

sem_t places_libres;
sem_t items_existants;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // [cite: 2013]

// --- ROUTINE DU PRODUCTEUR ---
void * producteur(void * arg) {
    for (int k = 0; k < NB_ITEMS; k++) {
        Donnee item;
        snprintf(item.texte, TAILLE_MSG, "ThreadMsg %d", k);
        
        sem_wait(&places_libres);
        pthread_mutex_lock(&mutex); // [cite: 2017]

        // Copie structure dans le tableau global
        tab[in] = item; 
        printf("-> Producteur : Ecrit '%s' index %d\n", item.texte, in);
        in = (in + 1) % N;
        
        pthread_mutex_unlock(&mutex); // [cite: 2018]
        sem_post(&items_existants);

        sleep(1);
    }
    pthread_exit(NULL);
}

// --- ROUTINE DU CONSOMMATEUR ---
void * consommateur(void * arg) {
    for (int k = 0; k < NB_ITEMS; k++) {
        Donnee item;
        
        sem_wait(&items_existants);
        pthread_mutex_lock(&mutex);

        // Lecture du tableau global
        item = tab[out]; 
        printf("<- Consommateur : Lu '%s' index %d\n", item.texte, out);
        out = (out + 1) % N;
        
        pthread_mutex_unlock(&mutex);
        sem_post(&places_libres);

        sleep(1);
    }
    pthread_exit(NULL);
}

// Le main reste identique à la correction précédente (création threads, join, destroy)
int main() {
    pthread_t th_prod, th_conso; //pthread_t sert à stocker l'identifiant unique d'un thread (processus léger)
				 //th_prod : Recevra l'identifiant du thread Producteur et de meme pour th_conso

    printf("--- Debut avec Threads (Strings) ---\n");

    sem_init(&places_libres, 0, N);
    sem_init(&items_existants, 0, 0);
    
    if (pthread_create(&th_prod, NULL, producteur, NULL) != 0) { // [cite: 1919]
        perror("pthread_create prod");
        exit(1);
    }

    if (pthread_create(&th_conso, NULL, consommateur, NULL) != 0) {
        perror("pthread_create conso");
        exit(1);
    }
    
    pthread_join(th_prod, NULL); // [cite: 1955]
    pthread_join(th_conso, NULL);

    printf("--- Fin des threads ---\n");

    sem_destroy(&places_libres);
    sem_destroy(&items_existants);
    pthread_mutex_destroy(&mutex);

    return 0;
}