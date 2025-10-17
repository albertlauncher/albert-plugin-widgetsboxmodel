// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/rankedqueryhandler.h>
class Window;

class ThemesQueryHandler : public albert::RankedQueryHandler
{
public:
    ThemesQueryHandler(Window *w);
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString defaultTrigger() const override;
    std::vector<albert::RankItem> rankItems(albert::QueryContext &) override;

private:
    Window *window;
};
