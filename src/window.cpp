// Copyright (c) 2022-2025 Manuel Schneider

#include "actionslist.h"
#include "debugoverlay.h"
#include "frame.h"
#include "util.h"
#include "inputline.h"
#include "theme.h"
#include "resizinglist.h"
#include "resultitemmodel.h"
#include "resultslist.h"
#include "settingsbutton.h"
#include "statetransitions.h"
#include "window.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDir>
#include <QKeyEvent>
#include <QMenu>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QSettings>
#include <QStateMachine>
#include <QStringListModel>
#include <QStyleFactory>
#include <QTimer>
#include <QWindow>
#include <albert/albert.h>
#include <albert/logging.h>
#include <albert/plugininstance.h>
#include <albert/pluginloader.h>
#include <albert/pluginmetadata.h>
#include <albert/query.h>
using namespace albert;
using namespace std;

namespace  {

static const struct {

    using ColorRole = QPalette::ColorRole;

    const double    settings_button_rps_idle = 0.2;
    const double    settings_button_rps_busy = 0.7;

    const bool      always_on_top                               = true;
    const bool      centered                                    = true;
    const bool      clear_on_hide                               = true;
    const bool      debug                                       = false;
    const bool      display_scrollbar                           = false;
    const bool      follow_cursor                               = true;
    const bool      hide_on_focus_loss                          = true;
    const bool      history_search                              = true;
    const bool      quit_on_close                               = false;
    const bool      shadow_client                               = true;
    const bool      shadow_system                               = false;
    const bool      disable_input_method                        = true;
    const char*     theme_dark                                  = "Default System Palette";
    const char*     theme_light                                 = "Default System Palette";
    const uint      max_results                                 = 5;

    const uint      general_spacing                             = 6;

    const uint      window_shadow_size                          = 80;
    const uint      window_shadow_offset                        = 8;
    const QColor    window_shadow_color                         = QColor(0, 0, 0, 92);

    const ColorRole input_background_brush                      = ColorRole::Base;
    const ColorRole input_border_brush                          = ColorRole::Highlight;
    const double    input_border_width                          = 0;
    const int       input_padding                               = general_spacing + (int)input_border_width - 1; // Plaintextedit has a margin of 1
    const double    input_border_radius                         = general_spacing + input_padding;

    const ColorRole window_background_brush                     = ColorRole::Window;
    const ColorRole window_border_brush                         = ColorRole::Highlight;
    const double    window_border_width                         = 1;
    const int       window_padding                              = general_spacing + (int)window_border_width;
    const double    window_border_radius                        = window_padding + input_border_radius;
    const int       window_spacing                              = window_padding;
    const int       window_width                                = 640;

    const int       input_font_size                             = QApplication::font().pointSize() + 9;

    const ColorRole settings_button_color                       = ColorRole::Button;
    const ColorRole settings_button_highlight_color             = ColorRole::Highlight;

    const ColorRole result_item_selection_background_brush      = ColorRole::Highlight;
    const ColorRole result_item_selection_border_brush          = ColorRole::Highlight;
    const double    result_item_selection_border_radius         = input_border_radius;
    const double    result_item_selection_border_width          = 0;
    const ColorRole result_item_selection_text_color            = ColorRole::HighlightedText;
    const ColorRole result_item_selection_subtext_color         = ColorRole::PlaceholderText;
    const int       result_item_padding                         = general_spacing;
    const ColorRole result_item_text_color                      = ColorRole::WindowText;
    const ColorRole result_item_subtext_color                   = ColorRole::PlaceholderText;
    const int       result_item_icon_size                       = 39;
    const int       result_item_text_font_size                  = QApplication::font().pointSize() + 4;
    const int       result_item_subtext_font_size               = QApplication::font().pointSize() - 1;
    const int       result_item_horizontal_spacing              = general_spacing;
    const int       result_item_vertical_spacing                = 2;

    const ColorRole action_item_selection_background_brush      = ColorRole::Highlight;
    const ColorRole action_item_selection_border_brush          = ColorRole::Highlight;
    const double    action_item_selection_border_radius         = input_border_radius;
    const double    action_item_selection_border_width          = 0;
    const ColorRole action_item_selection_text_color            = ColorRole::HighlightedText;
    const int       action_item_padding                         = general_spacing;
    const ColorRole action_item_text_color                      = ColorRole::WindowText;
    const int       action_item_font_size                       = QApplication::font().pointSize();

} defaults;


static const struct {

    const char* window_position                        = "windowPosition";

    const char *always_on_top                          = "alwaysOnTop";
    const char *centered                               = "showCentered";
    const char *clear_on_hide                          = "clearOnHide";
    const char *debug                                  = "debug";
    const char *display_scrollbar                      = "displayScrollbar";
    const char *follow_cursor                          = "followCursor";
    const char *hide_on_focus_loss                     = "hideOnFocusLoss";
    const char *history_search                         = "historySearch";
    const char *max_results                            = "itemCount";
    const char *quit_on_close                          = "quitOnClose";
    const char *shadow_client                          = "clientShadow";
    const char *shadow_system                          = "systemShadow";
    const char *theme_dark                             = "darkTheme";
    const char *theme_light                            = "lightTheme";
    const char *disable_input_method                   = "disable_input_method";

    const char* window_shadow_size                     = "window_background_brush";
    const char* window_shadow_offset                   = "window_shadow_offset";
    const char* window_shadow_brush                    = "window_shadow_brush";

    const char* window_background_brush                = "window_background_brush";
    const char* window_border_brush                    = "window_border_brush";
    const char* window_border_radius                   = "window_border_radius";
    const char* window_border_width                    = "window_border_width";
    const char* window_padding                         = "window_padding";
    const char* window_spacing                         = "window_spacing";
    const char* window_width                           = "window_width";

    const char* input_background_brush                 = "input_background_brush";
    const char* input_border_brush                     = "input_border_brush";
    const char* input_border_radius                    = "input_border_radius";
    const char* input_border_width                     = "input_border_width";
    const char* input_padding                          = "input_padding";
    const char *input_font_size                        = "input_font_size";

    const char *settings_button_color                  = "settings_button_color";
    const char *settings_button_highlight_color        = "settings_button_highlight_color";

    const char *result_item_selection_background_brush = "result_item_selection_background_brush";
    const char *result_item_selection_border_brush     = "result_item_selection_border_brush";
    const char *result_item_selection_border_radius    = "result_item_selection_border_radius";
    const char *result_item_selection_border_width     = "result_item_selection_border_width";
    const char *result_item_selection_text_color       = "result_item_selection_text_color";
    const char *result_item_selection_subtext_color    = "result_item_selection_subtext_color";
    const char *result_item_padding                    = "result_item_padding";
    const char *result_item_text_color                 = "result_item_text_color";
    const char *result_item_subtext_color              = "result_item_subtext_color";
    const char *result_item_icon_size                  = "result_item_icon_size";
    const char *result_item_text_font_size             = "result_item_text_font_size";
    const char *result_item_subtext_font_size          = "result_item_subtext_font_size";
    const char *result_item_horizontal_spacing         = "result_item_horizontal_spacing";
    const char *result_item_vertical_spacing           = "result_item_vertical_spacing";

    const char *action_item_selection_background_brush = "action_item_selection_background_brush";
    const char *action_item_selection_border_brush     = "action_item_selection_border_brush";
    const char *action_item_selection_border_radius    = "action_item_selection_border_radius";
    const char *action_item_selection_border_width     = "action_item_selection_border_width";
    const char *action_item_selection_text_color       = "action_item_selection_text_color";
    const char *action_item_padding                    = "action_item_padding";
    const char *action_item_text_color                 = "action_item_text_color";
    const char *action_item_font_size                  = "action_item_font_size";

} keys;

constexpr Qt::KeyboardModifier mods_mod[] = {
   Qt::ShiftModifier,
   Qt::MetaModifier,
   Qt::ControlModifier,
   Qt::AltModifier
};

constexpr Qt::Key mods_keys[] = {
    Qt::Key_Shift,
    Qt::Key_Meta,
    Qt::Key_Control,
    Qt::Key_Alt
};

}

