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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XY(x,y) (((y)*1024)+(x))

int main(int argc, char **argv)
{
    char pbuf[1024*1024];
    int px, py, oy;
    int vx, vy, vye;
    int ovx = 1024;
    int ovy = 1024;
    int dir;
    char prev;

    char mbuf[32256];
    int mloc;

    FILE *pinp;
    char pline[1024];

    int dlev;
    char *outbuf;

    if (argc < 2) {
        printf("Use: %s <program>\n", argv[0]);
        return 1;
    }

    /* get debug level */
    if (argc > 2) {
        dlev = atoi(argv[2]);
        outbuf = (char *) malloc(32256);
        outbuf[0] = '\0';
    } else {
        outbuf = NULL;
        dlev = 0;
    }

    memset(pbuf, 0, 1024*1024);
    py = 0;

    /* read in the file */
    pinp = fopen(argv[1], "r");
    if (pinp == NULL) {
        perror("fopen");
        return 1;
    }

    while (!feof(pinp)) {
        int ostrlen;

        fgets(pline, 1024, pinp);

        ostrlen = strlen(pline);
        if (pline[ostrlen-1] == '\n') {
            pline[ostrlen-1] = '\0';
        }

        strcpy(pbuf + XY(0, py), pline);
        py++;
    }

    fclose(pinp);


    px = 0;
    py = 0;
    dir = 3;

    memset(mbuf, 0, 32256);
    mloc = 2;

    while (1) {
        switch(pbuf[XY(px, py)]) {
            case '+': /* branch */
                /* go back */
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
                        mbuf[0] = getchar();
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
            for (oy = vy; pbuf[XY(0, oy)] != '\0' && oy <= vye; oy++) {
                printf("%.*s\n", 80, pbuf + XY(vx, oy));
            }
            pbuf[XY(px, py)] = prev;

            printf("\n%s\n", outbuf);

            usleep(1000000 / dlev);
        }

        /* now move */
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
