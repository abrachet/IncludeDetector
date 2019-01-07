
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct se_list_node {
    int entry;
    struct se_list_node* next;
};

struct se_list {
    int size;
    struct se_list_node* head;
};


void
_se_list_add(struct se_list* this, int se)
{
    struct se_list_node* node = (struct se_list_node*) malloc(sizeof(struct se_list_node) );
    this->size++;
    node->entry = se;

    const int value = se;

    struct se_list_node** ins = &this->head;

    for (; *ins && (*ins)->entry < value; ins = &(*ins)->next);

    node->next = *ins;
    *ins = node;
}

void
ll_print(struct se_list this)
{
    for (struct se_list_node* curr = this.head; curr; curr = curr->next)
        printf("%d\n", curr->entry);
}

    /*
    struct se_list sel = {0};

    int arr[3] = {5, 100, 3};

    for (int i = 0; i < 3; i++)
        _se_list_add(&sel, arr[i]);

    ll_print(sel);
    */


int main()
{   
    void* _label = &&label;

    goto *_label;

    return 0;

label:
    puts("hello");
}