/*####################################################################
#		SMP Messenger Client																						 #
#--------------------------------------------------------------------#
#		@Authors : Alexis GARDAVOIR && Samuel GARCIA										 #
#--------------------------------------------------------------------#
#		Définition des headers :: client.h  														 #
####################################################################*/

#include <stddef.h>
#ifndef __CLIENT_H__
#define __CLIENT_H__

/* Initialisation.
 * Connexion au serveur sur la machine donnee.
 * Utilisez localhost pour un fonctionnement local.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int Initialisation(char *machine);

/* Initialisation.
 * Connexion au serveur sur la machine donnee et au service donne.
 * Utilisez localhost pour un fonctionnement local.
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int InitialisationAvecService(char *machine, char *service);

/* Recoit un message envoye par le serveur.
 * retourne le message ou NULL en cas d'erreur.
 * Note : il faut liberer la memoire apres traitement.
 */
char *Reception();

/* Envoie un message au serveur.
 * Attention, le message doit etre termine par \n
 * renvoie 1 si �a c'est bien pass� 0 sinon
 */
int Emission(char *message);

/* Recoit des donnees envoyees par le serveur.
 * renvoie le nombre d'octets re�us, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int ReceptionBinaire(char *donnees, size_t tailleMax);

/* Envoie des donn�es au serveur en pr�cisant leur taille.
 * renvoie le nombre d'octets envoy�s, 0 si la connexion est ferm�e,
 * un nombre n�gatif en cas d'erreur
 */
int EmissionBinaire(char *donnees, size_t taille);

/* Ferme la connexion.
 */
void Terminaison();

/* Demande les identifiants de l'utilisateur.
 */
int DemandeID(char **username, char **password);

/* Pour vider le buffer
 */
void CleanBuffer();

/*  Interface utilisateur : Menu de connexion
 */
void Welcome();

/*  Interface utilisateur : Ecran de choix
 */
int ChoiceScreen(char *username, char *sessionid);

/* get_word() met dans la variable mot le number_word-ieme mot de texte et renvoie la taille de ceci de celui-ci */
/* /!\ mot doit être déclaré sous la forme mot[int] avant l'appel de la fonction*/
int get_word(char *texte, char *mot, int number_word);

/* Deconnexion de l'utilisateur
 */
int Logout();

void connectUser();

int writeMessage();

#endif
