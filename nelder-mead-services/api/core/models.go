package core

type Variable struct {
	Name  string
	Value int64
}

type OptimizationReplay struct {
	Variable      []Variable
	FunctionValue float64
}
