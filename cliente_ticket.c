// /*
// Aluna: Gabriela Barrozo Guedes
// Matricula: 16/0121612
// */
// #include <stdio.h>
// #include <stdlib.h>
// #include <mqueue.h>
// #include <sys/stat.h>
// #define QUEUE "/chat-gabi"

// int main(){
//     mqd_t queue;
//     struct mq_attr attr;
//     char msg[550];

//     attr.mq_maxmsg = 10; // capacidade para 10 mensagens
//     attr.mq_msgsize = sizeof(msg) ; // tamanho de cada mensagem
//     attr.mq_flags = 0 ;

//     if ((queue = mq_open(QUEUE, O_RDWR | O_CREAT, 0666, &attr)) < 0){
//         perror("mq_open");
//         exit(1);
//     }
    

//     // recebe cada mensagem e imprime seu conteudo
//     while(1){
//         if ((mq_receive(queue, (void *)&msg, sizeof(msg), 0)) < 0){
//             perror("mq_receive:");
//             exit(1);
//         }
//         printf("Received msg value %s\n", msg);
//     }
//     return 0;
// }

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>

#include "common.h"

int main(int argc, char **argv)
{
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open("/chat-gabi", O_CREAT | O_RDONLY, 0644, &attr);
    CHECK((mqd_t)-1 != mq);

    do {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        if (! strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
        {
            must_stop = 1;
        }
        else
        {
            printf("Received: %s\n", buffer);
        }
    } while (!must_stop);

    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(mq));
    CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));

    return 0;
}