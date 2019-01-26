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
    MainLoopDetector<N> m_loop_detector;
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
    explicit Run() :
        m_f(nullptr),
        m_s(),
        m_loop_detector(m_s)
    {
    }

    void reset(Field<N> const& f)
    {
        // note that the m_loop_detector is not reset
        m_f = &f;
        m_s.reset();
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
        m_s.print();
        fflush(stdout);
        printf("%d\n\n", steps);
        usleep(200000);
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
            if (m_s.get_mem()) {
                m_s.d = (m_s.d+1)%4; // turn right
            } else {
                m_s.d = (m_s.d+3)%4; // turn left
            }
        }
        if (get(m_s.pos) == '*') {
            switch (m_s.d) {
                case 0: /* up */
                    m_s.decr_mem_loc();
                    break;
                case 1: /* right */
                    m_s.incr_mem();
                    break;
                case 2: /* down */
                    m_s.incr_mem_loc();
                    break;
                case 3: /* left */
                    m_s.decr_mem();
                    break;
            }
            if (m_s.memory_out_of_bounds())
            {
                return StepResult::overflow;
            }
        }
        return StepResult::ok;
    }

    bool detect_loop(unsigned int step)
    {
        const unsigned int start_detection_steps = 30;
        const unsigned int stop_detection_steps = 5000;
        if (step > stop_detection_steps) {
            return false;
        }
        if (step == start_detection_steps ||
            (m_loop_detection_period && step == m_previous_state_step + m_loop_detection_period)) {
            if (m_loop_detection_period && m_loop_detector.detect_loop())
            {
                if (debug_level != 0) {
                    std::cout << "Loop detected:" << std::endl;
                    m_f->print();
                }
                return true;
            }
            m_s.set_loop_detector(&m_loop_detector);
            m_loop_detector.start();
            m_previous_state_step = step;
            ++m_loop_detection_period;
            if (debug_level) {
                std::cout << "Setting loop detection period to " << m_loop_detection_period << std::endl;
            }
        }
        return false;
    }

    enum class ResultType { finite, infinite, error, LAST_VALUE=error };
    struct Result
    {
        ResultType type;
        unsigned int steps;
    };


    Result execute(unsigned int max_steps)
    {
        for(unsigned int step = 0; step != max_steps; ++step) {
            StepResult step_result = do_step();

            if (debug_level != 0) {
                print_state(step);
            }

            if (step_result == StepResult::done) {
                return Result{ResultType::finite, step};
            }
            else if (step_result == StepResult::overflow) {
                if (debug_level != 0) {
                    std::cout << "Overflow detected:" << std::endl;
                    m_f->print();
                }
                return Result{ResultType::error, 0};
            }

            if (detect_loop(step)) {
                return Result{ResultType::infinite, 0};
            }

        }
        return Result{ResultType::error, 0};
    }

    std::vector<int> const& get_serials_used() const
    {
        return serials_used;
    }
};

