#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>

const int max_size = 10;

int XY(int x, int y)
{
    return y*max_size+x;
}

class Field
{
public:
    Field()
    {
        memset(pbuf, 0, max_size*max_size);
    }

    bool operator==(Field const& other) const
    {
        return std::equal(std::begin(pbuf), std::end(pbuf), std::begin(other.pbuf));
    }

    bool operator!=(Field const& other) const
    {
        return !(*this == other);
    }

    char get(int x, int y) const
    {
        return pbuf[XY(x, y)];
    }

    void set(int x, int y, char c)
    {
        pbuf[XY(x, y)] = c;
    }

    void set(int y, std::string const& line)
    {
        strcpy(pbuf + XY(0, y), line.c_str());
    }

    int size() const
    {
        int sz = 0;
        for (int y = 0; get(0, y) != '\0' && y <= max_size; y++) {
           for (int x = 0; get(0, x) != '\0' && x <= max_size; x++) {
               char c = get(x,y);
               if (c == '+' || c == '*') {
                   sz = std::max(sz, x);
                   sz = std::max(sz, y);
               }
           }
        }
        return sz;
    }

    void print() const
    {
        print(-1, -1);
    }

    void print(int px, int py) const
    {
        for (int y = 0; get(0, y) != '\0' && y <= max_size; y++) {
           for (int x = 0; get(0, x) != '\0' && x <= max_size; x++) {
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
    char pbuf[max_size*max_size];
};

Field first_field(int size)
{
    Field f;
    for (int y = 0; y != size-1; y++) {
        for (int x = 0; x != size-1; x++) {
            f.set(x, y, ' ');
        }
    }
    for( int i = 0; i != size; ++i )
    {
        f.set(i, size-1, '+');
        f.set(size-1, i, '+');
        f.set(i, size, '\0');
        f.set(size, i, '\0');
    }
    return f;
}

Field next(Field const& orig)
{
    Field f = orig;
    int size = f.size();
    int x = 0;
    int y = 0;
    bool done = false;
    while(!done) {
        char c = f.get(x, y);
        switch(c) {
            case '+':
                f.set(x, y, '*');
                done = true;
                break;
            case '*':
                f.set(x, y, ' ');
                break;
            default:
                f.set(x, y, '+');
                done = true;
                break;
        }
        x = (x+1) % size;
        if ( x == 0 ) {
            y = (y+1) % size;
            if ( y == 0 )
            {
                done = true;
            }
        }
    }
    return f;
}

constexpr unsigned long powr(unsigned long a, unsigned long b)
{
    unsigned long r = 1;
    for ( unsigned long i = 0; i != b; ++i) {
        r *= a;
    }
    return r;
}

Field read_file(std::string const& filename)
{
    Field f;
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

void move(int dir, int& px, int& py, bool& out_of_bounds)
{
    switch (dir) {
        case 0: /* up */
            py--;
            break;
        case 1: /* right */
            px++;
            break;
        case 2: /* down */
            py++;
            break;
        case 3: /* left */
            px--;
            break;
    }
    /* quit if < 0 */
    out_of_bounds = px < 0 || py < 0;
}

unsigned int execute(Field const& f, char *outbuf, unsigned int max_steps, unsigned int dlev)
{
    int px = 0;
    int py = -1;
    int dir = 2;

    const int mem_size = 128;
    char mbuf[mem_size];
    memset(mbuf, 0, mem_size);
    int mloc = mem_size/2;

    unsigned int steps = 0;
    while(true) {
        int next_x = px;
        int next_y = py;
        bool out_of_bounds = false;
        move(dir, next_x, next_y, out_of_bounds);
        if (out_of_bounds) {
            return steps;
        }
        while(f.get(next_x, next_y) == '+') {
            if (mbuf[mloc]) {
                dir = (dir+1)%4; // turn right
            } else {
                dir = (dir+3)%4; // turn left
            }
            next_x = px;
            next_y = py;
            move(dir, next_x, next_y, out_of_bounds);
            if (out_of_bounds) {
                return steps;
            }
        }
        px = next_x;
        py = next_y;
        ++steps;
        if (f.get(px, py) == '*') {
//            if (mloc == 1 && (dir == 1 || dir == 3)) {
//                /* IO */
//                if (mbuf[0] == 0) { /* input */
//                    mbuf[0] = static_cast<char>(getchar());
//                } else { /* output */
//                    /* if debugging, buffer output */
//                    if (dlev != 0) {
//                        sprintf(outbuf+strlen(outbuf), "%c", mbuf[0]);
//                    } else {
//                        putchar(mbuf[0]);
//                        fflush(stdout);
//                    }
//                    mbuf[0] = 0;
//                }
//            } else { /* not IO */
                switch (dir) {
                    case 0: /* up */
                        mloc--;
                        break;
                    case 1: /* right */
                        mbuf[mloc]++;
                        break;
                    case 2: /* down */
                        mloc++;
                        break;
                    case 3: /* left */
                        mbuf[mloc]--;
                        break;
                }
                if (mloc < 0 || mloc >= mem_size)
                {
                    return 0;
                }
//            }
        }

        /* if debugging, output */
        if (dlev != 0) {
            f.print(px, py);
            fflush(stdout);
            printf("%d\n\n", steps);
            usleep(1000000 / dlev);
        }

        if (steps > max_steps) {
            return 0;
        }
    }
}

int main(int argc, char **argv)
{
//    if (argc < 2) {
//        printf("Use: %s <program>\n", argv[0]);
//        return 1;
//    }

    char *outbuf = nullptr;
    unsigned int dlev;

    /* get debug level */
    if (argc > 2) {
        dlev = static_cast<unsigned int>(atoi(argv[2]));
        outbuf = static_cast<char *>(malloc(32256));
        outbuf[0] = '\0';
    } else {
        dlev = 0;
    }

//    Field ff = read_file(argv[1]);
//    execute(ff, outbuf, 100, 10);
//    return 0;

    const unsigned int size = 6;
    Field orig = first_field(size);
    Field f = orig;
    Field best_field = orig;
    unsigned long iter = 0;
    unsigned int max_steps = 0;
    do
    {
//        f.print();
        unsigned int steps = execute(f, outbuf, 400, 0);
//        printf("Total steps: %d\n", steps);
        if ( steps > max_steps ) {
            printf("Found new best with total steps: %d\n", steps);
            f.print();
            max_steps = steps;
            best_field = f;
        }

        f = next(f);
        ++iter;
        if (iter % 10000000 == 0) {
            const unsigned long total_steps = powr(3, (size-1)*(size-1));
            printf("iter = %ld / %ld (%g%%)\n", iter, total_steps, 100.0 * iter / total_steps);
            fflush(stdout);
        }
    }
    while (f != orig);
    printf("Number of fields: %ld, maximum number of steps: %d\n", iter, max_steps);
    best_field.print();
    return 0;
}
