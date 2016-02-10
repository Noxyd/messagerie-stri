#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <errno.h>

#include "server.h"

/*permet d'obtenir la date*/
#include <time.h>

#define TRUE 1
#define FALSE 0
#define LONGUEUR_TAMPON 4096

#define LIMIT_CHAR 20


#ifdef WIN32
#define perror(x) printf("%s : code d'erreur : %d\n", (x), WSAGetLastError())
#define close closesocket
#define socklen_t int
#endif



/* Variables cachees */

/* le socket d'ecoute */
int socketEcoute;
/* longueur de l'adresse */
socklen_t longeurAdr;
/* le socket de service */
int socketService;
/* le tampon de reception */
char tamponClient[LONGUEUR_TAMPON];
int debutTampon;
int finTampon;


/* Initialisation.
 * Creation du serveur.
 */
int Initialisation() {
	return InitialisationAvecService("13214");
}

/* Initialisation.
 * Creation du serveur en pr�cisant le service ou num�ro de port.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int InitialisationAvecService(char *service) {
	int n;
	const int on = 1;
	struct addrinfo	hints, *res, *ressave;

	#ifdef WIN32
	WSADATA	wsaData;
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR)
	{
		printf("WSAStartup() n'a pas fonctionne, erreur : %d\n", WSAGetLastError()) ;
		WSACleanup();
		exit(1);
	}
	memset(&hints, 0, sizeof(struct addrinfo));
    #else
	bzero(&hints, sizeof(struct addrinfo));
	#endif

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(NULL, service, &hints, &res)) != 0)  {
     		printf("Initialisation, erreur de getaddrinfo : %s", gai_strerror(n));
     		return 0;
	}
	ressave = res;

	do {
		socketEcoute = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (socketEcoute < 0)
			continue;		/* error, try next one */

		setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
#ifdef BSD
		setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
#endif
		if (bind(socketEcoute, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		close(socketEcoute);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
     		perror("Initialisation, erreur de bind.");
     		return 0;
	}

	/* conserve la longueur de l'addresse */
	longeurAdr = res->ai_addrlen;

	freeaddrinfo(ressave);
	/* attends au max 4 clients */
	listen(socketEcoute, 4);
	printf("Creation du serveur reussie sur %s.\n", service);

	return 1;
}

/* Attends qu'un client se connecte.
 */
int AttenteClient() {
	struct sockaddr *clientAddr;
	char machine[NI_MAXHOST];

	clientAddr = (struct sockaddr*) malloc(longeurAdr);
	socketService = accept(socketEcoute, clientAddr, &longeurAdr);
	if (socketService == -1) {
		perror("AttenteClient, erreur de accept.");
		return 0;
	}
	if(getnameinfo(clientAddr, longeurAdr, machine, NI_MAXHOST, NULL, 0, 0) == 0) {
		printf("Client sur la machine d'adresse %s connecte.\n", machine);
	} else {
		printf("Client anonyme connecte.\n");
	}
	free(clientAddr);
	/*
	 * Reinit buffer
	 */
	debutTampon = 0;
	finTampon = 0;

	return 1;
}

/* Recoit un message envoye par le serveur.
 */
