// Copyright (c) 2024-2026 Manuel Schneider

#include "style.h"
#include "stylefileparser.h"
#include <QApplication>
#include <QFont>
using namespace Qt::StringLiterals;
using namespace std;

namespace
{
struct {
    struct {
        QString base             = "palette.base"_L1;
        QString text             = "palette.text"_L1;
        QString window           = "palette.window"_L1;
        QString window_text      = "palette.window_text"_L1;
        QString button           = "palette.button"_L1;
        QString button_text      = "palette.button_text"_L1;
        QString bright_text      = "palette.bright_text"_L1;
        QString light            = "palette.light"_L1;
        QString mid              = "palette.mid"_L1;
        QString dark             = "palette.dark"_L1;
        QString placeholder_text = "palette.placeholder_text"_L1;
        QString highlight        = "palette.highlight"_L1;
        QString highlighted_text = "palette.highlighted_text"_L1;
        QString accent           = "palette.accent"_L1;
        QString link             = "palette.link"_L1;
        QString link_visited     = "palette.link_visited"_L1;
    } palette;

    struct {
        QString background_brush = "window.background_brush"_L1;
        QString border_brush     = "window.border_brush"_L1;
        QString border_radius    = "window.border_radius"_L1;
        QString border_width     = "window.border_width"_L1;
        QString padding          = "window.padding"_L1;
        QString shadow_brush     = "window.shadow_brush"_L1;
        QString shadow_offset    = "window.shadow_offset"_L1;
        QString shadow_size      = "window.shadow_size"_L1;
        QString spacing          = "window.spacing"_L1;
        QString width            = "window.width"_L1;
    } window;

    struct {
        QString background_brush = "input.background_brush"_L1;
        QString border_brush     = "input.border_brush"_L1;
        QString border_radius    = "input.border_radius"_L1;
        QString border_width     = "input.border_width"_L1;
        QString font_size        = "input.font_size"_L1;
        QString trigger_color    = "input.trigger_color"_L1;
        QString action_color     = "input.action_color"_L1;
        QString hint_color       = "input.hint_color"_L1;
        QString padding          = "input.padding"_L1;
    } input;

    struct {
        QString color           = "settings_button.color"_L1;
        QString highlight_color = "settings_button.highlight_color"_L1;
    } settings_button;

    struct {
        struct {
            QString background_brush = "actions.selection.background_brush"_L1;
            QString border_brush     = "actions.selection.border_brush"_L1;
            QString border_radius    = "actions.selection.border_radius"_L1;
            QString border_width     = "actions.selection.border_width"_L1;
            QString text_color       = "actions.selection.text_color"_L1;
        } selection;
        QString font_size  = "actions.font_size"_L1;
        QString padding    = "actions.padding"_L1;
        QString text_color = "actions.text_color"_L1;
    } actions;

    struct {
        struct {
            QString background_brush = "results.selection.background_brush"_L1;
            QString border_brush     = "results.selection.border_brush"_L1;
            QString border_radius    = "results.selection.border_radius"_L1;
            QString border_width     = "results.selection.border_width"_L1;
            QString subtext_color    = "results.selection.subtext_color"_L1;
            QString text_color       = "results.selection.text_color"_L1;
        } selection;
        QString horizontal_space  = "results.horizontal_space"_L1;
        QString icon_size         = "results.icon_size"_L1;
        QString padding           = "results.padding"_L1;
        QString subtext_color     = "results.subtext_color"_L1;
        QString subtext_font_size = "results.subtext_font_size"_L1;
        QString text_color        = "results.text_color"_L1;
        QString text_font_size    = "results.text_font_size"_L1;
        QString vertical_space    = "results.vertical_space"_L1;
    } results;
} static const key;
}  // namespace

Style Style::fromApplicationPalette() { return fromPalette(QApplication::palette()); }

