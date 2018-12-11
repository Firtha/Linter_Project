// Alo

#include <stdio.h>
#include <stdlib.h>

int globalVar;
double globalNumber;
int stateVar;

int returnVar();

globalVar = stateVar + globalNumber;

int main(int argc,char** argv){
    char tab[6][15] = {"Hello","World","I","feel","really","great"}, tabBis[6] = "Hello";
    
    printf("Hello World !");
    
    char* oneWord;
    int globalVar;
    char testingGlobal;
    nbChar = 0;
    
    int i;
    int y, x;
    for(i=0;i<15;i++)
    {
        char* oneWord = malloc(sizeof(char)*15);
        char moreWord = 'c';
        int rightTest;
        if(undeclaredVar){
            printf("Alriiight...");
            int testingDeclar;
            int moreTesting;
            char moreWord;
        }
        printf("Spacing test"); 
        if(y){
        	printf("y used !");
        }
    }
    
    return 0;
}

char testingGlobal;
char* testingGlobal;

int returnVar(){
    int returnedVal = 1;
    char testingGlobal;
    return returnedVal;
}

double globalNumber;

char* noPrototype(){
    char* myTab = malloc(sizeof(char)*25);
    char tab[25] = "I love green grass";
    int i;
    int max = strlen(tab);
    
    for(i=0;i<max;i++){
        int cptTest;
        myTab[i] = tab[i];
        if(cptTest){
            int anotherTry;
            double globalNumber;
        }
    }
    myTab[max] = '\0';
    
    return myTab;
}

int isRecursive(char* fileContent){
    int lengthContent = strlen(fileContent);
    int i;
    int searchingState = 0;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'r' && fileContent[i+2] == 'e' && fileContent[i+3] == 'c' && fileContent[i+4] == 'u' && fileContent[i+5] == 'r' && fileContent[i+6] == 's' && fileContent[i+7] == 'i' && fileContent[i+8] == 'v' && fileContent[i+9] == 'e'){
            searchingState = 1;
            i = i + 9;
        }

        if(searchingState){
            if(fileContent[i] == 't' && fileContent[i+1] == 'r' && fileContent[i+2] == 'u' && fileContent[i+3] == 'e'){
                return 1;
            }else if(fileContent[i] == 'f' && fileContent[i+1] == 'a' && fileContent[i+2] == 'l' && fileContent[i+3] == 's' && fileContent[i+4] == 'e'){
                return 0;
            }
        }
    }

    return -1;
}

int agregateIsRecursive(int nbExtend, char** fileNames){
    char* fileContent;
    char* tmpName;

    int i;
    int recursiveState = 0;
    int tmpState;

    for(i=nbExtend-1;i>=0;i--){
        tmpName = malloc(sizeof(char) * 256);
        strcpy(tmpName, "conf/");
        strcat(tmpName, fileNames[i]);

        fileContent = getFileContent(tmpName);

        tmpState = isRecursive(fileContent);
        if(tmpState > -1){
            recursiveState = tmpState;
        }

        free(tmpName);
        free(fileContent);
    }

    fileContent = getFileContent("conf/main.lconf");

    tmpState = isRecursive(fileContent);
    if(tmpState > -1){
        recursiveState = tmpState;
    }

    free(fileContent);

    return recursiveState;
}

int getNbExtendLayer(char* fileContent){
    char** confExtends;
    char* extendContent;
    int nbFileExtend;
    int nbLayers = 1;


    confExtends = getConfExtends(fileContent);
    nbFileExtend = getNbFilesExtended(fileContent);

    while(nbFileExtend > 0){
        char tmpName[256] = "conf/";
        int fileState;

        strcat(tmpName, confExtends[0]);
        fileState = dispErrMessg(tmpName, 1, 0);
        if(fileState == 1){
            return nbLayers;
        }else{
            free(confExtends);
            extendContent = getFileContent(tmpName);

            confExtends = getConfExtends(extendContent);
            nbFileExtend = getNbFilesExtended(extendContent);
            free(extendContent);

            nbLayers++;

            int nbLayers;
        }
    }
    free(confExtends);

    return nbLayers;
}

