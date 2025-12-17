#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>    // Pour les threads et mutex [cite: 1866]
#include <semaphore.h>  // Pour les sémaphores de comptage

#define N 10            // Taille du tampon
#define NB_ITEMS 20     // Nombre d'items à traiter

// --- VARIABLES GLOBALES (Partagées par défaut entre threads) ---

int tab[N];
int i = 0; // Index écriture
int j = 0; // Index lecture

// Outils de synchronisation
sem_t places_libres;
sem_t items_existants;
// Initialisation statique du mutex [cite: 1972]
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

// --- ROUTINE DU PRODUCTEUR ---
// Doit retourner void* et prendre void* en argument [cite: 1890]

// signature obligatoire d'une fonction pour qu'elle puisse être utilisée comme un thread avec la bibliothèque pthread
void * producteur(void * arg) {
    // Utilisation de 'k' pour la boucle pour ne pas confondre avec l'index global 'i'
    for (int k = 0; k < NB_ITEMS; k++) {
        int item = k * 10;  // Production d'un entier
        
        sem_wait(&places_libres); // Attente d'une place libre [cite: 1688]

        pthread_mutex_lock(&mutex); // Section critique [cite: 1976]

        tab[i] = item;
        printf("Ecriture de %d a l'index %d\n", item, i);
        i = (i + 1) % N; // Gestion circulaire
        
        pthread_mutex_unlock(&mutex); // Fin section critique [cite: 1977]
        sem_post(&items_existants); // Signal qu'un item est disponible [cite: 1689]

        sleep(1); // Ajout du point-virgule manquant
    }
    pthread_exit(NULL); // Fin du thread [cite: 1915]
}

// --- ROUTINE DU CONSOMMATEUR ---
void * consommateur(void * arg) {
    for (int k = 0; k < NB_ITEMS; k++) {
        int item; 
        
        sem_wait(&items_existants); // Attente d'un item

        pthread_mutex_lock(&mutex);

        item = tab[j];
        printf("Lecture de %d a l'index %d\n", item, j);
        j = (j + 1) % N;
        
        pthread_mutex_unlock(&mutex);
        sem_post(&places_libres); // Signal qu'une place est libre

        sleep(1); // Ajout du point-virgule manquant
    }
    pthread_exit(NULL); // Fin du thread
}
        
int main() {
    pthread_t th_prod, th_conso; // pthread_t sert à stocker l'identifiant unique d'un thread (processus léger) [cite: 1875]
                                 // th_prod : Recevra l'identifiant du thread Producteur et de meme pour th_conso

    printf("--- Debut avec Threads ---\n");

    // Initialisation des sémaphores [cite: 1751]
    // 0 signifie partagé entre threads du même processus, N est la valeur initiale
    sem_init(&places_libres, 0, N);
    sem_init(&items_existants, 0, 0);
    
    // Création des threads [cite: 1875]
    // Correction de la typo 'phthread_create' -> 'pthread_create'
    if (pthread_create(&th_prod, NULL, producteur, NULL) != 0) {
        perror("pthread_create prod");
        exit(1);
    }

    if (pthread_create(&th_conso, NULL, consommateur, NULL) != 0) {
        perror("pthread_create conso");
        exit(1);
    }
    
    // Attente de la fin des threads (équivalent du wait pour les processus) [cite: 1914]
    pthread_join(th_prod, NULL);
    pthread_join(th_conso, NULL);

    // Nettoyage [cite: 1754]
    // Correction des noms de variables (sem_vide -> places_libres)
    sem_destroy(&places_libres);
    sem_destroy(&items_existants);
    pthread_mutex_destroy(&mutex); // Destruction du mutex [cite: 1974]

    return 0;
}