Window::Window(PluginInstance &p) :
    plugin(p),
    themes(findThemes()),
    input_frame(new Frame(this)),
    input_line(new InputLine(input_frame)),
    spacer_left(new QSpacerItem(0, 0)),
    spacer_right(new QSpacerItem(0, 0)),
    settings_button(new SettingsButton(input_frame)),
    results_list(new ResultsList(this)),
    actions_list(new ActionsList(this)),
    dark_mode(haveDarkSystemPalette()),
    current_query{nullptr}
{
    initializeUi();
    initializeProperties();
    initializeWindowActions();
    initializeStatemachine();

    input_frame->installEventFilter(this);
    settings_button->installEventFilter(this);
    results_list->hide();
    actions_list->hide();
    actions_list->setMaxItems(100);
    settings_button->hide();

    // Reproducible UX
    auto *style = QStyleFactory::create("Fusion");
    style->setParent(this);
    setStyleRecursive(this, style);

    connect(input_line, &InputLine::textChanged,
            this, [this]{ emit inputChanged(input_line->text()); });

    connect(settings_button, &SettingsButton::clicked,
            this, &Window::onSettingsButtonClick);

    QPixmapCache::setCacheLimit(1024 * 50);  // 50 MB
}

Window::~Window() {}

void Window::initializeUi()
{
    // Identifiers

    setObjectName("window");
    input_frame->setObjectName("inputFrame");
    settings_button->setObjectName("settingsButton");
    input_line->setObjectName("inputLine");
    results_list->setObjectName("resultsList");
    actions_list->setObjectName("actionList");


    // Structure

    auto *input_frame_layout = new QHBoxLayout(input_frame);
    input_frame_layout->addItem(spacer_left);
    input_frame_layout->addWidget(input_line, 0, Qt::AlignTop);  // Needed to remove ui flicker
    input_frame_layout->addItem(spacer_right);
    input_frame_layout->addWidget(settings_button, 0, Qt::AlignTop);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(input_frame);
    layout->addWidget(results_list);
    layout->addWidget(actions_list);
    layout->addStretch(0);


    // Properties

    // contentsMargins: setShadowSize
    // layout.contentsMargins: setWindowPadding
    // input_frame.contentsMargins: setInputPadding
    // input_frame_layout.contentsMargins: Have to be set to zero

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    input_frame->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    input_frame_layout->setContentsMargins(0,0,0,0);
    input_frame_layout->setSpacing(0);

    input_line->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    settings_button->setFocusPolicy(Qt::NoFocus);
    settings_button->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

    results_list->setFocusPolicy(Qt::NoFocus);
    results_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    results_list->setAutoFillBackground(false);

    actions_list->setFocusPolicy(Qt::NoFocus);
    actions_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
}

void Window::initializeProperties()
{
    auto s = plugin.settings();
    setAlwaysOnTop(
        s->value(keys.always_on_top,
                 defaults.always_on_top).toBool());
    setClearOnHide(
        s->value(keys.clear_on_hide,
                 defaults.clear_on_hide).toBool());
    setDisplayScrollbar(
        s->value(keys.display_scrollbar,
                 defaults.display_scrollbar).toBool());
    setFollowCursor(
        s->value(keys.follow_cursor,
                 defaults.follow_cursor).toBool());
    setHideOnFocusLoss(
        s->value(keys.hide_on_focus_loss,
                 defaults.hide_on_focus_loss).toBool());
    setHistorySearchEnabled(
        s->value(keys.history_search,
                 defaults.history_search).toBool());
    setMaxResults(
        s->value(keys.max_results,
                 defaults.max_results).toUInt());
    setQuitOnClose(
        s->value(keys.quit_on_close,
                 defaults.quit_on_close).toBool());
    setShowCentered(
        s->value(keys.centered,
                 defaults.centered).toBool());
    setDisableInputMethod(
        s->value(keys.disable_input_method,
                 defaults.disable_input_method).toBool());
    setDebugMode(
        s->value(keys.debug,
                 defaults.debug).toBool());


    setWindowShadowSize(
        s->value(keys.window_shadow_size,
                 defaults.window_shadow_size).toUInt());

    setWindowShadowOffset(
        s->value(keys.window_shadow_offset,
                 defaults.window_shadow_offset).toUInt());

    setWindowBorderRadius(
        s->value(keys.window_border_radius,
                 defaults.window_border_radius).toDouble());

    setWindowBorderWidth(
        s->value(keys.window_border_width,
                 defaults.window_border_width).toDouble());

    setWindowPadding(
        s->value(keys.window_padding,
                 defaults.window_padding).toUInt());

    setWindowSpacing(
        s->value(keys.window_spacing,
                 defaults.window_spacing).toUInt());

    setWindowWidth(
        s->value(keys.window_width,
                 defaults.window_width).toInt());


    setInputPadding(
        s->value(keys.input_padding,
                 defaults.input_padding).toUInt());

    setInputBorderRadius(
        s->value(keys.input_border_radius,
                 defaults.input_border_radius).toDouble());

    setInputBorderWidth(
        s->value(keys.input_border_width,
                 defaults.input_border_width).toDouble());

    setInputFontSize(
        s->value(keys.input_font_size,
                 defaults.input_font_size).toInt());


    setResultItemSelectionBorderRadius(
        s->value(keys.result_item_selection_border_radius,
                 defaults.result_item_selection_border_radius).toDouble());

    setResultItemSelectionBorderWidth(
        s->value(keys.result_item_selection_border_width,
                 defaults.result_item_selection_border_width).toDouble());

    setResultItemPadding(
        s->value(keys.result_item_padding,
                 defaults.result_item_padding).toUInt());

    setResultItemIconSize(
        s->value(keys.result_item_icon_size,
                 defaults.result_item_icon_size).toUInt());

    setResultItemTextFontSize(
        s->value(keys.result_item_text_font_size,
                 defaults.result_item_text_font_size).toUInt());

    setResultItemSubtextFontSize(
        s->value(keys.result_item_subtext_font_size,
                 defaults.result_item_subtext_font_size).toUInt());

    setResultItemHorizontalSpace(
        s->value(keys.result_item_horizontal_spacing,
                 defaults.result_item_horizontal_spacing).toUInt());

    setResultItemVerticalSpace(
        s->value(keys.result_item_vertical_spacing,
                 defaults.result_item_vertical_spacing).toUInt());


    setActionItemSelectionBorderRadius(
        s->value(keys.action_item_selection_border_radius,
                 defaults.action_item_selection_border_radius).toDouble());

    setActionItemSelectionBorderWidth(
        s->value(keys.action_item_selection_border_width,
                 defaults.action_item_selection_border_width).toDouble());

    setActionItemFontSize(
        s->value(keys.action_item_font_size,
                 defaults.action_item_font_size).toUInt());

    setActionItemPadding(
        s->value(keys.action_item_padding,
                 defaults.action_item_padding).toUInt());


    if (auto t = s->value(keys.theme_light).toString(); themes.contains(t))
        theme_light_ = t;
    if (auto t = s->value(keys.theme_dark).toString(); themes.contains(t))
        theme_dark_ = t;

    // applyTheme requires a valid window for the message box. Set theme later.
    QTimer::singleShot(0, [this]{ applyTheme(dark_mode ? theme_dark_ : theme_light_); });

    s = plugin.state();
    if (!showCentered() && s->contains(keys.window_position)
        && s->value(keys.window_position).canConvert<QPoint>())
        move(s->value(keys.window_position).toPoint());
}

