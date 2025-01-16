#include "model.h"
#include "interface.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

// #include "model.h"
// #include "interface.h"
// #include <stdlib.h>
// #include <stddef.h>
// #include <string.h>
// #include <stdio.h>
// #include <stdbool.h>
// #include <ctype.h>


//call an enumeration called the equation type and the cell content

enum eqnType{
    OPERATOR, OPERAND, INVALID, REF_CELL,


};


enum cellContent{
    NUM, TXT, BLANK, EQN,
    

};


//structure to represent the excel spreadsheet
struct excelSpreadSheet{
    int row;

    int col;
    
    struct cell** cells;
};


//structure that represent a cell in the excel spreadsheet
struct cell{
    enum cellContent type;

    union{
    
        char* text;
        double number;
    
    }celcontent;

};


//structure that represent equation's elements 
struct equationElemts{

    enum eqnType type;
    
    union{

        char* referenceCell;
    
        char operatorSymbol;
    
        char operator;
    
        double operand;

    
    }celcontent2;


};




struct excelSpreadSheet* spreadsheet = NULL;

//Function that duplicates a string up to a specific length
char *custom_strnduplicate(const char *string, size_t n){

    size_t length = strlen(string);

    char *duplicate = malloc(length+1);

    if(duplicate != NULL){
    
        strncpy(duplicate, string, length);
    
        duplicate[length] = '\0';
    
    }


    return duplicate;



}


//Function that free memory allocated for the equation elements
void freeEqnElmnts(struct equationElemts* elmnt){

    for (size_t i = 0; elmnt[i].type != INVALID; i++){

        if(elmnt[i].type == REF_CELL){
        
            free(elmnt[i].celcontent2.referenceCell);
        
        }
    
    }
    
    
    free(elmnt);
}

//Function for clearing the cell memory of a cell 
void clearCellMemory(struct cell cellVariable){

    if(cellVariable.type == TXT){

        free(cellVariable.celcontent.text);

    }

}

//initialization of the model
void model_init() {

    //Allocate memory for the excel spreadsheet
    int defineCols = 7;
    int defineRows = 10;

    spreadsheet = (struct excelSpreadSheet*)malloc(sizeof(struct excelSpreadSheet));

    spreadsheet->cells = (struct cell**)malloc(defineRows * sizeof(struct cell*));
    
    spreadsheet->row = defineRows;
    
    spreadsheet->col = defineCols;

    //initialize each cell to be blank
    for(int i = 0; i < defineRows; i++){

        spreadsheet->cells[i] = (struct cell*)malloc(defineCols * sizeof(struct cell));
        for (int j = 0; j < defineCols; j++){

            spreadsheet->cells[i][j].type = BLANK;

            spreadsheet->cells[i][j].celcontent.text = NULL;
        }

    }


    
}





//Function that parse an equation and returns its elements
struct equationElemts* parse_eqn(const char* equation){

    size_t currentPosition = 0;

    size_t elementIndex = 0;
    
    size_t eqnLength = strlen(equation);
    
    struct equationElemts* elmnt = (struct equationElemts*)malloc((eqnLength + 1)*sizeof(struct equationElemts));


    while (currentPosition < eqnLength){

        //check for the operator '+' and '-'
        if (equation[currentPosition] == '-' || equation[currentPosition] == '+'){
            elmnt[elementIndex].type = OPERATOR;
            elmnt[elementIndex].celcontent2.operatorSymbol = equation[currentPosition];
            ++currentPosition;
        }

        else if(isalpha(equation[currentPosition])){
            //check for references to another cell
            elmnt[elementIndex].type = REF_CELL;

            //find the end of the cell references
            size_t cellReferenceEnd = currentPosition + 1;
            while(isalpha(equation[cellReferenceEnd])){
                ++cellReferenceEnd;
            }

            //duplicate the reference cell's string
            elmnt[elementIndex].celcontent2.referenceCell = custom_strnduplicate(&equation[currentPosition], (cellReferenceEnd-currentPosition));

            currentPosition = cellReferenceEnd;
        }

        else if(isdigit(equation[currentPosition])){
            //check for numeric operand
            elmnt[elementIndex].type = OPERAND;
            elmnt[elementIndex].celcontent2.operand = atof(&equation[currentPosition]);

            //move to end of numeric operand
            while (equation[currentPosition] == '.' || isdigit(equation[currentPosition])){
                ++currentPosition;
            }
        }

        else{
            //means invalid character in the equation
            elmnt[elementIndex].type = INVALID;
            ++currentPosition;
        }

        ++elementIndex;
    }

