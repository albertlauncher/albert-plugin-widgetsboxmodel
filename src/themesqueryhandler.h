// Copyright (c) 2022-2026 Manuel Schneider

#pragma once
#include <albert/generatorqueryhandler.h>
class Window;

class ThemesQueryHandler : public albert::GeneratorQueryHandler
{
public:
    ThemesQueryHandler(Window *w);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    albert::ItemGenerator items(albert::QueryContext context) override;

private:
    Window *window;
};
