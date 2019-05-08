#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <time.h>

struct sub_pid_list{
    int pid;
    struct list_head list;
};

void main (void) {
    srand(time(0));

    printf("\n---------------------------------------------------------\n");
    printf("---------");
    printf("\nLOAD LIST\n");
    printf("---------\n\n");
    struct list_head *pos;
    struct sub_pid_list pid_list;
    INIT_LIST_HEAD(&pid_list.list);
    struct sub_pid_list *temp;

    for (int i = 0; i < 5; i++) {
        temp = (struct sub_pid_list *)malloc(sizeof(struct sub_pid_list));
        int to_add = rand()%100;
        printf("TO ADD: %d\n", to_add);
        temp->pid = to_add;
        printf("TEMP PID: %d\n", temp->pid);
        list_add_tail(&(temp->list), &(pid_list.list));
    }

    printf("\n-----------");
    printf("\nLIST LOADED\n");
    printf("-----------\n");
    printf("\n-------------------------------");
    printf("\nTRAVERSE LIST WITH LIST FOREACH\n");
    printf("-------------------------------\n\n");

    list_for_each(pos, &pid_list.list){
        temp = list_entry(pos, struct sub_pid_list, list);
        printf("LIST TRAVERSE RESULT: %d\n", temp->pid);
    }

    printf("\n------------------");
    printf("\nEND LIST TRAVERSAL\n");
    printf("------------------");

    printf("\n---------------------------------------------------------\n");
}
