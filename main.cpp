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

void read_file(std::string const& filename, char pbuf[1024*1024])
{
  std::ifstream infile(filename);

  unsigned int py = 0;
  for( std::string line; getline( infile, line ); )
  {
    if (line.back() == '\n') {
        line.pop_back();
    }
    strcpy(pbuf + XY(0, py), line.c_str());
    py++;
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



    char pbuf[1024*1024];
    memset(pbuf, 0, 1024*1024);
    int py = 0;

    read_file(argv[1], pbuf);

    int px = 0;
    py = 0;
    int dir = 3;

    char mbuf[32256];
    memset(mbuf, 0, 32256);
    int mloc = 2;

    char prev;
    unsigned int steps = 1;
    while(true) {
        switch(pbuf[XY(px, py)]) {
            case '+': /* branch */
                /* go back */
               --steps;
                switch (dir) {
                    case 1: /* up */
                        py++;
                        break;
                    case 2: /* right */
                        px--;
                        break;
                    case 3: /* down */
                        py--;
                        break;
                    case 4: /* left */
                        px++;
                        break;
                }

                if (mbuf[mloc]) {
                    dir++;
                } else {
                    dir--;
                }

                if (dir == 0) dir = 4;
                if (dir == 5) dir = 1;
                break;

            case '*': /* memory operator */
                if (mloc == 1 && (dir == 2 || dir == 4)) {
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
                        case 1: /* up */
                            mloc--;
                            break;
                        case 2: /* right */
                            mbuf[mloc]++;
                            break;
                        case 3: /* down */
                            mloc++;
                            break;
                        case 4: /* left */
                            mbuf[mloc]--;
                            break;
                    }
                }
                break;
        }

        int vx, vy, vye;
        int ovx = 1024;
        int ovy = 1024;

        /* if debugging, output */
        if (dlev != 0) {
            /*system("clear");*/

            /* center with a screen at 80x25 */
            vx = px / 40;
            vx *= 40;
            vx -= 40;
            vx = (vx < 0) ? 0 : vx;
            vy = py / 12;
            vy *= 12;
            vy -= 12;
            vy = (vy < 0) ? 0 : vy;
            vye = vy + 25;

            /* only blank the screen if necessary */
            if (ovx != vx || ovy != vy) {
                printf("\033[2J");  /* clear the screen */
                fflush(stdout);
            }
            ovx = vx;
            ovy = vy;

            printf("\033[0;0f"); /* go to the top-left */
            fflush(stdout);

            prev = pbuf[XY(px, py)];
            pbuf[XY(px, py)] = '@';
            for (int oy = vy; pbuf[XY(0, oy)] != '\0' && oy <= vye; oy++) {
                printf("%.*s\n", 80, pbuf + XY(vx, oy));
            }
            pbuf[XY(px, py)] = prev;

            //printf("\n%s\n", outbuf);
            printf("%d\n\n", steps);

            usleep(1000000 / dlev);
        }

        /* now move */
        ++steps;
        switch (dir) {
            case 1: /* up */
                py--;
                break;
            case 2: /* right */
                px++;
                break;
            case 3: /* down */
                py++;
                break;
            case 4: /* left */
                px--;
                break;
        }

        /* quit if < 0 */
        if (px < 0 || py < 0) {
            return 0;
        }
    }
}
