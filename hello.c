#define SDL_MAIN_USE_CALLBACKS 1
#define MENU 0
#define HERNIPLOCHA 1
#define NASTAVENI 2 
#define HRAJEZASTAVEN 3
#define KONECHRY 4
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_mixer.h>
#include "src/hra.c"
//https://wiki.libsdl.org/SDL3/SDL_Event

/*vytvoření globalních proměných */
SDL_Window* window;
SDL_Window* vyberklavesy;
SDL_Renderer* renderer;
SDL_FRect ctverechra;
SDL_FRect ctverecnastaveni;
SDL_FRect ramecek_reset;
SDL_FRect zvuk1;
int currentState = MENU;
SDL_FRect ramecek_zpet;
SDL_FRect obdelniky_key_Binds;
SDL_FRect vyber1, vyber2, vyber3, vyber4, vyber5;
SDL_FRect lobbyl, reset;
static bool jepolenstavene = false;
static char skore1[255]; 
bool cekam_na_klavesu = false;
int vybrany_bind = 0; 
int cekacka = 0;
char skore[255];
SDL_Keycode nova_klavesa = SDLK_UNKNOWN;
SDL_Texture* obrazek_textura;
SDL_Surface* obrazekcesta;

/*globalni proměné hudba*/
SDL_AudioStream *stream = NULL;
SDL_AudioSpec spec;
Uint8 *audio_data = NULL;
Uint32 audio_len = 0;
Uint32 audio_pos = 0;
bool hudbahraje = true;




//struktura pro nataveni hry 
typedef struct {
    int obtiznost;
    SDL_Keycode vlevo;    
    SDL_Keycode vpravo;   
    SDL_Keycode dolu;     
    SDL_Keycode otocit;   
    SDL_Keycode swap;     
    SDL_Keycode pauza;    

}natvenihry;

SDL_Keycode holder[6];

void inicializuj_defaultni_nastaveni(natvenihry *nastaveni,int j, SDL_Keycode h) {
    
    if(j == 0){
        nastaveni->obtiznost = 1;
        holder[0] = nastaveni->vlevo = SDLK_LEFT;
        holder[1] =nastaveni->vpravo = SDLK_RIGHT;
        holder[2] =nastaveni->dolu = SDLK_DOWN;
        holder[3] =nastaveni->otocit = SDLK_UP;
        holder[4] =nastaveni->swap = SDLK_LSHIFT;
        holder[5] =nastaveni->pauza = SDLK_P;
    }
    else if(j == 1){
        nastaveni->obtiznost = 1;
        holder[0] = nastaveni->vlevo = h;
        nastaveni->vpravo = holder[1];
        nastaveni->dolu = holder[2];
        nastaveni->otocit = holder[3];
        nastaveni->swap = holder[4];
        nastaveni->pauza = holder[5];
    }
    else if(j == 2){
        nastaveni->obtiznost = 1;
        nastaveni->vlevo = holder[0];
        holder[1] = nastaveni->vpravo = h;
        nastaveni->dolu = holder[2];
        nastaveni->otocit = holder[3];
        nastaveni->swap = holder[4];
        nastaveni->pauza = holder[5];
    }
    else if(j == 4){
        nastaveni->obtiznost = 1;
        nastaveni->vlevo = holder[0];
        nastaveni->vpravo = holder[1];
        holder[2] =nastaveni->dolu = h;
        nastaveni->otocit = holder[3];
        nastaveni->swap = holder[4];
        nastaveni->pauza = holder[5];
    }
    else if(j == 5){
        nastaveni->obtiznost = 1;
        nastaveni->vlevo = holder[0];
        nastaveni->vpravo = holder[1];
        nastaveni->dolu = holder[2];
        holder[3] =nastaveni->otocit = h;
        nastaveni->swap = holder[4];
        nastaveni->pauza = holder[5];
    }
    else if(j == 6){
        nastaveni->obtiznost = 1;
        nastaveni->vlevo = holder[0];
        nastaveni->vpravo = holder[1];
        nastaveni->dolu = holder[2];
        nastaveni->otocit = holder[3];
        holder[4] =nastaveni->swap = h;
        nastaveni->pauza = holder[5];
    }
    else if(j == 7){
        nastaveni->obtiznost = 1;
        nastaveni->vlevo = holder[0];
        nastaveni->vpravo = holder[1];
        nastaveni->dolu = holder[2];
        nastaveni->otocit = holder[3];
        nastaveni->swap = holder[4];
        holder[5] =nastaveni->pauza = h;
    }
    else{
        nastaveni->obtiznost = j-7;
        nastaveni->vlevo = holder[0];
        nastaveni->vpravo = holder[1];
        nastaveni->dolu = holder[2];
        nastaveni->otocit = holder[3];
        nastaveni->swap = holder[4];
        nastaveni->pauza = holder[5];
    }
    
}

