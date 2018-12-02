#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

typedef struct lineLevels lineLevels;
struct lineLevels{
    int identifier;
    int startLevel;
    int startingLine;
    int endLevel;
    int endingLine;
    char*** declaredVars;           // int, float, char, double, long, struct
    int* nbVars;
    lineLevels* sonStructs;
    int nbSons;
};

int testDisplay = 0;

// Recuperation contenu d'un fichier
char* getFileContent(char* path);

// Récupération des valeurs des règles dans les fichiers de configuration
int* getConfRules(char* fileContent);
int* agregateRulesValues(int nbExtend, char** fileNames);

// Récupération des héritages d'un fichier de configuration
int getNbFilesExtended(char* fileContent);
char** getConfExtends(char* fileContent);

// Récupération de la chaine de nom de fichiers de configuration (récursif)
int getNbExtendLayer(char* fileContent);
char** getAllExtendedFiles(char* currFileName, char* fileContent, int nbExtends);

// Récupération des fichiers exclus du traitement par les fichiers de configuration
int getNbFilesExcluded(char* fileContent);
int agregateNbFilesExcluded(int nbExtend, char** fileNames);
char** getConfExcluded(char* fileContent);
char** agregateConfExcluded(int nbExtend, char** fileNames, int nbFilesExcluded);

// Récupération de la valeur du critère récursif dans les fichiers de configuration
int isRecursive(char* fileContent);
int agregateIsRecursive(int nbExtend, char** fileNames);
void dispDirContent(char* path, int searchType, int nbFilesExcluded, char** excludedFiles, int typeExec, int* rulesValues);

// Fonctions relatives au parsing d'un fichier de code source
int getNbPrimaryLevels(char* fileContent);
lineLevels* getAllLevels(char* fileContent, int nbPrimaries);

int getNbInsiderLevels(char* fileContent, int startInd, int endInd);
lineLevels* getInsidersLevels(char* fileContent, int nbPrimaries, lineLevels motherStruct);

int* levelVarCount(char* fileContent, lineLevels currStruct);
int countVarsOnLine(char* fileContent, int startInd);

char*** levelVarStorage(char* fileContent, lineLevels currStruct);
char** extractVarsOfLine(char* fileContent, int startInd, int nbVarsOnLine, int lineNumber);

void freeAllStructs(lineLevels* primaryStructs, int nbStructs);

// Fonctions simple de traitement spécifique
// (isText = vérif code ASCII d'un char)
// dispErrMessg + isExistingFile = vérif d'existence (et de présence de contenu) et affichage d'erreur en fonction
int isText(char c);
int isExistingFile(char* path);
int dispErrMessg(char* fileName, int typeOf, int dispChx);

// Fonctions relatives aux vérifications des règles
void verifSourceCode(char* path, int* rulesValues);
void onlyOneDeclar(char* line);
int lookForType(char* fileContent, int startInd);


//! Parsing
//! Mettre en place des structures :
//!      chaque structure correspond à un "niveau"
//!      chaque structure correspond à un intervalle de lignes relatifs au niveau
//!      chaque structure peut contenir un taleau de structures filles (si niveau contenu dans un autre niveau)
//!      En résumé : struct linesLevels
//!                  int startLevel = X;
//!                  int endLevel = X;
//!                  char*** declaredVars;   <- Type(int, double, ...) -> nom de variable
//!                  linesLevels daughters = {..., ..., ...}
//!                  int nbDaughters = X;
//!
//!      La lecture du fichier commencera toujours par déterminer tous les niveaux (lecture simple sans traitement)
//!          Chaque déclaration de variable dans un niveau est stocké dans le tableau de var du niveau
//!      Les repérages se feront via une itération sur les structures principales (orphelines)
//!          Une fois un intervalle correspondant trouvé, on continue sur ses filles jusqu'à trouver le dernier niveau
//!
//!  Formation des enfants :
//!      Repérage basé sur les accolades
//!          - ouvrantes = création de structure fille (création puis création puis ...)
//!          - fermantes = fermeture de la structure courante, on remonte a son parent (etc...)
//!
//!  Recuperation des variables de chaque structure :
//!      Déclaration : ajout au tableau de variables de la structure courante
//!      Utilisation : vérification de déclaration dans les structures parentes jusqu'à la structure courante
//!                          ** Commence par vérifier les variables globales puis le main puis, ensuite, une structure
//!      Assignation Type : partie la plus compliqué, repérage du type déclaré pour chacune des variables et comparaison


/*
    LES REGLES
---------------------------------------------------------------
    PARTIE I : Valeur référencée dans int* rulesValues
---------------------------------------------------------------
    Accolades sur la meme ligne que la condition        0
- array-bracket-eol = off

    Un espace de chaque coté des opérateurs             1
- operators-spacing = off

    Espace a droite de chaque virgule                   2
- comma-spacing = off

    Indentation (nb espaces par indent)                 3
- indent = off

    Commentaire multi lignes en début de fichier        4
- comments-header = off

    Nombre de caracteres maximum par ligne              5
- max-line-numbers = off

    Nombre de lignes maximum par fichier                6
- max-file-line-numbers = off

    Pas d'espace en fin de ligne                        7
- no-trailing-spaces = off
---------------------------------------------------------------

PARTIE II :
---------------------------------------------------------------
    Une seule variable déclarée par ligne               8
- no-multi-declaration = off

    Pas de variable déclarée mais pas utilisée          9
- unused-variable = off

    Pas de variable utilisée mais non déclarée          10
- undeclared-variable = off

    Pas de fonction sans son prototype                  11
- no-prototype = off

    Pas de fonction déclarée mais non utilisée          12
- unused-function = off

    Pas de fonction utilisée mais non déclarée          13
- undeclared-function = off
---------------------------------------------------------------

PARTIE III :
---------------------------------------------------------------
    Une affectation de variable/valeur doit etre fait du même type      14
- variable-assignment-type = off

    Les paramètres reçus par les fonctions doivent avoir le bon type    15
- function-parameters-type = off

*/

