#pragma once

#include "field.h"
#include <array>

template <int N> class Monitor;

template <int N>
class State
{
public:
    const static int mem_size = 30000;

private:
    std::array<int, mem_size>::size_type m_min_mloc{0};
    std::array<int, mem_size>::size_type m_max_mloc{0};
    Monitor<N>* m_monitor{nullptr};

public:

    std::array<int, mem_size> mbuf{};
    std::array<int, mem_size>::size_type mloc{0};
    Pos<N> pos{-1, 0};
    int d = 1;

    void reset()
    {
        pos = Pos<N>{-1, 0};
        d = 1;
        std::fill(std::begin(mbuf)+m_min_mloc, std::begin(mbuf)+m_max_mloc + 1, 0);
        mloc = mem_size/2;
        m_min_mloc = mloc;
        m_max_mloc = mloc;
        m_monitor = nullptr;
    }

    bool operator==(State<N> const& other) const {
        return pos == other.pos &&
                d == other.d &&
                mbuf == other.mbuf &&
                mloc == other.mloc;
    }

    void mem_used() const {
        if (m_monitor) {
            m_monitor->mem_used();
        }
    }

    int get_mem() const
    {
        mem_used();
        return mbuf[mloc];
    }

    void incr_mem()
    {
        mem_used();
        ++mbuf[mloc];
    }

    void decr_mem()
    {
        mem_used();
        --mbuf[mloc];
    }

    void incr_mem_loc()
    {
        ++mloc;
        if ( mloc > m_max_mloc ) {
            m_max_mloc = mloc;
        }
    }

    void decr_mem_loc()
    {
        --mloc;
        if ( mloc < m_min_mloc ) {
            m_min_mloc = mloc;
        }
    }

    bool memory_out_of_bounds() const
    {
        return mloc < 0 || mloc >= mem_size;
    }

    void set_monitor(Monitor<N>* monitor) {
        m_monitor = monitor;
    }

    void print() const
    {
        std::cout << "mloc=" << mloc << "   ";
        for (int i = -9; i != 10; ++i) {
            std::cout << static_cast<int>(mbuf[mloc+i]) << " ";
        }
        std::cout << std::endl;
    }
};

template <int N>
class Monitor
{
private:
    State<N> const& m_s;
    State<N> m_original;
    using mem_loc_type = typename std::array<int, State<N>::mem_size>::size_type;
    mem_loc_type m_min_mloc{0};
    mem_loc_type m_max_mloc{0};

public:
    explicit Monitor(State<N> const& state) : m_s(state) {}

    bool detect_loop() const {
        if (m_s.pos != m_original.pos ||
            m_s.d != m_original.d ) {
            return false;
        }
        mem_loc_type mem_move = m_s.mloc - m_original.mloc;
        for (mem_loc_type mloc = m_min_mloc; mloc != m_max_mloc+1; ++mloc) {
            if (m_s.mbuf[mloc+mem_move] != m_original.mbuf[mloc])
            {
                return false;
            }
        }
        return true;
    }

    void start() {
        m_min_mloc = m_s.mloc;
        m_max_mloc = m_s.mloc;
        m_original = m_s;
    }

    void mem_used() {
        m_max_mloc = std::max(m_max_mloc, m_s.mloc);
        m_min_mloc = std::min(m_min_mloc, m_s.mloc);
    }
};

