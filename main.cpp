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
    const static int mem_size = 256;
    const static int mem_limit = 256;
    std::array<int, mem_size> mbuf{};
    int mloc = mem_size/2;

    bool memory_out_of_bounds() const
    {
        return mloc < 0 || mloc >= mem_size ||
        mbuf[mloc] == -mem_limit ||
        mbuf[mloc] == +mem_limit;
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
                if (debug_level) {
                    std::cout << "mloc=" << s.mloc << "   " <<
                                 static_cast<int>(s.mbuf[s.mloc-1]) << " " <<
                                 static_cast<int>(s.mbuf[s.mloc]) << " " <<
                                 static_cast<int>(s.mbuf[s.mloc+1]) << std::endl;
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
                usleep(100000);
            }
        }
        return 0;
    }
};

unsigned long myPow(unsigned long x, unsigned int p)
{
    unsigned long result = 1;
    for (unsigned int i = 0; i != p; ++i) {
        result *= x;
    }
    return result;
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

    const int SIZE = 5;
    unsigned long iter = 0;
    unsigned long iter_start = 0;
    unsigned long last_iter_div = -1;
    unsigned int max_steps = 0;
    Field<SIZE> orig = from_iter<SIZE>(iter_start);
    Field<SIZE> f = orig;
    Field<SIZE> best_field = orig;
    do
    {
        Run<SIZE> r(f);
        unsigned int steps = r.execute(4000);
        if ( steps > max_steps ) {
            std::cout << "Found #" << iter << " new best with total steps: " << steps << std::endl;
            f.print();
            max_steps = steps;
            best_field = f;
        }

        unsigned int skip_size = Pos<SIZE>(SIZE-1, SIZE-1).get() - r.max_pos().get();
        const unsigned int nr_of_symbols = 3;
        unsigned long incr_steps = myPow(nr_of_symbols, skip_size);
        if (debug_level) {
            std::cout << "with max_pos " << r.max_pos() << ", skipping " << skip_size << " positions -> incrementing " << incr_steps << " steps" << std::endl;
        }
        f.next(r.max_pos());
        iter += incr_steps;
        unsigned long iter_div = iter / 1000000000;
        if (iter_div != last_iter_div || debug_level != 0) {
            last_iter_div = iter_div;
            const unsigned long total_steps = powr(3, SIZE*SIZE);
            printf("iter = %ld / %ld (%g%%)\n", iter, total_steps, 100.0 * iter / total_steps);
            fflush(stdout);
        }
    }
    while (f != orig);
    printf("Number of fields: %ld, maximum number of steps: %d\n", iter, max_steps);
    best_field.print();
    return 0;
}
