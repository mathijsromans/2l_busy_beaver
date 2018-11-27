#include "field.h"
#include <array>
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
void move(int d, int& x, int& y)
{
    switch (d) {
        case 0: /* up */
            y--;
            break;
        case 1: /* right */
            x++;
            break;
        case 2: /* down */
            y++;
            break;
        case 3: /* left */
            x--;
            break;
    }
}

template <int N>
bool out_of_bounds(int x, int y)
{
    return x < 0 || y < 0 || x >= N || y >= N;
}

class State
{
public:
    int x = 0;
    int y = -1;
    int d = 2;
    const static int mem_size = 128;
    std::array<char, mem_size> mbuf{};
    int mloc = mem_size/2;
};

template <int N>
unsigned int execute(Field<N> const& f, unsigned int max_steps, unsigned int dlev)
{
    State s;

    unsigned int steps = 0;
    while(true) {
        int next_x = s.x;
        int next_y = s.y;
        move<N>(s.d, next_x, next_y);
        if (out_of_bounds<N>(next_x, next_y)) {
            return steps;
        }
        while(f.get(next_x, next_y) == '+') {
            if (s.mbuf[s.mloc]) {
                s.d = (s.d+1)%4; // turn right
            } else {
                s.d = (s.d+3)%4; // turn left
            }
            next_x = s.x;
            next_y = s.y;
            move<N>(s.d, next_x, next_y);
            if (out_of_bounds<N>(next_x, next_y)) {
                return steps;
            }
        }
        s.x = next_x;
        s.y = next_y;
        ++steps;
        if (f.get(s.x, s.y) == '*') {
            switch (s.d) {
                case 0: /* up */
                    s.mloc--;
                    break;
                case 1: /* right */
                    s.mbuf[s.mloc]++;
                    break;
                case 2: /* down */
                    s.mloc++;
                    break;
                case 3: /* left */
                    s.mbuf[s.mloc]--;
                    break;
            }
            if (s.mloc < 0 || s.mloc >= s.mem_size)
            {
                return 0;
            }
        }

        /* if debugging, output */
        if (dlev != 0) {
            f.print(s.x, s.y);
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

int main()
{
//    test_next();
//    return 0;

    const int SIZE = 4;
    Field<SIZE> orig = first_field<SIZE>();
    Field<SIZE> f = orig;
    Field<SIZE> best_field = orig;
    unsigned long iter = 0;
    unsigned int max_steps = 0;
    do
    {
        unsigned int steps = execute(f, 400, 0);
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
