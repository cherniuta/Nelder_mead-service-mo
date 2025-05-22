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

Vertex expand_point(const std::vector<double>& centroid, const Vertex& reflected,
                    double gamma, int n, ObjectiveFunction f, void* context) {
    Vertex expanded(n);
    for (int i = 0; i < n; ++i) {
        expanded.x[i] = centroid[i] + gamma * (reflected.x[i] - centroid[i]);
    }
    expanded.value = f(expanded.x.data(), n, context);
    return expanded;
}

Vertex contract_point(const std::vector<double>& centroid, const Vertex& target,
                      double beta, int n, ObjectiveFunction f, void* context) {
    Vertex contracted(n);
    for (int i = 0; i < n; ++i) {
        contracted.x[i] = centroid[i] + beta * (target.x[i] - centroid[i]);
    }
    contracted.value = f(contracted.x.data(), n, context);
    return contracted;
}

void shrink_simplex(std::vector<Vertex>& simplex, double delta,
                    ObjectiveFunction f, void* context) {
    const Vertex& best = simplex[0];
    for (size_t i = 1; i < simplex.size(); ++i) {
        for (size_t j = 0; j < simplex[i].x.size(); ++j) {
            simplex[i].x[j] = best.x[j] + delta * (simplex[i].x[j] - best.x[j]);
        }
        simplex[i].value = f(simplex[i].x.data(), simplex[i].x.size(), context);
    }
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
            Vertex expanded = expand_point(centroid, reflected, params->gamma, n, f, context);
            simplex[n] = (expanded.value < reflected.value) ? expanded : reflected;
        } else if (reflected.value < simplex[n - 1].value) {
            simplex[n] = reflected;
        } else {
            bool shrink = true;
            if (reflected.value < simplex[n].value) {
                Vertex contracted = contract_point(centroid, reflected, params->beta, n, f, context);
                if (contracted.value < reflected.value) {
                    simplex[n] = contracted;
                    shrink = false;
                }
            } else {
                Vertex contracted = contract_point(centroid, simplex[n], params->beta, n, f, context);
                if (contracted.value < simplex[n].value) {
                    simplex[n] = contracted;
                    shrink = false;
                }
            }

            if (shrink) {
                shrink_simplex(simplex, params->delta, f, context);
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
