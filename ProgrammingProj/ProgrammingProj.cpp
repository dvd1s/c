#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>   // Needed to enable ANSI colors in VS Code terminal

#define MAX 100

// ─── COLORS ───────────────────────────────────────────────────────────────────
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

// This function makes ANSI color codes work in VS Code / Windows terminal
void enableColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // Get the console handle
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);                  // Read current console settings
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;   // Add ANSI support flag
    SetConsoleMode(hOut, dwMode);                   // Apply updated settings
}

// ─── STRUCTURE ────────────────────────────────────────────────────────────────
typedef struct {
    char player1[50];   // Player 1 name
    char player2[50];   // Player 2 name
    int  score1;        // Player 1 score
    int  score2;        // Player 2 score
} Match;

// ─── GLOBALS ──────────────────────────────────────────────────────────────────
Match matches[MAX];   // Array of all matches
int   matchCount = 0; // How many matches stored

// ─── HELPERS ──────────────────────────────────────────────────────────────────

void clearScreen() {
    system("cls");   // Clear the Windows console
}

void pauseScreen() {
    printf("\nPress Enter to return...");
    getchar();   // Consume leftover newline
    getchar();   // Wait for Enter
    clearScreen();
}

// ─── FILE FUNCTIONS ───────────────────────────────────────────────────────────

void saveToFile() {
    FILE* f = fopen("data.txt", "w");   // Open for writing
    if (f == NULL) {
        printf(RED "  Error: Could not save to file!\n" RESET);
        return;
    }
    fprintf(f, "%d\n", matchCount);   // Write number of matches
    for (int i = 0; i < matchCount; i++) {
        fprintf(f, "%s %d %s %d\n",
            matches[i].player1, matches[i].score1,
            matches[i].player2, matches[i].score2);
    }
    fclose(f);
}

void loadFromFile() {
    FILE* f = fopen("data.txt", "r");   // Open for reading
    if (f == NULL) return;              // No file yet, skip
    fscanf(f, "%d", &matchCount);      // Read number of matches
    for (int i = 0; i < matchCount; i++) {
        fscanf(f, "%s %d %s %d",
            matches[i].player1, &matches[i].score1,
            matches[i].player2, &matches[i].score2);
    }
    fclose(f);
}

// ─── VALIDATION ───────────────────────────────────────────────────────────────

// Read and validate a player name (letters only, 2-49 chars)
int readName(const char* prompt, char* out) {
    char buf[100];
    printf("%s", prompt);
    scanf("%s", buf);
    int len = strlen(buf);
    if (len < 2 || len > 49) {
        printf(RED "  Invalid! Name must be 2-49 characters.\n" RESET);
        return 0;
    }
    for (int i = 0; i < len; i++) {
        if (!((buf[i] >= 'A' && buf[i] <= 'Z') ||
            (buf[i] >= 'a' && buf[i] <= 'z') ||
            buf[i] == '-')) {
            printf(RED "  Invalid! Name must contain only letters.\n" RESET);
            return 0;
        }
    }
    strcpy(out, buf);
    return 1;
}

// Read and validate a score (number between 0 and 21)
int readScore(const char* prompt, int* out) {
    printf("%s", prompt);
    int val;
    if (scanf("%d", &val) != 1) {
        printf(RED "  Invalid! Please enter a number.\n" RESET);
        while (getchar() != '\n');
        return 0;
    }
    if (val < 0 || val > 21) {
        printf(RED "  Invalid! Score must be 0-21.\n" RESET);
        return 0;
    }
    *out = val;
    return 1;
}

// ─── MENU FUNCTIONS ───────────────────────────────────────────────────────────

void addMatch() {
    clearScreen();
    printf(CYAN "=== ADD MATCH ===\n\n" RESET);

    if (matchCount >= MAX) {
        printf(RED "  Match list is full!\n" RESET);
        pauseScreen();
        return;
    }

    char p1[50], p2[50];
    int  s1, s2;

    printf("Player 1\n");
    while (!readName("  Name (letters only, 2-49 chars): ", p1));
    while (!readScore("  Score (0-21): ", &s1));

    printf("\nPlayer 2\n");
    while (!readName("  Name (letters only, 2-49 chars): ", p2));
    while (!readScore("  Score (0-21): ", &s2));

    if (s1 == s2) {
        printf(RED "\n  Invalid! Scores cannot be equal (no draws).\n" RESET);
        pauseScreen();
        return;
    }

    // Save the match into the array
    strcpy(matches[matchCount].player1, p1);
    strcpy(matches[matchCount].player2, p2);
    matches[matchCount].score1 = s1;
    matches[matchCount].score2 = s2;
    matchCount++;

    saveToFile();   // Save to file right away

    printf(GREEN "\n  Match saved successfully!\n" RESET);
    pauseScreen();
}