char** getConfExtends(char* fileContent){
    char** extendsFile = NULL;

    int uselessVar;

    int lengthContent = strlen(fileContent);
    int i;
    int j;
    int x;
    int startInd;
    int endInd;
    int isFilled = 0;
    int nbFiles = 0;
    int searchingState = 0;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'e' && fileContent[i+2] == 'x' && fileContent[i+3] == 't' && fileContent[i+4] == 'e' && fileContent[i+5] == 'n' && fileContent[i+6] == 'd' && fileContent[i+7] == 's'){           if(testDisplay){printf("=excluded spotted : %d\n",i);}
            searchingState = 1;
            startInd = i+8;
            i = i + 7;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            endInd = i;
            searchingState = 0;
            break;
        }

        if(searchingState){
            if(fileContent[i] != '\n'){
                if(isFilled == 0){
                    nbFiles++;
                }
                isFilled = 1;
            }else{
                isFilled = 0;
            }
        }
        int isFilled;
    }

    if(nbFiles > 0){
        extendsFile = malloc(sizeof(char*)*(nbFiles+1));
        for(i=0;i<nbFiles+1;i++){
            extendsFile[i] = malloc(sizeof(char)*256);
        }

        j = 0;
        for(i=startInd;i<endInd;i++){
            if(fileContent[i] != '\n'){
                x = 0;

                // On isole le nom du fichier
                while(fileContent[i+x] != '\n'){
                    extendsFile[j][x] = fileContent[i+x];
                    x++;
                }
                extendsFile[j][x] = '\0';
                i = i + x;
                j++;
            }
        }
    }

    char extendsFile;

    return extendsFile;
}

int* agregateRulesValues(int nbExtend, char** fileNames){
    int* tab = malloc(sizeof(int)*17);
    int* tabSpec;
    int i;
    int x;

    char* fileContent;
    char* tmpName;

    for(i=0;i<17;i++){
        tab[i] = 0;
    }

    for(i=nbExtend-1;i>=0;i--){
        tmpName = malloc(sizeof(char) * 256);
        strcpy(tmpName, "conf/");
        strcat(tmpName, fileNames[i]);

        fileContent = getFileContent(tmpName);
        tabSpec = getConfRules(fileContent);

        for(x=0;x<17;x++){
            if(tabSpec[x] != -1){
                tab[x] = tabSpec[x];
            }
        }

        free(tabSpec);
        free(fileContent);
        free(tmpName);
    }

    fileContent = getFileContent("conf/main.lconf");
    tabSpec = getConfRules(fileContent);

    for(x=0;x<17;x++){
        if(tabSpec[x] != -1){
            tab[x] = tabSpec[x];
        }
    }

    free(tabSpec);
    free(fileContent);

    return tab;
}

int getNbFilesExtended(char* fileContent){
    int nbFiles = 0;
    int isFilled = 0;
    int lengthContent = strlen(fileContent);
    int searchingState = 0;
    int i;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'e' && fileContent[i+2] == 'x' && fileContent[i+3] == 't' && fileContent[i+4] == 'e' && fileContent[i+5] == 'n' && fileContent[i+6] == 'd' && fileContent[i+7] == 's'){
            searchingState = 1;
            i = i + 7;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            searchingState = 0;
            break;
        }

        if(searchingState){
            if(fileContent[i] != '\n'){
                if(isFilled == 0){
                    nbFiles++;
                }
                isFilled = 1;
            }else{
                isFilled = 0;
            }
        }
    }

    return nbFiles-1;
}

int getNbFilesExcluded(char* fileContent){
    int nbFiles = 0;
    int lengthContent = strlen(fileContent);
    int searchingState = 0;
    int i;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'e' && fileContent[i+2] == 'x' && fileContent[i+3] == 'c' && fileContent[i+4] == 'l' && fileContent[i+5] == 'u' && fileContent[i+6] == 'd' && fileContent[i+7] == 'e' && fileContent[i+8] == 'd'){
            searchingState = 1;
            i = i + 8;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            searchingState = 0;
        }

        if(searchingState){
            if(fileContent[i] == '-' && fileContent[i+1] == ' '){
                nbFiles++;
            }
        }
    }

    return nbFiles;
}

