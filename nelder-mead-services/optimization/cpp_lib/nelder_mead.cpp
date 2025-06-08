#include "nelder_mead.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>


NelderMeadParams create_default_params() {
    NelderMeadParams params;
    params.alpha = 1.0;
    params.beta = 0.5;
    params.gamma = 2.0;
    params.delta = 0.5;
    params.max_iter = 1000;
    params.tolerance = 1e-6;
    return params;
}

struct Vertex {
    std::vector<double> x;
    double value;

    Vertex(int n) : x(n) {}
};

std::vector<Vertex> create_initial_simplex(const double* x0, int n, double step_size,
                                         ObjectiveFunction f, void* context) {
    std::vector<Vertex> vertices;
    vertices.reserve(n + 1);

    vertices.emplace_back(n);
    std::copy(x0, x0 + n, vertices.back().x.begin());
    vertices.back().value = f(vertices.back().x.data(), n, context);

    for (int i = 0; i < n; ++i) {
        vertices.emplace_back(n);
        std::copy(x0, x0 + n, vertices.back().x.begin());
        vertices.back().x[i] += step_size;
        vertices.back().value = f(vertices.back().x.data(), n, context);
    }

    return vertices;
}

std::vector<double> compute_centroid(const std::vector<Vertex>& vertices, int n, int worst_idx) {
    std::vector<double> centroid(n, 0.0);
    
    for (int i = 0; i < (int)vertices.size(); ++i) {
        if (i != worst_idx) {
            for (int j = 0; j < n; ++j) {
                centroid[j] += vertices[i].x[j];
            }
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


Vertex contract_point(const std::vector<double>& centroid, const Vertex& worst,
                     double beta, int n, ObjectiveFunction f, void* context) {
    Vertex contracted(n);
    for (int i = 0; i < n; ++i) {
        contracted.x[i] = centroid[i] + beta * (worst.x[i] - centroid[i]);
    }
    contracted.value = f(contracted.x.data(), n, context);
    return contracted;
}

void shrink_simplex(std::vector<Vertex>& vertices, double delta,
                    ObjectiveFunction f, void* context) {
    const Vertex& best = vertices[0];
    for (size_t i = 1; i < vertices.size(); ++i) {
        for (size_t j = 0; j < vertices[i].x.size(); ++j) {
            vertices[i].x[j] = best.x[j] + delta * (vertices[i].x[j] - best.x[j]);
        }
        vertices[i].value = f(vertices[i].x.data(), vertices[i].x.size(), context);
    }
}

int nelder_mead_optimize(ObjectiveFunction f, double* x, int n,
                        const NelderMeadParams* params, void* context,
                        double* final_value) {

    if (!x || !params || n <= 0) return -1;

    double initial_step = 0.1;
    auto vertices = create_initial_simplex(x, n, initial_step, f, context);

    for (int iter = 0; iter < params->max_iter; ++iter) {
        std::sort(vertices.begin(), vertices.end(),
                 [](const Vertex& a, const Vertex& b) { return a.value < b.value; });

        double max_diff = 0.0;
        for (size_t i = 1; i < vertices.size(); ++i) {
            max_diff = std::max(max_diff, std::abs(vertices[i].value - vertices[0].value));
        }
        if (max_diff < params->tolerance) {
            std::copy(vertices[0].x.begin(), vertices[0].x.end(), x);
            if (final_value) *final_value = vertices[0].value;
            return 0;
        }

        auto centroid = compute_centroid(vertices, n, n);

        auto reflected = reflect_point(centroid, vertices[n], params->alpha, n, f, context);

        if (reflected.value < vertices[0].value) {
            auto expanded = expand_point(centroid, reflected, params->gamma, n, f, context);
            if (expanded.value < reflected.value) {
                vertices[n] = expanded;
            } else {
                vertices[n] = reflected;
            }
        }
        else if (reflected.value < vertices[n-1].value) {
            vertices[n] = reflected;
        }
        else {
            bool do_shrink = true;
            if (reflected.value < vertices[n].value) {
                auto contracted = contract_point(centroid, reflected, params->beta, n, f, context);
                if (contracted.value < reflected.value) {
                    vertices[n] = contracted;
                    do_shrink = false;
                }
            }
            else {
                auto contracted = contract_point(centroid, vertices[n], params->beta, n, f, context);
                if (contracted.value < vertices[n].value) {
                    vertices[n] = contracted;
                    do_shrink = false;
                }
            }

            if (do_shrink) {
                shrink_simplex(vertices, params->delta, f, context);
            }
        }
    }

    std::sort(vertices.begin(), vertices.end(),
             [](const Vertex& a, const Vertex& b) { return a.value < b.value; });
    std::copy(vertices[0].x.begin(), vertices[0].x.end(), x);
    if (final_value) *final_value = vertices[0].value;
    return 1;
} 