    //mark the end of elements 
    elmnt[elementIndex].type = INVALID;
    return elmnt;
}



//Function that converts column letter to index
int columnLetterToIndex(char letter){


    return toupper(letter) - 'A';

}

//Function that converts cell reference to column and row indicies 
void cellReferenceToIndicies(const char* referenceCell, int* row, int* col){
   
    *col = columnLetterToIndex(referenceCell[0]);
    *row = atoi(&referenceCell[1]) - 1;

}

//Function that evaluates an equation's element
double evaluateEqnElmnt(const struct equationElemts element, const struct cell* cells){
    
    int col;
    
    int row;
 
    switch (element.type){

        //evaluate reference to another cell
        case REF_CELL:
            cellReferenceToIndicies(element.celcontent2.referenceCell, &row, &col);
            return cells[col].celcontent.number;

        

        //evaluate subtraction and addition
        case OPERATOR:
            if(element.celcontent2.operatorSymbol == '-'){
                return evaluateEqnElmnt(element, &cells[0]) - evaluateEqnElmnt(element, &cells[1]);
            }

            else if(element.celcontent2.operatorSymbol == '+'){
                return evaluateEqnElmnt(element, &cells[0]) + evaluateEqnElmnt(element, &cells[1]);
            }

         

            break;
        


        case OPERAND:
            //return numeric operand
            return element.celcontent2.operand;

        

        default:
            //default case then return 0.0
            return 0.0;
            break;
    }
}


//Function that evaluates an expression
bool evalExpression(char **express, float *result, int *err){
    char operator =  *express[0];

    //advance(express, 1);

    float leftValue = 0;

    float rightValue;

    //check for non-valid inputs, then *error = 1;
    if(operator != '+' || operator != '-' || operator != '=' ){
        *err = 1;

    }

    //evaluate whether the character is an equal sign
    if(operator == '='){
        *result = leftValue;
        //return evalExpression(express, result, err);

    }

    //evaluate whether the character is an addition sign
    if(operator == '+'){
        *result += leftValue;
        //return evalExpression(express, result, err);

    }

    //evaluate whether the character is a subtraction sign
    if(operator == '-'){
        *result -= leftValue;
        //return evalExpression(express, result, err);


    }



    //If the input is a number
    if(isdigit(*express[0])){

        char *endptr;
        float number = strtof(*express, &endptr);
        if (*endptr == '\0') {
            //it will successfully convert to a number
            leftValue = number;
            //advance(express, 1); //move to the next character after the number
            return evalExpression(express, result, err);
        } else {
            //invalid numeric value, then *error = 1
            *err = 1;
            return false;
        }
    }

   

    //If the expression given is valid, then that means there is no error
    *err = 0;


}


//Function that evaluates an equation and return its result as a string
const char* evaluateEquation(const char* text, const struct cell* cells){
    struct equationElemts* elmnt = parse_eqn(text);

    //check for invalid formula
    if(elmnt == NULL || elmnt[0].type == INVALID){
        return "Error - Formula is invalid";
    }



    //allocation of a static buffer for result
    static char resultString[32];

    //free memory allocated for equation(s) elements
    freeEqnElmnts(elmnt);


    //evaluate equation and format result
    double result = evaluateEqnElmnt(elmnt[0], cells);

    snprintf(resultString, sizeof(resultString), "%0.2f", result);

    return resultString;



}


//Function that sets the value of a cell based on text inputs
void set_cell_value(ROW row, COL col, char *text) {
   
    struct cell *cellVariable2 = &spreadsheet->cells[row][col];

    //clear cell memory
    clearCellMemory(*cellVariable2);

    if (text[0] == '=') {
        //if input starts with '=', then treat it as an equation
        cellVariable2->type = EQN;
        
        free(cellVariable2->celcontent.text);
        
        cellVariable2->celcontent.text = strdup(text);
        
        //evaluate equation and update display
        const char *result = evaluateEquation(cellVariable2->celcontent.text, &spreadsheet->cells[0][0]);
        
        update_cell_display(row, col, result);
        



    } 
    
    else {

        //if not staring with '=', then check if it is a text or a numeric 
        char *endptr;
        
        double number = strtod(text, &endptr);
        
        //is a numeric value
        if (*endptr == '\0') {
            cellVariable2->type = NUM;
            
            cellVariable2->celcontent.number = number;
        } 
        
        //is a text
        else {
            cellVariable2->type = TXT;
            
            cellVariable2->celcontent.text = strdup(text);
        }

        //update the display with input value 
        printf("Text val: %s\n", text);  // Debug print statement
       
       
        update_cell_display(row, col, text);
    }

    //free memory allocated for text inputs
    free(text);
}