int main(int argc, char **argv)
{
    char rulesName[16][50] = {"array-bracket-eol","operators-spacing","comma-spacing","indent","comments-header","max-line-numbers","max-file-line-numbers","no-trailing-spaces","no-multi-declaration","unused-variable","undeclared-variable","no-prototype","unused-function","undeclared-function","variable-assignment-type","function-parameters-type"};

    char initConf[128] = "conf/main.lconf";
    int i;

    // Tableau contenant la valeur de chaque règle
    //!  Les valeurs sont dans l'ordre définis sur le Sujet
    int* rulesValues;

    // Tableaux contenants les chaines de caracteres de chaque nom de fichier exclus et des extensions de conf a prendre en compte, avec leurs nombre associé
    char** excludedFiles;
    char** confExtends;
    int nbFileExcluded;
    int nbFileExtend;

    char* mainConfContent;

    char** extendedFilesName;

    int endProg = 0;
    int stop;
    int userChx;
    int nbRuleActive;
    int userQuit;
    int recursiveState;
    int nbExtendLayer = 0;

    while(endProg == 0){
        stop = 0;
        nbExtendLayer = 0;

        stop = dispErrMessg(initConf, 0, 1);

        if(stop == 0){
            mainConfContent = getFileContent(initConf);

            confExtends = getConfExtends(mainConfContent);
            nbFileExtend = getNbFilesExtended(mainConfContent);
        }

        //! Doit s'executer recursivement sur les extends du fichier (fichier en extend en premier)
        //! nbFileExtend+1

        if(nbFileExtend > 0){
            char* extendConfContent;
            int valueRtrn;
            char extendFilePath[256] = "conf/";

            strcat(extendFilePath,confExtends[0]);

            valueRtrn = dispErrMessg(extendFilePath, 1, 1);

            if(valueRtrn == 1){

                //! Traitement normal de main.lconf sans extension dans le cas ou l'extend est corrompus
                nbFileExcluded = getNbFilesExcluded(mainConfContent);
                excludedFiles = getConfExcluded(mainConfContent);

                rulesValues = getConfRules(mainConfContent);

                recursiveState = isRecursive(mainConfContent);


            }else{
                extendConfContent = getFileContent(extendFilePath);

                //! Nombre de "couche" d'extends, a associer avec un tableau de nom de fichiers en extends (ordre traitement très important)
                nbExtendLayer = getNbExtendLayer(extendConfContent);
                extendedFilesName = getAllExtendedFiles(confExtends[0], extendConfContent, nbExtendLayer);

                rulesValues = agregateRulesValues(nbExtendLayer, extendedFilesName);

                nbFileExcluded = agregateNbFilesExcluded(nbExtendLayer, extendedFilesName);

                excludedFiles = agregateConfExcluded(nbExtendLayer, extendedFilesName, nbFileExcluded);

                recursiveState = agregateIsRecursive(nbExtendLayer, extendedFilesName);

                //! Une fois tout les traitements terminés et les tableaux définis
                free(extendConfContent);
            }
        }else{

            //! Traitement normal de main.lconf si aucune extension repérée
            nbFileExcluded = getNbFilesExcluded(mainConfContent);
            excludedFiles = getConfExcluded(mainConfContent);

            rulesValues = getConfRules(mainConfContent);

            recursiveState = isRecursive(mainConfContent);
        }

        nbRuleActive = 0;
        for(i=0;i<16;i++){
            if(rulesValues[i] > 0){
                nbRuleActive++;
            }
        }

        while(stop == 0){
            system("cls");
            printf("                    ****Linter Project****\n\n");
            printf("                           MENU\n");
            printf("----------------------------------------------------------\n");
            printf("La configuration du Linter se base sur %d fichiers :\n",nbExtendLayer+1);
            printf("- main.lconf\n");
            for(i=0;i<nbExtendLayer;i++){
                printf("- %s\n",extendedFilesName[i]);
            }

            printf("\n%d Regles sont actives :\n",nbRuleActive);
            for(i=0;i<16;i++){
                if(rulesValues[i] > 0){
                    printf("- Numero %d : %s = ",i+1,rulesName[i]);
                }
                if(rulesValues[i] == 1){
                    printf("ON\n");
                }else if(rulesValues[i] > 0){
                    printf("%d\n",rulesValues[i]);
                }
            }
            printf("\n%d Fichiers sont a exclures :\n",nbFileExcluded);
            for(i=0;i<nbFileExcluded;i++){
                printf("- %s\n",excludedFiles[i]);
            }
            printf("\nMode Recherche Recursive : ");
            if(recursiveState){
                printf("ACTIF\n");
            }else{
                printf("INACTIF\n");
            }
            printf("\nVeuillez selectionner une des options de la liste :\n");
            printf("1 - Execution du Linter selon sa configuration\n");
            printf("2 - Lister les fichiers cibler par le Linter\n");
            printf("3 - Execution du Linter sur un fichier test : 'exemple.c'\n");
            printf("0 - QUITTER\n>>");
            userChx = -1;
            while(userChx < 0 || userChx > 3){
                scanf("%d",&userChx);
            }

            switch(userChx){
            case 1:
                system("cls");
                dispDirContent(".", recursiveState, nbFileExcluded, excludedFiles, 1, rulesValues);
                break;

            case 2:
                system("cls");
                dispDirContent(".", recursiveState, nbFileExcluded, excludedFiles, 0, rulesValues);
                break;

            case 3:
                system("cls");
                verifSourceCode("exemple.c", rulesValues);
                break;


            case 0:
                stop = 1;
                break;

            }
        }

        //! Avant de relancer le programme
        //! On libère l'espace associé à nos pointeurs
        for(i = 0;i<nbFileExtend;i++){
            free(confExtends[i]);
        }
        free(confExtends);

        for(i=0;i<nbFileExcluded;i++){
            free(excludedFiles[i]);
        }
        free(excludedFiles);

        free(rulesValues);
        free(mainConfContent);

        free(extendedFilesName);

        printf("\n\n\n\n");
        printf("Voulez vous relancer le programme ?\n");
        printf("1 - Oui\n");
        printf("2 - Non\n>>");

        userQuit = 0;
        while(userQuit < 1 || userQuit > 2){
            scanf("%d",&userQuit);
            printf("\n>>");
        }
        if(userQuit == 2){
            endProg = 1;
        }
    }

    return 0;
}

