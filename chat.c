#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

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

void open_person_queue(char *person_name){
    char queue_name[16] = "/chat-";

    strcat(queue_name, person_name);

    if ((person_queue = mq_open(queue_name, O_RDWR)) < 0)
    {
        perror("person name mq_open");
        exit(1);
    }
}

void close_person_queue(char *person_name){
    mq_close(person_queue);
}

int send_message(){
    char split[] = ":";
    msg message;
    memset(all_message, 0, sizeof(all_message));

    scanf("%[^\n]*c", all_message);
    getchar();

    message = build_message_to_send(all_message);

    open_person_queue(message.receiver);

    int send = mq_send(person_queue, (void *)&message.all_msg, strlen(message.all_msg), 0);
    if (send < 0){
        perror("Erro ao enviar");
        exit(1);
    }

    mq_close(person_queue);

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

        printf("NOVA MENSAGEM de %s:%s\n", message.sender, message.text);
        memset(response_message, 0, sizeof(response_message));
    }

    pthread_exit(NULL);
}

void open_user_queue(){
    char queue[16] = "/chat-";
    strcat(queue, user);

    if ((my_queue = mq_open(queue, O_RDWR | O_CREAT, 0644, &attr)) < 0){
        perror("mq_open");
        exit(1);
    }
    mq_close(my_queue);

    if ((my_queue = mq_open(queue, O_RDWR)) < 0){
        perror("Erro ao abrir a fila");
        exit(1);
    }
}

int main(){
    printf("Usuário: ");
    scanf("%[^\n]*c", user);
    getchar();
    printf("%s\n", user);

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(all_message);
    attr.mq_flags = 0;
    open_user_queue();
   
    pthread_t thread;

    pthread_create(&thread, NULL, receive_messages, NULL);
    printf("(Escreva a mensagem no formato: PARA:MENSAGEM)\n");

    while (send_message());
}
