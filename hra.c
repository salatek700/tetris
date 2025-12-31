#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "objektyvypsane.c"

/*vytvoření proměnýách */
static int hracipole[22][12];
static int swappole[6][6];

static int akt_x = 4;
static int akt_y = 0;
static int akt_rot = 0;
static int akt_typ = 1;
static int hold_typ = 1; 
static float posledni_pad = 0;
static float posledni_vstup = 0; 
static int body;
int a;
int promprohold = 1;

// Detekce kolizí
bool KontrolaKolize(int x, int y, int nr, int typ) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (kostky[typ].tvar[nr][i][j] != 0) {
                int obsazenox = x + j;
                int obsazenoy = y + i;
                
                if (obsazenox < 0 || obsazenox >= 12 || obsazenoy >= 22){
                    return true;
                } 
                
                if (hracipole[obsazenoy][obsazenox] != 0){
                    return true;
                } 
            }
        }
    }
    return false;
}

// Zápis kostky do mřížky po dopadu
void ZapisDoPole(int x, int y, int nr, int typ) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (kostky[typ].tvar[nr][i][j] != 0) {
                hracipole [y + i][x + j] = typ;
            }
        }
    }
}

/*mazaní řad*/
void SmazPlneRady( int obtiznost) {
    bool zmeneno;
    do {
        zmeneno = false; 
        for (int i = 20; i > 0; i--) { 
            bool je_plna = true;
            for (int j = 1; j < 11; j++) { 
                if (hracipole[i][j] == 0) {
                    je_plna = false;
                    break;
                }
            }
            if (je_plna) {
                for (int mazaniy = i; mazaniy > 1; mazaniy--) {
                    for (int mazanix = 1; mazanix < 11; mazanix++) {
                        hracipole[mazaniy][mazanix] = hracipole[mazaniy - 1][mazanix]; 
                    }
                }          
                a++; 
                zmeneno = true;  
                i++; 
            }
        }
    } 
    /*ukládání bodu*/
    while (zmeneno);
    if (a > 0) {
        body = body+(100 * obtiznost * a) * a;
        a=0;
    }
    
}