void freeAllStructs(lineLevels* primaryStructs, int nbStructs){
    int i;
    for(i=0;i<nbStructs;i++){
        if(primaryStructs[i].nbSons > 0){
            freeAllStructs(primaryStructs[i].sonStructs, primaryStructs[i].nbSons);
        }
    }
    free(primaryStructs);
}

//! LA FONCTION DEVRA RECEVOIR TOUTES LES INFOS DE CONFIG EN PARAMETRE AFIN D'EFFECTUER TOUTES LES VERIFS ELLE MEME
void verifSourceCode(char* path, int* rulesValues){
    char* fileContent;

    printf("                    ****Linter Project****\n\n\n");
    printf("Verification du fichier %s....\n\n", path);

    fileContent = getFileContent(path);

    int nbLines = 0;
    int lengthFile;
    int i;

    lengthFile = strlen(fileContent);

    int nbPrimaryLevels = getNbPrimaryLevels(fileContent);
    lineLevels* primaryStructs = getAllLevels(fileContent, nbPrimaryLevels);

    printf("\nLEVELS COUNTED : %d\n\n",nbPrimaryLevels);

    for(i=0;i<lengthFile;i++){
        if(fileContent[i] == '\n'){
            nbLines++;
        }
    }

    // Regle n°6 de nombre de ligne maximum pour un fichier
    if(rulesValues[6] > 0){
        if(nbLines > rulesValues[6]){
            printf("!! Warning Rule 6 : Too many lines for file (%d > %d)\n\n", nbLines, rulesValues[6]);
        }
    }

    printf("Le fichier %s contient %d lignes\n\n", path, nbLines);

    //! TRAITEMENT DU FICHIER LIGNE PAR LIGNE POUR LA PLUPART DES REGLES

    int x;
    int nbChar = 0;
    char line[512];
    for(i=0;i<nbLines+1;i++){
        x = 0;
        printf("%d ",i+1);
        if(i+1 < 100){
            printf(" ");
        }
        if(i+1 < 10){
            printf(" ");
        }


        while(fileContent[nbChar] != '\n' && nbChar < lengthFile){

            //! TRAITEMENT DU CONTENU DE LA LIGNE
            line[x] = fileContent[nbChar];

            x++;
            nbChar++;
        }
        line[x] = '\0';
        nbChar++;
        printf(": %s\n",line);

        // Regle n°5 de nombre de caracteres maximum pour une ligne
        if(rulesValues[5] > 0){
            if(x > rulesValues[5]){
                printf("!! Warning Rule 5 : Too many chars on line  (%d > %d)\n\n", x, rulesValues[5]);
            }
        }

        // Regle n°8 de non declaration multiple de variable sur une ligne
        if(rulesValues[8] > 0){
            onlyOneDeclar(line);
        }
    }


    printf("\n\n\n");
    freeAllStructs(primaryStructs, nbPrimaryLevels);
    free(fileContent);

    system("pause");
}

int countVarsOnLine(char* fileContent, int startInd){
    int nbVars = 0;
    int i = startInd;
    int varStarted = 0;
    int isNotVar = 0;
    int simpleQuoteOpened = 0;
    int doubleQuoteOpened = 0;

    while(fileContent[i] != ';'){
        if(fileContent[i] == 34){
            if(simpleQuoteOpened == 0){
                simpleQuoteOpened = 1;
            }else{
                simpleQuoteOpened = 0;
            }
        }else if(fileContent[i] == 44){
            if(doubleQuoteOpened == 0){
                doubleQuoteOpened = 1;
            }else{
                doubleQuoteOpened = 0;
            }
        }
        if(fileContent[i] == '(' || fileContent[i] == '{' || fileContent[i] == '['){
            isNotVar++;
        }else if(fileContent[i] == ')' || fileContent[i] == '}' || fileContent[i] == ']'){
            isNotVar--;
        }
        if((isText(fileContent[i]) || fileContent[i] == '*') && (fileContent[i-1] == ' ' || fileContent[i-1] == ',') && isNotVar == 0 && varStarted == 0 && simpleQuoteOpened == 0 && doubleQuoteOpened == 0){
            varStarted = 1;
            nbVars++;
        }
        if(varStarted && isNotVar == 0 && fileContent[i] == ','){
            varStarted = 0;
        }
        i++;
    }

    return nbVars;
}

char** extractVarsOfLine(char* fileContent, int startInd, int nbVarsOnLine, int lineNumber){
    int indexVars = 0;

    int i;
    int x;

    int isNotVar = 0;
    int simpleQuoteOpened = 0;
    int doubleQuoteOpened = 0;

    char** varsOnLine = malloc(sizeof(char*)*nbVarsOnLine);
    for(i=0;i<nbVarsOnLine;i++){
        varsOnLine[i] = malloc(sizeof(char)*50);
    }

    i = startInd;
    while(fileContent[i] != ';'){
        if(fileContent[i] == 34){
            if(simpleQuoteOpened == 0){
                simpleQuoteOpened = 1;
            }else{
                simpleQuoteOpened = 0;
            }
        }else if(fileContent[i] == 44){
            if(doubleQuoteOpened == 0){
                doubleQuoteOpened = 1;
            }else{
                doubleQuoteOpened = 0;
            }
        }
        if(fileContent[i] == '(' || fileContent[i] == '{' || fileContent[i] == '['){
            isNotVar++;
        }else if(fileContent[i] == ')' || fileContent[i] == '}' || fileContent[i] == ']'){
            isNotVar--;
        }

        if((isText(fileContent[i]) || fileContent[i] == '*') && (fileContent[i-1] == ' ' || fileContent[i-1] == ',') && isNotVar == 0 && simpleQuoteOpened == 0 && doubleQuoteOpened == 0 && indexVars < nbVarsOnLine){
            x = 0;
            while(isText(fileContent[i+x]) || fileContent[i+x] == '*'){
                varsOnLine[indexVars][x] = fileContent[i+x];
                x++;
            }
            varsOnLine[indexVars][x] = '\0';
            char indicateLine[15];
            sprintf(indicateLine,"-%d",lineNumber);
            strcat(varsOnLine[indexVars],indicateLine);

            indexVars++;
            i = i + x - 1;
        }
        i++;
    }

    return varsOnLine;
}

