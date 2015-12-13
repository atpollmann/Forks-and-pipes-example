#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 100

int extractRowCount(char* payload);
int extractColumnCount(char* payload);

/**
 * Gets the "lineNumber"th line in the \n line
 * terminated text and copy it to "line"
 *
 */
void getLineN(char *text, char *line, int lineNumber);

void processChild(int writePipe, int readPipe);
void processParent(char* payload, int writePipe);
void processPayload(char *payload, char *result);
int extractIntegerFromLine(char *line);
void parseMatrixFromString(char *text, int *matrix, int startLine, int rowCount, int columnCount);

void terminateWithMessage(const char* message);

int main(int argc, char **argv) {

    char *payload = "2\n3\n1 2 3\n4 5 6\n44 22 33\n12 56 69\n";
    int status;
    char result[BUFFER_SIZE];


    // Parent will write to fd1
    // Child will read from fd1
    //
    // Child will write to fd2
    // Parent will read from fd2
    int fd1[2], fd2[2];
    pid_t childPid;
    pipe(fd1);
    pipe(fd2);
    childPid = fork();

    if(childPid < 0) terminateWithMessage("No se pudo crear proceso");
    else if(childPid == 0) {
        close(fd1[1]);
        close(fd2[0]);
        processChild(fd2[1], fd1[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(fd1[0]);
        close(fd2[1]);
        processParent(payload, fd1[1]);
    }

    wait(&status);

    read(fd2[0], &result, sizeof(result));
    printf("Received from child:\n%s\n", result);

    return 0;

}

int extractRowCount(char* payload) {
    // Row count must be on first line
    char line[BUFFER_SIZE];
    getLineN(payload, line, 1);
    return extractIntegerFromLine(line);
}

int extractColumnCount(char* payload) {
    // Column count must be on second line
    char line[BUFFER_SIZE];
    getLineN(payload, line, 2);
    return extractIntegerFromLine(line);
}

int extractIntegerFromLine(char *line) {
    int number = -1;
    sscanf(line, "%d\n", &number);
    return number;
}

void processParent(char* payload, int writePipe) {
    // Write the payload to the pipe
    write(writePipe, payload, strlen(payload) + 1);
}

void processChild(int writePipe, int readPipe) {
    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    read(readPipe, buffer, sizeof(buffer));
    processPayload(buffer, result);
    write(writePipe, result, sizeof(result));
}

void terminateWithMessage(const char* message) {
    printf("%s\n", message);
    exit(EXIT_SUCCESS);
}

void processPayload(char *payload, char *result) {
    int i, sum;
    char sumBuffer[BUFFER_SIZE];
    char resultBuffer[BUFFER_SIZE];
    int rowCount = extractRowCount(payload);
    int columnCount = extractColumnCount(payload);
    int *matrix1 = (int *)malloc(rowCount * columnCount * sizeof(int));
    int *matrix2 = (int *)malloc(rowCount * columnCount * sizeof(int));
    parseMatrixFromString(payload, matrix1, 3, rowCount, columnCount);
    parseMatrixFromString(payload, matrix2, 3 + rowCount, rowCount, columnCount);
    for(i = 0; i < (rowCount * columnCount); i++) {
        sum = matrix1[i] + matrix2[i];
        sprintf(sumBuffer, "%d ", sum);
        strcat(resultBuffer, sumBuffer);
        if(((i+1) % columnCount) == 0) {
            strcat(resultBuffer, "\n");
        }
    }
    strcpy(result, resultBuffer);
}

void parseMatrixFromString(char *text, int *matrix, int startLine, int rowCount, int columnCount) {
    int i, j, k, curRow = 0, curLine = startLine, curCol;
    char curChar, intBuffer[BUFFER_SIZE], lineBuffer[BUFFER_SIZE];

    for(i = 0; i < rowCount; i++) {
        getLineN(text, lineBuffer, curLine);
        j = 0;
        k = 0;
        curCol = 0;
        do {
            intBuffer[j] = curChar = lineBuffer[k];
            if(intBuffer[j] == ' ' || intBuffer[j] == '\n') {
                intBuffer[j] = '\0';
                matrix[i * columnCount + curCol] = atoi(intBuffer);
                curCol++;
                j = -1;
            }
            k++;
            j++;
        } while(curChar != '\n');
        curLine++;
        curRow++;
    }

}

void getLineN(char *text, char *line, int lineNumber) {
    char buffer[BUFFER_SIZE];
    int i, j = 0, lineCount = 0;
    for(i = 0; i < BUFFER_SIZE; i++) {
        buffer[j] = text[i];
        if(text[i] == '\n') {
            lineCount++;
            j = -1;
            if(lineCount == lineNumber) {
                strcpy(line, buffer);
                break;
            }
        }
        j++;
    }
}



