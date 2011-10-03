#include <stdlib.h>

#define MAX 20

int vector[MAX];
int head_index;

void finished()
{
  // No-op. Used as a placeholder for FReD testing.
}

int list_len()
{
  return head_index;
}

void add_element(int i)
{
  vector[head_index++] = i;
}

int main()
{
  int i = 0;
  head_index = 0;
  for (i = 0; i < MAX; i++) {
    add_element(i);
  }

  finished();
  return 0;
}