void Window::initializeWindowActions()
{
    auto *a = new QAction(tr("Settings"), this);
    a->setShortcuts({QKeySequence("Ctrl+,")});
    a->setShortcutVisibleInContextMenu(true);
    connect(a, &QAction::triggered, this, [] { albert::showSettings(); });
    addAction(a);

    a = new QAction(tr("Hide on focus out"), this);
    a->setShortcuts({QKeySequence("Meta+h")});
    a->setShortcutVisibleInContextMenu(true);
    a->setCheckable(true);
    a->setChecked(hideOnFocusLoss());
    connect(a, &QAction::toggled, this, &Window::setHideOnFocusLoss);
    connect(this, &Window::hideOnFocusLossChanged, a, &QAction::setChecked);
    addAction(a);

    a = new QAction(tr("Show centered"), this);
    a->setShortcuts({QKeySequence("Meta+c")});
    a->setShortcutVisibleInContextMenu(true);
    a->setCheckable(true);
    a->setChecked(showCentered());
    connect(a, &QAction::toggled, this, &Window::setShowCentered);
    connect(this, &Window::showCenteredChanged, a, &QAction::setChecked);
    addAction(a);

    a = new QAction(tr("Clear on hide"), this);
    a->setShortcuts({QKeySequence("Meta+i")});
    a->setShortcutVisibleInContextMenu(true);
    a->setCheckable(true);
    a->setChecked(clearOnHide());
    connect(a, &QAction::toggled, this, &Window::setClearOnHide);
    connect(this, &Window::clearOnHideChanged, a, &QAction::setChecked);
    addAction(a);

    a = new QAction(tr("Debug mode"), this);
    a->setShortcuts({QKeySequence("Meta+d")});
    a->setShortcutVisibleInContextMenu(true);
    a->setCheckable(true);
    a->setChecked(debugMode());
    connect(a, &QAction::toggled, this, &Window::setDebugMode);
    connect(this, &Window::debugModeChanged, a, &QAction::setChecked);
    addAction(a);
}

static void setModelDeleteSelection(QAbstractItemView *v, QAbstractItemModel *m)
{
   // See QAbstractItemView::setModel documentation
    auto *sm = v->selectionModel();
    v->setModel(m);
    delete sm;
}

