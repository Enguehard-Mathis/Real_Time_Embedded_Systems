#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_RUNS 10000

long long tau1(void);
int cmp_double(const void *a, const void *b);

int main() {
    srand(42); // graine fixe pour la reproductibilite

    double temps[N_RUNS];
    struct timespec t0, t1;
    // on mesure N_RUNS fois le temps d'execution de tau1
    // on utilise CLOCK_MONOTONIC 
    for (int i = 0; i < N_RUNS; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        tau1();
        clock_gettime(CLOCK_MONOTONIC, &t1);
        // conversion en nanosecondes
        temps[i] = (t1.tv_sec - t0.tv_sec) * 1e9 + (t1.tv_nsec - t0.tv_nsec);
    }

    // tri pour lire les quantiles directement par indice
    qsort(temps, N_RUNS, sizeof(double), cmp_double);

    printf("Resultats sur %d executions de tau1 :\n\n", N_RUNS);
    printf("  Min  (0%%)   : %.0f ns\n",   temps[0]);
    printf("  Q1   (25%%)  : %.0f ns\n",   temps[N_RUNS / 4]);
    printf("  Q2   (50%%)  : %.0f ns\n",   temps[N_RUNS / 2]);
    printf("  Q3   (75%%)  : %.0f ns\n",   temps[3 * N_RUNS / 4]);
    printf("  Max  (100%%) : %.0f ns  <-- WCET = C1\n", temps[N_RUNS - 1]);

    return 0;
}

// La tache tau1 : produit de deux grands entiers aleatoires
// on prend des grands nombres pour bien solliciter le multiplieur 64-bit
long long tau1(void) {
    // nombres entre 1 milliard et 2 milliards
    long long a = (long long)rand() % 1000000000 + 1000000000;
    long long b = (long long)rand() % 1000000000 + 1000000000;
    return a * b;
}

// fonction de comparaison pour ordre croissant
int cmp_double(const void *a, const void *b) {
    double x = *(double *)a;
    double y = *(double *)b;
    if (x < y) return -1;
    if (x > y) return  1;
    return 0;
}

