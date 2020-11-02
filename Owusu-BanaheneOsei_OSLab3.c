#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define pass (void)0

void main(int argc, char *argv[]) {

  int processPipe[2];

  pipe(processPipe) == -1 ? exit(1) : pass;

  pid_t childProcess = fork();

  childProcess < 0 ? exit(1) : pass;

  if (!childProcess) {

    close(processPipe[0]);

    int sum1, pro1, dif1;
    float div1;

    for (int i = 1; i < argc; i++) {

      if (i == 1) {

        // initialize variables
        sum1 = pro1 = dif1 = atoi(argv[i]);
        div1 = atof(argv[i]);

      } else {

        // continue computation
        sum1 += atoi(argv[i]);
        pro1 *= atoi(argv[i]);
        dif1 -= atoi(argv[i]);
        div1 /= atof(argv[i]);
      }
    }

    write(processPipe[1], &sum1, sizeof(sum1));
    write(processPipe[1], &pro1, sizeof(pro1));
    write(processPipe[1], &dif1, sizeof(dif1));
    write(processPipe[1], &div1, sizeof(div1));

    close(processPipe[1]);

  } else {

    close(processPipe[1]);

    int sum2, pro2, dif2;
    float div2;

    read(processPipe[0], &sum2, sizeof(sum2));
    read(processPipe[0], &pro2, sizeof(pro2));
    read(processPipe[0], &dif2, sizeof(dif2));
    read(processPipe[0], &div2, sizeof(div2));

    printf("Sum -> %d\n", sum2);
    printf("Product -> %d\n", pro2);
    printf("Difference -> %d\n", dif2);
    printf("Divide -> %3.5f\n", div2);

    close(processPipe[0]);
  }
}