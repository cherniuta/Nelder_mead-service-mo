package optimization

import (
	"awesomeProject2/api/core"
	__ "awesomeProject2/proto/optimizator"
	"context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/protobuf/types/known/emptypb"
	"log/slog"
)

type Client struct {
	log    *slog.Logger
	client __.OptimizationClient
}

func NewClient(address string, log *slog.Logger) (*Client, error) {
	conn, err := grpc.NewClient(address, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		return nil, err
	}
	return &Client{
		client: __.NewOptimizationClient(conn),
		log:    log,
	}, nil
}

func (c Client) Ping(ctx context.Context) error {
	_, err := c.client.Ping(ctx, &emptypb.Empty{})
	return err
}

func (c Client) Optimization(ctx context.Context, function string, tolerance float64, maxIter int) (core.OptimizationReplay, error) {
	reply, err := c.client.Optimization(ctx, &__.OptimizationRequest{Function: function, Tolerance: tolerance, MaxIter: int64(maxIter)})
	if err != nil {
		return core.OptimizationReplay{}, err
	}

	variable := make([]core.Variable, 0)
	for _, item := range reply.Variable {
		variable = append(variable, core.Variable{Name: item.Name, Value: item.Value})
	}

	return core.OptimizationReplay{Variable: variable, FunctionValue: reply.GetFunctionValue()}, nil
}