/*call back audio*/
void SDL_AudioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    if (audio_data == NULL || audio_len == 0 || !hudbahraje){

    return;
    }

    while (additional_amount > 0) {
        int remaining = audio_len - audio_pos;
        int to_write;
        if(additional_amount < remaining){
            to_write = additional_amount ;
        } 
        else{
            to_write = remaining;
        }
        SDL_PutAudioStreamData(stream, audio_data + audio_pos, to_write);

        audio_pos += to_write;
        additional_amount -= to_write;
        if (audio_pos >= audio_len) {
            audio_pos = 0;
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]){
    /*nastavení prostředí */
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)){
        return SDL_APP_FAILURE;
    }
    window = SDL_CreateWindow("TETRIS",320,240, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_FULLSCREEN
    );
    renderer = SDL_CreateRenderer(window, NULL);

    natvenihry *nastaveni = (natvenihry *)SDL_calloc(1, sizeof(natvenihry));
    inicializuj_defaultni_nastaveni(nastaveni,0,SDLK_LEFT);
    
    *appstate = nastaveni; 

    /*obrazek na konci */
    obrazekcesta = SDL_LoadBMP("texture/depositphotos_21542481-stock-illustration-video-games-icons.bmp");
    obrazek_textura = SDL_CreateTextureFromSurface(renderer, obrazekcesta);

    /*audio */
    
    SDL_LoadWAV("audio/Matheeejovospecialsongforsalattetris.wav", &spec, &audio_data, &audio_len);
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_PutAudioStreamData(stream, audio_data, audio_len);
    SDL_SetAudioStreamGain(stream, 1.0f);
    
    
    return SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    /*otevirání souboru s nejlepším skore*/
    FILE *soubor = fopen("score.txt", "r");
    if (fscanf(soubor, "%s", skore) != 1) {
        SDL_strlcpy(skore, "0", sizeof(skore));
    }
    fclose(soubor);
    if(event->type == SDL_EVENT_QUIT){
        return SDL_APP_FAILURE;
    }
    /*ukončení pomocí escape*/
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_FAILURE; 
        }
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mouseX = event->button.x / 4.0f;
            float mouseY = event->button.y / 4.0f;

            // Kontrola, zda je klik uvnitř obdélníku tlačítka
            if (mouseX >= zvuk1.x && mouseX <= (zvuk1.x + zvuk1.w) &&
                mouseY >= zvuk1.y && mouseY <= (zvuk1.y + zvuk1.h)) {
                if(hudbahraje == true){
                    hudbahraje = false;
                }
                else{
                    hudbahraje = true;
                }
                    
            }
    }
    /*tlačitka v menu */
    if(currentState == MENU){
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mouseX = event->button.x / 4.0f;
            float mouseY = event->button.y / 4.0f;

            
            if (mouseX >= ctverechra.x && mouseX <= (ctverechra.x + ctverechra.w) &&
                mouseY >= ctverechra.y && mouseY <= (ctverechra.y + ctverechra.h)) {
                jepolenstavene = false;
                currentState = HERNIPLOCHA;
            }
        }
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mouseX = event->button.x / 4.0f;
            float mouseY = event->button.y / 4.0f;

            if (mouseX >= ctverecnastaveni.x && mouseX <= (ctverecnastaveni.x + ctverecnastaveni.w) &&
                mouseY >= ctverecnastaveni.y && mouseY <= (ctverecnastaveni.y + ctverecnastaveni.h)) {
                currentState = NASTAVENI;
            }
        }
    }
    /*tlačitko na herní pole*/
    else if(currentState == HERNIPLOCHA){
        natvenihry *nastaveni = (natvenihry *)appstate;
        if (event->type == SDL_EVENT_KEY_DOWN) {
            if (event->key.key == nastaveni->pauza) {
                currentState = HRAJEZASTAVEN;
            }
        }
    }
    /*tlačitka pro pauzu*/
    else if(currentState == HRAJEZASTAVEN){
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mouseX = event->button.x / 4.0f;
            float mouseY = event->button.y / 4.0f;

            
            if (mouseX >= ramecek_zpet.x && mouseX <= (ramecek_zpet.x + ramecek_zpet.w) &&
                mouseY >= ramecek_zpet.y && mouseY <= (ramecek_zpet.y + ramecek_zpet.h)) {
                currentState = HERNIPLOCHA;
            }
            else if (mouseX >= lobbyl.x && mouseX <= (lobbyl.x + lobbyl.w) &&
                mouseY >= lobbyl.y && mouseY <= (lobbyl.y + lobbyl.h)) {
                currentState = MENU;
            }
            else if (mouseX >= reset.x && mouseX <= (reset.x + reset.w) &&
                mouseY >= reset.y && mouseY <= (reset.y + reset.h)) {
                jepolenstavene = false;
            }
        }
    }
    /*tlačitka v nastaveni*/
    else if(currentState == NASTAVENI){

        natvenihry *nastaveni = (natvenihry *)appstate;
        if (cekam_na_klavesu && event->type == SDL_EVENT_KEY_DOWN) {
            nova_klavesa = event->key.key;

            inicializuj_defaultni_nastaveni(nastaveni, vybrany_bind, nova_klavesa);

            if (vyberklavesy) {
                SDL_DestroyWindow(vyberklavesy);
                vyberklavesy = NULL;
            }

            cekam_na_klavesu = false;
            vybrany_bind = 0;
            nova_klavesa = SDLK_UNKNOWN;

            return SDL_APP_CONTINUE;
        }

        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            float mouseX = event->button.x / 4.0f;
            float mouseY = event->button.y / 4.0f;

            // Kontrola, zda je klik uvnitř obdélníku tlačítka
            if (mouseX >= ramecek_zpet.x && mouseX <= (ramecek_zpet.x + ramecek_zpet.w) &&
                mouseY >= ramecek_zpet.y && mouseY <= (ramecek_zpet.y + ramecek_zpet.h)) {
                currentState = MENU;
            }
            else if(mouseX >= ramecek_reset.x && mouseX <= (ramecek_reset.x + ramecek_reset.w) &&
                mouseY >= ramecek_reset.y && mouseY <= (ramecek_reset.y + ramecek_reset.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 0, SDLK_LEFT);
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y) && mouseY <= ((obdelniky_key_Binds.y) + obdelniky_key_Binds.h)){
                    //levá
                    vybrany_bind = 1;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER LEVE KLAVESY",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y +15 ) && mouseY <= ((obdelniky_key_Binds.y +15 ) + obdelniky_key_Binds.h)){
                    vybrany_bind = 2;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER PRAVE KLAVESY",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y + 30) && mouseY <= ((obdelniky_key_Binds.y + 30) + obdelniky_key_Binds.h)){
                    //dolu
                    vybrany_bind = 4;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER ZRYCHLENÍ",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y + 45) && mouseY <= ((obdelniky_key_Binds.y + 45) + obdelniky_key_Binds.h)){
                    //otočit
                    vybrany_bind = 5;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER KLAVESY NA OTOCENI",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y + 60) && mouseY <= ((obdelniky_key_Binds.y + 60) + obdelniky_key_Binds.h)){
                    //swap
                    vybrany_bind = 6;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER SWAP KLAVESY",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= obdelniky_key_Binds.x && mouseX <= (obdelniky_key_Binds.x + obdelniky_key_Binds.w) &&
                mouseY >= (obdelniky_key_Binds.y + 75) && mouseY <= ((obdelniky_key_Binds.y + 75) + obdelniky_key_Binds.h)){
                    //pauza
                    vybrany_bind = 7;
                    cekam_na_klavesu = true;

                    if (!vyberklavesy) {
                        vyberklavesy = SDL_CreateWindow("VYBER PAUZA KLAVESY",320,240, 
                            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_BORDERLESS
                        );
                    }
            }
            else if(mouseX >= vyber1.x && mouseX <= (vyber1.x + vyber1.w) &&
                mouseY >= vyber1.y && mouseY <= (vyber1.y + vyber1.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 8, SDLK_LEFT);
            }
            else if(mouseX >= vyber2.x && mouseX <= (vyber2.x + vyber2.w) &&
                mouseY >= vyber2.y && mouseY <= (vyber2.y + vyber2.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 9, SDLK_LEFT);
            }
            else if(mouseX >= vyber3.x && mouseX <= (vyber3.x + vyber3.w) &&
                mouseY >= vyber3.y && mouseY <= (vyber3.y + vyber3.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 10, SDLK_LEFT);
            }
            else if(mouseX >= vyber4.x && mouseX <= (vyber4.x + vyber4.w) &&
                mouseY >= vyber4.y && mouseY <= (vyber4.y + vyber4.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 11, SDLK_LEFT);
            }
            else if(mouseX >= vyber5.x && mouseX <= (vyber5.x + vyber5.w) &&
                mouseY >= vyber5.y && mouseY <= (vyber5.y + vyber5.h)) {
                    inicializuj_defaultni_nastaveni(nastaveni, 12, SDLK_LEFT);
            }
            
            
        }

    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate){
    
    // 1. Vymazání obrazovky
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    //vykresleni stlumení 
    const char *zvuk;
    if(hudbahraje == true){
        zvuk = "ZTLUMIT";
        SDL_ResumeAudioStreamDevice(stream);
    }
    else if(hudbahraje == false){
        zvuk = "ZTLUMENO";
        SDL_PauseAudioStreamDevice(stream);
    }
    float x5 =0, y5=0;
    int winW, winH;
    SDL_GetWindowSize(window, &winW, &winH);
    
    // podminka dle které se zjisti zda je menu nebo hra zvolena 
    if(currentState == MENU){
        /*níže vykreslení objektu v menu */
        const char *message1 = "TETRIS";
        const char *message2 = "HRA";
        const char *message3 = "NASTAVENI";
        SDL_snprintf(skore1, sizeof(skore1), "NEJVYSSI SKORE JE: %s", skore);
        const char *message4 = skore1;
        
        float x1, y1, x2, y2, x3, y3, x4, y4;
        float zvetseni = 4.0f;
        
        SDL_SetRenderScale(renderer, zvetseni, zvetseni);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        x1 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message1))) / 2;
        y1 = ((winH / zvetseni) / 2) - 80.0f;

        x2 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message2))) / 2;
        y2 = ((winH / zvetseni) / 2) + 40.0f;

        x3 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message3))) / 2;
        y3 = ((winH / zvetseni) / 2) + 60.0f;

        x4 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message4))) / 2;
        y4 = ((winH / zvetseni) / 2) - 50.0f;

        ctverechra.w = 40.0f;  
        ctverechra.h = 15.0f;  
        ctverechra.x = (winW / zvetseni / 2.0f) - (ctverechra.w / 2.0f);
        ctverechra.y = y2 - 4.0f; 

        ctverecnastaveni.w = 100.0f;  
        ctverecnastaveni.h = 15.0f;  
        ctverecnastaveni.x = (winW / zvetseni / 2.0f) - (ctverecnastaveni.w / 2.0f);
        ctverecnastaveni.y = y3 - 4.0f; 

        SDL_RenderRect(renderer, &ctverechra);
        SDL_RenderRect(renderer, &ctverecnastaveni);
        SDL_RenderDebugText(renderer, x1, y1, message1);
        SDL_RenderDebugText(renderer, x2, y2, message2);
        SDL_RenderDebugText(renderer, x3, y3, message3);
        SDL_RenderDebugText(renderer, x4, y4, message4);

        /*zvuk*/

        x5 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(zvuk))) / 2 +160.0f;
        y5 = ((winH / zvetseni) / 2)-110.0f;
        zvuk1.w = 100.0f;  
        zvuk1.h = 15.0f;  
        zvuk1.x = (winW / zvetseni / 2.0f) - (zvuk1.w / 2.0f) +160.0f;
        zvuk1.y = y5 - 4.0f;

        SDL_RenderRect(renderer, &zvuk1);
        SDL_RenderDebugText(renderer, x5, y5, zvuk);
    }
    else if(currentState == HERNIPLOCHA){
        /*níže je poslaní do souboru hra a naslední vykreslení ztlumeni pro hru */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        natvenihry *nastaveni = (natvenihry *)appstate; 
        jepolenstavene=game(renderer,nastaveni->obtiznost,nastaveni->vlevo, nastaveni->vpravo, nastaveni->dolu, nastaveni->otocit, nastaveni->swap, nastaveni->pauza,jepolenstavene,skore);
        if(jepolenstavene == false){
            currentState = KONECHRY;
        }
        /*zvuk*/
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        x5 = 303.0f;
        y5 = 30.0f;
        zvuk1.w = 71.0f;  
        zvuk1.h = 15.0f;  
        zvuk1.x = x5 - 3.0f;
        zvuk1.y = y5-4.0f;

        SDL_RenderRect(renderer, &zvuk1);
        SDL_RenderDebugText(renderer, x5, y5, zvuk);
    }
    else if(currentState == KONECHRY){
        /*zde se vykresluje obrázek konec hry*/
        SDL_RenderTexture(renderer,obrazek_textura,NULL,NULL);
        cekacka++;
        if(cekacka == 2000){
            currentState = MENU;
            cekacka = 0;
        }
        
    }
    else if(currentState == HRAJEZASTAVEN){
        /*zde je vykreslování pro pauzu*/
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        const char *hlavickapauza = "HRA JE ZASTAVENA";
        const char *zpetpauza = "ZPET DO HRY";
        const char *odejit= "ODEJIT DO MENU";
        const char *resetext = "RESETOVAT PLOCHU";

        float zvetseni = 4.0f;
        float x_zpet, y_zpet;
        float x_hlavicka, y_hlavicka;
        float x_lobby, y_lobby;
        float x_reset, y_reset;


        SDL_SetRenderScale(renderer, zvetseni, zvetseni);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        ramecek_zpet.w = 150.0f;
        ramecek_zpet.h = 20.0f;
        ramecek_zpet.x = (winW / zvetseni / 2.0f) - (ramecek_zpet.w / 2.0f);
        ramecek_zpet.y = 180.0f;
        
        x_zpet = ramecek_zpet.x + (ramecek_zpet.w / 2.0f) - ((SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(zpetpauza)) / 2.0f);
        y_zpet = ramecek_zpet.y + (ramecek_zpet.h / 2.0f) - 4.0f;


        lobbyl.w = 150.0f;
        lobbyl.h = 20.0f;
        lobbyl.x = (winW / zvetseni / 2.0f) - (ramecek_zpet.w / 2.0f);
        lobbyl.y = 210.0f;
        
        x_lobby = lobbyl.x + (lobbyl.w / 2.0f) - ((SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(odejit)) / 2.0f);
        y_lobby = lobbyl.y + (lobbyl.h / 2.0f) - 4.0f;

        reset.w = 150.0f;
        reset.h = 20.0f;
        reset.x = (winW / zvetseni / 2.0f) - (ramecek_zpet.w / 2.0f);
        reset.y = 240.0f;
        
        x_reset = reset.x + (reset.w / 2.0f) - ((SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(resetext)) / 2.0f);
        y_reset = reset.y + (reset.h / 2.0f) - 4.0f;

        x_hlavicka = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(hlavickapauza))) / 2;
        y_hlavicka = 10.0f;



        SDL_RenderRect(renderer, &ramecek_zpet);
        SDL_RenderRect(renderer, &lobbyl);
        SDL_RenderRect(renderer, &reset);

        SDL_RenderDebugText(renderer, x_hlavicka, y_hlavicka, hlavickapauza);
        SDL_RenderDebugText(renderer, x_zpet, y_zpet, zpetpauza);
        SDL_RenderDebugText(renderer, x_reset, y_reset, resetext);
        SDL_RenderDebugText(renderer, x_lobby, y_lobby, odejit);

        /*zvuk*/
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        x5 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(zvuk))) / 2 +160.0f;
        y5 = ((winH / zvetseni) / 2)-110.0f;
        zvuk1.w = 100.0f;  
        zvuk1.h = 15.0f;  
        zvuk1.x = (winW / zvetseni / 2.0f) - (zvuk1.w / 2.0f) +160.0f;
        zvuk1.y = y5 - 4.0f;

        SDL_RenderRect(renderer, &zvuk1);
        SDL_RenderDebugText(renderer, x5, y5, zvuk);
    }
    else if(currentState == NASTAVENI){
        /*zde níže je vykreslování pro nastavení */
        natvenihry *nastaveni = (natvenihry *)appstate; 
        
        const char *hlavicka = "NASTAVENI";
        const char *sekce1 = "OBTIZNOST";
        const char *popisek_vyber = "VYBER";
        const char *sekce2 = "KEY BINDS";
        const char *popisky[] = {"VLEVO", "VPRAVO", "DOLU", "OTOCIT", "SWAP", "PAUZA"};
        SDL_Keycode kody[] = {
        nastaveni->vlevo, nastaveni->vpravo, nastaveni->dolu, 
        nastaveni->otocit, nastaveni->swap, nastaveni->pauza
        };
        const char *zpet = "ZPET";
        const char *reset_text = "RESET NA DEFAULT";

        const char *cislo1 = "1";
        const char *cislo2 = "2";
        const char *cislo3 = "3";
        const char *cislo4 = "4";
        const char *cislo5 = "5";

        float x_hlavicka, y_hlavicka;
        float x_sekce1, y_sekce1;
        float x_obtiznost, y_obtiznost;
        float x_vyber, y_vyber;
        float x_sekce2, y_sekce2;
        float x_zpet, y_zpet;
        float x_reset, y_reset;

        float zvetseni = 4.0f;

        SDL_SetRenderScale(renderer, zvetseni, zvetseni);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        x_hlavicka = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(hlavicka))) / 2;
        y_hlavicka = 10.0f;

        x_sekce1 = ((winW / zvetseni) / 2) - 100.0f;
        y_sekce1 = y_hlavicka + 30.0f;

        x_vyber = x_sekce1;
        y_vyber = y_sekce1 + 20.0f + 15.0f;

        float vyber_mezery = 18.0f;
        float vybe = 14.0f;
        float vyber_vyska = 15.0f;
        float vyber_start_x = ((winW / zvetseni) / 2) + 20.0f;

        vyber1.w = vybe;
        vyber1.h = vyber_vyska;
        vyber1.x = vyber_start_x;
        vyber1.y = y_vyber - 4.0f;

        vyber2.w = vybe;
        vyber2.h = vyber_vyska;
        vyber2.x = vyber1.x + vyber_mezery;
        vyber2.y = y_vyber - 4.0f;

        vyber3.w = vybe;
        vyber3.h = vyber_vyska;
        vyber3.x = vyber2.x + vyber_mezery;
        vyber3.y = y_vyber - 4.0f;

        vyber4.w = vybe;
        vyber4.h = vyber_vyska;
        vyber4.x = vyber3.x + vyber_mezery;
        vyber4.y = y_vyber - 4.0f;

        vyber5.w = vybe;
        vyber5.h = vyber_vyska;
        vyber5.x = vyber4.x + vyber_mezery;
        vyber5.y = y_vyber - 4.0f;

        float x_cislo1 = vyber1.x + 2.0f;
        float x_cislo2 = vyber2.x + 2.0f;
        float x_cislo3 = vyber3.x + 2.0f;
        float x_cislo4 = vyber4.x + 2.0f;
        float x_cislo5 = vyber5.x + 2.0f;
        float y_cisla = y_vyber;

        x_sekce2 = x_sekce1;
        y_sekce2 = y_vyber + 30.0f;

        float start_x = ((winW / zvetseni) / 2) - 100.0f;
        float start_y = y_sekce2 + 20.0f; 
        float tlacitko_x = ((winW / zvetseni) / 2) + 20.0f;

        obdelniky_key_Binds.w = 100.0f;
        obdelniky_key_Binds.h = 15.0f;
        obdelniky_key_Binds.x = tlacitko_x;
        obdelniky_key_Binds.y = start_y;

        for (int i = 0; i < 6; i++) {
            float y = start_y + (i * 15.0f); 

            SDL_RenderDebugText(renderer, start_x, y, popisky[i]);

            SDL_FRect r = { tlacitko_x, y - 4.0f, 100.0f, 15.0f };
            SDL_RenderRect(renderer, &r);


            const char *nazev_klavesy = SDL_GetKeyName(kody[i]);
            SDL_RenderDebugText(renderer, r.x + 2.0f, y, nazev_klavesy);
        }

        ramecek_reset.w = 180.0f;
        ramecek_reset.h = 20.0f;
        ramecek_reset.x = (winW / zvetseni / 2.0f) - (ramecek_reset.w / 2.0f);
        ramecek_reset.y = start_y + (5 * 15.0f) + 15.0f;

        x_reset = ramecek_reset.x + (ramecek_reset.w / 2.0f) - ((SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(reset_text)) / 2.0f);
        y_reset = ramecek_reset.y + (ramecek_reset.h / 2.0f) - 4.0f;

        ramecek_zpet.w = 40.0f;
        ramecek_zpet.h = 20.0f;
        ramecek_zpet.x = (winW / zvetseni / 2.0f) - (ramecek_zpet.w / 2.0f);
        ramecek_zpet.y =  ramecek_reset.y + 25.0f;
        
        x_zpet = ramecek_zpet.x + (ramecek_zpet.w / 2.0f) - ((SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(zpet)) / 2.0f);
        y_zpet = ramecek_zpet.y + (ramecek_zpet.h / 2.0f) - 4.0f;

        SDL_RenderRect(renderer, &vyber1);
        SDL_RenderRect(renderer, &vyber2);
        SDL_RenderRect(renderer, &vyber3);
        SDL_RenderRect(renderer, &vyber4);
        SDL_RenderRect(renderer, &vyber5);
        SDL_RenderRect(renderer, &ramecek_reset);
        SDL_RenderRect(renderer, &ramecek_zpet);

        SDL_RenderDebugText(renderer, x_hlavicka, y_hlavicka, hlavicka);
        SDL_RenderDebugText(renderer, x_sekce1, y_sekce1, sekce1); 
        SDL_RenderDebugText(renderer, x_vyber, y_vyber, popisek_vyber); 
        SDL_RenderDebugText(renderer, x_sekce2, y_sekce2, sekce2);
        SDL_RenderDebugText(renderer, x_reset, y_reset, reset_text);
        SDL_RenderDebugText(renderer, x_zpet, y_zpet, zpet);

        SDL_RenderDebugText(renderer, x_cislo1, y_cisla, cislo1);
        SDL_RenderDebugText(renderer, x_cislo2, y_cisla, cislo2);
        SDL_RenderDebugText(renderer, x_cislo3, y_cisla, cislo3);
        SDL_RenderDebugText(renderer, x_cislo4, y_cisla, cislo4);
        SDL_RenderDebugText(renderer, x_cislo5, y_cisla, cislo5);

        SDL_SetRenderDrawColor(renderer, 51, 240, 48, 255);
        if(nastaveni->obtiznost == 1 ){
            SDL_RenderFillRect(renderer, &vyber1);
        }
        else if(nastaveni->obtiznost == 2 ){
            SDL_RenderFillRect(renderer, &vyber2);
        }
        else if(nastaveni->obtiznost == 3 ){
            SDL_RenderFillRect(renderer, &vyber3);
        }
        else if(nastaveni->obtiznost == 4 ){
            SDL_RenderFillRect(renderer, &vyber4);
        }
        else if(nastaveni->obtiznost == 5 ){
            SDL_RenderFillRect(renderer, &vyber5);
        }

        /*zvuk*/
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        x5 = ((winW / zvetseni) - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(zvuk))) / 2 +160.0f;
        y5 = ((winH / zvetseni) / 2)-110.0f;
        zvuk1.w = 100.0f;  
        zvuk1.h = 15.0f;  
        zvuk1.x = (winW / zvetseni / 2.0f) - (zvuk1.w / 2.0f) +160.0f;
        zvuk1.y = y5 - 4.0f;

        SDL_RenderRect(renderer, &zvuk1);
        SDL_RenderDebugText(renderer, x5, y5, zvuk);
    }
    SDL_RenderPresent(renderer);
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result){
    /*zde je uvolňovani paměti */
    SDL_DestroyTexture(obrazek_textura);
    SDL_DestroySurface(obrazekcesta);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_free(audio_data);
    SDL_DestroyAudioStream(stream);;
    SDL_Quit();
}