int agregateNbFilesExcluded(int nbExtend, char** fileNames){
    char* fileContent;
    char* tmpName;

    int i;
    int nbFilesExcluded = 0;

    for(i=0;i<nbExtend;i++){
        tmpName = malloc(sizeof(char) * 256);
        strcpy(tmpName, "conf/");
        strcat(tmpName, fileNames[i]);

        fileContent = getFileContent(tmpName);
        nbFilesExcluded += getNbFilesExcluded(fileContent);

        free(fileContent);
        free(tmpName);
    }

    fileContent = getFileContent("conf/main.lconf");

    nbFilesExcluded += getNbFilesExcluded(fileContent);

    free(fileContent);

    return nbFilesExcluded;
}

char** getConfExcluded(char* fileContent){
    char** excludedFiles = NULL;

    int lengthContent = strlen(fileContent);
    int i;
    int j;
    int x;
    int startInd;
    int endInd;
    int nbFiles = 0;
    int searchingState = 0;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'e' && fileContent[i+2] == 'x' && fileContent[i+3] == 'c' && fileContent[i+4] == 'l' && fileContent[i+5] == 'u' && fileContent[i+6] == 'd' && fileContent[i+7] == 'e' && fileContent[i+8] == 'd'){
            startInd = i+9;
            searchingState = 1;
            i = i + 8;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            if(searchingState){
                endInd = i;
            }
            searchingState = 0;
        }

        if(searchingState){
            if(fileContent[i] == '-' && fileContent[i+1] == ' '){
                nbFiles++;
            }
        }
    }

    excludedFiles = malloc(sizeof(char*)*(nbFiles+1));
    for(i=0;i<nbFiles+1;i++){
        excludedFiles[i] = malloc(sizeof(char)*256);
    }

    j = 0;
    for(i=startInd;i<endInd;i++){
        if(fileContent[i] == '-' && fileContent[i+1] == ' '){
            i = i+2;
            x = 0;

            // On isole le nom de la règle
            while(fileContent[i+x] != '\n'){
                excludedFiles[j][x] = fileContent[i+x];
                x++;
            }
            excludedFiles[j][x] = '\0';

            i = i + x;
            j++;
        }
    }

    return excludedFiles;
}

char** agregateConfExcluded(int nbExtend, char** fileNames, int nbFilesExcluded){
    char** listOfFiles;
    char** tmpListFiles;
    char* fileContent;
    char* tmpName;

    int i;
    int y;
    int x = 0;
    int nbExcludOnFile;

    listOfFiles = malloc(sizeof(char*) * nbFilesExcluded);
    for(i=0;i<nbFilesExcluded;i++){
        listOfFiles[i] = malloc(sizeof(char) * 256);
    }

    for(i=0;i<nbExtend;i++){
        tmpName = malloc(sizeof(char) * 256);
        strcpy(tmpName, "conf/");
        strcat(tmpName, fileNames[i]);

        fileContent = getFileContent(tmpName);

        nbExcludOnFile = getNbFilesExcluded(fileContent);
        tmpListFiles = getConfExcluded(fileContent);

        for(y=0;y<nbExcludOnFile;y++){
            strcpy(listOfFiles[x], tmpListFiles[y]);
            x++;
        }

        free(fileContent);
        free(tmpName);
        free(tmpListFiles);
    }

    fileContent = getFileContent("conf/main.lconf");

    nbExcludOnFile = getNbFilesExcluded(fileContent);
    tmpListFiles = getConfExcluded(fileContent);

    for(y=0;y<nbExcludOnFile;y++){
        strcpy(listOfFiles[x], tmpListFiles[y]);
        x++;
    }

    free(fileContent);
    free(tmpListFiles);

    return listOfFiles;
}