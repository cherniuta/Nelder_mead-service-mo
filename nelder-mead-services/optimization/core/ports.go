package core

import "context"

type Optimizator interface {
	Optimization(ctx context.Context, query OptimizationQuery) (OptimizationReplay, error)
}