void Window::initializeStatemachine()
{
    //
    // States
    //

    auto s_root = new QState(QState::ParallelStates);

    auto s_settings_button_appearance = new QState(s_root);
    auto s_settings_button_hidden = new QState(s_settings_button_appearance);
    auto s_settings_button_visible = new QState(s_settings_button_appearance);
    auto s_settings_button_highlight = new QState(s_settings_button_appearance);
    auto s_settings_button_highlight_delay = new QState(s_settings_button_appearance);
    s_settings_button_appearance->setInitialState(s_settings_button_hidden);

    auto s_settings_button_spin = new QState(s_root);
    auto s_settings_button_slow = new QState(s_settings_button_spin);
    auto s_settings_button_fast = new QState(s_settings_button_spin);
    s_settings_button_spin->setInitialState(s_settings_button_slow);

    auto s_results = new QState(s_root);
    auto s_results_query_unset = new QState(s_results);
    auto s_results_query_set = new QState(s_results);
    s_results->setInitialState(s_results_query_unset);

    auto s_results_hidden = new QState(s_results_query_set);
    auto s_results_disabled = new QState(s_results_query_set);
    auto s_results_matches = new QState(s_results_query_set);
    auto s_results_fallbacks = new QState(s_results_query_set);
    s_results_query_set->setInitialState(s_results_hidden);

    auto s_results_match_items = new QState(s_results_matches);
    auto s_results_match_actions = new QState(s_results_matches);
    s_results_matches->setInitialState(s_results_match_items);

    auto s_results_fallback_items = new QState(s_results_fallbacks);
    auto s_results_fallback_actions = new QState(s_results_fallbacks);
    s_results_fallbacks->setInitialState(s_results_fallback_items);

    auto display_delay_timer = new QTimer(this);
    display_delay_timer->setInterval(250);
    display_delay_timer->setSingleShot(true);

    auto busy_delay_timer = new QTimer(this);
    busy_delay_timer->setInterval(250);
    busy_delay_timer->setSingleShot(true);


    //
    // Debug
    //

    // QObject::connect(s_settings_button_hidden, &QState::entered,
    //                  this, []{ CRIT << "s_settings_button_hidden"; });
    // QObject::connect(s_settings_button_visible, &QState::entered,
    //                  this, []{ CRIT << "s_settings_button_visible"; });
    // QObject::connect(s_settings_button_highlight, &QState::entered,
    //                  this, []{ CRIT << "s_settings_button_highlight"; });
    // QObject::connect(s_settings_button_highlight_delay, &QState::entered,
    //                  this, []{ CRIT << "s_settings_button_highlight_delay"; });
    // QObject::connect(s_results_query_unset, &QState::entered,
    //                  this, []{ CRIT << "s_results_query_unset"; });
    // QObject::connect(s_results_query_set, &QState::entered,
    //                  this, []{ CRIT << "s_results_query_set"; });
    // QObject::connect(s_results_hidden, &QState::entered,
    //                  this, []{ CRIT << "s_results_hidden"; });
    // QObject::connect(s_results_disabled, &QState::entered,
    //                  this, []{ CRIT << "s_results_disabled"; });
    // QObject::connect(s_results_match_items, &QState::entered,
    //                  this, []{ CRIT << "s_results_match_items"; });
    // QObject::connect(s_results_match_actions, &QState::entered,
    //                  this, []{ CRIT << "s_results_match_actions"; });
    // QObject::connect(s_results_fallback_items, &QState::entered,
    //                  this, []{ CRIT << "s_results_fallback_items"; });
    // QObject::connect(s_results_fallback_actions, &QState::entered,
    //                  this, []{ CRIT << "s_results_fallback_actions"; });
    // connect(input_line, &InputLine::textChanged, []{ CRIT << "InputLine::textChanged";});

    //
    // Transitions
    //

    // settingsbutton hidden ->

    addTransition(s_settings_button_hidden, s_settings_button_visible, InputFrameEnter);

    addTransition(s_settings_button_hidden, s_settings_button_highlight, SettingsButtonEnter);

    addTransition(s_settings_button_hidden, s_settings_button_highlight, QueryBusy,
                  [this] { return settings_button->isVisible(); });

    addTransition(s_settings_button_hidden, s_settings_button_highlight_delay, QueryBusy,
                  [this] { return settings_button->isHidden(); });


    // settingsbutton visible ->

    addTransition(s_settings_button_visible, s_settings_button_hidden, InputFrameLeave);

    addTransition(s_settings_button_visible, s_settings_button_highlight, SettingsButtonEnter);

    addTransition(s_settings_button_visible, s_settings_button_highlight, QueryBusy);


    // settingsbutton highlight ->

    addTransition(s_settings_button_highlight, s_settings_button_hidden, QueryIdle,
                  [this] { return !input_frame->underMouse() && !settings_button->underMouse(); });

    addTransition(s_settings_button_highlight, s_settings_button_visible, QueryIdle,
                  [this] { return input_frame->underMouse() && !settings_button->underMouse(); });

    addTransition(s_settings_button_highlight, s_settings_button_visible, SettingsButtonLeave,
                  [this] { return input_frame->underMouse(); });

    addTransition(s_settings_button_highlight, s_settings_button_hidden, SettingsButtonLeave,
                  [this] { return !input_frame->underMouse(); });


    // settingsbutton delay highlight ->

    addTransition(s_settings_button_highlight_delay, s_settings_button_highlight,
                  busy_delay_timer, &QTimer::timeout);

    addTransition(s_settings_button_highlight_delay, s_settings_button_highlight, InputFrameEnter);

    addTransition(s_settings_button_highlight_delay, s_settings_button_highlight, SettingsButtonEnter);

    addTransition(s_settings_button_highlight_delay, s_settings_button_hidden, QueryIdle);


    // settingsbutton spin

    addTransition(s_settings_button_slow, s_settings_button_fast, QueryBusy);

    addTransition(s_settings_button_fast, s_settings_button_slow, QueryIdle);


    // Query

    addTransition(s_results_query_unset, s_results_query_set, QuerySet);

    addTransition(s_results_query_set, s_results_query_unset, QueryUnset);


    // hidden ->

    addTransition(s_results_hidden, s_results_matches, QueryHaveMatches);

    addTransition(s_results_hidden, s_results_fallbacks, ShowFallbacks,
                  [this]{ return haveFallbacks(); });

    addTransition(s_results_hidden, s_results_fallbacks, QueryIdle,
                  [this]{ return haveFallbacks() && !current_query->isTriggered(); });


    // disabled ->

    addTransition(s_results_disabled, s_results_hidden,
                  display_delay_timer, &QTimer::timeout);

    addTransition(s_results_disabled, s_results_matches, QueryHaveMatches);

    addTransition(s_results_disabled, s_results_hidden, QueryIdle,
                  [this]{ return !haveFallbacks() || current_query->isTriggered(); });

    addTransition(s_results_disabled, s_results_fallbacks, QueryIdle,
                  [this]{ return haveFallbacks() && !current_query->isTriggered(); });


    // matches ->

    addTransition(s_results_matches, s_results_disabled, QuerySet);

    addTransition(s_results_matches, s_results_fallbacks, ShowFallbacks,
                  [this]{ return haveFallbacks(); });


    // match actions <->

    addTransition(s_results_match_items, s_results_match_actions, ShowActions);
    addTransition(s_results_match_items, s_results_match_actions, ToggleActions);

    addTransition(s_results_match_actions, s_results_match_items, HideActions);
    addTransition(s_results_match_actions, s_results_match_items, ToggleActions);


    // fallbacks ->

    addTransition(s_results_fallbacks, s_results_disabled, QuerySet);

    addTransition(s_results_fallbacks, s_results_hidden, HideFallbacks,
                  [this]{ return !haveMatches() && current_query->isActive(); });

    addTransition(s_results_fallbacks, s_results_matches, HideFallbacks,
                  [this]{ return haveMatches(); });


    // fallback actions <->

    addTransition(s_results_fallback_items, s_results_fallback_actions, ShowActions);
    addTransition(s_results_fallback_items, s_results_fallback_actions, ToggleActions);

    addTransition(s_results_fallback_actions, s_results_fallback_items, HideActions);
    addTransition(s_results_fallback_actions, s_results_fallback_items, ToggleActions);


    //
    // Behavior
    //

    // BUTTON

    QObject::connect(s_settings_button_hidden, &QState::entered, this, [this]{
        auto c = settings_button->color;
        c.setAlpha(0);
        color_animation_ = make_unique<QPropertyAnimation>(settings_button, "color");
        color_animation_->setEndValue(c);
        color_animation_->setEasingCurve(QEasingCurve::OutQuad);
        color_animation_->setDuration(500);
        connect(color_animation_.get(), &QPropertyAnimation::finished,
                settings_button, &SettingsButton::hide);
        color_animation_->start();
    });

    QObject::connect(s_settings_button_highlight_delay, &QState::entered,
                     this, [busy_delay_timer]{ busy_delay_timer->start(); });

    QObject::connect(s_settings_button_highlight_delay, &QState::exited,
                     this, [busy_delay_timer]{ busy_delay_timer->stop(); });

    QObject::connect(s_settings_button_visible, &QState::entered, this, [this]{
        settings_button->show();
        color_animation_ = make_unique<QPropertyAnimation>(settings_button, "color");
        color_animation_->setEndValue(settings_button_color_);
        color_animation_->setEasingCurve(QEasingCurve::OutQuad);
        color_animation_->setDuration(500);
        color_animation_->start();
    });

    QObject::connect(s_settings_button_highlight, &QState::entered, this, [this]{
        settings_button->show();
        color_animation_ = make_unique<QPropertyAnimation>(settings_button, "color");
        color_animation_->setEndValue(settings_button_color_highlight_);
        color_animation_->setEasingCurve(QEasingCurve::OutQuad);
        color_animation_->setDuration(500);
        color_animation_->start();
    });

    QObject::connect(s_settings_button_slow, &QState::entered, this, [this]{
        speed_animation_ = make_unique<QPropertyAnimation>(settings_button, "speed");
        speed_animation_->setEndValue(defaults.settings_button_rps_idle);
        speed_animation_->setEasingCurve(QEasingCurve::OutQuad);
        speed_animation_->setDuration(3000);
        speed_animation_->start();
    });

    QObject::connect(s_settings_button_fast, &QState::entered, this, [this]{
        speed_animation_ = make_unique<QPropertyAnimation>(settings_button, "speed");
        speed_animation_->setEndValue(defaults.settings_button_rps_busy);
        speed_animation_->setEasingCurve(QEasingCurve::InOutQuad);
        speed_animation_->setDuration(1000);
        speed_animation_->start();
    });


    // RESULTS

    QObject::connect(s_results_query_unset, &QState::entered, this, [this]{
        setModelDeleteSelection(results_list, nullptr);
        input_line->removeEventFilter(results_list);
    });

    QObject::connect(s_results_query_set, &QState::entered, this, [this]{
        installEventFilterKeepThisPrioritized(input_line, results_list);
    });

    QObject::connect(s_results_hidden, &QState::entered, this, [this]{
        results_list->hide();
    });

    QObject::connect(s_results_disabled, &QState::entered, this, [this, display_delay_timer]{
        display_delay_timer->start();
        input_line->removeEventFilter(results_list);
    });

    auto hideActions = [this]
    {
        actions_list->hide();
        input_line->removeEventFilter(actions_list);
        setModelDeleteSelection(actions_list, nullptr);
    };

    auto showActions = [this]
    {
        // if item is selected and has actions display
        if (auto current_index = results_list->currentIndex();
            current_index.isValid())
        {
            if (auto action_names = current_index.data(ItemRoles::ActionsListRole).toStringList();
                !action_names.isEmpty())
            {
                auto old_m = actions_list->model();
                auto new_m = new QStringListModel(action_names, actions_list);
                setModelDeleteSelection(actions_list, new_m);
                delete old_m;

                installEventFilterKeepThisPrioritized(input_line, actions_list);

                actions_list->show();
            }
        }
    };

    QObject::connect(s_results_matches, &QState::entered, this, [this]{
        results_model = make_unique<MatchItemsModel>(*current_query);
        setModelDeleteSelection(results_list, results_model.get());

        installEventFilterKeepThisPrioritized(input_line, results_list);
        connect(results_list, &ResizingList::activated, this, &Window::onMatchActivation);
        connect(actions_list, &ResizingList::activated, this, &Window::onMatchActionActivation);

        results_list->show();

        // let selection model currentChanged set input hint
        connect(results_list->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &current, const QModelIndex&) {
            if (current.isValid())
                input_line->setCompletion(current.data(ItemRoles::InputActionRole).toString());
        });

        // Initialize if we have a selected item
        if (results_list->currentIndex().isValid())
            input_line->setCompletion(results_list->currentIndex()
                                          .data(ItemRoles::InputActionRole).toString());
        else
            input_line->setCompletion();
    });

    QObject::connect(s_results_matches, &QState::exited, this, [this]{
        input_line->removeEventFilter(results_list);
        disconnect(results_list, &ResizingList::activated, this, &Window::onMatchActivation);
        disconnect(actions_list, &ResizingList::activated, this, &Window::onMatchActionActivation);
    });

    QObject::connect(s_results_match_actions, &QState::entered, this, showActions);
    QObject::connect(s_results_match_actions, &QState::exited, this, hideActions);

    QObject::connect(s_results_fallbacks, &QState::entered, this, [this]{
        results_model = make_unique<FallbackItemsModel>(*current_query);
        setModelDeleteSelection(results_list, results_model.get());

        installEventFilterKeepThisPrioritized(input_line, results_list);
        connect(results_list, &ResizingList::activated, this, &Window::onFallbackActivation);
        connect(actions_list, &ResizingList::activated, this, &Window::onFallbackActionActivation);

        results_list->show();
    });

    QObject::connect(s_results_fallbacks, &QState::exited, this, [this]{
        input_line->removeEventFilter(results_list);
        disconnect(results_list, &ResizingList::activated, this, &Window::onFallbackActivation);
        disconnect(actions_list, &ResizingList::activated, this, &Window::onFallbackActionActivation);
    });

    QObject::connect(s_results_fallback_actions, &QState::entered, this, showActions);
    QObject::connect(s_results_fallback_actions, &QState::exited, this, hideActions);


    state_machine = new QStateMachine(this);
    state_machine->addState(s_root);
    state_machine->setInitialState(s_root);
    state_machine->start();
}

