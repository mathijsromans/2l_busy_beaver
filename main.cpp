#include "field.h"
#include "run.h"
#include "state.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <iterator>

constexpr unsigned long powr(unsigned long a, unsigned long b)
{
    unsigned long r = 1;
    for ( unsigned long i = 0; i != b; ++i) {
        r *= a;
    }
    return r;
}

void test_next()
{
    const unsigned int SIZE = 3;
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
    const int SIZE = 6;
    unsigned long iter = 0;
    unsigned long iter_start = 0;
    unsigned long last_iter_div = -1;
    unsigned int max_steps = 0;
    Field<SIZE> orig = from_iter<SIZE>(iter_start);
    Field<SIZE> best_field = orig;
    Field<SIZE> f = orig;
    if (!filename.empty()) {
        f = read_file<SIZE>(filename);
    }
    auto start_time = std::chrono::steady_clock::now();
    Run<SIZE> r;
    do
    {
        r.reset(f);
        unsigned int steps = r.execute(10000);
        if ( steps > max_steps ) {
            std::cout << "Found #" << iter << " new best with total steps: " << steps << std::endl;
            f.print();
            max_steps = steps;
            best_field = f;
        }

//        unsigned int skip_size = Pos<SIZE>(SIZE-1, SIZE-1).serial() - r.max_pos_serial();
//        const unsigned int nr_of_symbols = 3;
//        unsigned long incr_steps = myPow(nr_of_symbols, skip_size);
        if (debug_level) {
            std::cout << "with get_serials_used: ";
            std::copy(r.get_serials_used().begin(), r.get_serials_used().end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
        }
        f.next(r.get_serials_used());
//        iter += incr_steps;
        unsigned long iter_div = iter / 1000000000;
        if (iter_div != last_iter_div || debug_level != 0) {
            last_iter_div = iter_div;
            const unsigned long total_steps = powr(3, SIZE*SIZE);
            printf("iter = %ld / %ld (%g%%)\n", iter, total_steps, 100.0 * iter / total_steps);
            fflush(stdout);
        }
    }
    while (f != orig);
    auto current_time = std::chrono::steady_clock::now();
    unsigned int duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    std::cout << "Evalution took " << duration_ms/1000 << " seconds, " << duration_ms%1000 << " milliseconds" << std::endl;

    printf("Number of fields: %ld, maximum number of steps: %d\n", iter, max_steps);
    best_field.print();
    return 0;
}


