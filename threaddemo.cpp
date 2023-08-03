#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

class test {
   public:
    int a;
    int b;
};

void* hello(void* input) {
    class test* x = (test*)input;
    printf("Hello, world! %d %d\n", x->a, x->b);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t tid;
    test* x = new test();
    x->a = 1;
    x->b = 2;

    pthread_create(&tid, NULL, hello, (void*)x);
    pthread_join(tid, NULL);
    return 0;
}