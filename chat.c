#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <dirent.h>
#include <signal.h>

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;36m"
#define RED "\033[0;31m"
#define MAGENTA "\033[0;35m"
#define RESET "\033[0m\n"

typedef struct _msg{
    char sender[10];
    char receiver[10];
    char text[500];
    char all_msg[550];
} msg;

char user[10];
char all_message[550];
char response_message[550];
struct mq_attr attr;
mqd_t my_queue, person_queue;

void list_users(){
    DIR *d;
    struct dirent *dir;
    d = opendir("/dev/mqueue/");
    if (d){
        printf(GREEN "\nLista de usuários: \n");
        while ((dir = readdir(d)) != NULL){
            char *chat, *username, *file = dir->d_name;
            char split[] = "-";
            chat = strtok(file, split);
            if (!strcmp(chat, "chat")){
                username = strtok(NULL, split);
                printf("* %s\n", username);

            }
        }
        printf(RESET);
        closedir(d);
    }
}

msg build_message_to_send(char writen_text[550]){
    int i, k, j;
    msg mensagem;
    char *receiver, *text;
    strcpy(mensagem.sender, user);
    strcpy(mensagem.receiver, "");
    strcpy(mensagem.text, "");
    strcpy(mensagem.all_msg, "");

    char split[] = ":";
    receiver = strtok(writen_text, split);
    text = strtok(NULL, split);
    strcpy(mensagem.receiver, receiver);
    strcpy(mensagem.text, text);

    strcat(mensagem.all_msg, mensagem.sender);
    strcat(mensagem.all_msg, ":");
    strcat(mensagem.all_msg, mensagem.receiver);
    strcat(mensagem.all_msg, ":");
    strcat(mensagem.all_msg, mensagem.text);
    return mensagem;
}
msg build_message_received(char writen_text[550]){
    int i, k, j;
    msg mensagem;
    char *receiver, *text, *from_user;
    strcpy(mensagem.sender, "");
    strcpy(mensagem.receiver, "");
    strcpy(mensagem.text, "");
    strcpy(mensagem.all_msg, writen_text);

    char split[] = ":";
    from_user = strtok(writen_text, split);
    receiver = strtok(NULL, split);
    text = strtok(NULL, split);
    
    strcpy(mensagem.sender, from_user);
    strcpy(mensagem.receiver, receiver);
    strcpy(mensagem.text, text);
    return mensagem;
}

void open_send_queue(char *person_name){
    char queue_name[16] = "/chat-";

    strcat(queue_name, person_name);

    if ((person_queue = mq_open(queue_name, O_WRONLY)) < 0){
        perror("person name mq_open");
        exit(1);
    }
}

void broadcast(char message[550]){
    DIR *d;
    struct dirent *dir;
    d = opendir("/dev/mqueue/");
    if (d){
        char *chat, *username,* file;
        while ((dir = readdir(d)) != NULL){
            file = dir->d_name;
            char split[] = "-";
            chat = strtok(file, split);
            if (!strcmp(chat, "chat")){
                username = strtok(NULL, split);
                open_send_queue(username);

                int send = mq_send(person_queue, message, strlen(message), 0);
                if (send < 0){
                    perror("Erro ao enviar");
                    exit(1);
                }
            }
        }
        closedir(d);
    }
}

int send_message(){
    char queue[16] = "/chat-", all_path[30] = "/dev/mqueue";
    msg message;
    memset(all_message, 0, sizeof(all_message));

    printf(">");
    scanf("%[^\n]*c", all_message);
    getchar();

    message = build_message_to_send(all_message);
    strcat(queue, message.receiver);
    strcat(all_path, queue);

    // if (fopen(all_path, "r") == NULL){
    //     printf(RED "UNKNOWNUSER %s\n" RESET, message.receiver);
    // }else{
        if(!strcmp(message.receiver, "all")){
            broadcast(message.all_msg);
        }else{
            open_send_queue(message.receiver);

            int send = mq_send(person_queue, (void *)&message.all_msg, strlen(message.all_msg), 0);
            if (send < 0){
                perror("Erro ao enviar");
                exit(1);
            }

            mq_close(person_queue);
        }
    // }

    return 1;
}
void *receive_messages(){

    char *sender_name;
    char *user_name;
    char *sender_message;
    msg message;

    while (1){
        int receive = mq_receive(my_queue, (void *)&response_message, sizeof(response_message), 0);
        message = build_message_received(response_message);
        if (!strcmp(message.receiver, "all")){
            printf(MAGENTA "BROADCAST de %s: %s" RESET, message.sender, message.text);
        }else{
        printf(BLUE "NOVA MENSAGEM de %s: %s" RESET , message.sender, message.text);

        }
        

        memset(response_message, 0, sizeof(response_message));
    }

    pthread_exit(NULL);
}

void open_user_queue(){
    char queue[16] = "/chat-", all_path[30] = "/dev/mqueue";
    strcat(queue, user);
    strcat(all_path, queue);

    if(!strcmp(user, "all")){
        printf(RED "Usuário inválido\n" RESET);
        exit(1);
    }else if (fopen(all_path, "r") == NULL){
        __mode_t old_umask = umask(0155);
        if ((my_queue = mq_open(queue, O_RDWR | O_CREAT, 0622, &attr)) < 0){
            umask(old_umask);
            perror("mq_open");
            exit(1);
        }
        mq_close(my_queue);

        if ((my_queue = mq_open(queue, O_RDWR)) < 0){
            perror("Erro ao abrir a fila");
            exit(1);
        }
    }else{
        printf("Esse usuário já existe :/\n");
        exit(1);
    }
}

void help(){
    printf(YELLOW "\nHELP - Aparece esta mensagem\n");
    printf("LISTA - Aparece a lista de usuários logados\n");
    printf("ENVIAR - Envia uma nova mensagem\n");
    printf("SAIR - Sair do chat\n" RESET);
}

void sigintHandler(int sig_num){
    /* Reset handler to catch SIGINT next time. 
       Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, sigintHandler);
    printf(RED "\n O programa não pode terminar com Ctrl+C! Tente SAIR. \n" RESET);
    fflush(stdout);
}

int main(){
    system("clear");
    signal(SIGINT, sigintHandler);
    char op[10], filepath[30] = "/dev/mqueue/chat-";

    printf("Bem vindo ao Chat!\n\n");
    printf("Escolha um username para participar: ");
    scanf("%[^\n]*c", user);
    getchar();

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(all_message);
    attr.mq_flags = 0;
    open_user_queue();
    
    pthread_t thread;

    pthread_create(&thread, NULL, receive_messages, NULL);
    
    help();
    scanf("%s", op);
    getchar();
    while(strcmp(op, "sair") && strcmp(op, "SAIR")){
        if (!strcmp(op, "help") || !strcmp(op, "HELP")){
            help();
        }else if (!strcmp(op, "lista") || !strcmp(op, "LISTA")){
            list_users();
        }else if (!strcmp(op, "enviar") || !strcmp(op, "ENVIAR")){
            printf(YELLOW "\nPara enviar uma mensagem escreva a mensagem no formato:\n");
            printf("\tusuario_de_destino:texto_da_mensagem\n"RESET);
            send_message();
        }else{
            printf(YELLOW "\nComando inválido. Digite HELP para listar os possiveis comandos.\n" RESET);
        }
        scanf("%s", op);
        getchar();
    }
    strcat(filepath, user);
    int s = remove(filepath);
    if(!s){
        printf(GREEN "Usuário desconectado\n" RESET );

    }
    return 0;

}
