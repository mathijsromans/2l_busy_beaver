#pragma once

#include "field.h"
#include <array>

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
