#include <stdio.h>

const char* welcome_tips = "Welcome to the TzDb Storage NDiskBase.\n\n"
                           "**************************************\n\n";

void printMess() {
    printf("%s", welcome_tips);
}


int main(int argc, char* argv[]) {
    printMess();
    return 0;
}