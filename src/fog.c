#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sodium.h>

// Prototypes
void fog(void);
void wind(void);
uint8_t *encode_file_type(const char *filename, size_t *out_len);
uint8_t *encode_file_size(size_t file_size, size_t *out_len);
int is_image_file(const char *filename);
unsigned int *assign_bits_to_images_xchacha(
    size_t total_bits,
    size_t n_images,
    const unsigned char *key,   // 32 octets
    const unsigned char *nonce  // 24 octets
);

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

    char *file_path = NULL;
    size_t bufsize = 0;

    printf("Entrez un chemin de fichier à cacher : ");
    ssize_t input_len = getline(&file_path, &bufsize, stdin);
    if (input_len == -1) {
        perror("getline");
        return;
    }
    file_path[strcspn(file_path, "\n")] = '\0';

    struct stat st;
    if (stat(file_path, &st) == -1) {
        perror("stat");
        free(file_path);
        return;
    }

    size_t file_size = st.st_size;
    printf("Taille du fichier à cacher : %zu octets\n", file_size);

    // Encodage du type de fichier
    size_t file_type_len;
    uint8_t *encoded_type = encode_file_type(file_path, &file_type_len);
    if (!encoded_type) {
        fprintf(stderr, "Erreur d'encodage du type de fichier.\n");
        free(file_path);
        return;
    }

    // Encodage de la taille du fichier
    size_t encoded_size_len;
    uint8_t *encoded_size = encode_file_size(file_size, &encoded_size_len);
    if (!encoded_size) {
        fprintf(stderr, "Erreur d'encodage de la taille du fichier.\n");
        free(encoded_type);
        free(file_path);
        return;
    }

    // Lecture du chemin vers le dossier image_bank
    char *image_folder = NULL;
    size_t folder_bufsize = 0;
    printf("Entrez le chemin vers le dossier image_bank : ");
    if (getline(&image_folder, &folder_bufsize, stdin) == -1) {
        perror("getline");
        free(file_path);
        free(encoded_type);
        free(encoded_size);
        return;
    }
    image_folder[strcspn(image_folder, "\n")] = '\0';

    // Parcours du dossier
    DIR *dir = opendir(image_folder);
    if (!dir) {
        perror("opendir");
        free(file_path);
        free(encoded_type);
        free(encoded_size);
        free(image_folder);
        return;
    }

    struct dirent *entry;  
    size_t total_message_bits = (file_size + file_type_len + encoded_size_len) * 8;

    

        // Demander une clé à l'utilisateur
    char password[256];
    printf("Entrez une clé symétrique : ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        perror("fgets");
        return;
    }
    password[strcspn(password, "\n")] = '\0';

    // Dériver la clé avec SHA256
    uint8_t key[crypto_stream_xchacha20_KEYBYTES]; // 32 octets
    crypto_hash_sha256(key, (const unsigned char *)password, strlen(password));
    printf("Clé dérivée (SHA256) : ");
    for (int i = 0; i < 32; ++i) printf("%02x", key[i]);
    printf("\n");

    // Créer la liste des fichiers images
    rewinddir(dir); // Revenir au début du dossier
    size_t img_count = 0;
    char **img_paths = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t path_len = strlen(image_folder) + strlen(entry->d_name) + 2;
        char *fullpath = malloc(path_len);
        snprintf(fullpath, path_len, "%s/%s", image_folder, entry->d_name);

        struct stat st_img;
        if (stat(fullpath, &st_img) == -1 || !S_ISREG(st_img.st_mode)) {
            free(fullpath);
            continue;
        }

        // Ajouter à la liste
        img_paths = realloc(img_paths, sizeof(char *) * (img_count + 1));
        img_paths[img_count++] = fullpath;
    }
    closedir(dir);

    // Calcul du nonce (XOR des 24 premiers octets de chaque image)
    uint8_t nonce[crypto_stream_xchacha20_NONCEBYTES] = {0}; // 24 octets
    uint8_t buffer[crypto_stream_xchacha20_NONCEBYTES];

    for (size_t i = 0; i < img_count; ++i) {
        FILE *f = fopen(img_paths[i], "rb");
        if (!f) continue;

        size_t read_bytes = fread(buffer, 1, crypto_stream_xchacha20_NONCEBYTES, f);
        fclose(f);

        if (read_bytes < crypto_stream_xchacha20_NONCEBYTES) continue;

        for (size_t j = 0; j < crypto_stream_xchacha20_NONCEBYTES; ++j) {
            nonce[j] ^= buffer[j];
        }
    }

    printf("Nonce généré : ");
    for (int i = 0; i < crypto_stream_xchacha20_NONCEBYTES; ++i) printf("%02x", nonce[i]);
    printf("\n");

    // Générer l’ordre pseudo-aléatoire des images (shuffle de Fisher-Yates)
    unsigned int *order = malloc(sizeof(unsigned int) * img_count);
    for (unsigned int i = 0; i < img_count; ++i) order[i] = i;

    // Générateur de flux pseudo-aléatoire avec XChaCha20
    uint8_t rand_bytes[img_count * 4]; // Assez pour des uint32_t
    crypto_stream_xchacha20(rand_bytes, sizeof(rand_bytes), nonce, key);

    // Appliquer Fisher-Yates avec rand_bytes
    for (int i = img_count - 1; i > 0; --i) {
        uint32_t r;
        memcpy(&r, &rand_bytes[(img_count - 1 - i) * 4], 4);
        unsigned int j = r % (i + 1);
        unsigned int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    printf("Ordre pseudo-aléatoire des images :\n");
    for (size_t i = 0; i < img_count; ++i) {
        printf(" %zu: %s\n", i + 1, img_paths[order[i]]);
    }
    size_t total_capacity = 0;

    printf("Images trouvées dans l'ordre pseudo-aléatoire (excluant la première) :\n");
    for (size_t i = 1; i < img_count; ++i) {  // Commence à 1 pour exclure la première image
        const char *img_path = img_paths[order[i]];

        struct stat img_stat;
        if (stat(img_path, &img_stat) == -1) {
            perror("stat image");
            continue;
        }

        printf(" - %s : %zu octets\n", img_path, (size_t)img_stat.st_size);

        total_capacity += img_stat.st_size * 3;
    }
    
    printf("Capacité totale disponible : %zu bits\n", total_capacity);
    printf("Taille du message à encoder : %zu bits\n", total_message_bits);

    if (total_capacity < total_message_bits) {
        fprintf(stderr, "Erreur : pas assez de capacité dans les images pour encoder le message.\n");
    } else {
        printf("Capacité suffisante pour encoder le message.\n");
    }
    
    unsigned int *bit_image_map = assign_bits_to_images_xchacha(
        total_message_bits, img_count, key, nonce
    );

    // Affichage des premières assignations pour debug
    printf("Premiers bits affectés aux images (index dans img_paths) :\n");
    for (size_t i = 0; i < total_message_bits; ++i) {
        printf("Bit %zu → image %u (%s)\n", i, bit_image_map[i], img_paths[bit_image_map[i]]);
    }

    // Libération mémoire
    free(file_path);
    free(encoded_type);
    free(encoded_size);
    free(image_folder);
}

