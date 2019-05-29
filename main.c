#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "common.h"

mqd_t queue, send_queue;

typedef struct _msg{
    char sender[10];
    char receiver[10];
    char text[500];
    char all_msg[550];
}msg;

void print(char txt[600]){
    printf("%s\n", txt);
}

int verify_username(char username[10]){
    char chat_file[100], all_path[200];
    strcpy(chat_file, "/chat-");
    strcat(chat_file, username);
    strcpy(all_path, "/dev/mqueue");
    strcat(all_path, chat_file);

    if (fopen(all_path, "r") == NULL){
        if ((queue = mq_open(chat_file, O_RDWR | O_CREAT, 0666, NULL)) < 0){
            perror("mq_open");
            exit(1);
        }
        printf("Usuário criado\n");
        return 1;
    }else{
        printf("Esse usuário já existe :/\n");
        return 0;
    }
}

msg build_message_to_send(char usr[10], char to[10], char text[500]){
    msg message;
    strcpy(message.sender, usr);
    strcpy(message.receiver, to);
    strcpy(message.text, text);

    strcpy(message.all_msg, message.sender);
    strcat(message.all_msg, ":");
    strcat(message.all_msg, message.receiver);
    strcat(message.all_msg,":" );
    strcat(message.all_msg, message.text);
    
    return message;
}

msg build_message_that_received(char all_text[550]){
    int i, k, j;
    msg message;
    strcpy(message.sender, "");
    strcpy(message.receiver, "");
    strcpy(message.text, "");
    strcpy(message.all_msg, all_text);

    for(i = 0; all_text[i] != ':'; i++){
        message.sender[i] = all_text[i];
    }
    for(k = 0; all_text[k+i+1] != ':'; k++){
        message.receiver[k] = all_text[k+i+1];
    }
    i = i + k;
    for(j = 0; all_text[j+i+2] != '\0'; j++){
        printf("%d: %c\n", j, all_text[j+i+2]);
        message.text[j] = all_text[j+i+2];
    }
    return message;
}

void send_message(msg msg_send){
    char chat[100], all_path[100];
    strcpy(chat,"/chat-");
    strcat(chat, msg_send.receiver);
    strcpy(all_path, "/dev/mqueue");
    strcat(all_path, chat);
    print(all_path);
    print(chat);
    if (fopen(all_path, "r") != NULL){
        if ((send_queue = mq_open(chat, O_WRONLY)) < 0){
            perror("mq_open");
            exit(1);
        }

        if (mq_send(queue, (void *)&msg_send.all_msg, sizeof(msg_send.all_msg), 0) < 0){
            perror("mq_send");
            exit(1);
        }
        print(chat);
        printf("Sent message with value %s\n", msg_send.all_msg);
        sleep(1);
    }else{
        printf("There is no chat for %s", msg_send.receiver);
    }
}

void send(char username[10]){
    mqd_t mq;
    char buffer[MAX_SIZE], dest[10],chat[100];
    msg mensagem;

    /* open the mail queue */
    CHECK((mqd_t)-1 != mq);


    printf("Send to server (enter \"exit\" to stop it):\n");
    printf("Para: ");
    scanf("%s", dest);
    strcpy(chat,"/chat-");
    strcat(chat, dest);
    print(chat);
    mq = mq_open(chat, O_WRONLY);
    do {

        printf("> ");

        scanf("%s",buffer);
        mensagem = build_message_to_send(username, dest, buffer);
  
        CHECK(0 <= mq_send(mq, mensagem.all_msg, MAX_SIZE, 0));

    } while (strncmp(buffer, MSG_STOP, strlen(MSG_STOP)));

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));
}

int main(){
    char username[10], a[10];
    int user_valid=0;
    system("clear");
    printf("Bem vindo ao Chat!\n\n");

    printf("Escolha um username para participar: ");
    scanf("%s", username);
    user_valid = verify_username(username);
    if(!user_valid){
        exit(0);
    }
    msg lala = build_message_to_send(username, "gabi", "ola, como vai?");
    send(username);
    return 0;
}
