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

const unsigned int debug_level = 0;

constexpr unsigned long powr(unsigned long a, unsigned long b)
{
    unsigned long r = 1;
    for ( unsigned long i = 0; i != b; ++i) {
        r *= a;
    }
    return r;
}

template <int N>
class State
{
public:
    Pos<N> pos{-1, 0};
    int d = 1;
    const static int mem_size = 128;
    std::array<char, mem_size> mbuf{};
    int mloc = mem_size/2;

    bool memory_out_of_bounds() const
    {
        return mloc < 0 || mloc >= mem_size;
    }
};

template <int N>
class Run
{
private:
    Field<N> const& m_f;
    Pos<N> m_max_pos;
public:
    explicit Run(Field<N> const& f) : m_f(f), m_max_pos(-1, -1) {}
    Pos<N> max_pos() const { return m_max_pos; }

    unsigned int execute(unsigned int max_steps)
    {
        State<N> s;
        for(unsigned int steps = 0; steps != max_steps; ++steps) {
            while(true) {
                Pos<N> next = s.pos;
                bool out_of_bounds = false;
                next.move(s.d, out_of_bounds);
                if (out_of_bounds) {
                    return steps;
                }
                if (m_f.get(next, m_max_pos) != '+') {
                    s.pos = next;
                    break;
                }
                if (s.mbuf[s.mloc]) {
                    s.d = (s.d+1)%4; // turn right
                } else {
                    s.d = (s.d+3)%4; // turn left
                }
            }
            if (m_f.get(s.pos, m_max_pos) == '*') {
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
                if (s.memory_out_of_bounds())
                {
                    return 0;
                }
            }

            if (debug_level != 0) {
                m_f.print(s.pos);
                fflush(stdout);
                printf("%d\n\n", steps);
                usleep(300000);
            }
        }
        return 0;
    }
};

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
        Run<SIZE> r(f);
        unsigned int steps = r.execute(400);
        if ( steps > max_steps ) {
            std::cout << "Found new best with total steps: " << steps << std::endl;
            f.print();
            std::cout << "with max_pos " << r.max_pos() << std::endl;

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