Style Style::fromPalette(const QPalette &p)
{
    using enum QPalette::ColorRole;

    const auto general_spacing = 5;
    const auto app_font_size   = (uint) QApplication::font().pointSize();

    // some extra space to compensate for the font padding;
    const auto input_border_radius  = 2 * general_spacing + 3;
    const auto window_border_width  = 1;
    const auto window_padding       = general_spacing + (int) window_border_width;
    const auto window_border_radius = window_padding + input_border_radius;

    const auto selected_text = p.color(HighlightedText);
    const auto selected_subtext = QColor(selected_text.red(),
                                         selected_text.green(),
                                         selected_text.blue(),
                                         lround(selected_text.alpha() * 0.5 ));

    const auto text = p.color(WindowText);
    const auto subtext = QColor(text.red(),
                                text.green(),
                                text.blue(),
                                lround(text.alpha() * 0.4));

    return Style{
        .palette = p,
        .window{
            .background_brush = p.color(Window),
            .border_brush     = p.color(Highlight),
            .border_radius    = window_border_radius,
            .border_width     = 1,
            .padding          = window_padding,
            .shadow_brush     = QColor(0, 0, 0, 128),
            .shadow_offset    = 6,
            .shadow_size      = 32,
            .spacing          = general_spacing,
            .width            = 640
        },
        .input{
            .action_color     = p.color(PlaceholderText),
            .background_brush = p.color(Base),
            .border_brush     = Qt::NoBrush,
            .border_radius    = input_border_radius,
            .border_width     = 0,
            .font_size        = app_font_size + 7,
            .hint_color       = p.color(PlaceholderText),
            .padding          = general_spacing,
            .trigger_color    = p.color(Accent),
        },
        .settings_button{
            .color           = p.color(Button),
            .highlight_color = p.color(Highlight),
        },
        .results{
            .selection={
                .background_brush = p.color(Highlight),
                .border_brush     = Qt::transparent,
                .border_radius    = input_border_radius,
                .border_width     = 0,
                .subtext_color    = selected_subtext,
                .text_color       = selected_text,
            },
            .horizontal_space  = general_spacing,
            .icon_size         = 34,
            .padding           = general_spacing,
            .subtext_color     = subtext,
            .subtext_font_size = app_font_size - 1,
            .text_color        = text,
            .text_font_size    = app_font_size + 3,
            .vertical_space    = 1,
        },
        .actions={
            .selection={
                .background_brush = p.color(Highlight),
                .border_brush     = p.color(Highlight),
                .border_radius    = input_border_radius,
                .border_width     = 0,
                .text_color       = p.color(HighlightedText),
            },
            .font_size  = app_font_size,
            .padding    = general_spacing,
            .text_color = p.color(WindowText),
        },
    };
}

