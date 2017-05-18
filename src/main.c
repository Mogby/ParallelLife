#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint16_t PORT_NUMBER = 8300;
const uint32_t MAX_CLIENTS = 16;

struct _client_args;

typedef struct _client_struct {
    int clientSocket;
    pthread_t clientThread;
    struct _client_args *args;
} Client;

typedef struct _listener_args_struct {
    int serverSocket;
    uint32_t clientsCount;
    Client *clientsList;
} ListenerArgs;

typedef struct _client_args {
    Client *client;
    sem_t semaphore;
    sem_t *finishSemaphore;

    uint32_t lowerBound;
    uint32_t upperBound;
    uint32_t realLowerBound;
    uint32_t realUpperBound;

    char *oldField;
    char *newField;
    uint32_t fieldWidth;

    uint32_t turnsCount;
} ClientArgs;

typedef struct _command_struct {
    char commandType;
    uint32_t lowerBound;
    uint32_t upperBound;
    uint32_t realLowerBound;
    uint32_t realUpperBound;
} Command;

void handleClient(void *args) {
    ClientArgs *clientArgs = args;

    sem_wait(&clientArgs->semaphore);

    Command command;

    command.commandType = 2;
    command.lowerBound = htonl(clientArgs->fieldWidth);
    write(clientArgs->client->clientSocket, &command, sizeof(command));

    while (clientArgs->turnsCount--) {
        sem_wait(&clientArgs->semaphore);

        command.commandType = 1;
        command.lowerBound = htonl(clientArgs->lowerBound);
        command.upperBound = htonl(clientArgs->upperBound);
        command.realLowerBound = htonl(clientArgs->realLowerBound);
        command.realUpperBound = htonl(clientArgs->realUpperBound);

        write(clientArgs->client->clientSocket, &command, sizeof(command));

        write(clientArgs->client->clientSocket, clientArgs->oldField + clientArgs->lowerBound,
              sizeof(char) * (clientArgs->upperBound - clientArgs->lowerBound));

        read(clientArgs->client->clientSocket, clientArgs->newField + clientArgs->realLowerBound,
             sizeof(char) * (clientArgs->realUpperBound - clientArgs->realLowerBound));

        sem_post(clientArgs->finishSemaphore);
    }

    command.commandType = 0;
    write(clientArgs->client->clientSocket, &command, sizeof(command));

    printf("Finished handling client %d\n", clientArgs->client->clientSocket);
}

void runListener(void* args) {
    ListenerArgs *listenerArgs = args;

    if (listen(listenerArgs->serverSocket, 16) < 0) {
        perror("ERROR Listening");
        exit(1);
    }

    int clientSocket;
    struct sockaddr_in clientAddress;
    size_t clientAddreessLength = sizeof(clientAddress);
    for(;;) {
        clientSocket = accept(listenerArgs->serverSocket, &clientAddress, &clientAddreessLength);

        if (clientSocket < 0) {
            perror("ERROR Accepting client");
        } else {
            ClientArgs *args = malloc(sizeof(ClientArgs));
            args->client = listenerArgs->clientsList + listenerArgs->clientsCount;
            args->client->args = args;
            sem_init(&args->semaphore, 0, 0);

            listenerArgs->clientsList[listenerArgs->clientsCount].clientSocket = clientSocket;
            pthread_create(&listenerArgs->clientsList[listenerArgs->clientsCount].clientThread,
                           NULL, handleClient, args);

            ++listenerArgs->clientsCount;

            printf("Client %d connected\n", clientSocket);
        }
    }
}

void runServer() {
    uint32_t width, height;
    puts("Input field width and height:");
    scanf("%u %u", &width, &height);

    uint32_t turnsCount;
    puts("Input turns count:");
    scanf("%u", &turnsCount);

    char *oldField, *newField;
    oldField = malloc(sizeof(char) * width * height);
    newField = malloc(sizeof(char) * width * height);
    bzero(newField, sizeof(char) * width * height);
    bzero(oldField, sizeof(char) * width * height);
    /*for (int i = 0; i < width * height; ++i) {
        oldField[i] = rand() & 1;
    }*/
    oldField[1] = 1;
    oldField[2 + width] = 1;
    oldField[0 + width + width] = 1;
    oldField[1 + width + width] = 1;
    oldField[2 + width + width] = 1;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("ERROR Opening socket");
        exit(1);
    }
    puts("Socket opened");

    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT_NUMBER);
    if (bind(serverSocket, &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR Binding socket");
        exit(1);
    }

    sem_t finishSemaphore;
    sem_init(&finishSemaphore, 0, 0);

    Client *clientsList = malloc(sizeof(Client) * MAX_CLIENTS);

    ListenerArgs args;
    args.serverSocket = serverSocket;
    args.clientsCount = 0;
    args.clientsList = clientsList;

    pthread_t listenerThread;
    pthread_create(&listenerThread, NULL, runListener, &args);

    char command[256];
    for(;;) {
        gets(command);
        if (!strcmp(command, "start")) {
            break;
        }
    }
    pthread_cancel(listenerThread);

    uint32_t fieldSize = width * height;
    uint32_t currentLowerBound = 0;
    for (int i = 0; i < args.clientsCount; ++i) {
        clientsList[i].args->finishSemaphore = &finishSemaphore;

        clientsList[i].args->realLowerBound = currentLowerBound;
        clientsList[i].args->realUpperBound = (currentLowerBound +
                fieldSize / args.clientsCount + (i < fieldSize % args.clientsCount ? 1 : 0));

        clientsList[i].args->lowerBound = (currentLowerBound <= width ?
                                           0 : currentLowerBound - width - 1);
        clientsList[i].args->upperBound = (clientsList[i].args->realUpperBound + width >= fieldSize ?
                                           fieldSize : clientsList[i].args->realUpperBound + width + 1);

        clientsList[i].args->oldField = oldField;
        clientsList[i].args->newField = newField;
        clientsList[i].args->fieldWidth = width;

        clientsList[i].args->turnsCount = turnsCount;

        currentLowerBound = clientsList[i].args->realUpperBound;
    }

    for (int i = 0; i < args.clientsCount; ++i) {
        sem_post(&clientsList[i].args->semaphore);
    }

    puts("\nINITIAL FIELD:");
    for (int i = 0; i < width * height; ++i) {
        if (i % width == 0) {
            puts("");
        }
        if (oldField[i] == 1) {
            printf("O");
        } else {
            printf(".");
        }
    }
    puts("");

    char *tmpField;
    while (turnsCount--) {
        for (int i = 0; i < args.clientsCount; ++i) {
            sem_post(&clientsList[i].args->semaphore);
        }

        for (int i = 0; i < args.clientsCount; ++i) {
            sem_wait(&finishSemaphore);
        }

        for (int i = 0; i < args.clientsCount; ++i) {
            tmpField = clientsList[i].args->newField;
            clientsList[i].args->newField = clientsList[i].args->oldField;
            clientsList[i].args->oldField = tmpField;
        }

        tmpField = newField;
        newField = oldField;
        oldField = tmpField;
    }

    for (int i = 0; i < args.clientsCount; ++i) {
        pthread_join(clientsList[i].clientThread, NULL);
        close(clientsList[i].clientSocket);
    }

    puts("\nRESULT FIELD:");
    for (int i = 0; i < width * height; ++i) {
        if (i % width == 0) {
            puts("");
        }
        if (oldField[i] == 1) {
            printf("O");
        } else {
            printf(".");
        }
    }

    close(serverSocket);
}