void Window::installEventFilterKeepThisPrioritized(QObject *watched, QObject *filter)
{
    // Eventfilters are processed in reverse order
    watched->removeEventFilter(this);
    watched->installEventFilter(filter);
    watched->installEventFilter(this);
}

bool Window::haveMatches() const { return current_query->matches().size() > 0; }

bool Window::haveFallbacks() const { return current_query->fallbacks().size() > 0; }

void Window::postCustomEvent(EventType event_type)
{ state_machine->postEvent(new Event(event_type)); } // takes ownership

void Window::onSettingsButtonClick(Qt::MouseButton button)
{
    if (button == Qt::LeftButton)
        albert::showSettings();

    else if (button == Qt::RightButton)
    {
        auto *menu = new QMenu(this);
        menu->addActions(actions());
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->popup(QCursor::pos());
    }
}

void Window::onMatchActivation(const QModelIndex &index)
{
    if (index.isValid())
        if (auto should_hide = current_query->activateMatch(index.row(), 0);
            should_hide != QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))  // xor
                hide();
}

void Window::onMatchActionActivation(const QModelIndex &index)
{
    if (index.isValid())
        if (auto should_hide = current_query->activateMatch(results_list->currentIndex().row(), index.row());
            should_hide != QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))  // xor
            hide();
}

void Window::onFallbackActivation(const QModelIndex &index)
{
    if (index.isValid())
        if (auto should_hide = current_query->activateFallback(index.row(), 0);
            should_hide != QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))  // xor
            hide();
}

void Window::onFallbackActionActivation(const QModelIndex &index)
{
    if (index.isValid())
        if (auto should_hide = current_query->activateFallback(results_list->currentIndex().row(), index.row());
            should_hide != QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))  // xor
            hide();
}

