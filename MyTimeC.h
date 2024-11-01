#ifndef MYTIMEC_H
#define MYTIMEC_H

#include <stdio.h>
#include <sys/time.h>

typedef struct MyTimeC
{
    struct timeval _t;
    void (*start)(struct MyTimeC *);
    double (*now)(struct MyTimeC *, const char *);
    double (*reset)(struct MyTimeC *, const char *);
}MyTimeC;

void start(struct MyTimeC *myTimeC)
{
    gettimeofday(&myTimeC->_t, NULL);
}

double now(struct MyTimeC *myTimeC, const char *key)
{
    struct timeval _n;
    gettimeofday(&_n, NULL);
    double duration = (1000000 * (_n.tv_sec - myTimeC->_t.tv_sec) + _n.tv_usec - myTimeC->_t.tv_usec) / 1000.0;

    if(key)
    {
        printf("%s time cost is: %f ms\n", key, duration);
    }

    return duration;
}

double reset(struct MyTimeC *myTimeC, const char *key)
{
    double duration = myTimeC->now(myTimeC, NULL);
    myTimeC->start(myTimeC);

    if(key)
    {
        printf("%s time cost is: %f ms\n", key, duration);
    }

    return duration;
}

void init(struct MyTimeC *myTimeC)
{
    start(myTimeC);
    myTimeC->start = start;
    myTimeC->now = now;
    myTimeC->reset = reset;
}

#endif //MYTIMEC_H