// Retourne un tableau de valeurs correspondant aux variables
int* levelVarCount(char* fileContent, lineLevels currStruct){
    char types[6][15] = {"int","float","char","double","long","struct"};
    int* tab = malloc(sizeof(int)*6);

    int i;
    int x;
    int z;

    int nbErr;

    int startLevel = currStruct.startLevel;
    int startingLine = currStruct.startingLine;
    int endLevel = currStruct.endLevel;
    int endingLine = currStruct.endingLine;

    for(i=0;i<6;i++){
        tab[i] = 0;
    }

    int nbSons = currStruct.nbSons;
    //! Récupère le nombre de valeurs pour définir les intervalles
    //!     Pour un fils : début et fin, associer ca avec début et fin du niveau parent pour définir intervalles
    int* intervals = malloc(sizeof(int)*nbSons*3);
    if(nbSons > 0){
        for(i=0;i<nbSons;i++){
            intervals[i*3] = currStruct.sonStructs[i].startLevel;
            intervals[i*3+1] = currStruct.sonStructs[i].endLevel;
            intervals[i*3+2] = currStruct.sonStructs[i].endingLine;
        }
    }
    int p = 0;

    int y = startLevel;
    for(i=startingLine;i<endingLine;i++){
        int saveInd = y;
        int isNotFunc = 1;
        while(fileContent[y] != '\n'){
            int searchState = 0;
            if(searchState == 2 && fileContent[y] == '{'){
                isNotFunc = 0;
            }
            if(fileContent[y] == '(' || fileContent[y] == ')'){
                searchState++;
            }
            y++;
        }
        y = saveInd;
        int stop = 1;
        while(fileContent[y] != '\n' && y < endLevel && stop == 1){
            if(y >= intervals[p*3] && p < nbSons && nbSons > 0){
                y = intervals[p*3+1];
                i = intervals[p*3+2] - 1; // Car boucle for donc i++ juste après
                stop = 0;
                p++;
            }
            for(x=0;x<6;x++){
                z = 0;
                nbErr = 0;
                if(types[x][z] == fileContent[y] && (fileContent[y-1] == ' ' || fileContent[y-1] == '\n' || fileContent[y-1] == '\t')){
                    int length = strlen(types[x]);
                    while(z < length){
                        if(fileContent[y+z] != types[x][z]){
                            nbErr++;
                        }
                        z++;
                    }
                    if(fileContent[y+z] != '*' && fileContent[y+z] != ' '){
                        nbErr++;
                    }
                    if(nbErr == 0 && isNotFunc){

                        y = y + z + 1;

                        int nbVarsOnLine = countVarsOnLine(fileContent, y - 1);
                        tab[x] += nbVarsOnLine;

                        x = 6;
                    }
                }
            }
            y++;
        }
        y++;
    }

    return tab;
}

// Retourne un tableau de valeurs correspondant aux variables
char*** levelVarStorage(char* fileContent, lineLevels currStruct){
    char types[6][15] = {"int","float","char","double","long","struct"};

    int i;
    int x;
    int z;

    int nbVars[6];
    for(i=0;i<6;i++){
        nbVars[i] = currStruct.nbVars[i];
    }

    int currNbVars[6] = {0,0,0,0,0,0};

    char*** structVars = malloc(sizeof(char**)*6);
    //! ALLOUER LE TABLEAU 3 DIMENSIONS
    for(i=0;i<6;i++){
        structVars[i] = malloc(sizeof(char*)*nbVars[i]);

        for(x=0;x<nbVars[i];x++){
            structVars[i][x] = malloc(sizeof(char)*50);
        }
    }

    int nbErr;
    int nbVarsOnLine;

    int startLevel = currStruct.startLevel;
    int startingLine = currStruct.startingLine;
    int endLevel = currStruct.endLevel;
    int endingLine = currStruct.endingLine;

    int nbSons = currStruct.nbSons;
    //! Récupère le nombre de valeurs pour définir les intervalles
    //!     Pour un fils : début et fin, associer ca avec début et fin du niveau parent pour définir intervalles
    int* intervals = malloc(sizeof(int)*nbSons*3);
    if(nbSons > 0){
        for(i=0;i<nbSons;i++){
            intervals[i*3] = currStruct.sonStructs[i].startLevel;
            intervals[i*3+1] = currStruct.sonStructs[i].endLevel;
            intervals[i*3+2] = currStruct.sonStructs[i].endingLine;
        }
    }
    int p = 0;

    int y = startLevel;
    for(i=startingLine;i<endingLine;i++){
        int saveInd = y;
        int isNotFunc = 1;
        while(fileContent[y] != '\n'){
            int searchState = 0;
            if(searchState == 2 && fileContent[y] == '{'){
                isNotFunc = 0;
            }
            if(fileContent[y] == '(' || fileContent[y] == ')'){
                searchState++;
            }
            y++;
        }
        y = saveInd;
        int stop = 1;
        while(fileContent[y] != '\n' && y < endLevel && stop == 1){
            if(y >= intervals[p*3] && p < nbSons && nbSons > 0){
                y = intervals[p*3+1];
                i = intervals[p*3+2] - 1; // Car boucle for donc i++ juste après
                stop = 0;
                p++;
            }
            for(x=0;x<6;x++){
                z = 0;
                nbErr = 0;
                if(types[x][z] == fileContent[y] && (fileContent[y-1] == ' ' || fileContent[y-1] == '\n' || fileContent[y-1] == '\t')){
                    int length = strlen(types[x]);
                    while(z < length){
                        if(fileContent[y+z] != types[x][z]){
                            nbErr++;
                        }
                        z++;
                    }
                    if(fileContent[y+z] != '*' && fileContent[y+z] != ' '){
                        nbErr++;
                    }
                    if(nbErr == 0 && isNotFunc){

                        y = y + z + 1;

                        nbVarsOnLine = countVarsOnLine(fileContent, y - 1);

                        char** tmpSavedVars = malloc(sizeof(char*) * nbVarsOnLine);
                        int h;
                        for(h=0;h<nbVarsOnLine;h++){
                            tmpSavedVars[h] = malloc(sizeof(char)*50);
                        }

                        tmpSavedVars = extractVarsOfLine(fileContent, y - 1, nbVarsOnLine, i);

                        for(h=0;h<nbVarsOnLine;h++){
                            strcpy(structVars[x][currNbVars[x]],tmpSavedVars[h]);
                            currNbVars[x]++;

                            free(tmpSavedVars[h]);
                        }
                        free(tmpSavedVars);
                        x = 6;
                    }
                }
            }
            y++;
        }
        y++;
    }

    printf("\n\nStruct %d ALL VARS :\n",currStruct.identifier);
    for(i=0;i<6;i++){
        printf("VARS OF %s = %d :\n",types[i],nbVars[i]);
        for(y=0;y<nbVars[i];y++){
            printf("----> %s\n",structVars[i][y]);
        }
    }
    printf("\n");

    return structVars;
}

