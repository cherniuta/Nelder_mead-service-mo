package core

type OptimizationQuery struct {
	Function  string
	Tolerance float64
	MaxIter   int64
}

type Variable struct {
	Name  string
	Value int64
}

type OptimizationReplay struct {
	Variable      []Variable
	FunctionValue float64
}
