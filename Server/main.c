#include <stdio.h>
#include <stdlib.h>
#include "server.h"

int main() {
	char *message = NULL;

	/*MARQ_ESSAI*/
	message = "CONNECT Alexis 0001";

	Initialisation();

	while(1) {
		int fini = 0;

		/*MARQ_ESSAIAttenteClient();*/

		while(!fini) {
			/*MARQ_ESSAI message = Reception();*/

			if(message != NULL) {
				printf("J'ai recu: %s\n", message);

				/* Le Serveur traite le message reçu */
				traitement(message);

				/* Le Serveur détruit le message */
				/*MARQ_ESSAI free(message);*/

				if(Emission("Test de message serveur.\n")!=1) {
					printf("Erreur d'emission\n");
				}
			} else {
				fini = 1;
			}
		}

		TerminaisonClient();
	}

	return 0;
}

