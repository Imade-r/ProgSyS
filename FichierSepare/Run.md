Pour lancer avec la version communicant:

gcc 3-producteur.c -o prod3 -lrt -lpthread
gcc 3-consommateur.c -o conso3 -lrt -lpthread
gcc communicant.c -o comm -lrt
3.  **Lancez dans 3 terminaux :**
* T1 : `./prod3`
* T2 : `./conso3`
* T3 : `./comm`