void showMatches() {
    clearScreen();
    printf(CYAN "=== MATCH LIST ===\n\n" RESET);

    if (matchCount == 0) {
        printf(YELLOW "  No matches recorded yet.\n" RESET);
        pauseScreen();
        return;
    }

    for (int i = 0; i < matchCount; i++) {
        int p1won = matches[i].score1 > matches[i].score2;  // Did player1 win?

        printf("  %d. ", i + 1);

        printf(p1won ? GREEN : RED);          // Winner = green, loser = red
        printf("%s", matches[i].player1);
        printf(RESET);

        printf(" %d - %d ", matches[i].score1, matches[i].score2);

        printf(!p1won ? GREEN : RED);
        printf("%s", matches[i].player2);
        printf(RESET "\n");
    }

    pauseScreen();
}

void showRanking() {
    clearScreen();
    printf(CYAN "=== RANKING (by wins) ===\n\n" RESET);

    if (matchCount == 0) {
        printf(YELLOW "  No matches recorded yet.\n" RESET);
        pauseScreen();
        return;
    }

    char players[MAX][50];
    int  wins[MAX] = { 0 };
    int  losses[MAX] = { 0 };
    int  playerCount = 0;

    for (int i = 0; i < matchCount; i++) {
        int p1 = -1, p2 = -1;

        // Find existing players
        for (int j = 0; j < playerCount; j++) {
            if (strcmp(players[j], matches[i].player1) == 0) p1 = j;
            if (strcmp(players[j], matches[i].player2) == 0) p2 = j;
        }

        // Add new players if not found
        if (p1 == -1) { strcpy(players[playerCount], matches[i].player1); p1 = playerCount++; }
        if (p2 == -1) { strcpy(players[playerCount], matches[i].player2); p2 = playerCount++; }

        // Update wins and losses
        if (matches[i].score1 > matches[i].score2) { wins[p1]++; losses[p2]++; }
        else { wins[p2]++; losses[p1]++; }
    }

    // Bubble sort by wins (highest first)
    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = 0; j < playerCount - i - 1; j++) {
            if (wins[j] < wins[j + 1]) {
                int tmp = wins[j]; wins[j] = wins[j + 1]; wins[j + 1] = tmp;
                tmp = losses[j]; losses[j] = losses[j + 1]; losses[j + 1] = tmp;
                char tmpName[50];
                strcpy(tmpName, players[j]);
                strcpy(players[j], players[j + 1]);
                strcpy(players[j + 1], tmpName);
            }
        }
    }

    // Print ranking
    for (int i = 0; i < playerCount; i++) {
        printf("  %d. ", i + 1);
        printf(CYAN "%-15s" RESET, players[i]);          // Name in cyan
        printf(" Wins: " GREEN "%d" RESET, wins[i]);     // Wins in green
        printf("  Losses: " RED "%d\n" RESET, losses[i]); // Losses in red
    }

    pauseScreen();
}

void showHistory() {
    clearScreen();
    printf(CYAN "=== PLAYER HISTORY ===\n\n" RESET);

    char name[50];
    while (!readName("  Player name (letters only, 2-49 chars): ", name));

    printf("\n  Matches for " CYAN "%s" RESET ":\n\n", name);

    int found = 0;
    for (int i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].player1, name) == 0 ||
            strcmp(matches[i].player2, name) == 0) {

            int won = (strcmp(matches[i].player1, name) == 0)
                ? (matches[i].score1 > matches[i].score2)
                : (matches[i].score2 > matches[i].score1);

            printf("  %d. %s %d - %d %s  [",
                i + 1,
                matches[i].player1, matches[i].score1,
                matches[i].score2, matches[i].player2);

            printf("%s", won ? GREEN "WIN" RESET : RED "LOSS" RESET);
            printf("]\n");
            found = 1;
        }
    }

    if (!found)
        printf(YELLOW "  No matches found for this player.\n" RESET);

    pauseScreen();
}

// ─── MENU ─────────────────────────────────────────────────────────────────────

void menu() {
    int choice;
    do {
        printf(CYAN "=== TABLE TENNIS MANAGER ===\n\n" RESET);
        printf("  " YELLOW "1" RESET ". Add Match\n");
        printf("  " YELLOW "2" RESET ". Show All Matches\n");
        printf("  " YELLOW "3" RESET ". Show Ranking\n");
        printf("  " YELLOW "4" RESET ". Show Player History\n");
        printf("  " YELLOW "0" RESET ". Exit\n\n");
        printf("  Choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1: addMatch();    break;
        case 2: showMatches(); break;
        case 3: showRanking(); break;
        case 4: showHistory(); break;
        case 0: printf(GREEN "\n  Goodbye!\n" RESET); break;
        default: printf(RED "  Invalid option.\n\n" RESET); break;
        }
    } while (choice != 0);
}

// ─── MAIN ─────────────────────────────────────────────────────────────────────

int main() {
    enableColors();   // Enable ANSI colors in VS Code / Windows terminal
    loadFromFile();   // Load saved matches from data.txt
    menu();           // Start the menu
    return 0;
}