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
	query, err := s.service.Optimization(ctx, core.OptimizationQuery{Function: request.Function, Tolerance: request.Tolerance, MaxIter: request.MaxIter})

	if err != nil {
		return nil, err
	}

	return &__.OptimizationReply{Variable: query.Variable, FunctionValue: query.FunctionValue}, nil
}
