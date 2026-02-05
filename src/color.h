// Copyright (c) 2024-2026 Manuel Schneider

#pragma once
#include <QBrush>
#include <QColor>
#include <QPalette>

QColor change(QColor color,
              std::optional<int> red,
              std::optional<int> green,
              std::optional<int> blue,
              std::optional<int> alpha,
              std::optional<int> hue,
              std::optional<int> saturation,
              std::optional<int> lightness);

QColor adjust(QColor color,
              std::optional<int> red,
              std::optional<int> green,
              std::optional<int> blue,
              std::optional<int> alpha,
              std::optional<int> hue,
              std::optional<int> saturation,
              std::optional<int> lightness);

QColor scale(QColor color,
             std::optional<double> red,
             std::optional<double> green,
             std::optional<double> blue,
             std::optional<double> alpha,
             std::optional<double> saturation,
             std::optional<double> lightness);

QColor multiply(QColor color,
                std::optional<double> red,
                std::optional<double> green,
                std::optional<double> blue,
                std::optional<double> alpha,
                std::optional<double> saturation,
                std::optional<double> lightness);
