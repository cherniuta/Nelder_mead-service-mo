package core

import "context"

type APIClient interface {
	Optimization(ctx context.Context, function string, tolerance float64, iter int) (OptimizationResult, error)
}

type TelegramClient interface {
	SendMessage(ctx context.Context, chatID int64, text string) error
	GetUpdatesChan() <-chan TelegramUpdate
}
