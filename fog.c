#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE 1024 // Taille du bloc de lecture

void process_bit(int bit) {
    // Exemple : afficher le bit
    printf("%d", bit);
}

// --- Description ---
// Ouvre un fichier et lit le contenu bit par bit
// --- Fonction appelées ---
// -> process_bit(int bit)
void process_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    uint8_t buffer[BLOCK_SIZE]; // Allocation sur la pile
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BLOCK_SIZE, file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            for (int j = 7; j >= 0; j--) {
                int bit = (buffer[i] >> j) & 1; // Extraire le j-ème bit
                process_bit(bit);
            }
        }
    }

    fclose(file);
}

int main() {
    const char *filename = "SimarisSigilGold.png";
    process_file(filename);
    return 0;
}
