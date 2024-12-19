#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

// Dimensions de la grille
int largeur = 20, hauteur = 20;

// Coordonnées et état du jeu
int x, y, fruitX, fruitY, score, gameOver, direction;
int longueurQueue = 0;
int queueX[100], queueY[100];
int vitesse = 100;
int pause = 0;
int sauvegardeExiste = 0;

// Variables pour la sauvegarde
int xSave, ySave, directionSave;
int queueXSave[100], queueYSave[100];
int longueurQueueSave, scoreSave;

// Configuration du terminal
void configTerminal(int enable) {
    static struct termios oldt, newt;
    if (enable == 0) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

// Initialisation du jeu
void setup() {
    gameOver = 0;
    direction = 0;
    x = largeur / 2;
    y = hauteur / 2;
    fruitX = rand() % largeur;
    fruitY = rand() % hauteur;
    score = 0;
    longueurQueue = 0;
    sauvegardeExiste = 0;
}

// Affichage du jeu avec interface améliorée
void draw() {
    system("clear"); // Effacer l'écran

    // Bordure supérieure
    for (int i = 0; i < largeur + 2; i++)
        printf("=");
    printf("\n");

    // Grille avec serpent et fruit
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            if (j == 0)
                printf("|"); // Bordure gauche

            if (i == y && j == x)
                printf("O"); // Tête du serpent
            else if (i == fruitY && j == fruitX)
                printf(" "); // Fruit
            else {
                int afficherQueue = 0;
                for (int k = 0; k < longueurQueue; k++) {
                    if (queueX[k] == j && queueY[k] == i) {
                        printf("o"); // Corps du serpent
                        afficherQueue = 1;
                    }
                }
                if (!afficherQueue)
                    printf(" ");
            }

            if (j == largeur - 1)
                printf("|"); // Bordure droite
        }
        printf("\n");
    }

    // Bordure inférieure
    for (int i = 0; i < largeur + 2; i++)
        printf("=");
    printf("\n");

    // Affichage du score et de la longueur
    printf("Score: %d | Longueur: %d\n", score, longueurQueue);
    printf("Contrôles : 'q' = Gauche, 'd' = Droite, 'z' = Haut, 's' = Bas\n");
    printf("Appuyez sur 'p' pour Pause/Reprise, 'r' pour Revenir au point de sauvegarde\n");
    printf("Appuyez sur 'x' pour quitter\n");
}

// Capture des touches du joueur
void input() {
    char touche;
    if (read(STDIN_FILENO, &touche, 1) != -1) {
        switch (touche) {
            case 'q':
                direction = 1;
                break;
            case 'd':
                direction = 2;
                break;
            case 'z':
                direction = 3;
                break;
            case 's':
                direction = 4;
                break;
            case 'p':
                pause = !pause;
                if (pause) {
                    sauvegarderEtat();
                }
                break;
            case 'r':
                if (sauvegardeExiste) {
                    restaurerEtat();
                }
                break;
            case 'x':
                gameOver = 1;
                break;
        }
    }
}

// Sauvegarde de l'état du jeu
void sauvegarderEtat() {
    xSave = x;
    ySave = y;
    directionSave = direction;
    scoreSave = score;
    longueurQueueSave = longueurQueue;
    for (int i = 0; i < longueurQueue; i++) {
        queueXSave[i] = queueX[i];
        queueYSave[i] = queueY[i];
    }
    sauvegardeExiste = 1;
    printf("Jeu sauvegardé !\n");
}

// Restauration de l'état du jeu
void restaurerEtat() {
    x = xSave;
    y = ySave;
    direction = directionSave;
    score = scoreSave;
    longueurQueue = longueurQueueSave;
    for (int i = 0; i < longueurQueue; i++) {
        queueX[i] = queueXSave[i];
        queueY[i] = queueYSave[i];
    }
    gameOver = 0; // On réinitialise le statut du jeu
    pause = 0;    // On sort du mode pause
    sauvegardeExiste = 0; // On ne peut restaurer qu'une seule fois
    printf("Jeu restauré !\n");
}

// Logique du jeu
void logic() {
    if (!pause) {
        int prevX = queueX[0];
        int prevY = queueY[0];
        int prev2X, prev2Y;
        queueX[0] = x;
        queueY[0] = y;

        // Déplacement de la queue
        for (int i = 1; i < longueurQueue; i++) {
            prev2X = queueX[i];
            prev2Y = queueY[i];
            queueX[i] = prevX;
            queueY[i] = prevY;
            prevX = prev2X;
            prevY = prev2Y;
        }

        // Déplacer le serpent
        switch (direction) {
            case 1:
                x--;
                break;
            case 2:
                x++;
                break;
            case 3:
                y--;
                break;
            case 4:
                y++;
                break;
            default:
                break;
        }

        // Collision avec les murs
        if (x >= largeur) x = 0; else if (x < 0) x = largeur - 1;
        if (y >= hauteur) y = 0; else if (y < 0) y = hauteur - 1;

        // Collision avec la queue
        for (int i = 0; i < longueurQueue; i++) {
            if (queueX[i] == x && queueY[i] == y)
                gameOver = 1;
        }

        // Mange le fruit
        if (x == fruitX && y == fruitY) {
            score += 10;
            fruitX = rand() % largeur;
            fruitY = rand() % hauteur;
            if (score >= 30) { // Commence à grandir seulement après avoir mangé 3 pommes
                longueurQueue++;
            }
        }
    }
}

// Choix du niveau de difficulté
void choisirNiveau() {
    int choix;
    printf("Choisissez le niveau de difficulté:\n");
    printf("1. Facile (Vitesse lente)\n");
    printf("2. Moyen (Vitesse moyenne)\n");
    printf("3. Difficile (Vitesse rapide)\n");
    scanf("%d", &choix);

    switch (choix) {
        case 1:
            vitesse = 150;
            break;
        case 2:
            vitesse = 100;
            break;
        case 3:
            vitesse = 50;
            break;
        default:
            printf("Choix invalide, le niveau moyen sera sélectionné par défaut.\n");
            vitesse = 100;
            break;
    }
}

// Fonction principale
int main() {
    srand(time(NULL)); // Initialiser les nombres aléatoires

    choisirNiveau(); // Choix du niveau
    setup(); // Initialisation du jeu

    configTerminal(0); // Capture les touches sans affichage

    // Boucle du jeu
    while (!gameOver) {
        draw();   // Affiche la grille
        input();  // Capture des touches
        logic();  // Logique du jeu
        usleep(vitesse * 1000); // Contrôle de la vitesse
    }

    configTerminal(1); // Restaure les paramètres du terminal
    printf("Game Over! Votre score final est: %d\n", score);
    return 0;
}
