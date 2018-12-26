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
    const unsigned int SIZE = 5;
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
    const int SIZE = 7;
    unsigned long iter = 0;
    unsigned long iter_start = 0;
    unsigned int max_steps = 0;
    Field<SIZE> orig = from_iter<SIZE>(iter_start);
    Field<SIZE> best_field = orig;
    Field<SIZE> f = orig;
    std::vector<Field<SIZE>> error_fields;
    unsigned int num_error_fields = 0;
    if (!filename.empty()) {
        f = read_file<SIZE>(filename);
        Run<SIZE> r;
        r.reset(f);
        Run<SIZE>::Result result = r.execute(1000000);
        if (result.type == Run<SIZE>::ResultType::error) {
            std::cout << "Field could not be evaluated: " << std::endl; }
        else if ( result.type == Run<SIZE>::ResultType::finite && result.steps > max_steps ) {
            std::cout << "Found new best with total steps: " << result.steps << std::endl;
        }
        return 0;
    }
    auto start_time = std::chrono::steady_clock::now();
    Run<SIZE> r;
    do
    {
        r.reset(f);
        Run<SIZE>::Result result = r.execute(1000000);
        if (result.type == Run<SIZE>::ResultType::error) {
//            std::cout << "Field could not be evaluated: " << std::endl;
//            f.print();
//            error_fields.push_back(f);
            ++num_error_fields;
        }
        else if ( result.type == Run<SIZE>::ResultType::finite && result.steps > max_steps ) {
            std::cout << "Found new best with total steps: " << result.steps << std::endl;
            f.print();
            max_steps = result.steps;
            best_field = f;
        }
        f.next(r.get_serials_used());
        if (iter % 1000000 == 0 || debug_level != 0) {
            printf("iter = %ld\n", iter);
            fflush(stdout);
        }
        ++iter;
    }
    while (f != orig);
    auto current_time = std::chrono::steady_clock::now();
    unsigned int duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    std::cout << SIZE << "x" << SIZE << std::endl;
    std::cout << "Evalution took " << duration_ms/1000 << " seconds, " << duration_ms%1000 << " milliseconds" << std::endl;
    std::cout << "There were " << num_error_fields << " fields with failed evaluation" << std::endl;

    printf("Number of fields: %ld, maximum number of steps: %d\n", iter, max_steps);
    best_field.print();
    return 0;
}


