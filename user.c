#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
struct message recve;
pthread_t thread;
struct message{
    char type;
    char message[1023];
};

//thread that prints every message received from kernel module
// FYI: we have not tested if you need another message struct or you can use the same one for receiving and sending
// NOTE: you only need one sock_fd for receiving and sending
void *recvthread(void *tid)
{
	while(1){
    	recvmsg(sock_fd, &msg, 0);
    	memcpy(&recve,NLMSG_DATA(nlh),sizeof(struct message));
    	printf("Received message= %s \n",recve.message);
	}
}

//init to initialize everything, only needs to be done onces. If you have any further things that needs to be initilized once
// please add it at the end of function.
void init(){
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

}

void publish(){
    struct message mes;
    printf("Type the message you would like to send to the kernel: ");
    char user_message[1023];
    gets(user_message);
    printf("Sending Message: %s to kernel\n", user_message);
    strcpy(mes.message, user_message);
    mes.type = 1; //for publish
    memcpy(NLMSG_DATA(nlh), &mes, sizeof(struct message));
    sendmsg(sock_fd, &msg, 0);
}


void register_subscriber(){
    struct message mes;
    printf("Registering PID with kernel module\n");
    strcpy(mes.message,"KERNEL_REGISTRATION");
    mes.type=0;// type 0 i.e. subscription registration
    memcpy(NLMSG_DATA(nlh),&mes,sizeof(struct message));
    sendmsg(sock_fd, &msg, 0);
}

void subscribe(){
    printf("Waiting for message from kernel\n");
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", NLMSG_DATA(nlh));
    close(sock_fd);
}

//main function
int main()
{
	init();
    // pthread_create(&thread, NULL, recvthread,NULL);
    register_subscriber();
    publish();
    subscribe();

}
