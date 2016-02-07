/*####################################################################
#		SMP Messenger Client																						 #
#--------------------------------------------------------------------#
#		@Authors : Alexis GARDAVOIR && Samuel GARCIA										 #
#--------------------------------------------------------------------#
#		Définition des fonctions :: client.h														 #
####################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "client.h"

#define EVAL_TRUE 1
#define EVAL_FALSE 0

#define LONGUEUR_TAMPON 4096

#define EXIT_FAILURE 1
#define EXIT_OK 0

/* Variables cachees */

/* le socket client */
int socketClient;

/* le tampon de reception */
char tamponClient[LONGUEUR_TAMPON];
int debutTampon;
int finTampon;

/* Initialisation.
 * Connexion au serveur sur la machine donnee.
 * Utilisez localhost pour un fonctionnement local.
 */
int Initialisation(char *machine) {
	return InitialisationAvecService(machine, "14215");
}

/* Initialisation.
 * Connexion au serveur sur la machine donnee et au service donne.
 * Utilisez localhost pour un fonctionnement local.
 */
int InitialisationAvecService(char *machine, char *service) {
	int n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(machine, service, &hints, &res)) != 0)  {
     		fprintf(stderr, "Initialisation, erreur de getaddrinfo : %s", gai_strerror(n));
     		return 0;
	}
	ressave = res;

	do {
		socketClient = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketClient < 0)
			continue;	/* ignore this one */

		if (connect(socketClient, res->ai_addr, res->ai_addrlen) == 0)
			break;		/* success */

		close(socketClient);	/* ignore this one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
     		perror("Initialisation, erreur de connect.");
     		return 0;
	}

	freeaddrinfo(ressave);
	printf("Connexion avec le serveur reussie.\n");

	return 1;
}

/* Recoit un message envoye par le serveur.
 */
char *Reception() {
	char message[LONGUEUR_TAMPON];
	int index = 0;
	int fini = EVAL_FALSE;
	int retour = 0;
	while(!fini) {
		/* on cherche dans le tampon courant */
		while((finTampon > debutTampon) &&
			(tamponClient[debutTampon]!='\n')) {
			message[index++] = tamponClient[debutTampon++];
		}
		/* on a trouve ? */
		if ((index > 0) && (tamponClient[debutTampon]=='\n')) {
			message[index++] = '\n';
			message[index] = '\0';
			debutTampon++;
			fini = EVAL_TRUE;
			return strdup(message);
		} else {
			/* il faut en lire plus */
			debutTampon = 0;
			retour = recv(socketClient, tamponClient, LONGUEUR_TAMPON, 0);
			if (retour < 0) {
				perror("Reception, erreur de recv.");
				return NULL;
			} else if(retour == 0) {
				fprintf(stderr, "Reception, le serveur a ferme la connexion.\n");
				return NULL;
			} else {
				/*
				 * on a recu "retour" octets
				 */
				finTampon = retour;
			}
		}
	}
	return NULL;
}

/* Envoie un message au serveur.
 * Attention, le message doit etre termine par \n
 */
int Emission(char *message) {
	if(strstr(message, "\n") == NULL) {
		fprintf(stderr, "Emission, Le message n'est pas termine par \\n.\n");
	}
	int taille = strlen(message);
	if (send(socketClient, message, taille,0) == -1) {
        perror("Emission, probleme lors du send.");
        return 0;
	}
	printf("Emission de %d caracteres.\n", taille+1);
	return 1;
}

/* Recoit des donnees envoyees par le serveur.
 */
int ReceptionBinaire(char *donnees, size_t tailleMax) {
	int dejaRecu = 0;
	int retour = 0;
	/* on commence par recopier tout ce qui reste dans le tampon
	 */
	while((finTampon > debutTampon) && (dejaRecu < tailleMax)) {
		donnees[dejaRecu] = tamponClient[debutTampon];
		dejaRecu++;
		debutTampon++;
	}
	/* si on n'est pas arrive au max
	 * on essaie de recevoir plus de donnees
	 */
	if(dejaRecu < tailleMax) {
		retour = recv(socketClient, donnees + dejaRecu, tailleMax - dejaRecu, 0);
		if(retour < 0) {
			perror("ReceptionBinaire, erreur de recv.");
			return -1;
		} else if(retour == 0) {
			fprintf(stderr, "ReceptionBinaire, le serveur a ferme la connexion.\n");
			return 0;
		} else {
			/*
			 * on a recu "retour" octets en plus
			 */
			return dejaRecu + retour;
		}
	} else {
		return dejaRecu;
	}
}

/* Envoie des donn�es au serveur en pr�cisant leur taille.
 */
int EmissionBinaire(char *donnees, size_t taille) {
	int retour = 0;
	retour = send(socketClient, donnees, taille, 0);
	if(retour == -1) {
		perror("Emission, probleme lors du send.");
		return -1;
	} else {
		return retour;
	}
}


/* Ferme la connexion.
 */
void Terminaison() {
	close(socketClient);
}

/* Pour vider le buffer */
void CleanBuffer(){
	/* Variables */
	int c = 0;

	/* Début */
	while(c != '\n' && c != EOF){
		c = getchar();
	}

}

/* Demande des identifiants utilisateur
 */
