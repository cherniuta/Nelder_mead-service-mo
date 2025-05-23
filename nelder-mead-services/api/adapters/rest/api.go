package rest

import (
	"awesomeProject2/api/core"
	"encoding/json"
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
	FunctionValue float64    `json:"functionValue"`
}

func NewOptimizationHandler(log *slog.Logger, optimizator core.Optimizator) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		var (
			tolerance float64
			maxIter   int
			err       error
		)
		functionStr := r.URL.Query().Get("function")
		if functionStr != "" {
			toleranceStr := r.URL.Query().Get("tolerance")
			if toleranceStr != "" {
				tolerance, err = strconv.ParseFloat(toleranceStr, 64)
				if err != nil {
					log.Error("wrong tolerance", "value", toleranceStr)
					http.Error(w, "bad tolerance", http.StatusBadRequest)
					return
				}
				if tolerance < 0 {
					log.Error("wrong tolerance", "value", toleranceStr)
					http.Error(w, "bad tolerance", http.StatusBadRequest)
					return
				}
				maxIterStr := r.URL.Query().Get("iter")
				if maxIterStr != "" {
					maxIter, err = strconv.Atoi(maxIterStr)
					if err != nil {
						log.Error("wrong iter", "value", maxIterStr)
						http.Error(w, "bad iter", http.StatusBadRequest)
						return
					}
					if tolerance < 0 {
						log.Error("wrong iter", "value", maxIterStr)
						http.Error(w, "bad iter", http.StatusBadRequest)
						return
					}
				} else {
					log.Error("wrong iter", "value", maxIterStr)
					http.Error(w, "bad iter", http.StatusBadRequest)
					return
				}

			} else {
				log.Error("wrong tolerance", "value", toleranceStr)
				http.Error(w, "bad tolerance", http.StatusBadRequest)
				return
			}

		} else {
			log.Error("wrong function", "value", functionStr)
			http.Error(w, "bad function", http.StatusBadRequest)
			return

		}

		reply, err := optimizator.Optimization(r.Context(), functionStr, tolerance, maxIter)
		if err != nil {
			log.Error("problems with optimizing the function", "error", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return

		}
		response := OptimizationResponse{
			Variable:      make([]Variable, 0),
			FunctionValue: reply.FunctionValue,
		}

		for _, item := range reply.Variable {
			response.Variable = append(response.Variable, Variable{Name: item.Name, Value: item.Value})
		}

		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(response); err != nil {
			log.Error("cannot encode reply", "error", err)
			http.Error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}

	}
}
