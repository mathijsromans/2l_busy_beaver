#pragma once

#include "global.h"
#include "field.h"
#include "state.h"
#include <bitset>
#include <unistd.h>

template <int N>
class Run
{
private:
    Field<N> const* m_f;
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
        return m_f->get(p);
    }

public:
    explicit Run() : m_f(nullptr) {}

    void reset(Field<N> const& f)
    {
        m_f = &f;
        m_s.reset();
//        m_previous_state.reset(); // NOTE: for optimization this step is skipped!
        m_previous_state_step = 0;
        m_loop_detection_period = 0;
        serial_used.reset();
        serials_used.clear();
    }

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
        m_f->print(m_s.pos);
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
                    m_f->print();
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
                    m_f->print();
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

