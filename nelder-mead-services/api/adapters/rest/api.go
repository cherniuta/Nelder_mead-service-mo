package rest

import (
	"awesomeProject2/api/core"
	"encoding/json"
	"fmt"
	"log/slog"
	"net/http"
	"strconv"
)

type PingResponse struct {
	Replies map[string]string `json:"replies"`
}

func NewPingHandler(log *slog.Logger, pingers map[string]core.Pinger) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		var response PingResponse
		response.Replies = make(map[string]string)

		for name, address := range pingers {
			err := address.Ping(r.Context())
			if err == nil {
				response.Replies[name] = "ok"

			} else {
				response.Replies[name] = "unavailable"
			}

		}

		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(response); err != nil {
			log.Error("cannot encode reply", "error", err)
			http.Error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}

	}
}

type Variable struct {
	Name  string `json:"name"`
	Value int64  `json:"value"`
}
type OptimizationResponse struct {
	Variable      []Variable `json:"variable"`
	FunctionValue float64    `json:"function_value"`
}

func NewOptimizationHandler(log *slog.Logger, optimizator core.Optimizator) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		functionStr := r.URL.Query().Get("function")
		if functionStr == "" {
			log.Error("missing function parameter")
			http.Error(w, "function parameter is required", http.StatusBadRequest)
			return
		}

		tolerance, err := parsePositiveFloatParam(r, "tolerance")
		if err != nil {
			log.Error("invalid tolerance", "error", err)
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}

		maxIter, err := parsePositiveIntParam(r, "iter")
		if err != nil {
			log.Error("invalid iter", "error", err)
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}

		reply, err := optimizator.Optimization(r.Context(), functionStr, tolerance, maxIter)
		if err != nil {
			log.Error("optimization failed", "error", err)
			http.Error(w, "optimization failed", http.StatusInternalServerError)
			return
		}

		response := OptimizationResponse{
			Variable:      make([]Variable, len(reply.Variable)),
			FunctionValue: reply.FunctionValue,
		}

		for i, item := range reply.Variable {
			response.Variable[i] = Variable{Name: item.Name, Value: item.Value}
		}

		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(response); err != nil {
			log.Error("failed to encode response", "error", err)
			http.Error(w, "Internal Server Error", http.StatusInternalServerError)
		}
	}
}

func parsePositiveFloatParam(r *http.Request, param string) (float64, error) {
	valueStr := r.URL.Query().Get(param)
	if valueStr == "" {
		return 0, fmt.Errorf("%s parameter is required", param)
	}

	value, err := strconv.ParseFloat(valueStr, 64)
	if err != nil {
		return 0, fmt.Errorf("invalid %s value", param)
	}

	if value < 0 {
		return 0, fmt.Errorf("%s must be positive", param)
	}

	return value, nil
}

func parsePositiveIntParam(r *http.Request, param string) (int, error) {
	valueStr := r.URL.Query().Get(param)
	if valueStr == "" {
		return 0, fmt.Errorf("%s parameter is required", param)
	}

	value, err := strconv.Atoi(valueStr)
	if err != nil {
		return 0, fmt.Errorf("invalid %s value", param)
	}

	if value < 0 {
		return 0, fmt.Errorf("%s must be positive", param)
	}

	return value, nil
}
