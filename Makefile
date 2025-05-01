# Dossiers
SRC_DIR = src
BUILD_DIR = build

# Fichier source et cible
SRC_FILE = $(SRC_DIR)/fog.c
OUTPUT = $(BUILD_DIR)/fog

# Cible par défaut
all: $(OUTPUT)

# Compilation
$(OUTPUT): $(SRC_FILE)
	mkdir -p $(BUILD_DIR)
	gcc $(SRC_FILE) -o $(OUTPUT) -lsodium

# Nettoyage
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean