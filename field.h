#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>
#include <unistd.h>

template <int N>
int XY(int x, int y)
{
    return y*N+x;
}

template <int N>
class Pos
{
public:
    int m_x, m_y;

    Pos(int x, int y) : m_x(x), m_y(y) {}

    int get() const
    {
        return m_y*N + m_x;
    }

    bool operator==(Pos other) const
    {
        return m_x == other.m_x && m_y == other.m_y;
    }

    void move(int d, bool& out_of_bounds)
    {
        switch (d) {
            case 0: /* up */
                if ( m_y == 0 ) {
                    out_of_bounds = true;
                    return;
                }
                --m_y;
                break;
            case 1: /* right */
                if ( m_x == N-1 ) {
                    out_of_bounds = true;
                    return;
                }
                ++m_x;
                break;
            case 2: /* down */
                if ( m_y == N-1 ) {
                    out_of_bounds = true;
                    return;
                }
                ++m_y;
                break;
            case 3: /* left */
                if ( m_x == 0 ) {
                    out_of_bounds = true;
                    return;
                }
                --m_x;
                break;
        }
    }
};

template <int N>
class Field
{
public:
    Field()
    {
        memset(pbuf, 0, N*N);
    }

    bool operator==(Field<N> const& other) const
    {
        return std::equal(std::begin(pbuf), std::end(pbuf), std::begin(other.pbuf));
    }

    bool operator!=(Field<N> const& other) const
    {
        return !(*this == other);
    }

    char get(Pos<N> p) const
    {
        return pbuf[p.get()];
    }

    char get(int x, int y) const
    {
        return pbuf[XY<N>(x, y)];
    }

    void set(int x, int y, char c)
    {
        pbuf[XY<N>(x, y)] = c;
    }

    void next()
    {
        const char order[] = {' ', '+', '*'};
        unsigned int sizeOfArray = sizeof(order) / sizeof(order[0]);
        for (unsigned int i = N*N-1; i != static_cast<unsigned>(-1); --i) {
            for (unsigned int j = 0; j != sizeOfArray-1; ++j) {
                if (pbuf[i] == order[j]) {
                    pbuf[i] = order[j+1];
                    return;
                }
            }
            pbuf[i] = order[0];
        }
    }

    constexpr int size()
    {
        return N;
    }

    void print() const
    {
        print(Pos<N>{-1, -1});
    }

    void print(Pos<N> p) const
    {
        for (int y = 0; y != N; y++) {
           for (int x = 0; x != N; x++) {
                char c = get(x, y);
                if( Pos<N>{x, y} == p ) {
                    c = '@';
                }
                printf("%c", c);
            }
            printf("\n");
        }
        printf("\n");
    }

private:
    char pbuf[N*N];
};

template <int N>
Field<N> first_field()
{
    Field<N> f;
    for (int y = 0; y != N; y++) {
        for (int x = 0; x != N; x++) {
            f.set(x, y, ' ');
        }
    }
    return f;
}

template <int N>
Field<N> read_file(std::string const& filename)
{
    Field<N> f;
    std::ifstream infile(filename);
    int y = 0;
    for( std::string line; getline( infile, line ); ) {
        if (line.back() == '\n') {
            line.pop_back();
        }
        f.set(y, line);
        y++;
    }
    return f;
}

