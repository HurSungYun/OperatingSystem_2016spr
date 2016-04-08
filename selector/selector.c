#include<stdio.h>
#include<linux/rotation.h>

#define ROTLOCK_READ 385
#define ROTLOCK_WRITE 386
#define ROTUNLOCK_READ 387
#define ROTUNLOCK_WRITE 388

#define DEFAULT_DEGREE 180
#define DEFAULT_RANGE 30

FILE *input_integer;

int main(int argc, char* argv[])
{
  int ret;
  int count = atoi(argv[1]);  /* TODO: when argument is NULL */

  struct rotation_range test_range;
  test_range.rot.degree = DEFAULT_DEGREE;
  test_range.degree_range = DEFAULT_RANGE;

  printf("Selector program executed\n");

  
  while(1){
    printf("syscall started\n");
    ret = syscall(ROTLOCK_WRITE, &test_range);
    printf("syscall ended\n");
    input_integer = fopen("integer","w");
    printf("file created\n");
    fprintf(input_integer, "%d", count);
    printf("current number : %d\n",count);
    fclose(input_integer);
    ret = syscall(ROTUNLOCK_WRITE, &test_range);
    count++;
  }
  return 0;
}
