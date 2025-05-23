package rest

import (
	"awesomeProject2/telegram/core"
	"context"
	"fmt"
	"log/slog"
	"strconv"
	"strings"
	"sync"
)

type Handler struct {
	apiClient  core.APIClient
	tgClint    core.TelegramClient
	userStates map[int64]*core.UserState
	stateMu    sync.RWMutex
	log        *slog.Logger
}

func New(apiClient core.APIClient, tgClient core.TelegramClient, logger *slog.Logger) *Handler {
	return &Handler{apiClient: apiClient, tgClint: tgClient, log: logger}
}

func (h *Handler) HandleCommand(ctx context.Context, cmd string, chatID int64) error {
	switch cmd {
	case "/opt":
		h.stateMu.Lock()
		h.userStates[chatID] = &core.UserState{Step: "function"}
		h.stateMu.Unlock()

		return h.tgClint.SendMessage(ctx, chatID, "Введите вашу функцию")
	case "/help":
		return h.sendHelp(ctx, chatID)
	case "/start":
		return h.sendHello(ctx, chatID)
	default:
		return h.sendUnknownCommand(ctx, chatID)
	}
}

func (h *Handler) HandleRegularMessage(ctx context.Context, text string, chatID int64) error {
	h.stateMu.Lock()
	defer h.stateMu.Unlock()

	state, exists := h.userStates[chatID]
	if !exists {
		return h.sendUnknownCommand(ctx, chatID)
	}

	switch state.Step {
	case "function":
		state.Function = text
		state.Step = "tolerance"
		return h.tgClint.SendMessage(ctx, chatID, "Введите точность")
	case "tolerance":
		var (
			tolerance float64
			err       error
		)
		tolerance, err = strconv.ParseFloat(text, 64)
		if err != nil || tolerance <= 0 {
			return h.tgClint.SendMessage(ctx, chatID, "Введите число больше 0")
		}
		state.Tolerance = tolerance
		state.Step = "iter"
		return h.tgClint.SendMessage(ctx, chatID, "Введите количество итераций")
	case "iter":
		var (
			iter int
			err  error
		)
		iter, err = strconv.Atoi(text)
		if err != nil || iter <= 0 {
			return h.tgClint.SendMessage(ctx, chatID, "Введите число больше 0")
		}
		state.Iter = iter
		defer delete(h.userStates, chatID)

		results, err := h.apiClient.Optimization(ctx, state.Function, state.Tolerance, state.Iter)
		if err != nil {
			return fmt.Errorf("search failed: %w", err)
		}
		return h.sendOptimizationResults(ctx, chatID, results)
	default:
		return h.sendUnknownCommand(ctx, chatID)

	}
}
func (h *Handler) sendOptimizationResults(ctx context.Context, chatId int64, result core.OptimizationResult) error {
	msg := formatResultsHTML(result)
	return h.tgClint.SendMessage(ctx, chatId, msg)
}
func formatResultsHTML(result core.OptimizationResult) string {
	var builder strings.Builder

	builder.WriteString(`{"variables":[`)
	for i, variable := range result.Variable {
		if i > 0 {
			builder.WriteString(",")
		}
		builder.WriteString(fmt.Sprintf(`{"name":"%s","value":%v}`, variable.Name, variable.Value))
	}
	builder.WriteString(fmt.Sprintf(`],"functionValue":%.15f}`, result.FunctionValue))

	return builder.String()
}
func (h *Handler) sendHelp(ctx context.Context, chatID int64) error {
	return h.tgClint.SendMessage(ctx, chatID, msgHelp)
}

func (h *Handler) sendHello(ctx context.Context, chatID int64) error {
	return h.tgClint.SendMessage(ctx, chatID, msgHello)
}

func (h *Handler) sendUnknownCommand(ctx context.Context, chatID int64) error {
	return h.tgClint.SendMessage(ctx, chatID, msgUnknownCommand)
}
