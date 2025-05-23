package core

// #cgo CXXFLAGS: -std=c++11
// #cgo LDFLAGS: -L. -lnelder_mead
/*
#include "nelder_mead.h"
#include <stdlib.h>

// Функция-обертка для вызова Go-функции из C++
extern double goObjectiveFunction(double* x, int n, void* context);
*/
import "C"

import (
	"context"
	"fmt"
	"log/slog"
	"math"
	"regexp"
	"strconv"
	"strings"
	"unsafe"
)

type Service struct {
	log *slog.Logger
}

func NewService(log *slog.Logger) (*Service, error) {
	return &Service{
		log: log}, nil
}

type OptimizationFunction interface {
	Evaluate(vars []float64) float64
	Dimension() int
	VarNames() []string
}

type ParsedFunction struct {
	expression string
	variables  []string
	tokens     []string
	dimension  int
}

func (f *ParsedFunction) Evaluate(vars []float64) float64 {
	values := make(map[string]float64)
	for i, name := range f.variables {
		values[name] = vars[i]
	}
	return evaluateExpression(f.tokens, values)
}

func (f *ParsedFunction) Dimension() int {
	return f.dimension
}

func (f *ParsedFunction) VarNames() []string {
	return f.variables
}

var currentFunction OptimizationFunction

//export goObjectiveFunction
func goObjectiveFunction(x *C.double, n C.int, context unsafe.Pointer) C.double {
	slice := unsafe.Slice(x, int(n))

	vars := make([]float64, len(slice))
	for i, v := range slice {
		vars[i] = float64(v)
	}

	result := currentFunction.Evaluate(vars)
	return C.double(result)
}

func parseFunction(expr string) (*ParsedFunction, error) {
	expr = strings.ToLower(strings.ReplaceAll(expr, " ", ""))

	varRegex := regexp.MustCompile(`x\d+`)
	vars := varRegex.FindAllString(expr, -1)

	uniqueVars := make(map[string]bool)
	var variables []string
	for _, v := range vars {
		if !uniqueVars[v] {
			uniqueVars[v] = true
			variables = append(variables, v)
		}
	}

	tokens := tokenizeExpression(expr)

	return &ParsedFunction{
		expression: expr,
		variables:  variables,
		tokens:     tokens,
		dimension:  len(variables),
	}, nil
}

func tokenizeExpression(expr string) []string {
	expr = regexp.MustCompile(`([\+\-\*\/\(\)\^])`).ReplaceAllString(expr, " $1 ")
	return strings.Fields(expr)
}

func evaluateExpression(tokens []string, values map[string]float64) float64 {
	postfix := infixToPostfix(tokens)

	var stack []float64

	for _, token := range postfix {
		switch token {
		case "+", "-", "*", "/", "^":
			b := stack[len(stack)-1]
			stack = stack[:len(stack)-1]
			a := stack[len(stack)-1]
			stack = stack[:len(stack)-1]

			var result float64
			switch token {
			case "+":
				result = a + b
			case "-":
				result = a - b
			case "*":
				result = a * b
			case "/":
				result = a / b
			case "^":
				result = math.Pow(a, b)
			}
			stack = append(stack, result)
		default:
			if val, ok := values[token]; ok {
				stack = append(stack, val)
			} else {
				if num, err := strconv.ParseFloat(token, 64); err == nil {
					stack = append(stack, num)
				}
			}
		}
	}

	if len(stack) > 0 {
		return stack[len(stack)-1]
	}
	return 0
}

func infixToPostfix(tokens []string) []string {
	precedence := map[string]int{
		"+": 1,
		"-": 1,
		"*": 2,
		"/": 2,
		"^": 3,
	}

	var output []string
	var stack []string

	for _, token := range tokens {
		switch token {
		case "(":
			stack = append(stack, token)
		case ")":
			for len(stack) > 0 && stack[len(stack)-1] != "(" {
				output = append(output, stack[len(stack)-1])
				stack = stack[:len(stack)-1]
			}
			if len(stack) > 0 {
				stack = stack[:len(stack)-1] // Remove "("
			}
		case "+", "-", "*", "/", "^":
			for len(stack) > 0 && stack[len(stack)-1] != "(" &&
				precedence[stack[len(stack)-1]] >= precedence[token] {
				output = append(output, stack[len(stack)-1])
				stack = stack[:len(stack)-1]
			}
			stack = append(stack, token)
		default:
			output = append(output, token)
		}
	}

	for len(stack) > 0 {
		output = append(output, stack[len(stack)-1])
		stack = stack[:len(stack)-1]
	}

	return output
}

func toFloat64Slice(x []C.double) []float64 {
	result := make([]float64, len(x))
	for i, v := range x {
		result[i] = float64(v)
	}
	return result
}

func (s *Service) Optimization(ctx context.Context, query OptimizationQuery) (OptimizationReplay, error) {
	f, err := parseFunction(query.Function)
	if err != nil {
		fmt.Printf("Error parsing function: %v\n", err)
		return OptimizationReplay{}, err
	}

	currentFunction = f

	params := C.create_default_params()

	params.tolerance = C.double(query.Tolerance)
	params.max_iter = C.int(query.MaxIter)

	n := f.Dimension()
	x := make([]C.double, n)
	for i := range x {
		x[i] = 1.0
	}

	var finalValue C.double

	result := C.nelder_mead_optimize(
		(C.ObjectiveFunction)(unsafe.Pointer(C.goObjectiveFunction)),
		(*C.double)(&x[0]),
		C.int(n),
		&params,
		nil,
		&finalValue,
	)
	if result != 0 {
		return OptimizationReplay{}, ErrOptimizationFailed
	}

	variables := make([]Variable, 0, f.Dimension())
	for i, val := range x {
		variables = append(variables, Variable{
			Name:  f.VarNames()[i],
			Value: int64(val),
		})
	}

	return OptimizationReplay{
		Variable:      variables,
		FunctionValue: float64(finalValue),
	}, nil
}