int DemandeID(char **username, char **password){
	/* Variables */
	char tmp_username[20];
	char tmp_password[20];
	int try = 0;		//variable de Test
	int taille_id;
	int taille_pass;

	/* Saisie du nom d'utilisateur */
	printf("Veuillez saisir vos identifiants de connexion : \n\n");
	printf("\tIdentifiant: ");
	try = scanf("%19s",&tmp_username );
	if (!try ) {
		puts("Erreur lors de la saisie de l'identifiant.");
		return EXIT_FAILURE;
	}
	CleanBuffer();
	/* Saisie du mot de passe */
	printf("\tMot de passe: ");
	try = scanf("%20s",&tmp_password );

	CleanBuffer();

	/* Lien des identifiants de connexion avec les var tmp*/
	taille_id = strlen(tmp_username);
	taille_pass = strlen(tmp_password);
	*username = malloc(taille_id * sizeof(char)+1);
	*password = malloc(taille_pass * sizeof(char)+1);
	if(strcpy(*username, tmp_username) == NULL){
		puts("client.c: Erreur lors de la copie de l'identifiant.");
		return EXIT_FAILURE;
	}
	if(strcpy(*password, tmp_password) == NULL){
		puts("client.c: Erreur lors de la copie du mot de passe.");
		return EXIT_FAILURE;
	}
	//*password = getPass("Mot de passe: ");

	return EXIT_OK;
}

void Welcome() {
	system("clear");
	printf("**************************************************************\n");
	printf("**************************************************************\n\n");
	printf("\tCLIENT DE MESSAGERIE BMP (Basic Mail Protocol)\n\n");
	printf("**************************************************************\n");
	printf("**************************************************************	\n");
}

int ChoiceScreen(char *username, char *sessionid){
	/* Variables */
	int choice;						//Choix de l'utilisateur

	/* Début */
	//system("clear");
	Welcome();
	printf("\nBienvenue %s (%s), voici les actions possibles :\n", username, sessionid);
	printf("\t1° LIRE les nouveaux messages.\n");
	printf("\t2° ENVOYER un message.\n");
	printf("\t3° DECONNECTER la session.\n\n");
	printf("\tVeuillez choisir une action :");
	scanf("%d",&choice);

	switch(choice){
		case 1:

			break;
		case 3:
			if(Logout(sessionid) == 1){
				puts("Erreur lors de la déconnexion.");
				ChoiceScreen(username, sessionid);
			}
			return EXIT_OK;
			break;
		default:
			ChoiceScreen(username,sessionid);
	}
}

/* get_word() met dans la variable mot le number_word-ieme mot de texte et renvoie la taille de ceci de celui-ci */
/* /!\ mot doit être déclaré sous la forme mot[int] avant l'appel de la fonction*/
int get_word(char *texte, char *mot, int number_word){

	/* varibles */
	int nbr_char = 0; /*nombre de caractere actuellement parcourru*/
	char char_crt = 0; /*valeur du caractere courant*/
	int stop = 0; /*variable booléenne servant à stopper la boucle de parcours*/
	int nb_word = number_word; /*permet de compter le nombre de mot*/
	int ind_word = 0; /*indice du mot, permettant l'incrémentation du mot renvoyé*/

	if(texte != NULL){ /*si le texte n'est pas vide : traitement et parcours*/
		do { /*parcours du texte*/
			char_crt = texte[nbr_char]; /*acquisition de la lettre courante*/
			nbr_char++;

			if(nb_word == 0 && (char_crt == '\n' || char_crt == ' ' || char_crt == '\0')){ /*si nous sommes en cours d'acquisition mais qu'on a un espace, un retour à la ligne ou la fin du texte*/
				mot[ind_word] = '\0'; /*on ajoute à mot un EOF*/
				stop = 1;
			}
			else if(nb_word == 0){ /*si on recherche un mot*/
				mot[ind_word] = char_crt; /*acquisition de la lettre courante et incrémentation de ind_word*/
				ind_word++;
			}
			else if(char_crt == '\n' || char_crt == ' '){ /*Si on a un espace ou un retour à la ligne*/
				nb_word--; /*on passe à un autre mot*/
			}

			if(char_crt == '\0') /*si le texte est fini*/
				stop = 1; /*On stoppe la boucle*/

		} while(!stop);

		/*On retourne la taille du mot*/
		return ind_word;
	}
	else /*sinon on retourne 0*/
		return 0;

}

/*cmp_word(char *wd1, char *wd2, lg) sert à comparer deux mots (wd1 et wd2) : il renvoie 1 en cas de correspondance, 0 si les mots sont différents, lg est la longueur des mots*/
int cmp_word(char *wd1, char *wd2, int lg){

	int retour = 1;
	int i = 0;

	/*printf("CMP : %s et %s\n", wd1, wd2);*/

	do {
		if(wd1[i] != wd2[i]) {
			retour =  0; /*si deux lettres sont différentes*/
		}

		i+=1; /*incrémentation du i - parcours*/
	} while(i < lg && retour); /*On continue tant que i < lg (parcours) et que retour == 1, cad qu'aucune différence n'a encore été trouvée*/
	return retour;

}


int Logout(char *sessionid){
	char requete[1024] = {0};

	system("clear");
	printf("Fin de connexion, à bientôt ...\n");
	strcat(requete,"DISCONNECT ");
	strcat(requete, sessionid);
	Emission(requete);
	Terminaison();
	return EXIT_OK;
}