//Function that clears the contents of a cell
void clear_cell(ROW row, COL col) {
    //get cell variable
    struct cell cellVariable3 = spreadsheet->cells[row][col];

    //clear memory of cell
    clearCellMemory(cellVariable3);
    
    //set cell type to blank
    cellVariable3.type = BLANK;

    //update ddisplay with empty string 
    update_cell_display(row, col, "");
}

//Function that gets the textual value of a cell
char *get_textual_value(ROW row, COL col) {
    
    //get cell variable 
    const struct cell (cellVariable3) = spreadsheet->cells[row][col];

    //check type of cell and return its corresponding value 
    if(cellVariable3.type == NUM){
        //is a numeric value, so format as string
        static char numberStr[100];
        snprintf(numberStr, sizeof(numberStr), "%0.2f", cellVariable3.celcontent.number);
        return numberStr;
    }

    //is text value 
    else if(cellVariable3.type == TXT){
        return cellVariable3.celcontent.text;

    }

 

    else{
        //blank cell, so return NULL
        return NULL;
    }
  
}




//Function that checks if a string starts with a specific prefix -> the specific index will be referring to the inputs mentioned in the assignment, '+', '-', and '='
bool starts_with(const char *text, const char *prefix) {

    //check for NULL pointers
    if(text == NULL || prefix  == NULL){
        printf("Error: NULL pointer.\n");
    }
    
    //recursive function to check each character in an expression
    return (starts_with(text + 1, prefix + 1) && *text == *prefix) || *prefix == '\0';
}

//Function that advances to the next character in the input
void advance(const char **input, int step) {

    //increment input pointer by specific num of steps    
    for(int k = 0; k < step; k++){
        ++*input;
    }
}



// #include "model.h"
// #include "interface.h"
// #include <stdlib.h>
// #include <string.h>
// #include <stdio.h>
// #include <stdbool.h>
// #include <ctype.h>

// // Enumerations for equation type and cell content
// enum eqnType { OPERATOR, OPERAND, INVALID, REF_CELL };
// enum cellContent { NUM, TXT, BLANK, EQN };

// // Structures for the spreadsheet and cells
// struct excelSpreadSheet {
//     int row, col;
//     struct cell** cells;
// };

// struct cell {
//     enum cellContent type;
//     union {
//         char* text;
//         double number;
//     } content;
// };

// struct equationElemts {
//     enum eqnType type;
//     union {
//         char* referenceCell;
//         char operatorSymbol;
//         double operand;
//     } content;
// };

// struct excelSpreadSheet* spreadsheet = NULL;

// // Utility to duplicate a string up to a specific length
// // Custom implementation of strndup
// char* custom_strnduplicate(const char* string, size_t n) {
//     size_t length = strnlen(string, n);
//     char* duplicate = malloc(length + 1); // Allocate memory for the string and null terminator
//     if (!duplicate) {
//         fprintf(stderr, "Memory allocation error.\n");
//         exit(EXIT_FAILURE);
//     }
//     strncpy(duplicate, string, length);
//     duplicate[length] = '\0'; // Ensure the string is null-terminated
//     return duplicate;
// }


// // Free memory allocated for equation elements
// void freeEqnElmnts(struct equationElemts* elements) {
//     for (size_t i = 0; elements[i].type != INVALID; i++) {
//         if (elements[i].type == REF_CELL) {
//             free(elements[i].content.referenceCell);
//         }
//     }
//     free(elements);
// }

// // Clear memory associated with a cell
// void clearCellMemory(struct cell* cell) {
//     if (cell->type == TXT) {
//         free(cell->content.text);
//     }
//     cell->type = BLANK;
// }

// // Initialize the spreadsheet
// void model_init() {
//     int rows = 10, cols = 7;
//     spreadsheet = malloc(sizeof(struct excelSpreadSheet));
//     spreadsheet->row = rows;
//     spreadsheet->col = cols;
//     spreadsheet->cells = malloc(rows * sizeof(struct cell*));

