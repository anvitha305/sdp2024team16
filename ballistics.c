#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define GRAVITY 9.81

struct BallisticsResult {
    double *time;
    double *velocity;
    double *angle;
    double *x;
    double *y;
};

struct BallisticsResult ballistics(double initV, double initA, double initX, double initY, double dt, double dragCoeff, double stopY, double stopX, int printable) {
    struct BallisticsResult retDict;
    double gravity = GRAVITY;
    double curY = initY;
    double curX = initX;
    int index = 1;
    double time = dt;

    
    retDict.time = (double *)malloc(sizeof(double));
    retDict.velocity = (double *)malloc(sizeof(double));
    retDict.angle = (double *)malloc(sizeof(double));
    retDict.x = (double *)malloc(sizeof(double));
    retDict.y = (double *)malloc(sizeof(double));

    retDict.time[0] = 0;
    retDict.velocity[0] = initV;
    retDict.angle[0] = initA;
    retDict.x[0] = initX;
    retDict.y[0] = initY;

    if (printable) {
        printf("(%0.3f, %0.3f) - V:%0.3f A:%0.3f\n", retDict.x[0], retDict.y[0], retDict.velocity[0], retDict.angle[0] * (180.0 / M_PI));
    }

    while (curX <= stopX && curY >= stopY) {
        double dv = -gravity * sin(retDict.angle[index - 1]) - dragCoeff * retDict.velocity[index - 1] * retDict.velocity[index - 1];
        double da = -gravity * cos(retDict.angle[index - 1]) / retDict.velocity[index - 1];
        double dx = retDict.velocity[index - 1] * cos(retDict.angle[index - 1]);
        double dy = retDict.velocity[index - 1] * sin(retDict.angle[index - 1]);

        retDict.velocity = realloc(retDict.velocity, (index + 1) * sizeof(double));
        retDict.angle = realloc(retDict.angle, (index + 1) * sizeof(double));
        retDict.x = realloc(retDict.x, (index + 1) * sizeof(double));
        retDict.y = realloc(retDict.y, (index + 1) * sizeof(double));
        retDict.time = realloc(retDict.time, (index + 1) * sizeof(double));

        retDict.velocity[index] = retDict.velocity[index - 1] + dv * dt;
        retDict.angle[index] = retDict.angle[index - 1] + da * dt;
        retDict.x[index] = retDict.x[index - 1] + dx * dt;
        retDict.y[index] = retDict.y[index - 1] + dy * dt;
        retDict.time[index] = time;

        curY = retDict.y[index];
        curX = retDict.x[index];

        if (printable) {
            printf("Time %0.3f: (%0.3f, %0.3f) - V:%0.3f A:%0.3f\n", time, retDict.x[index], retDict.y[index], retDict.velocity[index], retDict.angle[index] * (180.0 / M_PI));
        }

        index++;
        time += dt;
    }

    return retDict;
}

double calcTheta(double v0, double dist, double dt, double coeff) {
    double upperAngle = 45;
    double lowerAngle = -90;
    double tryAngle = 10;
    double curDX = 0;
    int index = 0;

    struct BallisticsResult traj;
    traj = ballistics(v0, tryAngle * (M_PI / 180.0), 0, 0, dt, coeff, 0, 10000, 0);

    if (dist < 0 || traj.x[index - 1] < dist) {
        return 90 * (M_PI / 180.0);
    }

    while (1) {
        traj = ballistics(v0, tryAngle * (M_PI / 180.0), 0, 0, dt, coeff, 0, 10000, 0);
        curDX = dist - traj.x[index - 1];

        if (fabs(curDX) < 0.1) {
            break;
        } else if (curDX < 0) {
            upperAngle = tryAngle;
            tryAngle = (upperAngle + lowerAngle) / 2;
        } else {
            lowerAngle = tryAngle;
            tryAngle = (upperAngle + lowerAngle) / 2;
        }
    }

    return tryAngle;
}

int main() {
    double start_time = clock() / (double)CLOCKS_PER_SEC;
    double initV = 75; 
    double initA = 3 * (M_PI / 180.0); 
    double initX = 0; 
    double initY = 0;
    double dt = 0.0002; 
    double dragCoeff = 0.0003747;

    struct BallisticsResult traj = ballistics(initV, initA, initX, initY, dt, dragCoeff, 0, 10000, 0);

    /* Uncomment the code below if you want to plot the trajectory
    double *times = malloc(50 * sizeof(double));
    double *dists = malloc(50 * sizeof(double));
    for (int i = 0; i < 50; i++) {
        dt = pow(10, -5 + i * 0.02);
        traj = ballistics(initV, initA, initX, initY, dt, dragCoeff, 0, 10000, 0);
        times[i] = dt;
        dists[i] = traj.x[index - 1];
    }
    // Plotting code can be added here using external libraries.
    free(times);
    free(dists);
    */

    printf("--- %lf seconds ---\n", (clock() / (double)CLOCKS_PER_SEC) - start_time);

    

    
    free(traj.time);
    free(traj.velocity);
    free(traj.angle);
    free(traj.x);
    free(traj.y);

    return 0;
}