QString Window::input() const { return input_line->text(); }

void Window::setInput(const QString &text) { input_line->setText(text); }

void Window::setQuery(Query *q)
{
    if(current_query)
    {
        disconnect(current_query, nullptr, this, nullptr);
        if (current_query->isActive())  // for statemachine integrity
            postCustomEvent(QueryIdle);
    }

    current_query = q;
    postCustomEvent(current_query ? QuerySet : QueryUnset);

    if(q)
    {
        input_line->setTriggerLength(q->trigger().length());
        input_line->setSynopsis(q->synopsis());
        input_line->setCompletion();

        connect(current_query, &Query::matchesAdded,
                this, [this]{ postCustomEvent(QueryHaveMatches); });

        connect(current_query, &Query::activeChanged,
                this, [this](bool a){ postCustomEvent(a ? QueryBusy : QueryIdle); });
    }
}

map<QString, QString> Window::findThemes() const
{
    map<QString, QString> t;
    for (const auto &path : plugin.dataLocations())
        for (const auto &ini_file_info : QDir(path/"themes")
                                             .entryInfoList(QStringList("*.ini"),
                                                            QDir::Files|QDir::NoSymLinks))
            t.emplace(ini_file_info.baseName(), ini_file_info.canonicalFilePath());
    return t;
}

void Window::applyTheme(const QString& name)
{
    if (name.isNull())
        applyTheme(Theme{});
    else
    {
        auto path = themes.at(name);  // names assumed to exist

        try {
            applyTheme(Theme::read(path));
        } catch (const runtime_error &e) {
            applyTheme(Theme());
            WARN << e.what();
            albert::warning(QString("%1:%2\n\n%3")
                                .arg(tr("Failed loading theme"), name, e.what()));
        }
    }
}

void Window::applyTheme(const Theme &theme)
{
    QPixmapCache::clear();

    setPalette(theme.palette);

    setShadowBrush(theme.window_shadow_brush);
    setFillBrush(theme.window_background_brush);
    setBorderBrush(theme.window_border_brush);

    input_frame->setFillBrush(theme.input_background_brush);
    input_frame->setBorderBrush(theme.input_border_brush);

    input_line->setTriggerColor(theme.input_trigger_color);
    input_line->setHintColor(theme.input_hint_color);

    settings_button_color_ = theme.settings_button_color;
    settings_button->color = theme.settings_button_color;
    settings_button->color.setAlpha(0);
    settings_button_color_highlight_ = theme.settings_button_highlight_color;

    results_list->setSelectionBackgroundBrush(theme.result_item_selection_background_brush);
    results_list->setSelectionBorderBrush(theme.result_item_selection_border_brush);
    results_list->setSelectionSubextColor(theme.result_item_selection_subtext_color);
    results_list->setSelectionTextColor(theme.result_item_selection_text_color);
    results_list->setSubtextColor(theme.result_item_subtext_color);
    results_list->setTextColor(theme.result_item_text_color);

    actions_list->setSelectionBackgroundBrush(theme.action_item_selection_background_brush);
    actions_list->setSelectionBorderBrush(theme.action_item_selection_border_brush);
    actions_list->setSelectionTextColor(theme.action_item_selection_text_color);
    actions_list->setTextColor(theme.action_item_text_color);
}

bool Window::darkMode() const { return dark_mode; }

bool Window::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
        windowHandle()->startSystemMove();

    else if (event->type() == QEvent::Show)
    {
        // If showCentered or off screen (e.g. display disconnected) move into visible area
        if (showCentered_ || !screen())
        {
            QScreen *screen = nullptr;
            if (followCursor_){
                if (screen = QGuiApplication::screenAt(QCursor::pos()); !screen){
                    WARN << "Could not retrieve screen for cursor position. Using primary screen.";
                    screen = QGuiApplication::primaryScreen();
                }
            }
            else
                screen = QGuiApplication::primaryScreen();

            move(screen->geometry().center().x() - frameSize().width() / 2,
                 screen->geometry().top() + screen->geometry().height() / 5);
        }

#if not defined Q_OS_MACOS // steals focus on macos
        raise();
        activateWindow();
#endif
        emit visibleChanged(true);
    }

    else if (event->type() == QEvent::Hide)
    {
        plugin.state()->setValue(keys.window_position, pos());

        /*
         * Prevent the flicker when the window is shown
         *
         * Qt sends a resize event when the window is shown.
         *
         * When the window was expanded on hide the resize happens on show which introduces ugly
         * flicker. Force the resize to happen before the window is hidden.
         *
         * This may be removed when the frontend does not use the input changed > set query smell
         * anymore.
         */
        setQuery(nullptr);
        // Setting the query seems not to be sufficient. Hide the lists manually.
        results_list->hide();
        actions_list->hide();
        // looks like the layoutsystem is not synchronous
        QCoreApplication::processEvents();

        QPixmapCache::clear();

        emit visibleChanged(false);
    }

    else if (event->type() == QEvent::ThemeChange)
    {
#ifdef Q_OS_LINUX
        // No automatic palette update on GNOME
        QApplication::setPalette(QApplication::style()->standardPalette());
#endif
        dark_mode = haveDarkSystemPalette();
        applyTheme((dark_mode) ? theme_dark_ : theme_light_);
    }

    else if (event->type() == QEvent::Close)
    {
        if(quitOnClose_)
            qApp->quit();
        else
            hide();
    }

    else if (event->type() == QEvent::WindowActivate)
    {
        //
        // Hiding/Showing a window does not generate a Leave/Enter event. As such QWidget does not
        // update the internal underMouse property on show if the window is has been hidden and the
        // mouse pointer moved outside the widget.
        //
        if (auto w = QApplication::widgetAt(QCursor::pos()); w)
        {
            QEvent synth(QEvent::Enter);
            QApplication::sendEvent(w, &synth);
            for (auto p : getParents(w))
                QApplication::sendEvent(p, &synth);
        }
    }

    else if (event->type() == QEvent::WindowDeactivate)
    {
        //
        // Hiding/Showing a window does not generate a Leave/Enter event. As such QWidget does not
        // update the internal underMouse property on show if the window is has been hidden and the
        // mouse pointer moved outside the widget.
        //
        if (auto w = QApplication::widgetAt(QCursor::pos()); w)
        {
            QEvent synth(QEvent::Leave);
            QApplication::sendEvent(w, &synth);
            for (auto p : getParents(w))
                QApplication::sendEvent(p, &synth);
        }

        if(hideOnFocusLoss_)
            setVisible(false);
    }

    // DEBG << event->type();

    return QWidget::event(event);
}

