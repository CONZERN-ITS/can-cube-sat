#include <ncurses.h>

int main(int argv, char *argc[]) {
    initscr();                   
    printw("Hello world!\n");  
    refresh();                  
    getch();                     
    endwin();                    
    return 0;
}
