// Copyright (c) 2023-2025 Manuel Schneider

#include "util.h"
#include <QApplication>
#include <QStyle>
#include <QStyleHints>
#include <QWidget>

bool haveDarkSystemPalette()
{
    if (const auto sh = qApp->styleHints()->colorScheme();
        sh == Qt::ColorScheme::Dark)
        return true;
    else if (sh == Qt::ColorScheme::Light)
        return false;
    else {
        auto pal = QApplication::palette();
        return pal.color(QPalette::WindowText).lightness()
               > pal.color(QPalette::Window).lightness();
    }
}

void setStyleRecursive(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);
    for (auto child : widget->findChildren<QWidget*>())
        setStyleRecursive(child, style);
}