//     for (int i = 0; i < rows; i++) {
//         spreadsheet->cells[i] = calloc(cols, sizeof(struct cell));
//     }
// }

// // Convert column letter to index
// int columnLetterToIndex(char letter) {
//     return toupper(letter) - 'A';
// }

// // Convert cell reference to row and column indices
// void cellReferenceToIndices(const char* reference, int* row, int* col) {
//     *col = columnLetterToIndex(reference[0]);
//     *row = atoi(&reference[1]) - 1;
// }

// // Parse an equation into its elements
// struct equationElemts* parse_eqn(const char* equation) {
//     size_t len = strlen(equation), pos = 0, index = 0;
//     struct equationElemts* elements = malloc((len + 1) * sizeof(struct equationElemts));

//     while (pos < len) {
//         if (equation[pos] == '+' || equation[pos] == '-') {
//             elements[index].type = OPERATOR;
//             elements[index].content.operatorSymbol = equation[pos];
//             pos++;
//         } else if (isalpha(equation[pos])) {
//             elements[index].type = REF_CELL;
//             size_t end = pos + 1;
//             while (isalnum(equation[end])) end++;
//             elements[index].content.referenceCell = custom_strnduplicate(&equation[pos], end - pos);
//             pos = end;
//         } else if (isdigit(equation[pos]) || equation[pos] == '.') {
//             elements[index].type = OPERAND;
//             elements[index].content.operand = strtod(&equation[pos], NULL);
//             while (isdigit(equation[pos]) || equation[pos] == '.') pos++;
//         } else {
//             elements[index].type = INVALID;
//             pos++;
//         }
//         index++;
//     }
//     elements[index].type = INVALID;
//     return elements;
// }

// // Evaluate a single equation element
// double evaluateEqnElement(const struct equationElemts* element, struct cell** cells) {
//     if (element->type == REF_CELL) {
//         int row, col;
//         cellReferenceToIndices(element->content.referenceCell, &row, &col);
//         if (cells[row][col].type == NUM) {
//             return cells[row][col].content.number;
//         }
//         fprintf(stderr, "Invalid cell reference.\n");
//         return 0.0;
//     } else if (element->type == OPERAND) {
//         return element->content.operand;
//     }
//     return 0.0;
// }

// // Evaluate an equation
// double evaluateEquation(const char* equation, struct cell** cells) {
//     struct equationElemts* elements = parse_eqn(equation);
//     double result = 0.0;
//     char operator = '+';

//     for (size_t i = 0; elements[i].type != INVALID; i++) {
//         if (elements[i].type == OPERATOR) {
//             operator = elements[i].content.operatorSymbol;
//         } else {
//             double value = evaluateEqnElement(&elements[i], cells);
//             if (operator == '+') {
//                 result += value;
//             } else if (operator == '-') {
//                 result -= value;
//             }
//         }
//     }

//     freeEqnElmnts(elements);
//     return result;
// }

// // Set cell value
// void set_cell_value(ROW row, COL col, char* text) {
//     struct cell* targetCell = &spreadsheet->cells[row][col];
//     clearCellMemory(targetCell);

//     if (text[0] == '=') {
//         targetCell->type = EQN;
//         targetCell->content.text = strdup(text);
//         double result = evaluateEquation(text + 1, spreadsheet->cells);
//         char resultStr[32];
//         snprintf(resultStr, sizeof(resultStr), "%.2f", result);
//         update_cell_display(row, col, resultStr);
//     } else {
//         char* endptr;
//         double number = strtod(text, &endptr);
//         if (*endptr == '\0') {
//             targetCell->type = NUM;
//             targetCell->content.number = number;
//         } else {
//             targetCell->type = TXT;
//             targetCell->content.text = strdup(text);
//         }
//         update_cell_display(row, col, text);
//     }
//     free(text);
// }

// // Clear cell
// void clear_cell(ROW row, COL col) {
//     struct cell* targetCell = &spreadsheet->cells[row][col];
//     clearCellMemory(targetCell);
//     update_cell_display(row, col, "");
// }

// // Get textual value of a cell
// char* get_textual_value(ROW row, COL col) {
//     struct cell* cell = &spreadsheet->cells[row][col];
//     static char buffer[32];

//     if (cell->type == NUM) {
//         snprintf(buffer, sizeof(buffer), "%.2f", cell->content.number);
//         return buffer;
//     } else if (cell->type == TXT) {
//         return cell->content.text;
//     }
//     return NULL;
// }