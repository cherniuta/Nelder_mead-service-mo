#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

typedef struct {
    double alpha;
    double beta;
    double gamma;
    double delta;
    int max_iter;
    double tolerance;
} NelderMeadParams;

typedef double (*ObjectiveFunction)(const double* x, int n, void* context);

int nelder_mead_optimize(
    ObjectiveFunction f,
    double* x,
    int n,
    const NelderMeadParams* params,
    void* context,
    double* final_value
);

NelderMeadParams create_default_params();

#endif
