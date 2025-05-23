package grpc

import (
	"awesomeProject2/optimization/core"
	__ "awesomeProject2/proto/optimizator"
	"context"
	"google.golang.org/protobuf/types/known/emptypb"
)

func NewServer(service core.Optimizator) *Server {
	return &Server{service: service}
}

type Server struct {
	__.UnimplementedOptimizationServer
	service core.Optimizator
}

func (s *Server) Ping(_ context.Context, _ *emptypb.Empty) (*emptypb.Empty, error) {
	return nil, nil
}

func (s *Server) Optimization(ctx context.Context, request *__.OptimizationRequest) (*__.OptimizationReply, error) {
	query := core.OptimizationQuery{
		Function:  request.GetFunction(),
		Tolerance: request.GetTolerance(),
		MaxIter:   request.GetMaxIter(),
	}

	result, err := s.service.Optimization(ctx, query)
	if err != nil {
		return nil, err
	}

	var variables []*__.Variable
	for _, v := range result.Variable {
		variables = append(variables, &__.Variable{
			Name:  v.Name,
			Value: v.Value,
		})
	}

	return &__.OptimizationReply{
		Variable:      variables,
		FunctionValue: result.FunctionValue,
	}, nil
}
