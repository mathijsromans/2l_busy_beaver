#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <cassert>
#include <unistd.h>

template <unsigned int N>
int XY(int x, int y)
{
    return y*N+x;
}

template <unsigned int N>
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
        int sizeOfArray = sizeof(order) / sizeof(order[0]);
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

    constexpr unsigned int size()
    {
        return N;
    }

    void print() const
    {
        print(-1, -1);
    }

    void print(int px, int py) const
    {
        for (int y = 0; y < N; y++) {
           for (int x = 0; x < N; x++) {
                char c = get(x, y);
                if( x == px && y == py ) {
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

template <unsigned int N>
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

template <unsigned int N>
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

