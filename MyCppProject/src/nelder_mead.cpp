#include "nelder_mead.h"
#include <vector>
#include <cmath>
#include <algorithm>

struct Vertex {
    std::vector<double> x;
    double value;
};

NelderMeadParams create_default_params() {
    NelderMeadParams p;
    p.alpha = 1.0;
    p.beta = 0.5;
    p.gamma = 2.0;
    p.delta = 0.5;
    p.max_iter = 200;
    p.tolerance = 1e-6;
    return p;
}

int nelder_mead_optimize(ObjectiveFunction f, double* x, int n,
                         const NelderMeadParams* params, void* context,
                         double* final_value) {
    if (!x || n <= 0 || !params) return -1;

    std::vector<Vertex> simplex(n + 1);
    double step = 0.1;

    // построим симплекс вручную
    for (int i = 0; i <= n; ++i) {
        simplex[i].x.resize(n);
        for (int j = 0; j < n; ++j) {
            simplex[i].x[j] = x[j];
            if (i > 0 && j == i - 1)
                simplex[i].x[j] += step;
        }
        simplex[i].value = f(simplex[i].x.data(), n, context);
    }

    for (int iter = 0; iter < params->max_iter; ++iter) {
        std::sort(simplex.begin(), simplex.end(), [](const Vertex& a, const Vertex& b) {
            return a.value < b.value;
        });

        std::vector<double> centroid(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                centroid[j] += simplex[i].x[j];
        for (int j = 0; j < n; ++j)
            centroid[j] /= n;

        // отражение
        std::vector<double> reflected(n);
        for (int i = 0; i < n; ++i)
            reflected[i] = centroid[i] + params->alpha * (centroid[i] - simplex[n].x[i]);
        double reflected_val = f(reflected.data(), n, context);

        if (reflected_val < simplex[0].value) {
            // расширение
            std::vector<double> expanded(n);
            for (int i = 0; i < n; ++i)
                expanded[i] = centroid[i] + params->gamma * (reflected[i] - centroid[i]);
            double expanded_val = f(expanded.data(), n, context);

            simplex[n].x = (expanded_val < reflected_val) ? expanded : reflected;
            simplex[n].value = std::min(expanded_val, reflected_val);
        }
        else if (reflected_val < simplex[n - 1].value) {
            simplex[n].x = reflected;
            simplex[n].value = reflected_val;
        }
        else {
            // сжатие
            std::vector<double> contracted(n);
            for (int i = 0; i < n; ++i)
                contracted[i] = centroid[i] + params->beta * (simplex[n].x[i] - centroid[i]);
            double contracted_val = f(contracted.data(), n, context);

            if (contracted_val < simplex[n].value) {
                simplex[n].x = contracted;
                simplex[n].value = contracted_val;
            } else {
                // усадка
                for (int i = 1; i <= n; ++i) {
                    for (int j = 0; j < n; ++j)
                        simplex[i].x[j] = simplex[0].x[j] + params->delta * (simplex[i].x[j] - simplex[0].x[j]);
                    simplex[i].value = f(simplex[i].x.data(), n, context);
                }
            }
        }

        double max_diff = 0.0;
        for (int i = 1; i <= n; ++i)
            max_diff = std::max(max_diff, std::abs(simplex[i].value - simplex[0].value));

        if (max_diff < params->tolerance)
            break;
    }

    for (int i = 0; i < n; ++i)
        x[i] = simplex[0].x[i];
    if (final_value)
        *final_value = simplex[0].value;

    return 0;
}