int getNbInsiderLevels(char* fileContent, int startInd, int endInd){
    int nbInsiders = 0;
    int onCount = 0;
    int insideBoundaries = 0;
    int nbLine = 0;
    int lineStartInd;
    int i;

    for(i=startInd+1;i<endInd-1;i++){
        if(fileContent[i] == '\n'){nbLine++;lineStartInd=i+1;}
        if(fileContent[i] == '{' && onCount == 0){
            if(lookForType(fileContent, lineStartInd) != 1){
                onCount = 1;
                nbInsiders++;
            }
        }else if(fileContent[i] == '{' && onCount == 1){
            insideBoundaries++;
        }

        if(fileContent[i] == '}' && insideBoundaries > 0){
            insideBoundaries--;
        }else if(fileContent[i] == '}' && onCount == 1 && insideBoundaries == 0){
            onCount = 0;
        }
    }

    return nbInsiders;
}

lineLevels* getInsidersLevels(char* fileContent, int nbPrimaries, lineLevels motherStruct){
    lineLevels* primaryStructs = malloc(sizeof(lineLevels)*nbPrimaries);

    //! TODO : Ranger les structures dans les bons tableaux
    //!         Primaires dans primaryStructs
    //!         Secondaires dans le tableau de structure de sa structure parente

    int onCount = 0;
    int insideBoundaries = 0;
    int i;
    int y = 0;
    int nbLine = 0;
    int lineStartInd;
    int startLevel = motherStruct.startLevel;
    int endLevel = motherStruct.endLevel;
    int currLines = motherStruct.startingLine;
    int structID = 1;

    for(i=startLevel+1;i<endLevel-1;i++){
        if(fileContent[i] == '\n'){nbLine++;lineStartInd=i+1;}
        if(fileContent[i] == '{' && onCount == 0){
            if(lookForType(fileContent, lineStartInd) != 1){
                onCount = 1;
                primaryStructs[y].startLevel = i;
                primaryStructs[y].startingLine = currLines + nbLine;
                primaryStructs[y].identifier = motherStruct.identifier*10+structID;
                structID++;
                printf("--- Struct number %d : started at %d\n",primaryStructs[y].identifier,currLines + nbLine+1);
            }
        }else if(fileContent[i] == '{' && onCount == 1){
            insideBoundaries++;
        }

        if(fileContent[i] == '}' && insideBoundaries > 0){
            insideBoundaries--;
        }else if(fileContent[i] == '}' && onCount == 1 && insideBoundaries == 0){
            onCount = 0;
            primaryStructs[y].endLevel = i;
            primaryStructs[y].endingLine = currLines + nbLine;
            printf("--- Struct number %d : ended at %d\n",primaryStructs[y].identifier,currLines + nbLine+1);


            //! TODO
            //!     Appel de différente fonction
            //!         Décompte nombre de fils (mais pas petit ou arrière petit fils)
            primaryStructs[y].nbSons = getNbInsiderLevels(fileContent, primaryStructs[y].startLevel, primaryStructs[y].endLevel);
            printf("--- Son Struct number %d : %d sons\n",primaryStructs[y].identifier,primaryStructs[y].nbSons);
            //!         Récupération des fils dans le tableau sonStructs de la structure primaire
            primaryStructs[y].sonStructs = getInsidersLevels(fileContent, primaryStructs[y].nbSons, primaryStructs[y]);
            //!         Décompte nombre de vars de chaque type pour allocation (var déclaré sur ce niveau mais pas sur un fils)
            primaryStructs[y].nbVars = levelVarCount(fileContent, primaryStructs[y]);
            //!         Récupération des vars de chaque type dans le tableau declaredVars de la structure primaire
            primaryStructs[y].declaredVars = levelVarStorage(fileContent, primaryStructs[y]);

            y++;
        }
    }

    return primaryStructs;
}

// Retourne un tableau de structure (des majeurs, contenant a l'intérieur d'autre structures filles)
lineLevels* getAllLevels(char* fileContent, int nbPrimaries){
    lineLevels* primaryStructs = malloc(sizeof(lineLevels)*nbPrimaries);

    //! TODO : Ranger les structures dans les bons tableaux
    //!         Primaires dans primaryStructs
    //!         Secondaires dans le tableau de structure de sa structure parente

    int onCount = 0;
    int insideBoundaries = 0;
    int lengthFile;
    int i;
    int y = 0;
    int nbLine = 0;
    int lineStartInd;
    int structID = 1;

    lengthFile = strlen(fileContent);
    for(i=0;i<lengthFile;i++){
        if(fileContent[i] == '\n'){nbLine++;lineStartInd=i+1;}
        if(fileContent[i] == '{' && onCount == 0){
            if(lookForType(fileContent, lineStartInd) != 1){
                onCount = 1;
                primaryStructs[y].startLevel = i;
                primaryStructs[y].startingLine = nbLine;
                primaryStructs[y].identifier = structID;
                structID++;
                printf("\nStruct number %d : started at %d\n",primaryStructs[y].identifier,nbLine+1);
            }
        }else if(fileContent[i] == '{' && onCount == 1){
            insideBoundaries++;
        }

        if(fileContent[i] == '}' && insideBoundaries > 0){
            insideBoundaries--;
        }else if(fileContent[i] == '}' && onCount == 1 && insideBoundaries == 0){
            onCount = 0;
            primaryStructs[y].endLevel = i;
            primaryStructs[y].endingLine = nbLine;
            printf("Struct number %d : ended at %d\n",primaryStructs[y].identifier,nbLine+1);


            //! TODO
            //!     Appel de différente fonction
            //!         Décompte nombre de fils (mais pas petit ou arrière petit fils)
            primaryStructs[y].nbSons = getNbInsiderLevels(fileContent, primaryStructs[y].startLevel, primaryStructs[y].endLevel);
            printf("Struct number %d : %d sons\n",primaryStructs[y].identifier,primaryStructs[y].nbSons);
            //!         Récupération des fils dans le tableau sonStructs de la structure primaire
            primaryStructs[y].sonStructs = getInsidersLevels(fileContent, primaryStructs[y].nbSons, primaryStructs[y]);
            //!         Décompte nombre de vars de chaque type pour allocation (var déclaré sur ce niveau mais pas sur un fils)
            primaryStructs[y].nbVars = levelVarCount(fileContent, primaryStructs[y]);
            //!         Récupération des vars de chaque type dans le tableau declaredVars de la structure primaire
            primaryStructs[y].declaredVars = levelVarStorage(fileContent, primaryStructs[y]);

            y++;
        }
    }

    return primaryStructs;
}