bool Window::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == input_line)
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *ke = static_cast<QKeyEvent *>(event);
            switch (ke->key()) {

            case Qt::Key_Up:
                // Move up in the history
                if (!results_list->currentIndex().isValid()
                    || ke->modifiers().testFlag(Qt::ShiftModifier)
                    || (results_list->currentIndex().row() == 0
                        && !ke->isAutoRepeat()))  // ... and first row (non repeat)
                {
                    input_line->next();
                    return true;
                }
                break;

            case Qt::Key_Down:
                // Move down in the history
                if (ke->modifiers().testFlag(Qt::ShiftModifier))
                {
                    input_line->previous();
                    return true;
                }
                break;

            case Qt::Key_P:
            case Qt::Key_K:
                if (ke->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Up,
                        ke->modifiers().setFlag(Qt::ControlModifier, false),
                        ke->text(), ke->isAutoRepeat());
                    QApplication::sendEvent(input_line, &syn);
                }
                break;

            case Qt::Key_N:
            case Qt::Key_J:
                if (ke->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Down,
                        ke->modifiers().setFlag(Qt::ControlModifier, false),
                        ke->text(), ke->isAutoRepeat());
                    QApplication::sendEvent(input_line, &syn);
                }
                break;

            case Qt::Key_H:
                if (ke->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Left,
                        ke->modifiers().setFlag(Qt::ControlModifier, false),
                        ke->text(), ke->isAutoRepeat());
                    QApplication::sendEvent(input_line, &syn);
                }
                break;

            case Qt::Key_L:
                if (ke->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Right,
                        ke->modifiers().setFlag(Qt::ControlModifier, false),
                        ke->text(), ke->isAutoRepeat());
                    QApplication::sendEvent(input_line, &syn);
                }
                break;

            case Qt::Key_Comma:{
                if (ke->modifiers() == Qt::ControlModifier || ke->modifiers() == Qt::AltModifier){
                    showSettings();
                    setVisible(false);
                    return true;
                }
                break;
            }

            case Qt::Key_Escape:
                setVisible(false);
                break;
            }

            if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
                && ke->modifiers() == mods_mod[mod_command])
            {
                postCustomEvent(ToggleActions);
                return true;
            }

            if (ke->key() == mods_keys[mod_actions])
            {
                postCustomEvent(ShowActions);
                return true;
            }

            if (ke->key() == mods_keys[mod_fallback])
            {
                postCustomEvent(ShowFallbacks);
                return true;
            }
        }

        else if (event->type() == QEvent::KeyRelease)
        {
            auto *ke = static_cast<QKeyEvent *>(event);

            if (ke->key() == mods_keys[mod_actions])
            {
                postCustomEvent(HideActions);
                return true;
            }

            else if (ke->key() == mods_keys[mod_fallback])
            {
                postCustomEvent(HideFallbacks);
                return true;
            }
        }
    }

    else if (watched == input_frame)
    {
        if (event->type() == QEvent::Enter)
            postCustomEvent(InputFrameEnter);

        else if (event->type() == QEvent::Leave)
            postCustomEvent(InputFrameLeave);
    }

    else if (watched == settings_button)
    {
        if (event->type() == QEvent::Enter)
            postCustomEvent(SettingsButtonEnter);

        else if (event->type() == QEvent::Leave)
            postCustomEvent(SettingsButtonLeave);
    }
    return false;
}

//
//  PROPERTIES
//

const QString &Window::themeLight() const { return theme_light_; }
void Window::setThemeLight(const QString &val)
{
    if (themeLight() == val)
        return;

    if (!themes.contains(val) && !val.isNull())
    {
        WARN << "Theme does not exist:" << val;
        return;
    }

    if (!dark_mode)
        applyTheme(val);

    theme_light_ = val;
    plugin.settings()->setValue(keys.theme_light, val);
    emit themeLightChanged(val);
}

const QString &Window::themeDark() const { return theme_dark_; }
void Window::setThemeDark(const QString &val)
{
    if (themeDark() == val)
        return;

    if (!themes.contains(val) && !val.isNull())
    {
        WARN << "Theme does not exist:" << val;
        return;
    }

    if (dark_mode)
        applyTheme(val);

    theme_dark_ = val;
    plugin.settings()->setValue(keys.theme_dark, val);
    emit themeDarkChanged(val);
}

bool Window::alwaysOnTop() const { return windowFlags() & Qt::WindowStaysOnTopHint; }
void Window::setAlwaysOnTop(bool val)
{
    if (alwaysOnTop() == val)
        return;

    setWindowFlags(windowFlags().setFlag(Qt::WindowStaysOnTopHint, val));
    plugin.settings()->setValue(keys.always_on_top, val);
    emit clearOnHideChanged(val);
}

bool Window::clearOnHide() const { return input_line->clear_on_hide; }
void Window::setClearOnHide(bool val)
{
    if (clearOnHide() == val)
        return;

    input_line->clear_on_hide = val;
    plugin.settings()->setValue(keys.clear_on_hide, val);
    emit clearOnHideChanged(val);
}

