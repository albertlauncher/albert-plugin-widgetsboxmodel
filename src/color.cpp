// Copyright (c) 2024-2025 Manuel Schneider

#include "color.h"
#include <algorithm>
using namespace std;


QColor change(QColor color,
              std::optional<int> red,
              std::optional<int> green,
              std::optional<int> blue,
              std::optional<int> alpha,
              std::optional<int> hue,
              std::optional<int> saturation,
              std::optional<int> lightness)
{
    if (red)
        color.setRed(clamp(*red, 0, 255));
    if (green)
        color.setGreen(clamp(*green, 0, 255));
    if (blue)
        color.setBlue(clamp(*blue, 0, 255));
    if (alpha)
        color.setAlpha(clamp(*alpha, 0, 255));

    if (hue || saturation || lightness)
    {
        int h, s, l, a;
        color.getHsl(&h, &s, &l, &a);
        if (hue)
            h = *hue % 360;
        if (saturation)
            s = clamp(*saturation, 0, 255);
        if (lightness)
            l = clamp(*lightness, 0, 255);
        color = QColor::fromHsl(h, s, l, a);
    }

    return color;
}


QColor adjust(QColor color,
              std::optional<int> red,
              std::optional<int> green,
              std::optional<int> blue,
              std::optional<int> alpha,
              std::optional<int> hue,
              std::optional<int> saturation,
              std::optional<int> lightness)
{
    if (red)
        color.setRed(clamp(color.red() + *red, 0, 255));
    if (green)
        color.setGreen(clamp(color.green() + *green, 0, 255));
    if (blue)
        color.setBlue(clamp(color.blue() + *blue, 0, 255));
    if (alpha)
        color.setAlpha(clamp(color.alpha() + *alpha, 0, 255));

    if (hue || saturation || lightness)
    {
        int h, s, l, a;
        color.getHsl(&h, &s, &l, &a);
        if (hue)
            h = (h + *hue) % 360;
        if (saturation)
            s = clamp(s + *saturation, 0, 255);
        if (lightness)
            l = clamp(l + *lightness, 0, 255);
        color = QColor::fromHsl(h, s, l, a);
    }

    return color;
}

static int scaleComponent(int val, double factor)
{
    if (factor == 0)
        return val;
    if (factor > 0)
        return static_cast<int>(lround(val + (255 - val) * factor));
    else  // factor < 0
        return static_cast<int>(lround(val * (1.0 + factor)));
}

QColor scale(QColor color,
             std::optional<double> red,
             std::optional<double> green,
             std::optional<double> blue,
             std::optional<double> alpha,
             std::optional<double> saturation,
             std::optional<double> lightness)
{
    if (red)
        color.setRed(clamp(scaleComponent(color.red(), *red), 0, 255));
    if (green)
        color.setGreen(clamp(scaleComponent(color.green(), *green), 0, 255));
    if (blue)
        color.setBlue(clamp(scaleComponent(color.blue(), *blue), 0, 255));
    if (alpha)
        color.setAlpha(clamp(scaleComponent(color.alpha(), *alpha), 0, 255));

    if (saturation || lightness)
    {
        int h, s, l, a;
        color.getHsl(&h, &s, &l, &a);
        if (saturation)
            s = clamp(scaleComponent(s, *saturation), 0, 255);
        if (lightness)
            l = clamp(scaleComponent(l, *lightness), 0, 255);
        color = QColor::fromHsl(h, s, l, a);
    }

    return color;
}

QColor multiply(QColor color,
                std::optional<double> red,
                std::optional<double> green,
                std::optional<double> blue,
                std::optional<double> alpha,
                std::optional<double> saturation,
                std::optional<double> lightness)
{
    if (red)
        color.setRed(clamp(static_cast<int>(lround(color.red() * *red)), 0, 255));
    if (green)
        color.setGreen(clamp(static_cast<int>(lround(color.green() * *green)), 0, 255));
    if (blue)
        color.setBlue(clamp(static_cast<int>(lround(color.blue() * *blue)), 0, 255));
    if (alpha)
        color.setAlpha(clamp(static_cast<int>(lround(color.alpha() * *alpha)), 0, 255));

    if (saturation || lightness)
    {
        int h, s, l, a;
        color.getHsl(&h, &s, &l, &a);
        if (saturation)
            s = clamp(static_cast<int>(lround(s * *saturation)), 0, 255);
        if (lightness)
            l = clamp(static_cast<int>(lround(l * *lightness)), 0, 255);
        color = QColor::fromHsl(h, s, l, a);
    }

    return color;
}
