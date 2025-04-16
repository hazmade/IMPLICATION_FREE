#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_FORMULA 100
#define MAX_VARIABLES 26

// Estructura para representar una subfórmula
typedef struct {
    char op;
    int left;
    int right;
} FormulaEntry;

FormulaEntry formulaMatrix[MAX_FORMULA];
int formulaSize = 0;

char variableMap[MAX_VARIABLES];
int variableCount = 0;

bool parseError = false;
int binaryOperatorsUsed = 0;

int getVariableIndex(char var) {
    for (int i = 0; i < variableCount; i++) {
        if (variableMap[i] == var) return i;
    }
    if (variableCount < MAX_VARIABLES) {
        variableMap[variableCount] = var;
        return variableCount++;
    }
    return -1;
}

int insertFormula(char op, int left, int right) {
    if (formulaSize < MAX_FORMULA) {
        formulaMatrix[formulaSize].op = op;
        formulaMatrix[formulaSize].left = left;
        formulaMatrix[formulaSize].right = right;
        return formulaSize++;
    }
    return -1;
}

bool areParenthesesBalanced(const char *formula) {
    int count = 0;
    while (*formula) {
        if (*formula == '(') count++;
        else if (*formula == ')') count--;
        if (count < 0) return false;
        formula++;
    }
    return count == 0;
}

int parseFormula(const char *formula, int *pos) {
    while (formula[*pos] == ' ') (*pos)++;

    if (formula[*pos] == '(') {
        (*pos)++;
        if (formula[*pos] == ')') {
            parseError = true;
            return -1;
        }

        int left = parseFormula(formula, pos);
        while (formula[*pos] == ' ') (*pos)++;

        char op = formula[*pos];
        if (op == '\0' || op == ')') {
            parseError = true;
            return -1;
        }
        (*pos)++;

        if (op == '-') {
            if (formula[*pos] != '>') {
                parseError = true;
                return -1;
            }
            (*pos)++;
            op = '>';
        }

        while (formula[*pos] == ' ') (*pos)++;
        int right = parseFormula(formula, pos);

        if (formula[*pos] != ')') {
            parseError = true;
            return -1;
        }
        (*pos)++;

        if (op == '&' || op == 'v' || op == '>') binaryOperatorsUsed++;
        return insertFormula(op, left, right);
    }
    else if (formula[*pos] == '~') {
        (*pos)++;
        int operand = parseFormula(formula, pos);
        return insertFormula('~', operand, -1);
    }
    else if (isalpha(formula[*pos])) {
        int varIndex = getVariableIndex(formula[*pos]);
        (*pos)++;
        return insertFormula('V', varIndex, -1);
    }

    parseError = true;
    return -1;
}

void convertImplicationFree() {
    int originalSize = formulaSize;
    for (int i = 0; i < originalSize; i++) {
        if (formulaMatrix[i].op == '>') {
            int negLeft = insertFormula('~', formulaMatrix[i].left, -1);
            int newOr = insertFormula('v', negLeft, formulaMatrix[i].right);
            formulaMatrix[i].op = formulaMatrix[newOr].op;
            formulaMatrix[i].left = formulaMatrix[newOr].left;
            formulaMatrix[i].right = formulaMatrix[newOr].right;
        }
    }
}

void removeDoubleNegations() {
    for (int i = 0; i < formulaSize; i++) {
        while (formulaMatrix[i].op == '~') {
            int target = formulaMatrix[i].left;
            if (formulaMatrix[target].op == '~') {
                formulaMatrix[i] = formulaMatrix[formulaMatrix[target].left];
            } else {
                break;
            }
        }
    }
}

void applyDeMorgan() {
    for (int i = 0; i < formulaSize; i++) {
        if (formulaMatrix[i].op == '~') {
            int target = formulaMatrix[i].left;
            if (formulaMatrix[target].op == '&') {
                formulaMatrix[i].op = 'v';
                formulaMatrix[i].left = insertFormula('~', formulaMatrix[target].left, -1);
                formulaMatrix[i].right = insertFormula('~', formulaMatrix[target].right, -1);
            } else if (formulaMatrix[target].op == 'v') {
                formulaMatrix[i].op = '&';
                formulaMatrix[i].left = insertFormula('~', formulaMatrix[target].left, -1);
                formulaMatrix[i].right = insertFormula('~', formulaMatrix[target].right, -1);
            }
        }
    }
}

void distributeDisjunctions() {
    for (int i = 0; i < formulaSize; i++) {
        if (formulaMatrix[i].op == 'v') {
            int left = formulaMatrix[i].left;
            int right = formulaMatrix[i].right;
            if (formulaMatrix[left].op == '&') {
                int newLeft = insertFormula('v', formulaMatrix[left].left, right);
                int newRight = insertFormula('v', formulaMatrix[left].right, right);
                formulaMatrix[i].op = '&';
                formulaMatrix[i].left = newLeft;
                formulaMatrix[i].right = newRight;
            } else if (formulaMatrix[right].op == '&') {
                int newLeft = insertFormula('v', left, formulaMatrix[right].left);
                int newRight = insertFormula('v', left, formulaMatrix[right].right);
                formulaMatrix[i].op = '&';
                formulaMatrix[i].left = newLeft;
                formulaMatrix[i].right = newRight;
            }
        }
    }
}

void printFormula(int index) {
    if (index == -1) return;
    if (formulaMatrix[index].op == '~') {
        printf("~");
        printFormula(formulaMatrix[index].left);
    } else if (formulaMatrix[index].op == 'V') {
        printf("%c", variableMap[formulaMatrix[index].left]);
    } else {
        printf("(");
        printFormula(formulaMatrix[index].left);
        printf(" %c ", formulaMatrix[index].op);
        printFormula(formulaMatrix[index].right);
        printf(")");
    }
}

int main() {
    char formula[MAX_FORMULA];
    printf("Ingrese la formula  ");
    scanf(" %[^\n]]", formula);

    if (!areParenthesesBalanced(formula)) {
        printf("Error:parentesis mal \n");
        return 1;
    }

    int pos = 0;
    int root = parseFormula(formula, &pos);

    if (parseError || root == -1 || formula[pos] != '\0') {
        printf("Error: la formula no esta bien \n");
        return 1;
    }

    if (binaryOperatorsUsed == 0) {
        printf("No puedo convertirla a CNF.\n");
        return 1;
    }

    convertImplicationFree();
    removeDoubleNegations();
    applyDeMorgan();
    distributeDisjunctions();

    printf("Fórmula en CNF:\n");
    printFormula(root);
    printf("\n");

    return 0;
}