bool Window::displayScrollbar() const
{ return results_list->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff; }
void Window::setDisplayScrollbar(bool val)
{
    if (displayScrollbar() == val)
        return;

    results_list->setVerticalScrollBarPolicy(val ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    plugin.settings()->setValue(keys.display_scrollbar, val);
    emit displayScrollbarChanged(val);
}

bool Window::followCursor() const { return followCursor_; }
void Window::setFollowCursor(bool val)
{
    if (followCursor() == val)
        return;

    followCursor_ = val;
    plugin.settings()->setValue(keys.follow_cursor, val);
    emit hideOnFocusLossChanged(val);
}

bool Window::hideOnFocusLoss() const { return hideOnFocusLoss_; }
void Window::setHideOnFocusLoss(bool val)
{
    if (hideOnFocusLoss() == val)
        return;

    hideOnFocusLoss_ = val;
    plugin.settings()->setValue(keys.hide_on_focus_loss, val);
    emit hideOnFocusLossChanged(val);
}

bool Window::historySearchEnabled() const { return input_line->history_search; }
void Window::setHistorySearchEnabled(bool val)
{
    if (historySearchEnabled() == val)
        return;

    input_line->history_search = val;
    plugin.settings()->setValue(keys.history_search, val);
    emit maxResultsChanged(val);
}

uint Window::maxResults() const { return results_list->maxItems(); }
void Window::setMaxResults(uint val)
{
    if (maxResults() == val)
        return;

    results_list->setMaxItems(val);
    plugin.settings()->setValue(keys.max_results, val);
    emit maxResultsChanged(val);
}

bool Window::quitOnClose() const { return quitOnClose_; }
void Window::setQuitOnClose(bool val)
{
    if (quitOnClose() == val)
        return;

    quitOnClose_ = val;
    plugin.settings()->setValue(keys.quit_on_close, val);
    emit quitOnCloseChanged(val);
}

bool Window::showCentered() const { return showCentered_; }
void Window::setShowCentered(bool val)
{
    if (showCentered() == val)
        return;

    showCentered_ = val;
    plugin.settings()->setValue(keys.centered, val);
    emit showCenteredChanged(val);
}

bool Window::debugMode() const { return debug_overlay_.get(); }
void Window::setDebugMode(bool val)
{
    if (debugMode() == val)
        return;

    results_list->setDebugMode(val);
    actions_list->setDebugMode(val);

    if (val)
    {
        debug_overlay_ = make_unique<DebugOverlay>();
        debug_overlay_->recursiveInstallEventFilter(this);
    }
    else
        debug_overlay_.reset();

    plugin.settings()->setValue(keys.debug, val);
    update();
    emit debugModeChanged(val);
}

bool Window::disableInputMethod() const { return input_line->disable_input_method_; }
void Window::setDisableInputMethod(bool val) { input_line->disable_input_method_ = val; }


uint Window::windowShadowSize() const { return shadowSize(); }
void Window::setWindowShadowSize(uint val) { setShadowSize(val); }

uint Window::windowShadowOffset() const { return shadowOffset(); }
void Window::setWindowShadowOffset(uint val) { setShadowOffset(val); }

double Window::windowBorderRadius() const { return radius(); }
void Window::setWindowBorderRadius(double val) { setRadius(val); }

double Window::windowBorderWidth() const { return borderWidth(); }
void Window::setWindowBorderWidth(double val) { setBorderWidth(val); }

uint Window::windowPadding() const { return layout()->contentsMargins().left(); }
void Window::setWindowPadding(uint val) { layout()->setContentsMargins(val, val, val, val); }

uint Window::windowSpacing() const { return layout()->spacing(); }
void Window::setWindowSpacing(uint val) { layout()->setSpacing(val); }

uint Window::windowWidth() const { return input_frame->width(); }
void Window::setWindowWidth(uint val) { input_frame->setFixedWidth(val); }


uint Window::inputPadding() const { return input_frame->contentsMargins().left(); }
void Window::setInputPadding(uint val) { input_frame->setContentsMargins(val, val, val, val); }

double Window::inputBorderRadius() const { return input_frame->radius(); }
void Window::setInputBorderRadius(double val) { input_frame->setRadius(val); }

double Window::inputBorderWidth() const { return input_frame->borderWidth(); }
void Window::setInputBorderWidth(double val) { input_frame->setBorderWidth(val); }

uint Window::inputFontSize() const { return input_line->fontSize(); }
void Window::setInputFontSize(uint val)
{
    input_line->setFontSize(val);

    // Fix for nicely aligned text.
    // The text should be idented by the distance of the cap line to the top.
    auto fm = input_line->fontMetrics();
    auto font_margin_fix = (fm.lineSpacing() - fm.capHeight() - fm.tightBoundingRect("|").width())/2 ;
    spacer_left->changeSize(font_margin_fix, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer_right->changeSize(font_margin_fix, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto input_line_height = fm.lineSpacing() + 2;  // 1px document margins
    settings_button->setFixedSize(input_line_height, input_line_height);
    settings_button->setContentsMargins(font_margin_fix, font_margin_fix,
                                        font_margin_fix, font_margin_fix);
}


double Window::resultItemSelectionBorderRadius() const { return results_list->borderRadius(); }
void Window::setResultItemSelectionBorderRadius(double val) { results_list->setBorderRadius(val); }

double Window::resultItemSelectionBorderWidth() const { return results_list->borderWidth(); }
void Window::setResultItemSelectionBorderWidth(double val) { results_list->setBorderWidth(val); }

uint Window::resultItemPadding() const { return results_list->padding(); }
void Window::setResultItemPadding(uint val) { results_list->setPadding(val); }

uint Window::resultItemIconSize() const { return results_list->iconSize(); }
void Window::setResultItemIconSize(uint val) { results_list->setIconSite(val); }

uint Window::resultItemTextFontSize() const { return results_list->textFontSize(); }
void Window::setResultItemTextFontSize(uint val) { results_list->setTextFontSize(val); }

uint Window::resultItemSubtextFontSize() const { return results_list->subtextFontSize(); }
void Window::setResultItemSubtextFontSize(uint val) { results_list->setSubextFontSize(val); }

uint Window::resultItemHorizontalSpace() const { return results_list->horizonzalSpacing(); }
void Window::setResultItemHorizontalSpace(uint val) { results_list->setHorizonzalSpacing(val); }

uint Window::resultItemVerticalSpace() const { return results_list->verticalSpacing(); }
void Window::setResultItemVerticalSpace(uint val) { results_list->setVerticalSpacing(val); }


double Window::actionItemSelectionBorderRadius() const { return actions_list->borderRadius(); }
void Window::setActionItemSelectionBorderRadius(double val) { actions_list->setBorderRadius(val); }

double Window::actionItemSelectionBorderWidth() const { return actions_list->borderWidth(); }
void Window::setActionItemSelectionBorderWidth(double val) { actions_list->setBorderWidth(val); }

uint Window::actionItemPadding() const { return actions_list->padding(); }
void Window::setActionItemPadding(uint val) { actions_list->setPadding(val); }

uint Window::actionItemFontSize() const { return actions_list->textFontSize(); }
void Window::setActionItemFontSize(uint val) { actions_list->setTextFontSize(val); }




// bool Window::displayClientShadow() const { return graphicsEffect() != nullptr; }
// void Window::setDisplayClientShadow(bool val)
// {
//     if (displayClientShadow() == val)
//         return;

//     if (val)
//     {
//         auto* effect = new QGraphicsDropShadowEffect(this);
//         effect->setBlurRadius(defaults.window_shadow_size);
//         effect->setColor(defaults.window_shadow_color);
//         effect->setXOffset(0.0);
//         effect->setYOffset(defaults.window_shadow_offset);
//         setGraphicsEffect(effect);  // takes ownership
//         setContentsMargins(defaults.window_shadow_size,
//                            defaults.window_shadow_size - (int)effect->yOffset(),
//                            defaults.window_shadow_size,
//                            defaults.window_shadow_size + (int)effect->yOffset());
//     }
//     else
//     {
//         setGraphicsEffect(nullptr);
//         setContentsMargins(0,0,0,0);
//     }

//     plugin.settings()->setValue(keys.shadow_client, val);
//     emit displayClientShadowChanged(val);
// }

// bool Window::displaySystemShadow() const
// { return !windowFlags().testFlag(Qt::NoDropShadowWindowHint); }
// void Window::setDisplaySystemShadow(bool val)
// {
//     if (displaySystemShadow() == val)
//         return;

//     setWindowFlags(windowFlags().setFlag(Qt::NoDropShadowWindowHint, !val));
//     plugin.settings()->setValue(keys.shadow_system, val);
//     emit displaySystemShadowChanged(val);
// }