void wind(void) {
    printf("Fonction de décryptage exécutée (wind).\n");
    // Ici tu implémenteras le décryptage réel plus tard
}

// Fonction pour encoder le type de fichier
uint8_t *encode_file_type(const char *filename, size_t *out_len) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename + strlen(filename) - 1) {
        fprintf(stderr, "Fichier sans extension.\n");
        return NULL;
    }

    const char *ext = dot + 1;
    size_t ext_len = strlen(ext);

    // Cas standard : 1 octet indicateur + 3 octets d'extension
    if (ext_len == 3) {
        // Allocation dynamique avec la taille fixe
        *out_len = 1 + 3;
        uint8_t *out = malloc(*out_len);
        if (!out) {
            fprintf(stderr, "Erreur d'allocation mémoire pour l'encodage du fichier.\n");
            return NULL;
        }
        out[0] = 0; // indicateur standard
        memcpy(&out[1], ext, 3); // 3 octets d'extension
        return out;
    }

    // Cas non-standard : extension de taille variable
    char sep = 'a'; // caractère de scission (peut être aléatoire dans [a-z])
    
    // Convertir la taille de l'extension en string
    size_t size_str_len = snprintf(NULL, 0, "%zu", ext_len); // Calcul de la taille requise pour le nombre
    char *size_str = malloc(size_str_len + 1); // Allocation dynamique pour la taille de l'extension
    if (!size_str) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la taille de l'extension.\n");
        return NULL;
    }
    snprintf(size_str, size_str_len + 1, "%zu", ext_len); // Conversion de la taille en chaîne de caractères

    // Allouer dynamiquement pour la taille totale de l'extension
    *out_len = 1 + 1 + size_str_len + 1 + ext_len;
    uint8_t *out = malloc(*out_len);
    if (!out) {
        free(size_str); // Libérer la mémoire allouée pour size_str si l'allocation échoue
        fprintf(stderr, "Erreur d'allocation mémoire pour l'encodage du fichier.\n");
        return NULL;
    }

    size_t i = 0;
    out[i++] = 1;         // indicateur non-standard
    out[i++] = sep;       // séparateur 1
    memcpy(&out[i], size_str, size_str_len); i += size_str_len; // taille de l'extension
    out[i++] = sep;       // séparateur 2
    memcpy(&out[i], ext, ext_len); // extension elle-même

    free(size_str); // Libérer la mémoire allouée pour size_str après usage

    return out;
}

