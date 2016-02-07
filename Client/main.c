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
#include <string.h>
#include "client.h"

#define EXIT_FAILURE 1
#define EXIT_OK 0
#define EVAL_TRUE 1
#define EVAL_FALSE 0

int main() {
	/* VARIABLES */
	char *req;								//Requête de l'utilisateur
	char *srvAddress; 				//Adresse du serveur
	char *username;						//Nom de connexion de l'utilisateur
	char *password;						//Mot de passe de l'utilisateur
	char *sessionid; 					//Identifiant de session
	char *reqRec; 						//requete reçue en réception
	int retour;								//Variable de retour
	int again;								//variable de boucle
	char requete[100] = {0};	//requete transmise au serveur

	/* INITIALISATION DES VARIABLES */
	srvAddress = malloc(sizeof(char)*1024);
	srvAddress = "localhost";
	reqRec = malloc(sizeof(char)*1024);
	sessionid = malloc(sizeof(char)*8);
	username = NULL;
	password = NULL;
	again = EVAL_FALSE;

	/* DEBUT */
	do{
		Welcome(); //Affichage de l'interface graphique

		//Initialisation de la connexion avec le serveur
		printf("Test de la connexion avec le serveur.\n");
		if(Initialisation(srvAddress) != 1) {
				printf("Le serveur est hors ligne ...\n");
				return EXIT_FAILURE;
		} else {
				printf("Le serveur est en ligne.");
		}
		//puts("Phase de test, on supprime le test de connexion au serveur.");
		printf("\n**************************************************************\n\n");

		/* Saise des identifiants */
		retour = DemandeID(&username, &password);
		if(retour != 0){
			return EXIT_FAILURE;
		}

		//connectUser(&username, &password);
		/* Création de la requete de connexion */
		strcat(requete,"CONNECT ");
		strcat(requete,username);
		strcat(requete," ");
		strcat(requete,password);
		strcat(requete,"\n");

		printf("%s\n", requete);	//Affichage de la requete envoyée
		/* Envoi des identifiants et récupération du session ID*/
		if(Emission(requete)!=1) {
				printf("Erreur d'émission\n");
				return 1;
		}

		/* Connexion au service, echec si le sessionID retourné est 1*/
		reqRec = Reception();
		get_word(reqRec, sessionid,2);
		printf("%s\n",reqRec );

		if(cmp_word(sessionid,"1",1)){
			printf("Erreur lors de la réception du session ID : val = %s\n", sessionid);
			return 1;
		}

		/* Traitement du choix de l'utilisateur */
		do{
			/* Proposition des différents cas et appel des fonctions correspondantes */
			again = ChoiceScreen(username, sessionid);
		}while(again != EVAL_FALSE);
	}while(1);
	/* -------------------------------------------------- */
/*	if(Emission("Test de req client1.\n")!=1) {
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

	Terminaison();*/

	/* FIN */
	return 0;
}