int getNbPrimaryLevels(char* fileContent){
    int nbPrimary = 0;
    int onCount = 0;
    int insideBoundaries = 0;
    int nbLine = 0;
    int lineStartInd;
    int lengthFile;
    int i;

    lengthFile = strlen(fileContent);
    for(i=0;i<lengthFile;i++){
        if(fileContent[i] == '\n'){nbLine++;lineStartInd=i+1;}
        if(fileContent[i] == '{' && onCount == 0){
            if(lookForType(fileContent, lineStartInd) != 1){
                onCount = 1;
            }
        }else if(fileContent[i] == '{' && onCount == 1){
            insideBoundaries++;
        }

        if(fileContent[i] == '}' && insideBoundaries > 0){
            insideBoundaries--;
        }else if(fileContent[i] == '}' && onCount == 1 && insideBoundaries == 0){
            onCount = 0;
            nbPrimary++;
        }
    }

    return nbPrimary;
}

int lookForType(char* fileContent, int startInd){
    char types[6][15] = {"int","float","char","double","long","struct"};
    int i = startInd;
    int y;
    int x;
    int length;
    int nbErr;
    int lineEnd = 0;
    int quoteFinded = 0;

    while(fileContent[i] != '\n'){
        x = 0;
        for(y=0;y<6;y++){
            nbErr = 0;
            if(types[y][x] == fileContent[i] && (fileContent[i-1] == ' ' || fileContent[y-1] == '\n' || fileContent[y-1] == '\t')){
                length = strlen(types[y]);
                while(x < length && nbErr == 0){
                    if(types[y][x] != fileContent[i+x]){
                        nbErr++;
                    }
                    if(x == length-1 && nbErr == 0){
                        if(fileContent[i+x+1] != ' ' && fileContent[i+x+1] != '*'){
                            nbErr++;
                        }
                    }
                    x++;
                }
                if(nbErr == 0){
                    while(lineEnd == 0){
                        if(fileContent[i] == '\n'){
                            lineEnd++;
                        }else if(fileContent[i] == ';'){
                            quoteFinded++;
                        }
                        i++;
                    }
                    if(quoteFinded == 1){
                        return 1;
                    }else{
                        return 2;
                    }
                }
            }
        }
        i++;
    }
    return 0;
}

char* getFileContent(char* path){
    FILE *config = fopen(path,"r");
    int i;
    int x;

    fseek(config, 0, SEEK_END);
    int lengthFile = ftell(config);
    fseek(config, 0, SEEK_SET);

    char* fileContent = malloc(sizeof(char)*(lengthFile+1));
    unsigned char tmpCar;

    i = x = 0;
    while(i < lengthFile){
        tmpCar = fgetc(config);
        if(tmpCar != '\0'){
            fileContent[x] = tmpCar;
            x++;
        }
        i++;
    }

    fileContent[x] = '\0';

    fclose(config);

    if(testDisplay){printf("%d char in conf file :\n%s<ENDFILE>\n",lengthFile,fileContent);}

    return fileContent;
}

char** getConfExtends(char* fileContent){
    char** extendsFile = NULL;

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
            if(testDisplay){printf("=extends SPOTTED\n");}
            searchingState = 1;
            startInd = i+8;
            i = i + 7;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            endInd = i;
            searchingState = 0;
            if(testDisplay){printf("=extends ENDED\n");}
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

    if(nbFiles > 0){
        extendsFile = malloc(sizeof(char*)*(nbFiles+1));
        for(i=0;i<nbFiles+1;i++){
            extendsFile[i] = malloc(sizeof(char)*256);
        }

        j = 0;
        for(i=startInd;i<endInd;i++){
            if(fileContent[i] != '\n'){
                x = 0;

                if(testDisplay){printf("\n\nExtendFile starting line\n");}

                // On isole le nom du fichier
                while(fileContent[i+x] != '\n'){
                    extendsFile[j][x] = fileContent[i+x];
                    x++;
                }
                extendsFile[j][x] = '\0';
                if(testDisplay){printf("ExtendFile : %s spotted\n",extendsFile[j]);}

                i = i + x;
                j++;
            }
        }
    }

    return extendsFile;
}

