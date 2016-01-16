/*####################################################################
#		SMP Messenger Client																						 #
#--------------------------------------------------------------------#
#		@Authors : Alexis GARDAVOIR && Samuel GARCIA										 #
#--------------------------------------------------------------------#
#		Programme principal :: main.c																		 #
######################################################################
#		Ce programme est un client SMP (Simple Mail Protocol) issue de   #
# 	RFC du même nom. Le but est donc la communication avec un 	     #
#		serveur SMP et donc l'échange de mails entre différents 				 #
#		utilisateurs. 																									 #
####################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main() {
	/* VARIABLES */
	char *req;						//Requête de l'utilisateur
	char *SrvAddress; 		//Adresse du serveur

	/* INITIALISATION DES VARIABLES */
	SrvAddress = malloc(1024);
	SrvAddress = "localhost";

	/* DEBUT */
	/* Initialisation de la connexion avec le serveur */
	printf("Test de la connexion avec le serveur.\n");
	if(Initialisation(SrvAddress) != 1) {
			printf("Erreur d'initialisation\n");
			return 1;
	} else {
			printf("Le serveur est en ligne.");
	}

	/* Saise des identifiants */

	/* Envoi des identifiants et résupération du session ID*/

	/* Connexion au service, echec si le sessionID retourné est 1*/

	/* Traitement du choix de l'utilisateur */
		/* Proposition des différents cas et appel des fonctions correspondantes */


	/* -------------------------------------------------- */
	if(Emission("Test de req client1.\n")!=1) {
		printf("Erreur d'�mission\n");
		return 1;
	}

	req = Reception();

	if(req != NULL) {
		printf("J'ai recu: %s\n", req);
		free(req);
	} else {
		printf("Erreur de r�ception\n");
		return 1;
	}

	req = Reception();
	if(req != NULL) {
		printf("J'ai recu: %s\n", req);
		free(req);
	} else {
		printf("Erreur de r�ception\n");
		return 1;
	}

	req = Reception();
	if(req != NULL) {
		printf("J'ai recu: %s\n", req);
		free(req);
	} else {
		printf("Erreur de r�ception\n");
		return 1;
	}

	Terminaison();

	/* FIN */
	return 0;
}
