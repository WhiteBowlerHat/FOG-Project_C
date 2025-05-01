#include <stdio.h>
#include <string.h>

// Prototypes
void fog(void);
void wind(void);

int main(void) {
    char choice[10];

    printf("Souhaitez-vous encrypter ou décrypter ? (e/d): ");
    if (fgets(choice, sizeof(choice), stdin) == NULL) {
        fprintf(stderr, "Erreur de lecture.\n");
        return 1;
    }

    // Supprimer le saut de ligne éventuel
    choice[strcspn(choice, "\n")] = 0;

    if (strcmp(choice, "e") == 0 || strcmp(choice, "encrypter") == 0) {
        fog();
    } else if (strcmp(choice, "d") == 0 || strcmp(choice, "decrypter") == 0) {
        wind();
    } else {
        printf("Choix invalide. Utilisez 'e' pour encrypter ou 'd' pour décrypter.\n");
        return 1;
    }

    return 0;
}

void fog(void) {
    printf("Fonction d'encryptage exécutée (fog).\n");
    // Ici tu implémenteras l'encryptage réel plus tard
}

void wind(void) {
    printf("Fonction de décryptage exécutée (wind).\n");
    // Ici tu implémenteras le décryptage réel plus tard
}
