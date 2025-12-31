TETRIS

compilace
gcc hello.c -o prog $(pkg-config --cflags --libs sdl3 sdl3-mixer sdl3-image)

a nasledné spuštění 
./prog


popisek k ovladání
tetris se zavírá ESC
zakladní ovladání bez upravy (možnost změnění bindu v nastaveni)
levá šipka - posun vlevo 
pravá šipka - posun pravo
dolní šipka - zrychlení pádu
horní šipka - měnění rotace objektu 
p  - pauza menu 
levý shift - podržení kostky 

popisek obtížností 
v nastavení je možnost si nastavit obtížnost 1 až 5
4 a 5 obtížnost přidává do hry 8 unikatní tvar
score se vykresluje nahoře v levém horním rohu  

popisek k hudbě 
v ramci programu je i zvuková custom nahrávka která se dá vypnout pomocí tlačítka ztlumit

uchovávání skore
program uchovává nejlepší skoré v souboru score.txt 

soubory se kterými se pracuje 
include/objeckty.h
src/hra.c
src/objektyvypsane.c
hello.c

autor produktu Marek Slavík


