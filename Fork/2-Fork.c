#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define N 10 
#define NB_ITEMS 20
#define TAILLE_MSG 64

// La structure qui remplace l'entier
typedef struct {
    char texte[TAILLE_MSG];
} Donnee;

typedef struct {
    Donnee tab[N]; // Tableau de structures Donnee
    int i; 
    int j; 
    sem_t places_libres;
    sem_t items_existants;
    sem_t mutex;
} Memoire_partagee;

int main() {
    // 1. Création mémoire partagée anonyme [cite: 1515, 1516]
    Memoire_partagee* partagee = mmap(NULL, sizeof(Memoire_partagee), 
                                      PROT_READ | PROT_WRITE, 
                                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (partagee == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // 2. Initialisation
    partagee->i = 0;
    partagee->j = 0;
    sem_init(&partagee->places_libres, 1, N);
    sem_init(&partagee->items_existants, 1, 0);
    sem_init(&partagee->mutex, 1, 1);

    pid_t pid = fork(); // [cite: 891]
    
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    // --- FILS (CONSOMMATEUR) ---
    if (pid == 0) {
        for(int k = 0; k < NB_ITEMS; k++) {
            Donnee item_recu; // Variable locale de type structure
        
            sem_wait(&partagee->items_existants); 
            sem_wait(&partagee->mutex);

            // COPIE AUTOMATIQUE grâce à la structure
            item_recu = partagee->tab[partagee->j]; 
            
            printf("<- Fils : lu '%s' à l'index %d\n", item_recu.texte, partagee->j);
            
            partagee->j = (partagee->j + 1) % N; 
        
            sem_post(&partagee->mutex);
            sem_post(&partagee->places_libres);

            sleep(1);
        }
        exit(0); 
    }
    // --- PÈRE (PRODUCTEUR) ---
    else {
        for(int k = 0; k < NB_ITEMS; k++) {
            Donnee item_a_envoyer;
            // On écrit une phrase dans la structure locale
            snprintf(item_a_envoyer.texte, TAILLE_MSG, "Colis numero %d", k * 10);
            
            sem_wait(&partagee->places_libres);
            sem_wait(&partagee->mutex);

            // COPIE AUTOMATIQUE vers la mémoire partagée
            partagee->tab[partagee->i] = item_a_envoyer;
            
            printf("-> Père : écrit '%s' à l'index %d\n", item_a_envoyer.texte, partagee->i);
            
            partagee->i = (partagee->i + 1) % N;

            sem_post(&partagee->mutex);
            sem_post(&partagee->items_existants); 
            
            sleep(1);
        }

        wait(NULL); // [cite: 940]
        printf("--- Fin du Père et nettoyage ---\n");

        sem_destroy(&partagee->places_libres);
        sem_destroy(&partagee->items_existants);
        sem_destroy(&partagee->mutex);
        munmap(partagee, sizeof(Memoire_partagee));
    }

    return 0;
}