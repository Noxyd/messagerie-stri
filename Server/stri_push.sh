#!/bin/bash
#Petit script pour mettre a jour un fichier passé en paramètre
#La description est à placer en deuxieme paramètre
COMM="$2 (`date`)"
ID=".id"

git add $1
git commit -m "$COMM"
git push 