// Encode la taille du message avec séparateurs alphabétiques
uint8_t *encode_file_size(size_t file_size, size_t *out_len) {
    char sep = 'a';  // séparateur alphabétique, peut être modifié ou aléatoire

    // Convertir la taille en chaîne de caractères
    char size_str[32];  // 32 est suffisant pour contenir un size_t en ASCII
    int len = snprintf(size_str, sizeof(size_str), "%zu", file_size);
    if (len < 0 || len >= (int)sizeof(size_str)) {
        fprintf(stderr, "Erreur de conversion de la taille.\n");
        return NULL;
    }

    // Allouer le buffer total : 1 (sep1) + len + 1 (sep2)
    *out_len = len + 2;
    uint8_t *out = malloc(*out_len);
    if (!out) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la taille du message.\n");
        return NULL;
    }

    out[0] = sep;
    memcpy(&out[1], size_str, len);
    out[len + 1] = sep;

    return out;
}

// Fonction pour vérifier si un fichier a une extension d'image
int is_image_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    return strcmp(ext, ".png") == 0;// || strcmp(ext, ".bmp") == 0 || strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0;
}



unsigned int *assign_bits_to_images_xchacha(
    size_t total_bits,
    size_t n_images,
    const unsigned char *key,   // 32 octets
    const unsigned char *nonce  // 24 octets
) {
    if (n_images == 0) return NULL;
    unsigned int *bit_assignments = malloc(sizeof(unsigned int) * total_bits);
    if (!bit_assignments) {
        fprintf(stderr, "Erreur d'allocation mémoire pour les assignations de bits.\n");
        return NULL;
    }

    // Générer un flux pseudo-aléatoire
    size_t rand_len = total_bits * sizeof(uint32_t);
    uint32_t *rand_stream = malloc(rand_len);
    if (!rand_stream) {
        free(bit_assignments);
        return NULL;
    }
    crypto_stream_xchacha20((unsigned char*)rand_stream, rand_len, nonce, key);

    // Assigner chaque bit à une image aléatoire
    for (size_t i = 0; i < total_bits; ++i) {
        bit_assignments[i] = rand_stream[i] % n_images;
    }

    free(rand_stream);
    return bit_assignments;
}