char *Reception() {
	char message[LONGUEUR_TAMPON];
	int index = 0;
	int fini = FALSE;
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
			fini = TRUE;
#ifdef WIN32
			return _strdup(message);
#else
			return strdup(message);
#endif
		} else {
			/* il faut en lire plus */
			debutTampon = 0;
			retour = recv(socketService, tamponClient, LONGUEUR_TAMPON, 0);
			if (retour < 0) {
				perror("Reception, erreur de recv.");
				return NULL;
			} else if(retour == 0) {
				fprintf(stderr, "Reception, le client a ferme la connexion.\n");
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

/* Envoie un message au client.
 * Attention, le message doit etre termine par \n
 */
int Emission(char *message) {
	int taille;
	if(strstr(message, "\n") == NULL) {
		fprintf(stderr, "Emission, Le message n'est pas termine par \\n.\n");
		return 0;
	}
	taille = strlen(message);
	if (send(socketService, message, taille,0) == -1) {
        perror("Emission, probleme lors du send.");
        return 0;
	}
	printf("Emission de %d caracteres.\n", taille+1);
	return 1;
}


/* Recoit des donnees envoyees par le client.
 */
int ReceptionBinaire(char *donnees, size_t tailleMax) {
	size_t dejaRecu = 0;
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
		retour = recv(socketService, donnees + dejaRecu, tailleMax - dejaRecu, 0);
		if(retour < 0) {
			perror("ReceptionBinaire, erreur de recv.");
			return -1;
		} else if(retour == 0) {
			fprintf(stderr, "ReceptionBinaire, le client a ferme la connexion.\n");
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

/* Envoie des donn�es au client en pr�cisant leur taille.
 */
int EmissionBinaire(char *donnees, size_t taille) {
	int retour = 0;
	retour = send(socketService, donnees, taille, 0);
	if(retour == -1) {
		perror("Emission, probleme lors du send.");
		return -1;
	} else {
		return retour;
	}
}



/* Ferme la connexion avec le client.
 */
void TerminaisonClient() {
	close(socketService);
}

/* Arrete le serveur.
 */
void Terminaison() {
	close(socketEcoute);
}

/* 
	FONCTIONS TIERCES
*/


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

/*
	Fonctions relatives au traitement d'un message
	à part la derniere fonction (traitement()), les autres
	ne sont pas mentionnées dans le .h car elles ne sont
	accessible qu'en passant par traitement()
*/ 

/*verifie qu'un ID_SESSION n'est pas déjà présent, renvoie 1 si il n'y a pas d'occurence de id dans le fichier*/
int ajouter_ID_SESSION_LIST(char *id){
	int retour = 1;

	FILE* liste = NULL; /*pointeur sur le fichier*/
	char ligne[LIMIT_CHAR] = ""; /*chaine de char accueillant la ligne courante dans le fichier*/

	char wd_in_list[LIMIT_CHAR] = ""; /*wd contenu dans la ligne courante*/
	int lg_wd_list = 0; /*longueur de l'wd contenu dans la ligne courante*/

	int OK = 0; /*variable booléenne permettant à la boucle de s'arrêter une fois l'id trouvé*/

	/*verification dans le fichier*/
	liste = fopen("liste_session", "r"); /*ouverture du fichier*/

	printf("CONNECT : Tentative d'ouverture du fichier liste_session.\n");

	if(liste != NULL){ /* si l'ouverture s'est correctement passée*/
		printf("CONNECT : ouverture du fichier liste_session réussie.\n");
		while(!OK && fgets(ligne, LIMIT_CHAR, liste) != NULL){ /* parcours du fichier liste*/
			lg_wd_list = get_word(ligne, wd_in_list, 0);			

			if(cmp_word(id, wd_in_list, lg_wd_list)){/*Si les id correspondent*/
				OK = 1;
				printf("CONNECT : une session portant le meme identifiant est deja ouverte.\n");
			}

		} /*fin du parcours*/

		fclose(liste);
		
		if(OK){ /*si un identifiant a été trouvée*/
			retour = 0;
		}

	}
	else{
		printf("CONNECT : ERREUR, incapable de lire liste_session.\n"); /*ERREUR lors de l'ouverture du fichier - ERR INTERNAL*/
		Emission("ERR INTERNAL unable_to_find_fic\n");
	}

	return retour;
}

/*Ajoute dans la liste des sessions un numéro de session, selon le numéro de ligne donné*/
int ajouter_ID_SESSION(char *id_user, int numero_ligne){
	FILE* liste = NULL; /*pointeur sur le fichier*/
	
	time_t timestamp = time(NULL); /*récupération du timestamp*/
	struct tm * t;

	t = localtime(&timestamp); /*récupération de la date actuelle*/
	
	/*verification dans le fichier*/
	liste = fopen("liste_session", "a"); /*ouverture du fichier*/

	char message[50] = "";

	/*creation id_session*/
	char id_session[8] = "";

	char id_1[2];
	char id_2[2];
	char id_3[2];
	char id_4[2];
	
	if(t->tm_sec < 10) /*récupération et formatage des secondes*/
		sprintf(id_4, "0%d", t->tm_sec);
	else
		sprintf(id_4, "%d", t->tm_sec);

	if(t->tm_min < 10) /*récupération et formatage des minutes*/
		sprintf(id_3, "0%d", t->tm_min);
	else
		sprintf(id_3, "%d", t->tm_min);

	if(t->tm_hour < 10) /*récupération et formatage des heures*/
		sprintf(id_2, "0%d", t->tm_hour);
	else
		sprintf(id_2, "%d", t->tm_hour);


	if(numero_ligne < 10) /* si le numéro de ligne < 10 */
		sprintf(id_1, "0%d", numero_ligne); /*on ajoute un zéro, décalage*/
	else /*sinon on laisse comme tel*/
		sprintf(id_1, "%d", numero_ligne);
 
	/*concatenation des 4 partie)*/
	strcat(id_1, id_2);
	strcat(id_3, id_4);
	strcat(id_1, id_3);

	strcat(id_session, id_1);

	printf("CONNECT : Creation d'un identifiant de session (%s)\n", id_session);


	/*on verifie si l'id_session existe deja*/
	if(ajouter_ID_SESSION_LIST(id_session)){
		/*verification dans le fichier*/
		liste = fopen("liste_session", "a"); /*ouverture du fichier*/
		if(liste != NULL){ /* si l'ouverture s'est correctement passée*/
			fputs(id_session, liste);
			fputc(' ', liste);
			fputs(id_user, liste);
			fputc('\n', liste);
		
			printf("CONNECT : Ajout de %s dans le fichier liste_session.\n", id_session);

			fclose(liste);
			strcat(message, "CONNECT ALLOW ");
			strcat(message, id_session);
			strcat(message, "\n");

			Emission(message);
		}
		else
			Emission("ERR INTERNAL unable_to_write_fic\n");
	}
	else {
		Emission("CONNECT DENY 1\n");
		printf("CONNECT : ERREUR Session deja presente.\n");	
	}
}

/* fonction utilisée dans traitement_CONNECT, retourne 1 si l'identifiant et le mot de passe correspondent*/
int traitement_CONNECT_LIST(char *id, char *pwd){
	FILE* liste = NULL; /*pointeur sur le fichier*/
	char ligne[LIMIT_CHAR*2+1] = ""; /*chaine de char accueillant la ligne courante dans le fichier*/

	int numero_ligne = 0;

	char wd_in_list[LIMIT_CHAR] = ""; /*wd contenu dans la ligne courante*/
	int lg_wd_list = 0; /*longueur de l'wd contenu dans la ligne courante*/

	int OK = 0; /*variable booléenne permettant à la boucle de s'arrêter une fois l'id trouvé*/

	/*verification dans le fichier*/
	liste = fopen("liste_id", "r"); /*ouverture du fichier*/

	if(liste != NULL){ /* si l'ouverture s'est correctement passée*/
		while(!OK && fgets(ligne, LIMIT_CHAR*2+1, liste) != NULL){ /* parcours du fichier liste*/
			lg_wd_list = get_word(ligne, wd_in_list, 0);			
			
			numero_ligne+=1; /*incrementation du numero de ligne*/

			if(cmp_word(id, wd_in_list, lg_wd_list)){/*Si les id correspondent*/
				OK = 1;
				printf("CONNECT : Utilisateur trouve.\n");			
			}

		} /*fin du parcours*/

		fclose(liste);
		
		if(OK){ /*si un identifiant a été trouvée*/
			lg_wd_list = get_word(ligne, wd_in_list, 1); /*récupération du mot de passe associé*/

			if(cmp_word(pwd, wd_in_list, lg_wd_list)){/*Si les pwd correspondent*/
				/*creer un numero de session et envoyer*/	
				printf("CONNECT : Correspondante du mot de passe.\n");	
				ajouter_ID_SESSION(id, numero_ligne);
			}
			else{
				Emission("CONNECT DENY 1\n");/*refus de la connection - pwd introuvable*/
				OK = 0;
			}
		}			
		else
			Emission("CONNECT DENY 1\n");/*refus de la connection - id introuvable*/

	}
	else{
		printf("CONNECT : LISTE, fichier inexistant\n"); /*ERREUR lors de l'ouverture du fichier - ERR INTERNAL*/
		Emission("ERR INTERNAL unable_to_find_fic\n");
	}
	
	return OK;

}

int traitement_CONNECT(char *message){
	char id[LIMIT_CHAR]; /*identifiant*/
	char pwd[LIMIT_CHAR]; /*mot de passe*/
	

	if(get_word(message, id, 1) && get_word(message, pwd, 2)){ /*vérification de l'existance et acquisition de l'identifiant et du mot de passe*/
		printf("CONNECT : REQUEST OK\n");
		Emission("SEND OK 0\n");
		traitement_CONNECT_LIST(id, pwd);
	}
	else
		Emission("SEND ERROR 0001\nMauvais nombre de param. dans CONNECT\n");
				
	printf("CONNECT : %s with %s\n", id, pwd);
	return 0;
}

/*fonction permettant l'envoie d'une liste de mail*/
int send_mail_LIST(char *id){
	FILE* liste = NULL; /*pointeur sur le fichier*/
	char ligne[LIMIT_CHAR*2+1] = ""; /*chaine de char accueillant la ligne courante dans le fichier*/
	
	char wd_in_list[LIMIT_CHAR] = ""; /*wd contenu dans la ligne courante*/
	int lg_wd_list = 0; /*longueur de l'wd contenu dans la ligne courante*/

	char data[LIMIT_CHAR*4+3] = ""; /*va accueillir les 4 lignes à envoyer*/

	int numero_ligne = 0;

	char id_mail[LIMIT_CHAR]; /*premier mot*/
	char user_from[LIMIT_CHAR]; /* nom de l'expéditeur (3e mot)*/
	
	int OK = 0;

	/*verification dans le fichier*/
	liste = fopen("liste_message", "r"); /*ouverture du fichier*/

	if(liste != NULL){ /* si l'ouverture s'est correctement passée*/
		while(!OK && fgets(ligne, LIMIT_CHAR*2+1, liste) != NULL){ /* parcours du fichier liste*/

			lg_wd_list = get_word(ligne, id_mail, 0); /*possible id_mail à envoyer*/
			numero_ligne+=1; /*incrementation du numero de ligne*/
	

			fgets(ligne, LIMIT_CHAR*2+1, liste); /*récupération seconde ligne*/
			numero_ligne+=1; /*incrementation du numero de ligne*/

			lg_wd_list = get_word(ligne, user_from, 1); /*recuperation nom de l'expediteur*/

			fgets(ligne, LIMIT_CHAR*2+1, liste); /*récupération troisieme ligne*/
			numero_ligne+=1; /*incrementation du numero de ligne*/

			lg_wd_list = get_word(ligne, wd_in_list, 1); /*recuperation nom du destinataire*/

			if(cmp_word(id, wd_in_list, lg_wd_list)){/*Si les id correspondent*/
				/*si les id correspondent : on envoie 4 lignes contenant : id_mail/nFrom/nTo/nObj/n*/
				printf("HAVE : Mail trouve.\n");

				strcat(data, id_mail); /*ajout de l'id mail*/
				strcat(data, "\n");
				strcat(data, "From: "); /*ajout du From:*/
				strcat(data, user_from); /*ajout du user_From:*/
				strcat(data, "\n");
				
				strcat(data, ligne); /*ajout du To:*/

				fgets(ligne, LIMIT_CHAR*2+1, liste); /*ligne suivante*/
				numero_ligne+=1;
				strcat(data, ligne); /*ajout du Obj:*/
				
				/*MARQUEUR ICI*/
			
								
				
			}

		} /*fin du parcours*/

		fclose(liste);
		
	}
	else{
		printf("HAVE : LISTE, fichier inexistant\n"); /*ERREUR lors de l'ouverture du fichier - ERR INTERNAL*/
		Emission("ERR INTERNAL unable_to_find_fic\n");
	}

	return OK;
	

	
}

/* fonction utilisée dans traitement_HAVE, retourne 1 si l'identifiant de session se trouve dans le fichier*/
int traitement_HAVE_LIST(char *id, char *arg){
	FILE* liste = NULL; /*pointeur sur le fichier*/
	char ligne[LIMIT_CHAR*2+1] = ""; /*chaine de char accueillant la ligne courante dans le fichier*/

	char id_nom[LIMIT_CHAR] = ""; /*variable accueillant le nom de l'utilisateur de la session*/

	int numero_ligne = 0;

	char wd_in_list[LIMIT_CHAR] = ""; /*wd contenu dans la ligne courante*/
	int lg_wd_list = 0; /*longueur de l'wd contenu dans la ligne courante*/

	int OK = 0; /*variable booléenne permettant à la boucle de s'arrêter une fois l'id trouvé*/

	/*verification dans le fichier*/
	liste = fopen("liste_session", "r"); /*ouverture du fichier*/

	if(liste != NULL){ /* si l'ouverture s'est correctement passée*/
		while(!OK && fgets(ligne, LIMIT_CHAR*2+1, liste) != NULL){ /* parcours du fichier liste*/
			lg_wd_list = get_word(ligne, wd_in_list, 0);			
			
			numero_ligne+=1; /*incrementation du numero de ligne*/

			if(cmp_word(id, wd_in_list, lg_wd_list)){/*Si les id correspondent*/
				OK = 1;
				get_word(ligne, id_nom, 1); /*On récupère le nom de l'utilisateur*/
				printf("HAVE : ID de session trouve.\n");			
			}

		} /*fin du parcours*/

		fclose(liste);
		
		if(OK){ /*si un identifiant a été trouvée*/
			/*si un identifiant a été trouvée il faut soit :
			- si arg = 0 : envoie une liste de mail, la ligne de mail contient les quatres premieres lignes.
			- si arg != 0 : envoie un message précis dont l'ID est arg.*/
			 
			send_mail_LIST(id);
			

		}			
		else {
			Emission("CONNECT DENY 1\n");/*refus de la connection - id introuvable*/
			OK = 0;		
		}
	}
	else{
		printf("CONNECT : LISTE, fichier inexistant\n"); /*ERREUR lors de l'ouverture du fichier - ERR INTERNAL*/
		Emission("ERR INTERNAL unable_to_find_fic\n");
	}

	return OK;
}

int traitement_HAVE(char *message){
	char id[LIMIT_CHAR]; /*identifiant de session*/
	char arg[LIMIT_CHAR]; /*argument : doit être 0*/
	

	if(get_word(message, id, 1) && get_word(message, arg, 2)){ /*vérification de l'existance et acquisition de l'identifiant et du mot de passe*/
		printf("HAVE : REQUEST OK\n");
		Emission("SEND OK 0\n");
		traitement_HAVE_LIST(id, arg);
	}
	else
		Emission("SEND ERROR 0001\nMauvais nombre de param. dans HAVE\n");
				
	printf("HAVE : %s ask for messages.\n", id);
	return 0;
}

int traitement_SEND(char *message){
	printf("SEND\n");
	return 0;
}

int traitement_DISCONNECT(char *message){
	printf("DISCONNECT\n");
	return 0;
}

/* 
	La fonction int traitement(char *message) 
	s'occupe de la gestion du message reçu.
*/

int traitement(char *message){

	int retour;

	/*procédure composée de plusieures phase, p1 = phase 1, p2 = phase2 etc..*/
	
	char keyword[LIMIT_CHAR]; /*p1 - le mot recherché*/
	int lg_keyword; /*la longueur du mot recherché*/


	/*Phase 1 : comparer le premier mot*/
	lg_keyword = get_word(message, keyword, 0); /*récupération du 1er mot et de sa taille*/
	

	if(cmp_word(keyword, "CONNECT", lg_keyword)) /*traitement CONNECT*/
		retour = traitement_CONNECT(message);

	else if(cmp_word(keyword, "HAVE", lg_keyword)) /*traitement HAVE*/
		retour = traitement_HAVE(message);

	else if(cmp_word(keyword, "SEND", lg_keyword)) /*traitement SEND*/
		retour = traitement_SEND(message);

	else if(cmp_word(keyword, "DISCONNECT", lg_keyword)) /*traitement DISCONNECT*/
		retour = traitement_DISCONNECT(message);
	
	/*else Erreur : requete inconnue*/	
		
	getchar();
	return retour;
}

