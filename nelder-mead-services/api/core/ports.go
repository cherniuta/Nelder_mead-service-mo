package core

import "context"

type Optimizator interface {
	Optimization(context.Context, string, float64, int) (OptimizationReplay, error)
}

type Pinger interface {
	Ping(context.Context) error
}
