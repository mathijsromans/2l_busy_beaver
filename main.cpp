#include "field.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>
#include <unistd.h>


constexpr unsigned long powr(unsigned long a, unsigned long b)
{
    unsigned long r = 1;
    for ( unsigned long i = 0; i != b; ++i) {
        r *= a;
    }
    return r;
}

template <unsigned int N>
void move(int dir, int& px, int& py)
{
    switch (dir) {
        case 0: /* up */
            py--;
            break;
        case 1: /* right */
            px++;
            break;
        case 2: /* down */
            py++;
            break;
        case 3: /* left */
            px--;
            break;
    }
}

template <unsigned int N>
bool out_of_bounds(int px, int py)
{
    return px < 0 || py < 0 || px >= N || py >= N;
}

template <unsigned int N>
unsigned int execute(Field<N> const& f, char *outbuf, unsigned int max_steps, unsigned int dlev)
{
    int px = 0;
    int py = -1;
    int dir = 2;

    const int mem_size = 128;
    char mbuf[mem_size];
    memset(mbuf, 0, mem_size);
    int mloc = mem_size/2;

    unsigned int steps = 0;
    while(true) {
        int next_x = px;
        int next_y = py;
        move<N>(dir, next_x, next_y);
        if (out_of_bounds<N>(next_x, next_y)) {
            return steps;
        }
        while(f.get(next_x, next_y) == '+') {
            if (mbuf[mloc]) {
                dir = (dir+1)%4; // turn right
            } else {
                dir = (dir+3)%4; // turn left
            }
            next_x = px;
            next_y = py;
            move<N>(dir, next_x, next_y);
            if (out_of_bounds<N>(next_x, next_y)) {
                return steps;
            }
        }
        px = next_x;
        py = next_y;
        ++steps;
        if (f.get(px, py) == '*') {
            switch (dir) {
                case 0: /* up */
                    mloc--;
                    break;
                case 1: /* right */
                    mbuf[mloc]++;
                    break;
                case 2: /* down */
                    mloc++;
                    break;
                case 3: /* left */
                    mbuf[mloc]--;
                    break;
            }
            if (mloc < 0 || mloc >= mem_size)
            {
                return 0;
            }
        }

        /* if debugging, output */
        if (dlev != 0) {
            f.print(px, py);
            fflush(stdout);
            printf("%d\n\n", steps);
            usleep(1000000 / dlev);
        }

        if (steps > max_steps) {
            return 0;
        }
    }
}

int myPow(int x, int p) {
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * myPow(x, p-1);
}

void test_next()
{
    const unsigned int SIZE = 4;
    Field<SIZE> orig = first_field<SIZE>();

    Field<SIZE> a = orig;
    for (unsigned i = 0; i != 10; ++i) {
        a.print();
        a.next();
    }

    unsigned int i = 1;
    Field<SIZE> f = orig;
    f.next();
    for (; f != orig; f.next())
    {
        ++i;
    }
    assert(i == myPow(3, SIZE*SIZE));
    std::cout << i << std::endl;
}

int main(int argc, char **argv)
{
//    if (argc < 2) {
//        printf("Use: %s <program>\n", argv[0]);
//        return 1;
//    }

//    test_next();
//    return 0;

    char *outbuf = nullptr;
    unsigned int dlev;

    /* get debug level */
    if (argc > 2) {
        dlev = static_cast<unsigned int>(atoi(argv[2]));
        outbuf = static_cast<char *>(malloc(32256));
        outbuf[0] = '\0';
    } else {
        dlev = 1;
    }

//    Field<SIZE> ff = read_file(argv[1]);
//    execute(ff, outbuf, 100, 10);
//    return 0;

    const unsigned int SIZE = 4;
    Field<SIZE> orig = first_field<SIZE>();
    Field<SIZE> f = orig;
    Field<SIZE> best_field = orig;
    unsigned long iter = 0;
    unsigned int max_steps = 0;
    do
    {
//        f.print();
        unsigned int steps = execute(f, outbuf, 400, 0);
//        printf("Total steps: %d\n", steps);
        if ( steps > max_steps ) {
            printf("Found new best with total steps: %d\n", steps);
            f.print();
            max_steps = steps;
            best_field = f;
        }

        f.next();
        ++iter;
        if (iter % 10000000 == 0) {
            const unsigned long total_steps = powr(3, (SIZE)*(SIZE));
            printf("iter = %ld / %ld (%g%%)\n", iter, total_steps, 100.0 * iter / total_steps);
            fflush(stdout);
        }
    }
    while (f != orig);
    printf("Number of fields: %ld, maximum number of steps: %d\n", iter, max_steps);
    best_field.print();
    return 0;
}
