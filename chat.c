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

typedef struct _chanel{
    char room[10];
    mqd_t queue;
    char users[100][10];
    int size;
} chanel;

char user[10];
char all_message[550];
char response_message[550];
struct mq_attr attr;
mqd_t my_queue, person_queue, chanel_queue;

chanel chanel_list[10] = {NULL};
int cl_position = 0;

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

int user_exists(const char *username){
    char filepath[30] = "/dev/mqueue/chat-";
    strcat(filepath, username);
    struct stat st;
    int result = stat(filepath, &st);
    return result == 0;
}
int room_exists(const char *room){
    char filepath[31] = "/dev/mqueue/canal-";
    strcat(filepath, room);
    struct stat st;
    int result = stat(filepath, &st);
    return result == 0;
}

int user_in_chanel(char *user, chanel c){
    for(int i=0; i<c.size; i++){
        if (strcmp(user, c.users[i]) == 0){
            return 1;
        }
    }
    return 0;
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

void open_send_queue_chanel(char *chanel_name){
    char queue_name[16] = "/canal-";

    strcat(queue_name, chanel_name);

    if ((chanel_queue = mq_open(queue_name, O_WRONLY)) < 0){
        perror("chanel name mq_open");
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

void send_chanel(msg message, chanel c){
    msg msg_chanel;
    strcpy(msg_chanel.receiver, "#");
    strcat(msg_chanel.receiver, message.receiver);
    strcpy(msg_chanel.sender, message.sender);
    strcpy(msg_chanel.text, message.text);
    strcpy(msg_chanel.all_msg, msg_chanel.sender);
    strcat(msg_chanel.all_msg, ":");
    strcat(msg_chanel.all_msg, msg_chanel.receiver);
    strcat(msg_chanel.all_msg, ":");
    strcat(msg_chanel.all_msg, msg_chanel.text);

    char username[10];
    for(int i = 0; i < c.size; i++){
        strcpy(username, c.users[i]);
        open_send_queue(username);
        int send = mq_send(person_queue, msg_chanel.all_msg, strlen(msg_chanel.all_msg), 0);
        if (send < 0){
            perror("Erro ao enviar");
            exit(1);
        }
    }
}

int send_message(){
    msg message;
    memset(all_message, 0, sizeof(all_message));

    printf(">");
    scanf("%[^\n]*c", all_message);
    getchar();

    message = build_message_to_send(all_message);
    
    if (!strcmp(message.receiver, "all")){
        broadcast(message.all_msg);

    } else if(user_exists(message.receiver)) {
        open_send_queue(message.receiver);

        int send = mq_send(person_queue, (void *)&message.all_msg, strlen(message.all_msg), 0);
        if (send < 0) {
            perror("Erro ao enviar");
            exit(1);
        }

        mq_close(person_queue);
    }else{
        printf(RED "UNKNOWNUSER %s\n" RESET, message.receiver);
    }

    return 1;
}

int send_message_chanel(){
    msg message;
    memset(all_message, 0, sizeof(all_message));

    printf(">");
    scanf("%[^\n]*c", all_message);
    getchar();

    message = build_message_to_send(all_message);
    
    if (!strcmp(message.receiver, "all")){
        broadcast(message.all_msg);

    } else if(room_exists(message.receiver)) {
        open_send_queue_chanel(message.receiver);

        int send = mq_send(chanel_queue, (void *)&message.all_msg, strlen(message.all_msg), 0);
        if (send < 0) {
            perror("Erro ao enviar");
            exit(1);
        }

        mq_close(chanel_queue);
    }else{
        printf(RED "UNKNOWNCHANEL %s\n" RESET, message.receiver);
    }

    return 1;
}
void *receive_messages(){
    msg message;

    while (1){
        int receive = mq_receive(my_queue, (void *)&response_message, sizeof(response_message), 0);
        message = build_message_received(response_message);
        if (!strcmp(message.receiver, "all")){
            printf(MAGENTA "BROADCAST de %s: %s" RESET "\n", message.sender, message.text);
        }else if(message.receiver[0] == '#'){
            printf(MAGENTA "%s: MENSAGEM de %s: %s" RESET "\n", message.receiver, message.sender, message.text);

        }else{
        printf(BLUE "MENSAGEM de %s: %s" RESET "\n" , message.sender, message.text);
        }
        memset(response_message, 0, sizeof(response_message));
    }

    pthread_exit(NULL);
}

void *receive_messages_chanel(){
    chanel rec_chanel = chanel_list[cl_position -1];
    msg message;

    while (1){
        int receive = mq_receive(rec_chanel.queue, (void *)&response_message, sizeof(response_message), 0);
        message = build_message_received(response_message);

        if(user_in_chanel(message.sender, rec_chanel)){
            send_chanel(message, rec_chanel);
        }else{
            strcpy(message.text, "NOT A MEMBER\0");
            send_chanel(message, rec_chanel);
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
    }else if (!user_exists(user)){
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

void open_chanel_queue(chanel new_chanel){
    char queue[17] = "/canal-", all_path[30] = "/dev/mqueue";
    strcat(queue, new_chanel.room);
    strcat(all_path, queue);

    if (!strcmp(new_chanel.room, "all")){
        printf(RED "Nome de sala inválido\n" RESET);
    }else if (!room_exists(new_chanel.room)){
        __mode_t old_umask = umask(0155);
        if ((new_chanel.queue = mq_open(queue, O_RDWR | O_CREAT, 0622, &attr)) < 0){
            umask(old_umask);
            perror("mq_open");
            exit(1);
        }
        mq_close(new_chanel.queue);

        if ((new_chanel.queue = mq_open(queue, O_RDWR)) < 0){
            perror("Erro ao abrir a fila");
            exit(1);
        }

        chanel_list[cl_position] = new_chanel;
        cl_position ++;
    }else{
        printf("Essa sala já existe :/\n");
    }
}

void help(){
    printf(YELLOW "\nHELP - Aparece esta mensagem\n");
    printf("LISTAR - Aparece a lista de usuários logados\n");
    printf("ENVIAR - Envia uma nova mensagem\n");
    printf("CRIAR - Cria uma nova sala\n");
    printf("SALA - Envia uma nova mensagem para uma sala\n");
    printf("SAIR - Sair do chat\n" RESET);
}

void sigintHandler(int sig_num){
    signal(SIGINT, sigintHandler);
    printf(RED "\n O programa não pode terminar com Ctrl+C! Tente SAIR. \n" RESET);
    fflush(stdout);
}

void create_room(){
    if(cl_position < 9){
        mqd_t my_chanel;
        chanel new_chanel;
        char room[10];
        printf(">");
        scanf("%[^\n]*c", room);
        getchar();
        strcpy(new_chanel.room , room);
        new_chanel.queue = my_chanel;
        strcpy(new_chanel.users[0], user);
        new_chanel.size = 1;

        open_chanel_queue(new_chanel);
        pthread_t thread_chanel;
        pthread_create(&thread_chanel, NULL, receive_messages_chanel, NULL);
    }
}

void remove_chanels(){
    char room[10];
    for(int i = 0; i < cl_position; i++){
        char filepath[31] = "/dev/mqueue/canal-";
        strcpy(room , chanel_list[i].room);
        strcat(filepath, room);
        int s = remove(filepath);
        if(!s){
            printf(GREEN "\nSala %s desativada\n" RESET, room );

        }
    }
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
        }else if (!strcmp(op, "listar") || !strcmp(op, "LISTAR")){
            list_users();
        }else if (!strcmp(op, "enviar") || !strcmp(op, "ENVIAR")){
            printf(YELLOW "\nPara enviar uma mensagem escreva a mensagem no formato:\n");
            printf("\tusuario_de_destino:texto_da_mensagem\n");
            printf("\nPara um BROADCAST escreva a mensagem no formato:\n");
            printf("\tall:texto_da_mensagem\n"RESET);
            send_message();
        }else if(!strcmp(op, "criar") || !strcmp(op, "CRIAR")){
            printf(YELLOW "\nEscreva o nome da sala que deseja criar\n" RESET);
            create_room();
        }else if(!strcmp(op, "sala") || !strcmp(op, "SALA")){
            printf(YELLOW "\nPara enviar uma mensagem para uma sala escreva a mensagem no formato:\n");
            printf("\tsala_de_destino:texto_da_mensagem\n" RESET);
            send_message_chanel();
        }else{
            printf(YELLOW "\nComando inválido. Digite HELP para listar os possiveis comandos.\n" RESET);
        }
        printf("\n");
        scanf("%s", op);
        getchar();
    }
    remove_chanels();
    strcat(filepath, user);
    int s = remove(filepath);
    if(!s){
        printf(GREEN "Usuário desconectado\n" RESET );

    }
    return 0;

}
