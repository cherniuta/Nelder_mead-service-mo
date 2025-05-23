#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

#ifdef __cplusplus
extern "C" {
#endif


typedef double (*ObjectiveFunction)(double* x, int n, void* context);

typedef struct {
    double tolerance;      // Точность для критерия остановки
    int max_iter;         // Максимальное число итераций
    double alpha;         // Коэффициент отражения (обычно 1.0)
    double gamma;         // Коэффициент растяжения (обычно 2.0)
    double rho;          // Коэффициент сжатия (обычно 0.5)
    double sigma;        // Коэффициент глобального сжатия (обычно 0.5)
} OptimizationParams;


OptimizationParams create_default_params(void);


int nelder_mead_optimize(
    ObjectiveFunction f,      // Целевая функция
    double* x,               // Начальное приближение (будет содержать результат)
    int n,                   // Размерность задачи
    OptimizationParams* params, // Параметры метода
    void* context,           // Пользовательский контекст
    double* final_value      // Итоговое значение функции
);

#ifdef __cplusplus
}
#endif

#endif // NELDER_MEAD_H