#include <SDL3/SDL.h>
#include <stdbool.h>


typedef enum {
    prazdnepole = 0,
    I = 1,
    O = 2,
    T = 3,
    Z = 4,
    S = 5,
    L = 6,
    J = 7,
    SPECIAL = 8,
    okraj = 9,
}identifikatortvaru;
typedef struct {
    identifikatortvaru id;
    SDL_Color barva;
    int tvar[4][4][4]; 
}objectdata;

extern objectdata kostky[10];



