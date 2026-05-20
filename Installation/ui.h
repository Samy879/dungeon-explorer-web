#ifndef UI_H
#define UI_H

#include "donjon.h"
#include "joueur.h"

/* Type public (pointeur vers structure privée) */
typedef struct sUI *tUI;

/* Création de l'interface utilisateur (UI) */
tUI UI_Creer(void);

/* Destruction de l'interface utilisateur */
void UI_Liberer(tUI *pui);

/* Positionne le message qui doit être affiché sur l'interface */
void UI_DefinirMessage(tUI ui, const char *fmt, ...);

/* Affichage de l'interface utilisateur */
void UI_Afficher(tUI ui, tDonjon d, tJoueur j);

/* Lecture d'une ligne saisie par l'utilisateur (une commande à exécuter)
 * La fonction alloue dynamiquement la ligne lue (sans le '\n'),
 * il faudra donc la libérer (free) après utilisation. */
char *UI_LireLigne(tUI ui);

#endif
