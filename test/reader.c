#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

void checkpoint_now()
{
  printf("Checkpoint now.\n");
}

int main()
{
  int fd = -1;
  size_t retval;
  int int_retval;
  FILE *file;
  int buf[2] = {16, 32};
  int in_buf[2] = {0};

  file = tmpfile();
  assert(file != NULL);
  
  /* Write some data to the file. */
  retval = fwrite(&buf[0], sizeof(buf[0]), 1, file);
  assert(retval == 1);
  retval = fwrite(&buf[1], sizeof(buf[0]), 1, file);
  assert(retval == 1);
  int_retval = fseek(file, 0, SEEK_SET);
  assert(int_retval == 0);

  /* Read first half. */
  retval = fread(&in_buf[0], sizeof(in_buf[0]), 1, file);
  assert(retval == 1);
  checkpoint_now();

  retval = fread(&in_buf[1], sizeof(in_buf[0]), 1, file);
  assert(retval == 1);

  fclose(file);

  assert(in_buf[0] == buf[0] && in_buf[1] == buf[1]);

  return 0;
}