int* getConfRules(char* fileContent){
    // Noms de chaque règles, permet comparaison pour repérage des règles en cours de traitement
    char rulesName[16][50] = {"array-bracket-eol","operators-spacing","comma-spacing","indent","comments-header","max-line-numbers","max-file-line-numbers","no-trailing-spaces","no-multi-declaration","unused-variable","undeclared-variable","no-prototype","unused-function","undeclared-function","variable-assignment-type","function-parameters-type"};

    int* tab = malloc(sizeof(int)*16);
    int currRule;
    int ruleValue;
    int i;
    int j;
    int x;

    //! Initialisation a -1 pour repérer les règles non définis dans le fichier de config lu
    for(i=0;i<16;i++){
        tab[i] = -1;
    }

    if(testDisplay){printf("Starting getConfRules\n");}

    char* tmpRuleName = NULL;
    int lengthFile = strlen(fileContent);

    // Parcours l'intégralité du contenu du fichier de conf et repère les différentes règles
    int searchingState = 0;
    for(i=0;i<lengthFile;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'r' && fileContent[i+2] == 'u' && fileContent[i+3] == 'l' && fileContent[i+4] == 'e' && fileContent[i+5] == 's'){
            if(testDisplay){printf("=rules spotted\n");}
            searchingState = 1;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            if(testDisplay){printf("Ending =rules\n");}
            searchingState = 0;
        }

        // Si l'on se positionne sur la partie des règles (=rules)
        if(searchingState == 1){
            x = 0;

            // On repère un début de ligne de règles
            if(fileContent[i] == '-' && fileContent[i+1] == ' '){
                tmpRuleName = malloc(sizeof(char)*50);
                i = i+2;

                if(testDisplay){printf("\n\nRule starting line\n");}

                // On isole le nom de la règle
                while(fileContent[i+x] != ' '){
                    tmpRuleName[x] = fileContent[i+x];
                    x++;
                }
                tmpRuleName[x] = '\0';
                i = i + x + 3;

                if(testDisplay){printf("Rule : %s spotted\n",tmpRuleName);}

                // On retrouve de quelle règle il s'agit
                for(j=0;j<16;j++){
                    if(strcmp(tmpRuleName,rulesName[j]) == 0){
                        currRule = j;
                        j = 16;
                    }
                }

                if(testDisplay){printf("Rule is number %d\n",currRule);}

                // On recupere la valeur associée a la regle dans le cas ou celle ci est différente de OFF (0)
                if((fileContent[i] == 'o' && fileContent[i+1] == 'n') || (fileContent[i] == 'O' && fileContent[i+1] == 'N')){
                    if(testDisplay){printf("Rule is ON\n\n\n");}
                    tab[currRule] = 1;
                }else if((fileContent[i] == 'o' && fileContent[i+1] == 'f' && fileContent[i+2] == 'f') || (fileContent[i] == 'O' && fileContent[i+1] == 'F' && fileContent[i+2] == 'F')){
                    if(testDisplay){printf("Rule is OFF\n\n\n");}
                    tab[currRule] = 0;
                }else{
                    x = 0;
                    ruleValue = 0;
                    while(fileContent[i+x] != '\n'){
                        ruleValue *= 10;
                        ruleValue = ruleValue + fileContent[i+x] - 48;
                        x++;
                    }

                    if(testDisplay){printf("Rule is ON with %d as value\n\n\n",ruleValue);}

                    tab[currRule] = ruleValue;
                }
            }

            free(tmpRuleName);
        }
    }

    if(testDisplay){printf("Ending getConfRules\n");}

    if(testDisplay){
        printf("\n\nEtat des 16 regles :\n");
        for(i=0;i<16;i++){
            printf("%d - %s --> %d\n",i,rulesName[i],tab[i]);
        }
    }

    return tab;
}

