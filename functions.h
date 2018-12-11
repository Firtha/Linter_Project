typedef struct lineLevels lineLevels;
struct lineLevels{
    int identifier;
    int levelNumber;

    int startLevel;
    int startingLine;
    int endLevel;
    int endingLine;
    char*** declaredVars;           // int, float, char, double, long, struct
    int* nbVars;
    lineLevels* sonStructs;
    int nbSons;

    char*** globalVars;
    int* nbGlobal;

    int dadAddr;
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

int getLineOfVar(char* varName);
char* extractNameFromNameVar(char* varName);

int* getNbGlobalVars(char* fileContent, lineLevels* primaryStructs, int nbPrimaries);
char*** getGlobalVars(char* fileContent, lineLevels* primaryStructs, int nbPrimaries, int* nbVars);

char*** levelVarStorage(char* fileContent, lineLevels currStruct);
char** extractVarsOfLine(char* fileContent, int startInd, int nbVarsOnLine, int lineNumber);

lineLevels assignGlobalTabsInSons(lineLevels currStruct, char*** globalVars, int* nbGlobal);

void freeAllStructs(lineLevels* primaryStructs, int nbStructs);

// Fonctions simple de traitement spécifique
// (isText = vérif code ASCII d'un char)
// dispErrMessg + isExistingFile = vérif d'existence (et de présence de contenu) et affichage d'erreur en fonction
int isText(char c);
int isExistingFile(char* path);
int dispErrMessg(char* fileName, int typeOf, int dispChx);

// Fonctions relatives aux vérifications des règles
void verifSourceCode(char* path, int* rulesValues);
int lookForType(char* fileContent, int startInd);

int* getMultiDeclarOnLine(char* fileContent, lineLevels* primaryStructs, int nbPrimaryLevels, int nbLines);
int* structNbVarsOnLines(lineLevels currStruct, int* tabOfLines);

int* getVarsDeclaredButUnused(char* fileContent, int nbLines, lineLevels* primaryStructs, int nbPrimaries);
int checkIfVarUsedInStructAndSons(char* fileContent, char* nameVar, lineLevels* primaryStructs, int nbPrimaries, int isGlobal, int levelNumber, int identifier);

// Fonctions de reperage de deuxieme declaration d'une variable
int* getLinesOfAlreadyDeclaredVars(char* fileContent, int nbLines, lineLevels* primaryStructs, int nbPrimaries);
int* goToStructSonsForCheck(lineLevels* primaryStructs, int nbPrimaries, lineLevels currStruct, int* tabOfLines, int chxExec, char* fileContent);

int checkIfGlobalExistInGlobal(char*** globalVars, int* nbGlobal, int typeIndex, int varIndex);
int checkIfExistInStruct(lineLevels* primaryStructs, int nbPrimaries, lineLevels currStruct, int typeIndex, int varIndex);

lineLevels getStructWithLevelAndIdentifier(int targetLevel, int targetIdentifier, lineLevels* primaryStructs, int nbPrimaries);
lineLevels getSonStructWithLevelAndIdentifier(int targetLevel, int targetIdentifier, lineLevels currStruct, int* checkPointer);

int varAlreadyDeclaredInScope(lineLevels* primaryStructs, int nbPrimaries, char* nameVar);

// Fonctions relatives à la règle 15 (Problèmes d'affectation)

int getAffectWithWrongType(char* line, int currLine, lineLevels* primaryStructs, int nbPrimaries);
int compareWithSonsVar(char* varName, lineLevels currStruct, int currLine);
int compareWithAllVars(char* varName, lineLevels* primaryStructs, int nbPrimaries, int currLine);
int verifIfInteger(char* stringSrc);
char** varExtractFromLine(char* line, lineLevels* primaryStructs, int nbPrimaries, int* nbGets, int currLine);

// Fonctions règles P2
int operatorSpacing(char* line);
int bracketEndOfLine(char* line);
int verifyComment(char* line);
int spaceBeforeChariot(char* line);
int commaSpacing(char* line);