#pragma once

#include "field.h"
#include <array>

template <int N> class Monitor;

/**
 * Array with efficient reset
 */
template <int SZ>
class TrackedArray
{
public:
    using size_type = typename std::array<int, SZ>::size_type;

    TrackedArray() {
        std::fill(data.begin(), data.end(), 0);
    }

    bool operator==(TrackedArray<SZ> const& other) const {
        return data == other.data;
    }

    int& get_ref(size_type n) {
        mmin_used = std::min(mmin_used, n);
        mmax_used = std::max(mmax_used, n);
        return data[n];
    }

    int get(size_type n) const {
        return data[n];
    }

    void set(size_type n, int value) {
        mmin_used = std::min(mmin_used, n);
        mmax_used = std::max(mmax_used, n);
        data[n] = value;
    }

    void clear()
    {
        if (mmin_used <= mmax_used)
        {
            std::fill(data.begin() + mmin_used, data.begin() + mmax_used + 1, 0);
        }
        mmin_used = SZ;
        mmax_used = 0;
    }

private:
    std::array<int, SZ> data{};
    size_type mmin_used = SZ;
    size_type mmax_used = 0;
};

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

    TrackedArray<mem_size> mbuf;
    TrackedArray<mem_size>::size_type mloc{0};
    Pos<N> pos{-1, 0};
    int d = 1;

    void reset()
    {
        pos = Pos<N>{-1, 0};
        d = 1;
        mbuf.clear();
        mloc = mem_size/2;
        m_min_mloc = mloc;
        m_max_mloc = mloc;
        m_monitor = nullptr;
    }

    bool operator==(State<N> const& other) const {
        return pos == other.pos &&
                d == other.d &&
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
        return mbuf.get(mloc);
    }

    void incr_mem()
    {
        mem_used();
        ++mbuf.get_ref(mloc);
    }

    void decr_mem()
    {
        mem_used();
        --mbuf.get_ref(mloc);
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
            std::cout << static_cast<int>(mbuf.get(mloc+i)) << " ";
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
            if (m_s.mbuf.get(mloc+mem_move) != m_original.mbuf.get(mloc))
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

