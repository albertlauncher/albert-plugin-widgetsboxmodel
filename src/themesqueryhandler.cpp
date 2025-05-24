// Copyright (c) 2022-2025 Manuel Schneider

#include "themesqueryhandler.h"
#include "window.h"
#include <albert/albert.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
using namespace Qt::StringLiterals;
using namespace albert::util;
using namespace albert;
using namespace std;

ThemesQueryHandler::ThemesQueryHandler(Window *w) : window(w) {}

QString ThemesQueryHandler::id() const
{ return QStringLiteral("widgetboxmodel_themes"); }

QString ThemesQueryHandler::name() const
{ return QStringLiteral("Themes"); }

QString ThemesQueryHandler::description() const
{ return Window::tr("Switch themes"); }

QString ThemesQueryHandler::defaultTrigger() const { return u"theme "_s; }

void ThemesQueryHandler::handleTriggerQuery(Query &query)
{
    vector<Action> actions;

    Matcher matcher(query);

    auto sytem_title = Window::tr("System");

    if (auto m = matcher.match(sytem_title); m)
    {
        actions.emplace_back(u"setlight"_s,
                             Window::tr("Use in light mode"),
                             [&]{ window->setThemeLight({}); });

        actions.emplace_back(u"setdark"_s,
                             Window::tr("Use in dark mode"),
                             [&]{ window->setThemeDark({}); });

        if (window->darkMode())
            std::swap(actions[0], actions[1]);

        query.add(StandardItem::make(
            u"system_theme"_s,
            sytem_title,
            Window::tr("The system theme."),
            sytem_title,
            {QStringLiteral("gen:?&text=🎨")},
            actions
            ));
    }


    for (const auto &[name, path] : window->themes)
        if (auto m = matcher.match(name); m)
        {
            actions.clear();

            actions.emplace_back(u"setlight"_s,
                                 Window::tr("Use in light mode"),
                                 [&]{ window->setThemeLight(name); });

            actions.emplace_back(u"setdark"_s,
                                 Window::tr("Use in dark mode"),
                                 [&]{ window->setThemeDark(name); });

            if (window->darkMode())
                std::swap(actions[0], actions[1]);

            actions.emplace_back(u"open"_s, Window::tr("Open theme file"), [p = path] { open(p); });

            query.add(StandardItem::make(u"theme_%1"_s.arg(name),
                                         name,
                                         path,
                                         name,
                                         {QStringLiteral("gen:?&text=🎨")},
                                         actions));
        }
}
