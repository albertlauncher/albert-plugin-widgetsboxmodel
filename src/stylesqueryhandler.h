// Copyright (c) 2022-2026 Manuel Schneider

#pragma once
#include <albert/generatorqueryhandler.h>
class Window;

class StylesQueryHandler : public albert::GeneratorQueryHandler
{
public:
    StylesQueryHandler(Window *w);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    albert::ItemGenerator items(albert::QueryContext context) override;

private:
    Window *window;
};
