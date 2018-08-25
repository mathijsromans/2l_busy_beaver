/*
 * Copyright (c) 2004, 2005 Gregor Richards
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>

#define XY(x,y) (((y)*1024)+(x))

class Field
{
public:
    Field()
    {
        memset(pbuf, 0, 1024*1024);
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
        for (int y = 0; get(0, y) != '\0' && y <= 1024; y++) {
           for (int x = 0; get(0, x) != '\0' && x <= 1024; x++) {
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
        for (int y = 0; get(0, y) != '\0' && y <= 1024; y++) {
           for (int x = 0; get(0, x) != '\0' && x <= 1024; x++) {
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
    char pbuf[1024*1024];
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

void move(int dir, int& px, int& py)
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
}

unsigned int execute(Field const& f, char *outbuf, unsigned int max_steps, unsigned int dlev)
{
    int px = 0;
    int py = 0;
    int dir = 2;

    char mbuf[32256];
    memset(mbuf, 0, 32256);
    int mloc = 2;

    unsigned int steps = 1;
    while(true) {
        switch(f.get(px, py)) {
            case '+': /* branch */
                /* go back */
               --steps;
                switch (dir) {
                    case 0: /* up */
                        py++;
                        break;
                    case 1: /* right */
                        px--;
                        break;
                    case 2: /* down */
                        py--;
                        break;
                    case 3: /* left */
                        px++;
                        break;
                }

                if (mbuf[mloc]) {
                    dir = (dir+1)%4; // turn right
                } else {
                    dir = (dir+3)%4; // turn left
                }
                break;

            case '*': /* memory operator */
                if (mloc == 1 && (dir == 1 || dir == 3)) {
                    /* IO */
                    if (mbuf[0] == 0) { /* input */
                        mbuf[0] = static_cast<char>(getchar());
                    } else { /* output */
                        /* if debugging, buffer output */
                        if (dlev != 0) {
                            sprintf(outbuf+strlen(outbuf), "%c", mbuf[0]);
                        } else {
                            putchar(mbuf[0]);
                            fflush(stdout);
                        }
                        mbuf[0] = 0;
                    }
                } else { /* not IO */
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
                }
                break;
        }

        /* if debugging, output */
        if (dlev != 0) {
            f.print(px, py);
            fflush(stdout);
            //printf("\n%s\n", outbuf);
            printf("%d\n\n", steps);

            usleep(1000000 / dlev);
        }

        /* now move */
        ++steps;
        move(dir, px, py);

        /* quit if < 0 */
        if (px < 0 || py < 0) {
            return steps-1;
        }

        if (steps > max_steps) {
            return 0;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Use: %s <program>\n", argv[0]);
        return 1;
    }

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

//    Field f = read_file(argv[1]);
    Field orig = first_field(3);
    Field f = orig;
    unsigned int iter = 0;
    unsigned int max_steps = 0;
    do
    {
        f.print();
        unsigned int steps = execute(f, outbuf, 100, 400);
        printf("Total steps: %d\n", steps);
        max_steps = std::max(max_steps, steps);
        f = next(f);
        ++iter;
    }
    while (f != orig);
    printf("Number of fields: %d, maximum number of steps: %d\n", iter, max_steps);
    return 0;
}
