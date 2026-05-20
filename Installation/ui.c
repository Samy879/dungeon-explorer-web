#include "ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define RED     "[[R]]"
#define BLUE    "[[B]]"
#define CYAN    "[[C]]" // Correspond au gris #666 dans le CSS
#define RESET   "[[X]]"
#else
#define RED     "\x1b[31m"
#define BLUE    "\x1b[34m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#endif

/* ==============================
 *  Structure privée
 *  ============================== */
struct sUI { char message[256]; };

/* ==============================
 *  Création UI
 *  ============================== */
tUI UI_Creer(void) {
    tUI ui = malloc(sizeof(struct sUI));
    if (!ui) return NULL;
    ui->message[0] = '\0';
    return ui;
}

/* ==============================
 *  Destruction UI
 *  ============================== */
void UI_Liberer(tUI *pui) {
    if (pui && *pui) {
        free(*pui);
        *pui = NULL;
    }
}

/* ==============================
 *  Message
 *  ============================== */
void UI_DefinirMessage(tUI ui, const char *fmt, ...) {
    if (!ui || !fmt) return;
    va_list ap; va_start(ap, fmt);
    vsnprintf(ui->message, sizeof(ui->message), fmt, ap);
    va_end(ap);
}

/* ---- Affichage mini carte ---- */
static void render_map(tDonjon d, tJoueur j) {
    int w = DonjonW(d), h = DonjonH(d);
    int jx, jy; JoueurPosition(j, &jx, &jy);

    printf("%s+", CYAN);
    for (int x = 0; x < w; x++) printf("-");
    printf("+%s\n", RESET);

    for (int y = 0; y < h; y++) {
        printf("%s|%s", CYAN, RESET);
        for (int x = 0; x < w; x++) {
            tSalle s = DonjonSalle(d, x, y);
            char c = '?';

            if (jx == x && jy == y) c = '@';
            else if (!s || !SalleExiste(s)) c = '#';
            else if (!SalleEstVisitee(s)) c = '?';
            else if (SalleEnnemi(s) && EnnemiEstVivant(SalleEnnemi(s))) c = 'E';
            else c = '.';

            if (c == '@' || c == 'E') printf("%s%c%s", RED, c, RESET);
            else if (c == '#') printf("%s%c%s", BLUE, c, RESET);
            else printf("%c", c);
        }
        printf("%s|%s\n", CYAN, RESET);
    }

    printf("%s+", CYAN);
    for (int x = 0; x < w; x++) printf("-");
    printf("+%s\n", RESET);
}

/* ---- Bloc salle ---- */
static void render_room(tDonjon d, tJoueur j) {
    int jx, jy;
    JoueurPosition(j, &jx, &jy);
    tSalle s = DonjonSalle(d, jx, jy);

    if (!s) {
        fprintf(stderr, "ERREUR: impossible de trouver la salle dans laquelle est le joueur.\n");
        return;
    }

    int id = jy * DonjonW(d) + jx;

    printf("ROOM [id=%d] (x=%d y=%d)\n", id, jx, jy);
    printf("Desc: %s\n", s ? SalleDescription(s) : "");

    // Calcul et affichage des sorties (Exits)
    printf("Exits:");
    int any = 0;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    const char *lab[4] = {" N", " E", " S", " W"};
    for (int k = 0; k < 4; k++) {
        tSalle v = DonjonSalle(d, jx + dx[k], jy + dy[k]);
        if (v && SalleExiste(v)) {
            printf("%s", lab[k]);
            any = 1;
        }
    }
    if (!any) printf(" (none)");
    printf("\n");

    // Items (Gris discret si vide pour le rendu web)
    char *txt = InventaireVersChaine(SalleObjets(s));
    if (txt && strlen(txt) > 0 && strcmp(txt, "vide") != 0 && strcmp(txt, "aucun") != 0) {
        printf("Items: %s\n", txt);
    } else {
        printf("Items: %svide%s\n", CYAN, RESET);
    }
    if (txt) free(txt);

    // Ennemi (Rouge si vivant, gris si aucun)
    tEnnemi e = SalleEnnemi(s);
    if (e && EnnemiEstVivant(e)) {
        printf("Enemy: %s%s%s (pv=%d/%d, atk=%d, def=%d)\n",
               RED, EnnemiNom(e), RESET,
               EnnemiPV(e), EnnemiPVMax(e),
               EnnemiAttaque(e), EnnemiDefense(e));
    } else {
        printf("Enemy: %saucun%s\n", CYAN, RESET);
    }
}

/* ---- Bloc joueur ---- */
static void render_player(const tUI ui, const tJoueur j) {
    printf("PLAYER pv=%d/%d atk=%d def=%d\n",
           JoueurPV(j), JoueurPVMax(j), JoueurAttaque(j), JoueurDefense(j));

    // Inventaire (Gris discret si vide)
    char *inv = InventaireVersChaine(JoueurInventaire(j));
    if (inv && strlen(inv) > 0 && strcmp(inv, "vide") != 0 && strcmp(inv, "aucun") != 0) {
        printf("Inv: %s\n", inv);
    } else {
        printf("Inv: %svide%s\n", CYAN, RESET);
    }
    if (inv) free(inv);

    printf("Msg: %s\n", ui->message[0] ? ui->message : "(none)");
}

/* ==============================
 *  Affichage global
 *  ============================== */
void UI_Afficher(tUI ui, tDonjon d, tJoueur j) {
    printf("[[CLR]]\n");

    // 1. Panneau CARTE
    printf("[[MAP]]\n");
    render_map(d, j);

    // 2. Panneau SALLE
    printf("[[ROOM]]\n");
    render_room(d, j);

    // 3. Panneau JOUEUR
    printf("[[PLAYER]]\n");
    render_player(ui, j);

    fflush(stdout);
}

/* ==============================
 *  Lecture ligne
 *  ============================== */
char *UI_LireLigne(tUI ui) {
    char buf[256];

    #ifdef __EMSCRIPTEN__
    while (emscripten_run_script_int("stdinBuffer.length") == 0) emscripten_sleep(50);
    #endif

    if (!fgets(buf, sizeof(buf), stdin)) return NULL;

    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = '\0';

    char *s = malloc(strlen(buf) + 1);
    if (s) strcpy(s, buf);
    return s;
}
