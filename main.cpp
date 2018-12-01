#include "field.h"
#include <array>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <deque>
#include <vector>
#include <iostream>
#include <cassert>
#include <unistd.h>

const unsigned int debug_level = 0;
//const unsigned int debug_level = 1;
const std::string filename;
//const std::string filename = "../2l_busy_beaver/2l_busy_beaver/files/infinite_loop.2l";

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

    bool operator==(State<N> const& other) const {
        return pos == other.pos &&
                d == other.d &&
                mbuf == other.mbuf &&
                mloc == other.mloc;
    }


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
    State<N> m_s;
    State<N> m_previous_state;
    unsigned int m_previous_state_step{0};
    unsigned int m_loop_detection_period{0};
    std::bitset<N*N> serial_used;
    std::vector<int> serials_used;

    char get(Pos<N> p)
    {
        int s = p.serial();
        if (!serial_used.test(s)) {
            serial_used.set(s);
            serials_used.push_back(s);
        }
        return m_f.get(p);
    }

public:
    explicit Run(Field<N> const& f) : m_f(f) {}
    int max_pos_serial() const
    {

        unsigned int i = serial_used.size()-1;
        for(; i != static_cast<unsigned int>(-1); --i) {
            if (serial_used.test(i)) {
                break;
            }
        }
        if (i == static_cast<unsigned int>(-1)) {
            i = serial_used.size()-1;
        }
        return i;
    }

    void print_state(unsigned int steps)
    {
        m_f.print(m_s.pos);
        std::cout << "mloc=" << m_s.mloc << "   ";
        for (int i = -4; i != 5; ++i) {
            std::cout << static_cast<int>(m_s.mbuf[m_s.mloc+i]) << " ";
        }
        std::cout << std::endl;
        fflush(stdout);
        printf("%d\n\n", steps);
        usleep(500000);
    }

    enum class StepResult { ok, done, overflow };

    StepResult do_step()
    {
        while(true) {
            Pos<N> next = m_s.pos;
            bool out_of_bounds = false;
            next.move(m_s.d, out_of_bounds);
            if (out_of_bounds) {
                return StepResult::done;
            }
            if (get(next) != '+') {
                m_s.pos = next;
                break;
            }
            if (m_s.mbuf[m_s.mloc]) {
                m_s.d = (m_s.d+1)%4; // turn right
            } else {
                m_s.d = (m_s.d+3)%4; // turn left
            }
        }
        if (get(m_s.pos) == '*') {
            switch (m_s.d) {
                case 0: /* up */
                    m_s.mloc--;
                    break;
                case 1: /* right */
                    m_s.mbuf[m_s.mloc]++;
                    break;
                case 2: /* down */
                    m_s.mloc++;
                    break;
                case 3: /* left */
                    m_s.mbuf[m_s.mloc]--;
                    break;
            }
            if (m_s.memory_out_of_bounds())
            {
                return StepResult::overflow;
            }
        }
        return StepResult::ok;
    }

    bool loop_detected(unsigned int step)
    {
        if (step == 25 ||
            (m_loop_detection_period && step == m_previous_state_step + m_loop_detection_period)) {
            if (m_loop_detection_period && m_previous_state == m_s)
            {
                if (debug_level != 0) {
                    std::cout << "Loop detected:" << std::endl;
                    m_f.print();
                }
                return true;
            }
            m_previous_state = m_s;
            m_previous_state_step = step;
            ++m_loop_detection_period;
            if (debug_level) {
                std::cout << "Setting loop detection period to " << m_loop_detection_period << std::endl;
            }
        }
        return false;
    }

    unsigned int execute(unsigned int max_steps)
    {
        for(unsigned int step = 0; step != max_steps; ++step) {
            StepResult step_result = do_step();
            if (step_result == StepResult::done) {
                return step;
            }
            else if (step_result == StepResult::overflow) {
                if (debug_level != 0) {
                    std::cout << "Overflow detected:" << std::endl;
                    m_f.print();
                }
                return 0; // presume infinite
            }

            if (loop_detected(step)) {
                return 0;
            }

            if (debug_level != 0) {
                print_state(step);
            }
        }
        return 0;
    }

    std::vector<int> const& get_serials_used() const
    {
        return serials_used;
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


