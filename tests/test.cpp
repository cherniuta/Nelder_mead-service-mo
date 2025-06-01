#include "C:\Users\adsim\for work and studing\projects\nelder-mead\nelder_mead.h"
#include "pch.h"
#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>
#include <random>

using ObjectiveFunction = double (*)(const double*, int, void*);

class NelderMeadTest : public ::testing::Test {
protected:
    void SetUp() override {
        SetConsoleOutputCP(CP_UTF8);
        std::locale::global(std::locale(""));
        params = create_default_params();  // Инициализация параметров
        params.max_iter = 2000;
    }

    void run_optimization_test(
        const char* name,
        ObjectiveFunction func,
        const double* expected_min,
        const double* initial_point,
        double tolerance = 1e-4)
    {
        SCOPED_TRACE(name);

        const int n = 2;
        double x[n] = { initial_point[0], initial_point[1] };
        double final_value = 0.0;

        std::cout << "\n=== Тест функции: " << name << " ===\n";
        std::cout << "Начальная точка: (" << x[0] << ", " << x[1] << ")\n";
        std::cout << "Ожидаемый минимум: (" << expected_min[0] << ", " << expected_min[1] << ")\n";

        int result = nelder_mead_optimize(
            func,
            x,
            n,
            &params,
            nullptr,
            &final_value
        );

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Результат:\n";
        std::cout << "  Код: " << result << "\n";
        std::cout << "  Точка: (" << x[0] << ", " << x[1] << ")\n";
        std::cout << "  Значение: " << final_value << "\n";

        EXPECT_TRUE(result == 0 || result == 1) << "Ошибка оптимизации";
        EXPECT_NEAR(x[0], expected_min[0], tolerance);
        EXPECT_NEAR(x[1], expected_min[1], tolerance);
    }
    void run_random_start_test(
        const char* name,
        double (*func)(const double*, int, void*),
        const double* expected_min,
        int num_tests = 5,
        double range = 5.0,
        double tolerance = 1e-3)
    {
        std::mt19937 gen(42);
        std::uniform_real_distribution<double> dist(-range, range);

        for (int i = 0; i < num_tests; ++i) {
            double initial_point[2] = { dist(gen), dist(gen) };
            std::ostringstream test_name;
            test_name << name << " (случайный старт " << i + 1 << ")";
            run_optimization_test(test_name.str().c_str(), func, expected_min, initial_point, tolerance);
        }
    }

    NelderMeadParams params;  // Параметры метода
};
 

// 1 Функция Розенброка (минимум в (1, 1))
double rosenbrock_func(const double* x, int n, void* context) {
    double a = 1.0 - x[0];
    double b = x[1] - x[0] * x[0];
    return a * a + 100.0 * b * b;
}
// 2 Функция Химмельблау (минимум в (3, 2))
double himmelblau_func(const double* x, int n, void* context) {
    double a = x[0] * x[0] + x[1] - 11.0;
    double b = x[0] + x[1] * x[1] - 7.0;
    return a * a + b * b;
}
// 3 Функция Растригина (минимум в (0, 0))
double rastrigin_func(const double* x, int n, void* context) {
    const double A = 10.0;
    return A * 2 + (x[0] * x[0] - A * cos(2 * M_PI * x[0])) +
        (x[1] * x[1] - A * cos(2 * M_PI * x[1]));
}
// 4 Функция Била (минимум в (0, 0))
double beale_func(const double* x, int n, void* context) {
    double a = 1.5 - x[0] + x[0] * x[1];
    double b = 2.25 - x[0] + x[0] * x[1] * x[1];
    double c = 2.625 - x[0] + x[0] * x[1] * x[1] * x[1];
    return a * a + b * b + c * c;
}
// 4 Функция Экли (минимум в (0, 0))
double ackley_func(const double* x, int n, void* context) {
    const double a = 20.0, b = 0.2, c = 2 * M_PI;
    double sum1 = x[0] * x[0] + x[1] * x[1];
    double sum2 = cos(c * x[0]) + cos(c * x[1]);
    return -a * exp(-b * sqrt(0.5 * sum1)) - exp(0.5 * sum2) + a + exp(1.0);
}
// 5 Функция Букина (минимум в (-10, 1))
double bukin_func(const double* x, int n, void* context) {
    return 100 * sqrt(fabs(x[1] - 0.01 * x[0] * x[0])) + 0.01 * fabs(x[0] + 10);
}

// 6 Функция Матьяса (минимум в (0.23, 0.23))
double matyas_func(const double* x, int n, void* context) {
    return 0.26 * (x[0] * x[0] + x[1] * x[1]) - 0.48 * x[0] * x[1];
}

