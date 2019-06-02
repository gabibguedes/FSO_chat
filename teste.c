#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define QUEUE_NAME  "/testqueue"
#define MAX_SIZE    1024

static void * queue_server(void *pars);
static void * queue_client(void *parc);

static void * queue_server(void *pars) {
    mqd_t mq;
    unsigned int sender;
    int bytes_read;
    struct mq_attr attr;
    char buffer[MAX_SIZE];
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;
    mq = mq_open(QUEUE_NAME, O_RDONLY | O_NONBLOCK | O_CREAT, 0666, &attr);
    printf("mq_receive : %d\n",mq);
    printf("SERVER: None %d %d \n", errno, bytes_read);
    memset(buffer, 0x00, sizeof(buffer));
    mq_unlink (QUEUE_NAME);
    while(1) {
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, &sender);
        if(bytes_read >= 0) {
            printf("SERVER: Received message: %s\n", buffer);
        } else {
            printf("SERVER: None %d %d \n", errno, bytes_read);
        }
        sleep(1);
    }
    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    return NULL;
}


static void * queue_client(void *parc) {
    mqd_t mq;
    char buffer[MAX_SIZE];
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;
    mq = mq_open(QUEUE_NAME, O_CREAT|O_WRONLY|O_NONBLOCK , 0666,&attr);
    printf("mq_send : %d\n",mq);
    int count = 0;
    while(1) {
        snprintf(buffer, sizeof(buffer), "MESSAGE %d", count++);
        printf("CLIENT: Send message... \n");
        int bytes_read = mq_send(mq, buffer, MAX_SIZE, 0);
            printf("CLIENT: send %d %d \n", errno, bytes_read);
        sleep(3);
    }
    mq_close(mq);
    return NULL;
}

int main() {

pthread_t client, server;
printf("Start...\n");
pthread_create(&server, NULL, &queue_server, NULL);
pthread_create(&client, NULL, &queue_client, NULL);
pthread_join(server, NULL);
pthread_join(client, NULL);
printf("Done...\n");
return (EXIT_SUCCESS);
}