bool game(SDL_Renderer* renderer, int obtiznost, SDL_Keycode levo, SDL_Keycode pravo, SDL_Keycode dolu, SDL_Keycode otocit, SDL_Keycode swap, SDL_Keycode pauza,bool jepolenstavene,char skore[255]){
    /*nahravní rozměru pole*/
    char str[255];
    sprintf(str, "%d", body);
    if (!jepolenstavene) {
        for (int i = 0; i < 22; i++) {
            for (int j = 0; j < 12; j++) {
                if(i == 21 || j == 0 || j == 11){
                hracipole[i][j] = 9;
                } else {
                    hracipole[i][j] = 0;
                }
            }
        }
        return true;
        /*nahodný vyběr tvaru*/
        if(obtiznost>3){ 
            akt_typ = (int)(SDL_GetTicks() % 8) + 1;
        }
        else{
            akt_typ = (int)(SDL_GetTicks() % 7) + 1;
        }
    }

    float ted = SDL_GetTicks();
    /*získaní aktualně ziskané klavesy*/
    const bool *klavesy = SDL_GetKeyboardState(NULL);

    /*reakce na stisknuté klávesy*/
    if (ted > posledni_vstup + 100) {
        if (klavesy[SDL_GetScancodeFromKey(levo, NULL)]) {
            if (!KontrolaKolize(akt_x - 1, akt_y, akt_rot, akt_typ)){
                akt_x--;
            } 
            posledni_vstup = ted;
        }
        if (klavesy[SDL_GetScancodeFromKey(pravo, NULL)]) {
            if (!KontrolaKolize(akt_x + 1, akt_y, akt_rot, akt_typ)){
                akt_x++;
            } 
            posledni_vstup = ted;
        }
        if (klavesy[SDL_GetScancodeFromKey(otocit, NULL)]) {
            int nova_rot = (akt_rot + 1) % 4;
            if (!KontrolaKolize(akt_x, akt_y, nova_rot, akt_typ)){
                akt_rot = nova_rot;
            } 
            posledni_vstup = ted;
        }
        if(promprohold == 0){
            if (klavesy[SDL_GetScancodeFromKey(swap, NULL)]) {
                int b = akt_typ;
                akt_typ = hold_typ;
                hold_typ = b;
                akt_x = (int)(SDL_GetTicks() % 6) + 2;
                akt_y = 0;
                promprohold = 1;
            }
        }
        
    }

    int rychlost = 1100 - (obtiznost * 200); 
    bool tlacitko_dolu = klavesy[SDL_GetScancodeFromKey(dolu, NULL)];
    float aktualni_interval;
    if (tlacitko_dolu == true) {
        aktualni_interval = 50;     
    } else {
        aktualni_interval = rychlost; 
    }
    /*zjišťovaní kolize a nasledné mazání řad*/
    if (ted > posledni_pad + aktualni_interval) {
        if (!KontrolaKolize(akt_x, akt_y + 1, akt_rot, akt_typ)) {
            akt_y++;
        } 
        else {
            promprohold = 0;
            ZapisDoPole(akt_x, akt_y, akt_rot, akt_typ);
            SmazPlneRady(obtiznost);
            
            akt_y = 0;
            akt_x = (int)(SDL_GetTicks() % 6) + 2;
            akt_rot = 0;
            if(obtiznost > 3){
                akt_typ = (int)(SDL_GetTicks() % 8) + 1;
            }
            else{
                akt_typ = (int)(SDL_GetTicks() % 7) + 1;
            }
            
            //konec
            if (KontrolaKolize(akt_x, akt_y, akt_rot, akt_typ)) {
                if(atoi(skore)<body){
                    FILE *soubor = fopen("score.txt", "w");
                    fprintf(soubor, "%s", str);
                    fclose(soubor);
                }
                body = 0;
                return false; 
            }
        }
        posledni_pad = ted;
    }
    
    float kostkyrozmer = 12.0f; 
    float plochapozicex = 120.0f;    
    float plochapozicey = 5.0f;

    //herni pole
    for (int i = 0; i < 22; i++) {
        for (int j = 0; j < 12; j++) {
            int id = hracipole[i][j];
            SDL_Color c = kostky[id].barva;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_FRect r = { plochapozicex + j * kostkyrozmer, plochapozicey + i * kostkyrozmer, kostkyrozmer - 1, kostkyrozmer - 1 };
            SDL_RenderFillRect(renderer, &r);
        }
    }

    //aktulani kostka
    SDL_Color c_akt = kostky[akt_typ].barva;
    SDL_SetRenderDrawColor(renderer, c_akt.r, c_akt.g, c_akt.b, 255);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (kostky[akt_typ].tvar[akt_rot][i][j] != 0) {
                SDL_FRect r = { 
                    plochapozicex + (akt_x + j) * kostkyrozmer, 
                    plochapozicey + (akt_y + i) * kostkyrozmer, 
                    kostkyrozmer - 1, kostkyrozmer - 1 
                };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
    

    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if(i == 5 || j == 0 || j == 5 || i == 0){
                swappole[i][j] = 9;
            } else {
                swappole[i][j] = 0;
            }
        }
    }

    //holder swapu
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            int id = swappole[i][j];
            SDL_Color c = kostky[id].barva;
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_FRect r = { plochapozicex + j * kostkyrozmer + 180.0f, plochapozicey + i * kostkyrozmer + 50.0f, kostkyrozmer - 1, kostkyrozmer - 1 };
            SDL_RenderFillRect(renderer, &r);
        }
    }

    //vykresleni kostky
    SDL_Color a_akt = kostky[hold_typ].barva;
    SDL_SetRenderDrawColor(renderer, a_akt.r, a_akt.g, a_akt.b, 255);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (kostky[hold_typ].tvar[0][i][j] != 0) {
                SDL_FRect l = { 
                    plochapozicex + j * kostkyrozmer + 192.0f, 
                    plochapozicey + i * kostkyrozmer + 70.0f, 
                    kostkyrozmer - 1, kostkyrozmer - 1 
                };
                SDL_RenderFillRect(renderer, &l);
            }
        }
    }
    
    SDL_RenderDebugText(renderer, 1.0f,2.0f , str);
    SDL_SetRenderDrawColor(renderer, 74, 100, 217, 255);
    SDL_RenderDebugText(renderer, 317.0f, 45.0f , "SWAPS");
};