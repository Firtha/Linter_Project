#include <stdio.h>
#include <stdlib.h>

int returnVar();

int main(int argc, char** argv){
    char tab[6][15] = {"Hello","World","I","feel","really","great"};
    
    printf("Hello World !");
    
    nbChar = 0;
    
    int i;
    int y, x;
    for(i=0;i<15;i++)
    {
        if(undeclaredVar){
            printf("Alriiight...");
        }
        printf("Spacing test"); 
    }
    
    return 0;
}

int returnVar(){
    return 1;
}

char* noPrototype(){
    char* myTab = malloc(sizeof(char)*25);
    char tab[25] = "I love green grass";
    int i;
    int max = strlen(tab);
    
    for(i=0;i<max;i++){
        myTab[i] = tab[i];
    }
    myTab[max] = '\0';
    
    return myTab;
}