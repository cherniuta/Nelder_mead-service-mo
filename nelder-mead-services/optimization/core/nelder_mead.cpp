#include "nelder_mead.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>

OptimizationParams create_default_params(void) {
    OptimizationParams params;
    params.tolerance = 1e-6;
    params.max_iter = 1000;
    params.alpha = 1.0;    // коэффициент отражения
    params.gamma = 2.0;    // коэффициент растяжения
    params.rho = 0.5;     // коэффициент сжатия
    params.sigma = 0.5;    // коэффициент глобального сжатия
    return params;
}


struct Vertex {
    std::vector<double> x;
    double value;

    Vertex(int n) : x(n) {}
    
    bool operator<(const Vertex& other) const {
        return value < other.value;
    }
};


std::vector<Vertex> create_initial_simplex(ObjectiveFunction f, const double* x0, int n, void* context) {
    std::vector<Vertex> vertices;
    

    vertices.push_back(Vertex(n));
    std::copy(x0, x0 + n, vertices[0].x.begin());
    vertices[0].value = f(vertices[0].x.data(), n, context);

    for (int i = 0; i < n; ++i) {
        vertices.push_back(Vertex(n));
        std::copy(x0, x0 + n, vertices[i + 1].x.begin());
        
        if (vertices[i + 1].x[i] == 0) {
            vertices[i + 1].x[i] = 0.00025;
        } else {
            vertices[i + 1].x[i] *= 1.05;
        }
        
        vertices[i + 1].value = f(vertices[i + 1].x.data(), n, context);
    }
    
    return vertices;
}

std::vector<double> compute_centroid(const std::vector<Vertex>& vertices, int n) {
    std::vector<double> centroid(n, 0.0);
    int num_points = vertices.size() - 1;
    
    for (int i = 0; i < num_points; ++i) {
        for (int j = 0; j < n; ++j) {
            centroid[j] += vertices[i].x[j];
        }
    }
    
    for (int j = 0; j < n; ++j) {
        centroid[j] /= num_points;
    }
    
    return centroid;
}

Vertex reflect_point(const std::vector<double>& centroid, const Vertex& worst, double alpha, int n) {
    Vertex reflected(n);
    for (int i = 0; i < n; ++i) {
        reflected.x[i] = centroid[i] + alpha * (centroid[i] - worst.x[i]);
    }
    return reflected;
}


Vertex expand_point(const std::vector<double>& centroid, const Vertex& reflected, double gamma, int n) {
    Vertex expanded(n);
    for (int i = 0; i < n; ++i) {
        expanded.x[i] = centroid[i] + gamma * (reflected.x[i] - centroid[i]);
    }
    return expanded;
}

Vertex contract_point(const std::vector<double>& centroid, const Vertex& worst, double rho, int n) {
    Vertex contracted(n);
    for (int i = 0; i < n; ++i) {
        contracted.x[i] = centroid[i] + rho * (worst.x[i] - centroid[i]);
    }
    return contracted;
}

void shrink_simplex(std::vector<Vertex>& vertices, double sigma, int n) {
    for (size_t i = 1; i < vertices.size(); ++i) {
        for (int j = 0; j < n; ++j) {
            vertices[i].x[j] = vertices[0].x[j] + sigma * (vertices[i].x[j] - vertices[0].x[j]);
        }
    }
}

bool check_convergence(const std::vector<Vertex>& vertices, double tolerance) {
    double mean = 0.0;
    for (const auto& vertex : vertices) {
        mean += vertex.value;
    }
    mean /= vertices.size();
    
    double variance = 0.0;
    for (const auto& vertex : vertices) {
        double diff = vertex.value - mean;
        variance += diff * diff;
    }
    variance /= vertices.size();
    
    return std::sqrt(variance) < tolerance;
}

int nelder_mead_optimize(
    ObjectiveFunction f,
    double* x,
    int n,
    OptimizationParams* params,
    void* context,
    double* final_value
) {
    if (!x || !params || n <= 0) return -1;

    auto vertices = create_initial_simplex(f, x, n, context);
    
    for (int iter = 0; iter < params->max_iter; ++iter) {
        std::sort(vertices.begin(), vertices.end());

        if (check_convergence(vertices, params->tolerance)) {
            break;
        }

        auto centroid = compute_centroid(vertices, n);

        auto reflected = reflect_point(centroid, vertices.back(), params->alpha, n);
        reflected.value = f(reflected.x.data(), n, context);
        
        if (reflected.value < vertices[0].value) {

            auto expanded = expand_point(centroid, reflected, params->gamma, n);
            expanded.value = f(expanded.x.data(), n, context);
            
            if (expanded.value < reflected.value) {
                vertices.back() = expanded;
            } else {
                vertices.back() = reflected;
            }
        }
        else if (reflected.value < vertices[vertices.size()-2].value) {

            vertices.back() = reflected;
        }
        else {

            bool do_shrink = true;
            
            if (reflected.value < vertices.back().value) {

                auto contracted = contract_point(centroid, reflected, params->rho, n);
                contracted.value = f(contracted.x.data(), n, context);
                
                if (contracted.value <= reflected.value) {
                    vertices.back() = contracted;
                    do_shrink = false;
                }
            }
            else {
                auto contracted = contract_point(centroid, vertices.back(), params->rho, n);
                contracted.value = f(contracted.x.data(), n, context);
                
                if (contracted.value < vertices.back().value) {
                    vertices.back() = contracted;
                    do_shrink = false;
                }
            }
            
            if (do_shrink) {
                shrink_simplex(vertices, params->sigma, n);
                for (size_t i = 1; i < vertices.size(); ++i) {
                    vertices[i].value = f(vertices[i].x.data(), n, context);
                }
            }
        }
    }
    

    std::sort(vertices.begin(), vertices.end());
    std::copy(vertices[0].x.begin(), vertices[0].x.end(), x);
    if (final_value) *final_value = vertices[0].value;
    
    return 0;
} 