#include<stdlib.h>
#include<stdio.h>

struct list_el {
  int val;
  struct list_el * next;
};

typedef struct list_el item;

item * newItem(int i);
int list_len(item *elt);

// Make head global, so list_len(head) always works.
item * head = NULL;

int main() {
  item * tail;
  int i;

  head = newItem(1);
  tail = head;
  printf(" NODE VAL: %d\n", tail->val);
  for(i=2;i<=20;i++) {
    tail->next = newItem(i);
    tail = tail->next;
    printf(" NODE VAL: %d\n", tail->val);
  }
  printf("Linked list length is now: %d\n", list_len(head));
  printf ("Ok we crossed the limit."
          "  Let's go back to just before the list had size 10.\n");
  return 0;
}

item * newItem(int i) {
  item * tmp = malloc(sizeof(item));
  tmp->val = i;
  tmp->next = NULL;
  return tmp;
}
int list_len(item *elt) {
  int count = 0;
  while (elt != NULL) {
    elt = elt->next;
    count++;
  }
  return count;
}
