#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#define NETLINK_USER 31 //port number between userspace and kernel (must be exact match)

struct sock *nl_sk = NULL;

//message struct to know type and content
struct message{
    char type;
    char message[1023];
};

struct sub_pid_list{
    int pid;
    struct list_head list;
};

//LIST VARS
struct list_head *pos, *for_freeing;
struct sub_pid_list pid_list;
struct sub_pid_list *temp;
//END LIST VARS

//function get called upon receiving any message from userspace
static void hello_nl_recv_msg(struct sk_buff *skb)
{
    struct message mes;
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    //msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data; //get data section of the socket buffer (i.e. message from userspace)
    memcpy(&mes, nlmsg_data(nlh),sizeof(struct message));
    if(mes.type==0)
    {
        printk(KERN_INFO"HEY: received subscription, message= %s",mes.message);
        //ADD PID TO LIST
        temp = (struct sub_pid_list *)malloc(sizeof(struct sub_pid_list));
        temp->pid = nlh->nlmsg_pid;
        list_add_tail(&(temp->list), &(pid_list.list));
        //END ADD PID TO LIST
    }
    else
    {
        printk(KERN_INFO "HEY: DATA\n");
        printk(KERN_INFO "HEY: Received publish, message= %s\n",mes.message);
    }
    //printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    list_for_each(pos, &pid_list.list){
        temp = list_entry(pos, struct sub_pid_list, list);
        res = nlmsg_unicast(nl_sk, skb_out, temp->pid);
        if (res < 0) {
            printk(KERN_INFO "Error while sending back to user\n");
        }
    }
}

//function gets called upon inserting module
static int __init hello_init(void)
{

    printk("Entering: %s\n", __FUNCTION__);
    //nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg, NULL, THIS_MODULE);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    //LIST INIT
    INIT_LIST_HEAD(&pid_list.list);


    return 0;
}
//function gets called upon removing module
static void __exit hello_exit(void)
{
    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);

    //LIST FREE
    list_for_each_safe(pos, for_freeing, &pid_list.list){
        temp = list_entry(pos, struct sub_pid_list, list);
        printk(KERN_INFO "FREEING ITEM: %d\n", temp->pid);
        list_del(pos);
        free(temp);
    }

}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");
