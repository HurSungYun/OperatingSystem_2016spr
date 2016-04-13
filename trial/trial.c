#include<stdio.h>
#include<linux/rotation.h>
#include<linux/types.h>
#define ROTLOCK_READ 385
#define ROTLOCK_WRITE 386
#define ROTUNLOCK_READ 387
#define ROTUNLOCK_WRITE 388

#define DEFAULT_DEGREE 90
#define DEFAULT_RANGE 90

FILE *input_integer;

int main(int argc, char* argv[])
{
  int ret, n, i, flag, t;
  struct rotation_range test_range;
  test_range.rot.degree = DEFAULT_DEGREE;
  test_range.degree_range = DEFAULT_RANGE;
  
  while(true){
    printf("read started\n");
    ret = syscall(ROTLOCK_READ, &test_range);
    input_integer = fopen("integer","r");
    fscanf(input_integer, "%d", &n);
    
    printf("read %d\n",n);

    t = n;
    // if ( n == 1 ) ERROR!!!!

    while(t!=1){
      flag = 0;
      for(i = 2;i <= t; i++){
        if(t % i == 0){
          printf("%d ",i);
          t = t / i;
          flag = 1;
          break;
        }
      }
    }
    printf("\n");
    fclose(input_integer);
    ret = syscall(ROTUNLOCK_READ, &test_range);
  }

  return 0;
} 
