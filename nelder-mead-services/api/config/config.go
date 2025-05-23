package config

import (
	"github.com/ilyakaznacheev/cleanenv"
	"log"
	"time"
)

type Config struct {
	LogLevel            string        `yaml:"log_level" env:"LOG_LEVEL" env-default:"DEBUG"`
	Address             string        `yaml:"api_address" env:"API_ADDRESS" env-default:"localhost:80"`
	Timeout             time.Duration `yaml:"timeout" env:"API_TIMEOUT" env-default:"5s"`
	OptimizationAddress string        `yaml:"optimization_address" env:"OPTIMIZATION_ADDRESS" env-default:"localhost:81"`
}

func MustLoad(configPath string) Config {
	var cfg Config
	if err := cleanenv.ReadConfig(configPath, &cfg); err != nil {
		log.Fatalf("cannot read config %q: %s", configPath, err)
	}
	return cfg
}