// 7 Функция Леви (минимум в (1, 1))
double levy_func(const double* x, int n, void* context) {
    double a = sin(3 * M_PI * x[0]);
    double b = x[0] - 1;
    double c = 1 + sin(3 * M_PI * x[1]) * sin(3 * M_PI * x[1]);
    double d = x[1] - 1;
    return a * a + b * b * (1 + a * a) + c * d * d;
}
// 8 Функция Три горба (Three-Hump Camel) - минимум в (0, 0)
double three_hump_camel_func(const double* x, int n, void* context) {
    return 2 * x[0] * x[0] - 1.05 * pow(x[0], 4) + pow(x[0], 6) / 6 + x[0] * x[1] + x[1] * x[1];
}
// 9 Функция Голдштейн - Прайса(Goldstein - Price) - минимум в(0, -1)
double goldstein_price_func(const double* x, int n, void* context) {
    double a = 1 + pow(x[0] + x[1] + 1, 2) * (19 - 14 * x[0] + 3 * x[0] * x[0] - 14 * x[1] + 6 * x[0] * x[1] + 3 * x[1] * x[1]);
    double b = 30 + pow(2 * x[0] - 3 * x[1], 2) * (18 - 32 * x[0] + 12 * x[0] * x[0] + 48 * x[1] - 36 * x[0] * x[1] + 27 * x[1] * x[1]);
    return a * b;
}
 

TEST_F(NelderMeadTest, RosenbrockFunction) {
    double expected[] = { 1.0, 1.0 };
    double initial[] = { -1.2, 1.0 };
    run_optimization_test("Функция Розенброка", rosenbrock_func, expected, initial, 1e-3);
}

TEST_F(NelderMeadTest, HimmelblauFunction) {
    double expected[] = { 3.0, 2.0 };
    double initial[] = { 0.0, 0.0 };
    run_optimization_test("Функция Химмельблау", himmelblau_func, expected, initial);
}

TEST_F(NelderMeadTest, RastriginFunction) {
    double expected[] = { 0.0, 0.0 };
    double initial[] = { 2.5, 3.5 };
    run_optimization_test("Функция Растригина", rastrigin_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, AckleyFunction) {
    double expected[] = { 0.0, 0.0 };
    double initial[] = { 3.0, 4.0 };
    run_optimization_test("Функция Экли", ackley_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, MultipleStartingPoints) {
    double expected[] = { 1.0, 1.0 };
    const double starting_points[][2] = {
        {-1.2, 1.0}, {0.0, 0.0}, {2.0, 2.0}, {-2.0, -1.0}
    };

    for (const auto& point : starting_points) {
        std::ostringstream test_name;
        test_name << "Розенброк (точка [" << point[0] << ", " << point[1] << "])";
        run_optimization_test(test_name.str().c_str(), rosenbrock_func, expected, point, 1e-3);
    }
}
TEST_F(NelderMeadTest, BealeFunction) {
    double expected[] = { 3.0, 0.5 };
    double initial[] = { 1.0, 1.0 };
    run_optimization_test("Функция Била", beale_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, GoldsteinPriceFunction) {
    double expected[] = { 0.0, -1.0 };
    double initial[] = { -1.0, 0.0 };
    run_optimization_test("Функция Голдштейн-Прайс", goldstein_price_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, BukinFunction) {
    double expected[] = { -10.0, 1.0 };
    double initial[] = { -8.0, 1.0 };
    run_optimization_test("Функция Букина", bukin_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, LevyFunction) {
    double expected[] = { 1.0, 1.0 };
    double initial[] = { 0.0, 0.0 };
    run_optimization_test("Функция Леви", levy_func, expected, initial, 1e-2);
}

TEST_F(NelderMeadTest, ThreeHumpCamelFunction) {
    double expected[] = { 0.0, 0.0 };
    double initial[] = { 1.0, 1.0 };
    run_optimization_test("Функция Три горба", three_hump_camel_func, expected, initial);
}
TEST_F(NelderMeadTest, MultipleStartingPointsRosenbrock) {
    double expected[] = { 1.0, 1.0 };
    run_random_start_test("Функция Розенброка", rosenbrock_func, expected, 5, 5.0, 1e-2);
}

TEST_F(NelderMeadTest, NearOptimumStart) {
    auto quadratic = [](const double* x, int n, void* context) {
        return pow(x[0] - 2.0, 2) + pow(x[1] - 3.0, 2);
        };
    double expected[] = { 2.0, 3.0 };
    double initial[] = { 2.1, 2.9 };
    run_optimization_test("Квадратичная (близкий старт)", quadratic, expected, initial, 1e-8);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

 