Style Style::fromStyleFile(const filesystem::path &path)
{
    StyleFileParser parser(path);

    // Read palette

    auto palette = QApplication::palette();

    // If any palette overrides use it
    if (const auto keys = parser.raw_entries | views::keys;
        ranges::find_if(keys, [](const auto &k) { return k.startsWith(u"palette."_s); })
        != keys.end())
    {
        const auto base        = parser.getMandatory<QColor>(key.palette.base);
        const auto text        = parser.getMandatory<QColor>(key.palette.text);
        const auto window      = parser.getMandatory<QColor>(key.palette.window);
        const auto window_text = parser.getMandatory<QColor>(key.palette.window_text);

        const auto button
            = parser.getOptional<QColor>(key.palette.button)
                  .value_or(window);

        const auto button_text
            = parser.getOptional<QColor>(key.palette.button_text)
                  .value_or(window_text);

        const auto bright_text
            = parser.getOptional<QColor>(key.palette.bright_text)
                  .value_or(QColor(255 - button_text.red(), 255 - button_text.green(),  // invert
                                   255 - button_text.blue(), button_text.alpha()));

        QColor light(button.lighter(140));
        QColor mid(button.darker(120));
        QColor dark(button.darker(140));

        palette = QPalette(window_text, button, light, dark, mid, text, bright_text, base, window);

        const auto accent = parser.getMandatory<QColor>(key.palette.accent);
        palette.setColor(QPalette::All, QPalette::Accent, accent);

        int h, s, v;
        const auto highlight
            = parser.getOptional<QColor>(key.palette.highlight)
                  .value_or(QColor(accent.red(), accent.green(), accent.blue(), lround(accent.alpha() * .5)));
        highlight.getHsv(&h, &s, &v);

        const auto highlight_inactive = QColor::fromHsv(h, 0, v);
        palette.setColor(QPalette::All, QPalette::Highlight, highlight_inactive);
        palette.setColor(QPalette::Active, QPalette::Highlight, highlight);

        const auto highlighted_text
            = parser.getOptional<QColor>(key.palette.highlighted_text)
                  .value_or(text);
        palette.setColor(QPalette::All, QPalette::HighlightedText, highlighted_text);

        const auto placeholder_text
            = parser.getOptional<QColor>(key.palette.placeholder_text)
                  .value_or(QColor(text.red(), text.green(), text.blue(), lround(text.alpha() * .5)));
        palette.setColor(QPalette::All, QPalette::PlaceholderText, placeholder_text);

        const auto link
            = parser.getOptional<QColor>(key.palette.link)
                  .value_or(accent);
        palette.setColor(QPalette::All, QPalette::Link, link);

        const auto link_visited
            = parser.getOptional<QColor>(key.palette.link_visited)
                  .value_or(accent);
        palette.setColor(QPalette::All, QPalette::LinkVisited, link_visited);
    }

    auto style = Style::fromPalette(palette);

    // Read optional values

    parser.getOptional(key.actions.font_size, &style.actions.font_size);
    parser.getOptional(key.actions.padding, &style.actions.padding);
    parser.getOptional(key.actions.selection.background_brush, &style.actions.selection.background_brush);
    parser.getOptional(key.actions.selection.border_brush, &style.actions.selection.border_brush);
    parser.getOptional(key.actions.selection.border_radius, &style.actions.selection.border_radius);
    parser.getOptional(key.actions.selection.border_width, &style.actions.selection.border_width);
    parser.getOptional(key.actions.selection.text_color, &style.actions.selection.text_color);
    parser.getOptional(key.actions.text_color, &style.actions.text_color);

    parser.getOptional(key.input.background_brush, &style.input.background_brush);
    parser.getOptional(key.input.border_brush, &style.input.border_brush);
    parser.getOptional(key.input.border_radius, &style.input.border_radius);
    parser.getOptional(key.input.border_width, &style.input.border_width);
    parser.getOptional(key.input.font_size, &style.input.font_size);
    parser.getOptional(key.input.trigger_color, &style.input.trigger_color);
    parser.getOptional(key.input.hint_color, &style.input.hint_color);
    parser.getOptional(key.input.action_color, &style.input.action_color);
    parser.getOptional(key.input.padding, &style.input.padding);

    parser.getOptional(key.results.horizontal_space, &style.results.horizontal_space);
    parser.getOptional(key.results.icon_size, &style.results.icon_size);
    parser.getOptional(key.results.padding, &style.results.padding);
    parser.getOptional(key.results.selection.background_brush, &style.results.selection.background_brush);
    parser.getOptional(key.results.selection.border_brush, &style.results.selection.border_brush);
    parser.getOptional(key.results.selection.border_radius, &style.results.selection.border_radius);
    parser.getOptional(key.results.selection.border_width, &style.results.selection.border_width);
    parser.getOptional(key.results.selection.subtext_color, &style.results.selection.subtext_color);
    parser.getOptional(key.results.selection.text_color, &style.results.selection.text_color);
    parser.getOptional(key.results.subtext_color, &style.results.subtext_color);
    parser.getOptional(key.results.subtext_font_size, &style.results.subtext_font_size);
    parser.getOptional(key.results.text_color, &style.results.text_color);
    parser.getOptional(key.results.text_font_size, &style.results.text_font_size);
    parser.getOptional(key.results.vertical_space, &style.results.vertical_space);

    parser.getOptional(key.settings_button.color, &style.settings_button.color);
    parser.getOptional(key.settings_button.highlight_color, &style.settings_button.highlight_color);

    parser.getOptional(key.window.background_brush, &style.window.background_brush);
    parser.getOptional(key.window.border_brush, &style.window.border_brush);
    parser.getOptional(key.window.border_radius, &style.window.border_radius);
    parser.getOptional(key.window.border_width, &style.window.border_width);
    parser.getOptional(key.window.padding, &style.window.padding);
    parser.getOptional(key.window.shadow_brush, &style.window.shadow_brush);
    parser.getOptional(key.window.shadow_offset, &style.window.shadow_offset);
    parser.getOptional(key.window.shadow_size, &style.window.shadow_size);
    parser.getOptional(key.window.spacing, &style.window.spacing);
    parser.getOptional(key.window.width, &style.window.width);

    return style;
}
