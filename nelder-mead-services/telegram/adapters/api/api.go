package api

import (
	"awesomeProject2/telegram/core"
	"context"
	"encoding/json"
	"fmt"
	"log/slog"
	"net/http"
	"net/url"
	"strconv"
	"time"
)

type APIClient struct {
	baseURL    string
	httpClient *http.Client
	log        *slog.Logger
}

func New(baseURL string, logger *slog.Logger) *APIClient {
	return &APIClient{
		baseURL:    baseURL,
		httpClient: &http.Client{Timeout: 90 * time.Second},
		log:        logger,
	}
}

func (c *APIClient) Optimization(ctx context.Context, function string, tolerance float64, iter int) (core.OptimizationResult, error) {
	params := url.Values{}
	params.Add("function", function)
	params.Add("tolerance", strconv.FormatFloat(tolerance, 'f', -1, 64))
	params.Add("iter", strconv.Itoa(iter))

	req, err := http.NewRequestWithContext(
		ctx,
		"GET",
		c.baseURL+"/api/optimization?"+params.Encode(),
		nil,
	)
	if err != nil {
		return core.OptimizationResult{}, fmt.Errorf("create request failed: %w", err)
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return core.OptimizationResult{}, fmt.Errorf("API request failed: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return core.OptimizationResult{}, fmt.Errorf("unexpected status code: %d", resp.StatusCode)
	}

	var result core.OptimizationResult
	if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
		return core.OptimizationResult{}, fmt.Errorf("decode response failed: %w", err)
	}

	return result, nil
}
