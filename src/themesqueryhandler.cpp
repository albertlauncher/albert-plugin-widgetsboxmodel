// Copyright (c) 2022-2025 Manuel Schneider

#include "themesqueryhandler.h"
#include "window.h"
#include <albert/icon.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

ThemesQueryHandler::ThemesQueryHandler(Window *w) : window(w) {}

QString ThemesQueryHandler::id() const { return u"themes"_s; }

QString ThemesQueryHandler::name() const { return Window::tr("Themes"); }

QString ThemesQueryHandler::description() const { return Window::tr("Switch themes"); }

//: The trigger
QString ThemesQueryHandler::defaultTrigger() const { return Window::tr("themes") + QChar::Space; }

static vector<Action> makeActions(Window *window, const QString& theme_name)
{
    vector<Action> actions;
    actions.emplace_back(u"setlight"_s,
                         Window::tr("Use in light mode"),
                         [window, theme_name]{ window->setThemeLight(theme_name); });

    actions.emplace_back(u"setdark"_s,
                         Window::tr("Use in dark mode"),
                         [window, theme_name]{ window->setThemeDark(theme_name); });

    if (window->darkMode())
        std::swap(actions[0], actions[1]);

    return actions;
}

static unique_ptr<Icon> makeIcon() { return Icon::grapheme(u"🎨"_s); }

vector<RankItem> ThemesQueryHandler::rankItems(QueryContext &ctx)
{
    Matcher matcher(ctx);
    vector<RankItem> items;
    const auto sytem_title = Window::tr("System");

    if (const auto m = matcher.match(sytem_title); m)
        items.emplace_back(StandardItem::make(u"system_theme"_s,
                                              sytem_title,
                                              Window::tr("The system theme."),
                                              makeIcon,
                                              makeActions(window, {})),
                           m);

    for (const auto &[name, path] : window->themes)
        if (const auto m = matcher.match(name); m)
        {
            auto actions = makeActions(window, name);
            actions.emplace_back(u"open"_s, Window::tr("Open theme file"), [path] { open(path); });
            items.emplace_back(StandardItem::make(u"theme_%1"_s.arg(name),
                                                  name,
                                                  path,
                                                  makeIcon,
                                                  ::move(actions)),
                               m);
        }

    return items;
}