int* agregateRulesValues(int nbExtend, char** fileNames){
    int* tab = malloc(sizeof(int)*16);
    int* tabSpec;
    int i;
    int x;

    char* fileContent;
    char* tmpName;

    for(i=0;i<16;i++){
        tab[i] = 0;
    }

    for(i=nbExtend-1;i>=0;i--){
        tmpName = malloc(sizeof(char) * 256);
        strcpy(tmpName, "conf/");
        strcat(tmpName, fileNames[i]);

        fileContent = getFileContent(tmpName);
        tabSpec = getConfRules(fileContent);

        for(x=0;x<16;x++){
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

    for(x=0;x<16;x++){
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
            if(testDisplay){printf("=excluded spotted : %d\n",i);}
            startInd = i+9;
            searchingState = 1;
            i = i + 8;
        }else if(fileContent[i] == '=' && fileContent[i+1] != ' '){
            if(searchingState){
                if(testDisplay){printf("Ending =excluded : %d\n",i);}
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

    if(testDisplay){printf("\n\nNb ExcludedFile spotted : %d\n",nbFiles);}

    excludedFiles = malloc(sizeof(char*)*(nbFiles+1));
    for(i=0;i<nbFiles+1;i++){
        excludedFiles[i] = malloc(sizeof(char)*256);
    }

    j = 0;
    for(i=startInd;i<endInd;i++){
        if(fileContent[i] == '-' && fileContent[i+1] == ' '){
            i = i+2;
            x = 0;

            if(testDisplay){printf("\n\nExcludedFile starting line\n");}

            // On isole le nom de la règle
            while(fileContent[i+x] != '\n'){
                excludedFiles[j][x] = fileContent[i+x];
                x++;
            }
            excludedFiles[j][x] = '\0';
            if(testDisplay){printf("ExcludedFile : %s spotted\n",excludedFiles[j]);}

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

void dispDirContent(char* path, int searchType, int nbFilesExcluded, char** excludedFiles, int typeExec, int* rulesValues){
    DIR * rep = opendir(path);
    int lengthName;
    int i;

    if (rep != NULL)
    {
        struct dirent * ent;

        printf("\n------------\n");
        while ((ent = readdir(rep)) != NULL)
        {
            if(strcmp(ent->d_name,".") != 0 && strcmp(ent->d_name,"..") != 0){
                lengthName = strlen(ent->d_name);

                int isFile = 0;
                for(i=0;i<lengthName;i++){
                    if(ent->d_name[i] == '.'){
                        isFile++;
                    }
                }

                int iniPos = 0;
                char tellFolder[256] = "Src";
                if(strcmp(path,".") == 0){
                    iniPos = 1;
                }else{
                    strcpy(tellFolder,path);
                }
                if(isFile){
                    int isExcluded = 0;
                    for(i=0;i<nbFilesExcluded;i++){
                        if(strcmp(ent->d_name, excludedFiles[i]) == 0){
                            isExcluded++;
                        }
                    }

                    int lengthFileName = strlen(ent->d_name);
                    int isCFile = 0;
                    if(ent->d_name[lengthFileName-2] == '.' && ent->d_name[lengthFileName-1] == 'c'){
                        isCFile++;
                    }

                    if(isCFile > 0){
                        if(isExcluded > 0){
                            if(typeExec == 0){
                                printf("File Excluded From %s : %s\n", tellFolder, ent->d_name);
                            }
                        }else{
                            if(typeExec == 0){
                                printf("File NOTExcluded From %s : %s\n", tellFolder, ent->d_name);
                            }else{
                                char tmpFilePath[150] = "";
                                if(strcmp(tellFolder,"Src") != 0){
                                    strcat(tmpFilePath,tellFolder);
                                    strcat(tmpFilePath,"/");
                                }
                                strcat(tmpFilePath,ent->d_name);
                                system("cls");
                                printf("Linter in progress\nFile %s, push any key to continue.\n",tmpFilePath);
                                system("pause");
                                system("cls");
                                verifSourceCode(tmpFilePath, rulesValues);
                            }
                        }
                    }
                }else if(searchType){
                    if(iniPos){
                        dispDirContent(ent->d_name, 1, nbFilesExcluded, excludedFiles, typeExec, rulesValues);
                    }else{
                        char newPath[256] = "";
                        strcpy(newPath, path);
                        strcat(newPath, "/");
                        strcat(newPath, ent->d_name);

                        dispDirContent(newPath, 1, nbFilesExcluded, excludedFiles, typeExec, rulesValues);
                    }
                }
            }
        }

        closedir(rep);
    }
}

// Renvoi 1 si le char est un chiffre ou une lettre (maj et min)
int isText(char c){
    if((c < 123 && c > 96) || (c < 91 && c > 64) || (c < 58 && c > 47)){
        return 1;
    }else{
        return 0;
    }
}

void onlyOneDeclar(char* line){
    char types[6][10] = {"int","float","double","char","long","struct"};
    char tmp[20];
    int varTyped = 0;
    int varFinded = 0;
    int i;
    int warned = 0;

    int lengthLine = strlen(line);

    int isNotFunc = 1;
    int isNotTab = 1;
    for(i=0;i<lengthLine;i++){
        if(line[i] == '(' || line[i] == ')'){
            isNotFunc = 0;
        }
        if(line[i] == '[' || line[i] == ']'){
            isNotTab = 0;
        }
    }

    // On ignore l'indentation
    i = 0;
    while(line[i] == ' '){
        i++;
    }

    // On autorise pour le moment seulement des declaration de type en minuscule (int, char, double, ...)
    int x = 0;
    while(isText(line[i])){
        tmp[x] = line[i];
        x++;
        i++;
    }
    tmp[x] = '\0';

    for(i=0;i<6;i++){
        if(strcmp(tmp, types[i]) == 0){
            varTyped = 1;
        }
    }

    if(varTyped && isNotFunc && isNotTab){
        while(i < lengthLine && isText(line[i])){
            varFinded = 1;
            i++;
        }
        if(varFinded){
            for(x=i-1;x<lengthLine;x++){
                if(line[x] == ','){
                    if(warned == 0){
                        printf("!! Warning Rule 8 : Multi Declaration spotted\n\n");
                        warned++;
                    }
                }
            }
        }
    }
}

int isRecursive(char* fileContent){
    int lengthContent = strlen(fileContent);
    int i;
    int searchingState = 0;

    for(i=0;i<lengthContent;i++){
        if(fileContent[i] == '=' && fileContent[i+1] == 'r' && fileContent[i+2] == 'e' && fileContent[i+3] == 'c' && fileContent[i+4] == 'u' && fileContent[i+5] == 'r' && fileContent[i+6] == 's' && fileContent[i+7] == 'i' && fileContent[i+8] == 'v' && fileContent[i+9] == 'e'){
            if(testDisplay){printf("=recursive spotted : %d\n",i);}
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

int isExistingFile(char* path){
    FILE *config = fopen(path,"r");

    if(config == NULL){
        return -1;
    }else{
        fseek(config, 0, SEEK_END);
        int lengthFile = ftell(config);
        fseek(config, 0, SEEK_SET);

        if(lengthFile < 20){
            return 0;
        }
        fclose(config);
    }

    return 1;
}

int dispErrMessg(char* fileName, int typeOf, int dispChx){
    int stateExist;

    stateExist = isExistingFile(fileName);

    if(stateExist < 1){
        if(dispChx){
            system("cls");
            printf("                    ****Linter Project****\n\n");
        }
        if(stateExist == 0){
            if(typeOf && dispChx){
                printf("-------------------------------\nWARNING MESSAGE : CONF FILE %s IS EMPTY.\n-------------------------------\n\n\n",fileName);
            }else if(dispChx){
                printf("-------------------------------\nFATAL ERROR : MAIN CONF FILE %s IS EMPTY.\n-------------------------------\n\n\n",fileName);
            }
        }else{
            if(typeOf && dispChx){
                printf("-------------------------------\nWARNING MESSAGE : CONF FILE %s DOESNT EXIST.\n-------------------------------\n\n\n",fileName);
            }else if(dispChx){
                printf("-------------------------------\nFATAL ERROR : MAIN CONF FILE %s DOESNT EXIST.\n-------------------------------\n\n\n",fileName);
            }
        }
        if(dispChx){
            system("pause");
        }
        return 1;
    }

    return 0;
}

char** getAllExtendedFiles(char* currFileName, char* fileContent, int nbExtends){
    char** confExtends;
    char* extendContent;
    int i;

    char** allExtendedFiles;

    allExtendedFiles = malloc(sizeof(char*) * (nbExtends+1));
    for(i=0;i<nbExtends;i++){
        allExtendedFiles[i] = malloc(sizeof(char) * 128);
    }

    strcpy(allExtendedFiles[0], currFileName);

    confExtends = getConfExtends(fileContent);

    for(i=1;i<nbExtends;i++){
        char tmpName[256] = "conf/";

        if(i > 1){
            strcat(tmpName, confExtends[0]);
            free(confExtends);
            extendContent = getFileContent(tmpName);

            confExtends = getConfExtends(extendContent);
            free(extendContent);
        }
        strcpy(allExtendedFiles[i],confExtends[0]);
    }
    free(confExtends);

    return allExtendedFiles;
}

// Démarre a 1 car appelé par un fichier en extend et non par le main.lconf
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
        }
    }
    free(confExtends);

    return nbLayers;
}
