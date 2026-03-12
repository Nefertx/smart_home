#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTabBar>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QVector>

class LoginWidget;
class HomeWidget;
class DeviceControlWidget;
class SceneWidget;
class HistoryWidget;
class AlarmWidget;
class SettingsWidget;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

protected:
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void onLoginSuccess(const QString& username);
    void onLogout();

private:
    void setupUI();
    void setupNavBar();
    void applyAdaptiveUiScale();
    void updateNavButtonStyles(double scale);
    void setupEnvSampling();
    void applyEnvSamplingInterval(int seconds);
    void onEnvSamplingTick();

    QWidget*             m_mainWidget   = nullptr;
    QStackedWidget*      m_stack        = nullptr;
    QWidget*             m_navBar       = nullptr;
    QLabel*              m_logoLabel    = nullptr;
    QLabel*              m_userLabel    = nullptr;
    QString              m_currentUser;
    int                  m_lastFontPt   = -1;
    int                  m_currentPage  = 0;
    QVector<QPushButton*> m_navButtons;
    QTimer*              m_envSampleTimer = nullptr;

    LoginWidget*         m_loginWidget  = nullptr;
    HomeWidget*          m_homeWidget   = nullptr;
    DeviceControlWidget* m_deviceWidget = nullptr;
    SceneWidget*         m_sceneWidget  = nullptr;
    HistoryWidget*       m_historyWidget= nullptr;
    AlarmWidget*         m_alarmWidget  = nullptr;
    SettingsWidget*      m_settingsWidget=nullptr;

    // 主内容区 StackedWidget 的页面索引
    enum Page { PAGE_HOME=0, PAGE_DEVICE, PAGE_SCENE, PAGE_HISTORY, PAGE_ALARM, PAGE_SETTINGS };
};
