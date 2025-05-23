package core

type OptimizationResult struct {
	Variable []struct {
		Name  string  `json:"name"`
		Value float64 `json:"value"`
	}
	FunctionValue float64 `json:"function_value"`
}

type TelegramUpdate struct {
	UpdateID int64            `json:"update_id"`
	Message  *TelegramMessage `json:"message"`
}

type TelegramMessage struct {
	Chat TelegramChat `json:"chat"`
	From TelegramUser `json:"from"`
	Text string       `json:"text"`
}

type TelegramChat struct {
	ID int64 `json:"id"`
}

type TelegramUser struct {
	ID       int64  `json:"id"`
	Username string `json:"username"`
}

type UserState struct {
	Step      string
	User      string
	Function  string
	Tolerance float64
	Iter      int
}
