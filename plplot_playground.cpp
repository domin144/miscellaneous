/*
 * SPDX-FileCopyrightText: 2016 Dominik Wójt <domin144@o2.pl>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <plplot/plstream.h>

int main(int argc, const char *argv[])
{
    const double xmin = -1.0;
    const double xmax = 1.0;
    const double ymin = -1.0;
    const double ymax = 1.0;

    const int n = 100;
    double x[n], y[n];
    for(int i = 0; i<100; i++)
    {
        x[i] = xmin + i * (xmax - xmin) / n;
        y[i] = x[i] * x[i] - 0.5;
    }

    {
        plstream my_plot;
        my_plot.parseopts(&argc, argv, PL_PARSE_FULL);
        my_plot.init();
        my_plot.env(xmin, xmax, ymin, ymax, 0, 0);
        my_plot.line(n, x, y);
    }

    return 0;
}
