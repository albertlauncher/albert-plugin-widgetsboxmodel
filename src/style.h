// Copyright (c) 2024-2026 Manuel Schneider

#pragma once
#include <filesystem>
#include <QBrush>
#include <QColor>
#include <QPalette>

class Style
{
public:
    QPalette palette;

    // Do sync with template.ini

    struct {
        QBrush background_brush;
        QBrush border_brush;
        double border_radius;
        double border_width;
        uint   padding;
        QBrush shadow_brush;
        uint   shadow_offset;
        uint   shadow_size;
        uint   spacing;
        uint   width;
    } window;

    struct {
        QColor action_color;
        QBrush background_brush;
        QBrush border_brush;
        double border_radius;
        double border_width;
        uint   font_size;
        QColor hint_color;
        uint   padding;
        QColor trigger_color;
    } input;

    struct {
        QColor color;
        QColor highlight_color;
    } settings_button;

    struct {
        struct {
            QBrush background_brush;
            QBrush border_brush;
            double border_radius;
            double border_width;
            QColor subtext_color;
            QColor text_color;
        } selection;
        uint   horizontal_space;
        uint   icon_size;
        uint   padding;
        QColor subtext_color;
        uint   subtext_font_size;
        QColor text_color;
        uint   text_font_size;
        int    vertical_space;
    } results;

    struct {
        struct {
            QBrush background_brush;
            QBrush border_brush;
            double border_radius;
            double border_width;
            QColor text_color;
        } selection;
        uint   font_size;
        uint   padding;
        QColor text_color;
    } actions;

    static Style fromPalette(const QPalette &p);
    static Style fromApplicationPalette();
    static Style fromStyleFile(const std::filesystem::path &path);
};
