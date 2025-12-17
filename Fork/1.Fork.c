#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>

#define N 10 // Taille du tampon
#define NB_ITEMS 20

typedef struct {
    int tab[N];
    int i; // indice d'ecriture (Producteur)
    int j; // indice de lecture (Consommateur)
    sem_t places_libres;
    sem_t items_existants;
    sem_t mutex;
} Memoire_partagee;

int main() {
    // 1. Creation memoire partagee
    Memoire_partagee* partagee = mmap(NULL, sizeof(Memoire_partagee), 
                                      PROT_READ | PROT_WRITE, 
                                      MAP_SHARED | MAP_ANONYMOUS, -1, 0); 

    if (partagee == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // 2. Initialisation des variables et semaphores 
    partagee->i = 0;
    partagee->j = 0;

    sem_init(&partagee->places_libres, 1, N);   // N places libres
    sem_init(&partagee->items_existants, 1, 0); // 0 items existants
    sem_init(&partagee->mutex, 1, 1);           // 1 clé (Mutex)

    // 3. Creation du processus
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork"); // CORRECTION : Ajout du point-virgule
        exit(1);
    }

    // --- FILS (CONSOMMATEUR) ---
    if (pid == 0) {
        for(int i = 0; i < NB_ITEMS; i++) {
            int item;
        
            sem_wait(&partagee->items_existants); 
            sem_wait(&partagee->mutex); 

            // Recuperation de l'item (Section Critique)
            item = partagee->tab[partagee->j]; 
            printf("<- Fils : lu %d\n", item); // Ajout d'un print pour voir ce qui se passe
            
            partagee->j = (partagee->j + 1) % N; 
        
            sem_post(&partagee->mutex); 
        
            // Signaler une place vide
            // CORRECTION : "place_libres" -> "places_libres" (faute de frappe) + Point-virgule
            sem_post(&partagee->places_libres); 

            sleep(1); 
        }
        exit(0); // Le fils s'arrête ici
    }
    
    // --- PERE (PRODUCTEUR) ---
    else {
        for(int i = 0; i < NB_ITEMS; i++) {
            int item = i * 10;  // Creation entier quelconque
            
            sem_wait(&partagee->places_libres); 
            sem_wait(&partagee->mutex);

            partagee->tab[partagee->i] = item;
            printf("-> Pere : ecrit %d\n", item); 
            
            
            partagee->i = (partagee->i + 1) % N; 

            sem_post(&partagee->mutex);
            sem_post(&partagee->items_existants);
            
            sleep(1);
        }

        // 4. Le Père attend que le fils ait fini
        wait(NULL); 
        
        printf("--- Fin du Père et nettoyage ---\n");

        // 5. Nettoyage (Uniquement fait par le père à la fin)
        sem_destroy(&partagee->places_libres);
        sem_destroy(&partagee->items_existants);
        sem_destroy(&partagee->mutex);
        munmap(partagee, sizeof(Memoire_partagee));
    } 

    return 0;

}