char getCell(char *buffer, int lowerBound, int upperBound, int index) {
    if (index < lowerBound || index >= upperBound) {
        return 0;
    }
    return buffer[index - lowerBound];
}

void handleBuffer(char *buffer, char *newBuffer, int width, Command command) {
    u_char neighboursCount;
    for (int i = command.realLowerBound; i < command.realUpperBound; ++i) {
        neighboursCount = getCell(buffer, command.lowerBound, command.upperBound, i - width) +
                getCell(buffer, command.lowerBound, command.upperBound, i + width);
        if (i % width) {
            neighboursCount += getCell(buffer, command.lowerBound, command.upperBound, i - width - 1) +
                    getCell(buffer, command.lowerBound, command.upperBound, i - 1) +
                    getCell(buffer, command.lowerBound, command.upperBound, i + width - 1);
        }
        if ((i % width) != width - 1) {
            neighboursCount += getCell(buffer, command.lowerBound, command.upperBound, i - width + 1) +
                               getCell(buffer, command.lowerBound, command.upperBound, i + 1) +
                               getCell(buffer, command.lowerBound, command.upperBound, i + width + 1);
        }

        if (buffer[i - command.lowerBound]) {
            if (neighboursCount == 2 || neighboursCount == 3)
                newBuffer[i - command.realLowerBound] = 1;
            else
                newBuffer[i - command.realLowerBound] = 0;
        } else {
            if (neighboursCount == 3)
                newBuffer[i - command.realLowerBound] = 1;
            else
                newBuffer[i - command.realLowerBound] = 0;
        }
    }
}

void runClient() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) {
        perror("ERROR Opening socket");
        exit(1);
    }

    puts("Enter server address:");
    char serverName[256];
    scanf("%s", serverName);

    struct hostent *server;
    server = gethostbyname(serverName);

    if (!server) {
        perror("ERROR Invalid host name");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    memcpy(&serverAddress.sin_addr, server->h_addr, server->h_length);
    serverAddress.sin_port = htons(PORT_NUMBER);

    if (connect(clientSocket, &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR Connecting to server");
        exit(1);
    }

    puts("Connected to server");

    char isRunning = 1;
    char *buffer = NULL;
    char *newBuffer;
    uint32_t width;
    Command command;
    while (isRunning) {
        read(clientSocket, &command, sizeof(command));
        if (command.commandType == 0) {
            isRunning = 0;
            puts("Received terminate request.");
        } else if (command.commandType == 1) {
            command.lowerBound = ntohl(command.lowerBound);
            command.upperBound = ntohl(command.upperBound);
            command.realLowerBound = ntohl(command.realLowerBound);
            command.realUpperBound = ntohl(command.realUpperBound);

            if (!buffer) {
                buffer = malloc(sizeof(char) * (command.upperBound - command.lowerBound));
                newBuffer = malloc(sizeof(char) * (command.realUpperBound - command.realLowerBound));
            }

            read(clientSocket, buffer, sizeof(char) * (command.upperBound - command.lowerBound));
            handleBuffer(buffer, newBuffer, width, command);
            write(clientSocket, newBuffer, sizeof(char) * (command.realUpperBound - command.realLowerBound));

            puts("Handled one turn");
        } else if (command.commandType == 2) {
            command.lowerBound = ntohl(command.lowerBound);
            printf("Width changed to %d\n", command.lowerBound);
            width = command.lowerBound;
        }
    }
}

int main(int argc, char **argv) {

    char command[256];

    char isServer = 2;
    while (isServer == 2) {
        gets(command);
        if (!strcmp(command, "runserver")) {
            isServer = 1;
        } else if (!strcmp(command, "runclient")) {
            isServer = 0;
        }
    }

    if (isServer) {
        runServer();
    } else {
        runClient();
    }

    return 0;
}
