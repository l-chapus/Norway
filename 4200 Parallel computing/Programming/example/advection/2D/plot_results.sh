#!/bin/bash

# Répertoire contenant les fichiers de données
DATA_DIR="data"

# Répertoire où enregistrer les images générées
OUTPUT_DIR="images"

# Créer le répertoire de sortie s'il n'existe pas
mkdir -p "$OUTPUT_DIR"

# Boucle à travers tous les fichiers .dat dans DATA_DIR
for DATA_FILE in "$DATA_DIR"/*.dat; do
    # Extraire le nom de base du fichier (sans extension)
    BASENAME=$(basename "$DATA_FILE" .dat)

    # Construire le nom de fichier de sortie PNG
    PNGFILE="$OUTPUT_DIR/${BASENAME}.png"

    # Générer l'image à partir du fichier de données
    cat <<EOSCRIPT | gnuplot -
set term png
set output "${PNGFILE}"
set zrange [0:1.5]
splot "$DATA_FILE" binary array=512x512 format='%double' with pm3d
EOSCRIPT

    echo "Image générée: $PNGFILE"
done
