#pragma once

#include "field.h"
#include <array>

template <int N>
class State
{
private:
    const static int mem_size = 10000;
    std::array<int, mem_size> mbuf{};
    std::array<int, mem_size>::size_type mloc{0};
    std::array<int, mem_size>::size_type m_min_mloc{0};
    std::array<int, mem_size>::size_type m_max_mloc{0};

public:
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
    }

    bool operator==(State<N> const& other) const {
        return pos == other.pos &&
                d == other.d &&
                mbuf == other.mbuf &&
                mloc == other.mloc;
    }

    int get_mem() const
    {
        return mbuf[mloc];
    }

    void incr_mem()
    {
        ++mbuf[mloc];
    }

    void decr_mem()
    {
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

    void print() const
    {
        std::cout << "mloc=" << mloc << "   ";
        for (int i = -4; i != 5; ++i) {
            std::cout << static_cast<int>(mbuf[mloc+i]) << " ";
        }
        std::cout << std::endl;
    }
};
