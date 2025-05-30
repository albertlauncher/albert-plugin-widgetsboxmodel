// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include "windowframe.h"
#include <QEvent>
#include <QPoint>
#include <QTimer>
#include <QWidget>
namespace albert {
class Query;
class PluginInstance;
}
class ActionDelegate;
class ActionsList;
class DebugOverlay;
class Frame;
class InputLine;
class ItemDelegate;
class Plugin;
class QEvent;
class QFrame;
class QKeyCombination;
class QListView;
class QPropertyAnimation;
class QSpacerItem;
class QStateMachine;
class ResultItemsModel;
class ResultsList;
class SettingsButton;
class Theme;

class Window : public WindowFrame
{
    Q_OBJECT

public:

    Window(albert::PluginInstance &plugin);
    ~Window();

    albert::PluginInstance const &plugin;

    QString input() const;
    void setInput(const QString&);

    void setQuery(albert::Query *query);

    const std::map<QString, QString> themes;

    bool darkMode() const;

private:

    void initializeUi();
    void initializeProperties();
    void initializeWindowActions();
    void initializeStatemachine();
    void installEventFilterKeepThisPrioritized(QObject *watched, QObject *filter);
    std::map<QString, QString> findThemes() const;
    void applyTheme(const QString& name);  // only for valid names, throws runtime_errors
    void applyTheme(const Theme &);

    bool haveMatches() const;
    bool haveFallbacks() const;

    void onSettingsButtonClick(Qt::MouseButton button);
    void onMatchActivation(const QModelIndex &);
    void onMatchActionActivation(const QModelIndex &);
    void onFallbackActivation(const QModelIndex &);
    void onFallbackActionActivation(const QModelIndex &);

    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    QStateMachine *state_machine;

    Frame *input_frame;
    InputLine *input_line;
    QSpacerItem *spacer_left;
    QSpacerItem *spacer_right;
    SettingsButton *settings_button;
    ResultsList *results_list;
    ActionsList *actions_list;
    bool dark_mode;

    albert::Query *current_query;
    QListView *keyboard_navigation_receiver;

    enum Mod {Shift, Meta, Contol, Alt};
    Mod mod_command = Mod::Contol;
    Mod mod_actions = Mod::Alt;
    Mod mod_fallback = Mod::Meta;

    QString theme_light_;  // null or exists in themes
    QString theme_dark_;   // null or exists in themes
    bool hideOnFocusLoss_;
    bool showCentered_;
    bool followCursor_;
    bool quitOnClose_;
    bool shadow_size_;
    bool shadow_offset_;
    bool edit_mode_;
    QColor settings_button_color_;
    QColor settings_button_color_highlight_;
    std::unique_ptr<DebugOverlay> debug_overlay_;
    std::unique_ptr<QPropertyAnimation> color_animation_;
    std::unique_ptr<QPropertyAnimation> speed_animation_;

    enum EventType {
        ShowActions = QEvent::User,
        HideActions,
        ToggleActions,
        ShowFallbacks,
        HideFallbacks,
        SettingsButtonEnter,
        SettingsButtonLeave,
        InputFrameEnter,
        InputFrameLeave,
    };

    struct Event : public QEvent {
        Event(EventType eventType) : QEvent((QEvent::Type)eventType) {}
    };

    void postCustomEvent(EventType type);

signals:

    void inputChanged(QString);
    void visibleChanged(bool);
    void queryChanged(albert::Query*);
    void queryActiveChanged(bool);  // Convenience signal to avoid reconnects
    void queryHasMatches();  // Convenience signal to avoid reconnects

public:

    const QString &themeLight() const;
    void setThemeLight(const QString& theme);

    const QString &themeDark() const;
    void setThemeDark(const QString& theme);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    bool clearOnHide() const;
    void setClearOnHide(bool b = true);

    bool displayScrollbar() const;
    void setDisplayScrollbar(bool value);

    bool followCursor() const;
    void setFollowCursor(bool b = true);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool b = true);

    bool historySearchEnabled() const;
    void setHistorySearchEnabled(bool b = true);

    uint maxResults() const;
    void setMaxResults(uint max);

    bool quitOnClose() const;
    void setQuitOnClose(bool b = true);

    bool showCentered() const;
    void setShowCentered(bool b = true);

    bool disableInputMethod() const;
    void setDisableInputMethod(bool b = true);

    bool debugMode() const;
    void setDebugMode(bool b = true);

    bool editModeEnabled() const;
    void setEditModeEnabled(bool v);


    uint windowShadowSize() const;
    void setWindowShadowSize(uint);

    uint windowShadowOffset() const;
    void setWindowShadowOffset(uint);


    uint windowWidth() const;
    void setWindowWidth(uint);

    uint windowPadding() const;
    void setWindowPadding(uint);

    uint windowSpacing() const;
    void setWindowSpacing(uint);

    double windowBorderRadius() const;
    void setWindowBorderRadius(double);

    double windowBorderWidth() const;
    void setWindowBorderWidth(double);


    uint inputPadding() const;
    void setInputPadding(uint);

    double inputBorderRadius() const;
    void setInputBorderRadius(double);

    double inputBorderWidth() const;
    void setInputBorderWidth(double);


    uint inputFontSize() const;
    void setInputFontSize(uint);


    double resultItemSelectionBorderRadius() const;
    void setResultItemSelectionBorderRadius(double);

    double resultItemSelectionBorderWidth() const;
    void setResultItemSelectionBorderWidth(double);

    uint resultItemIconSize() const;
    void setResultItemIconSize(uint);

    uint resultItemTextFontSize() const;
    void setResultItemTextFontSize(uint);

    uint resultItemSubtextFontSize() const;
    void setResultItemSubtextFontSize(uint);

    uint resultItemHorizontalSpace() const;
    void setResultItemHorizontalSpace(uint);

    uint resultItemVerticalSpace() const;
    void setResultItemVerticalSpace(uint);

    uint resultItemPadding() const;
    void setResultItemPadding(uint);


    double actionItemSelectionBorderRadius() const;
    void setActionItemSelectionBorderRadius(double);

    double actionItemSelectionBorderWidth() const;
    void setActionItemSelectionBorderWidth(double);


    uint actionItemFontSize() const;
    void setActionItemFontSize(uint);

    uint actionItemPadding() const;
    void setActionItemPadding(uint);

signals:

    void alwaysOnTopChanged(bool);
    void clearOnHideChanged(bool);
    void displayClientShadowChanged(bool);
    void displayScrollbarChanged(bool);
    void displaySystemShadowChanged(bool);
    void followCursorChanged(bool);
    void hideOnFocusLossChanged(bool);
    void historySearchEnabledChanged(bool);
    void maxResultsChanged(uint);
    void quitOnCloseChanged(bool);
    void showCenteredChanged(bool);
    void debugModeChanged(bool);
    void themeDarkChanged(QString);
    void themeLightChanged(QString);
    void editModeEnabledChanged(bool);
};
