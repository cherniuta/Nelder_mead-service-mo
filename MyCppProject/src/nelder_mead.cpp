#include "nelder_mead.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>

struct Vertex {
    std::vector<double> x;
    double value;

    Vertex(int n) : x(n), value(0.0) {}
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

std::vector<Vertex> create_initial_simplex(const double* x0, int n, double step_size,
                                           ObjectiveFunction f, void* context) {
    std::vector<Vertex> simplex;
    simplex.reserve(n + 1);

    Vertex v0(n);
    std::copy(x0, x0 + n, v0.x.begin());
    v0.value = f(v0.x.data(), n, context);
    simplex.push_back(v0);

    for (int i = 0; i < n; ++i) {
        Vertex v(n);
        std::copy(x0, x0 + n, v.x.begin());
        v.x[i] += step_size;
        v.value = f(v.x.data(), n, context);
        simplex.push_back(v);
    }

    return simplex;
}

std::vector<double> compute_centroid(const std::vector<Vertex>& vertices, int n, int exclude_idx) {
    std::vector<double> centroid(n, 0.0);
    for (int i = 0; i < vertices.size(); ++i) {
        if (i == exclude_idx) continue;
        for (int j = 0; j < n; ++j) {
            centroid[j] += vertices[i].x[j];
        }
    }
    for (int j = 0; j < n; ++j) {
        centroid[j] /= (vertices.size() - 1);
    }
    return centroid;
}

Vertex reflect_point(const std::vector<double>& centroid, const Vertex& worst,
                     double alpha, int n, ObjectiveFunction f, void* context) {
    Vertex reflected(n);
    for (int i = 0; i < n; ++i) {
        reflected.x[i] = centroid[i] + alpha * (centroid[i] - worst.x[i]);
    }
    reflected.value = f(reflected.x.data(), n, context);
    return reflected;
}

int nelder_mead_optimize(ObjectiveFunction f, double* x, int n,
                         const NelderMeadParams* params, void* context,
                         double* final_value) {
    if (!x || !params || n <= 0) return -1;

    auto simplex = create_initial_simplex(x, n, 0.1, f, context);

    for (int iter = 0; iter < params->max_iter; ++iter) {
        std::sort(simplex.begin(), simplex.end(),
                  [](const Vertex& a, const Vertex& b) { return a.value < b.value; });

        auto centroid = compute_centroid(simplex, n, n);
        Vertex reflected = reflect_point(centroid, simplex[n], params->alpha, n, f, context);

        if (reflected.value < simplex[0].value) {
            // расширение пока не вынесена
            std::vector<double> expanded(n);
            for (int i = 0; i < n; ++i)
                expanded[i] = centroid[i] + params->gamma * (reflected.x[i] - centroid[i]);
            double expanded_val = f(expanded.data(), n, context);

            if (expanded_val < reflected.value) {
                simplex[n].x = expanded;
                simplex[n].value = expanded_val;
            } else {
                simplex[n] = reflected;
            }
        } else if (reflected.value < simplex[n - 1].value) {
            simplex[n] = reflected;
        } else {
            // сжатие и усадка пока остались как раньше
            std::vector<double> contracted(n);
            for (int i = 0; i < n; ++i)
                contracted[i] = centroid[i] + params->beta * (simplex[n].x[i] - centroid[i]);
            double contracted_val = f(contracted.data(), n, context);

            if (contracted_val < simplex[n].value) {
                simplex[n].x = contracted;
                simplex[n].value = contracted_val;
            } else {
                for (int i = 1; i <= n; ++i) {
                    for (int j = 0; j < n; ++j) {
                        simplex[i].x[j] = simplex[0].x[j] + params->delta * (simplex[i].x[j] - simplex[0].x[j]);
                    }
                    simplex[i].value = f(simplex[i].x.data(), n, context);
                }
            }
        }

        double max_diff = 0.0;
        for (int i = 1; i <= n; ++i)
            max_diff = std::max(max_diff, std::abs(simplex[i].value - simplex[0].value));
        if (max_diff < params->tolerance) break;
    }

    std::copy(simplex[0].x.begin(), simplex[0].x.end(), x);
    if (final_value) *final_value = simplex[0].value;

    return 0;
}
