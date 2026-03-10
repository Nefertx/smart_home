#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTabBar>
#include <QLabel>
#include <QString>

class LoginWidget;
class HomeWidget;
class DeviceControlWidget;
class SceneWidget;
class HistoryWidget;
class AlarmWidget;
class SettingsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

public slots:
    void onLoginSuccess(const QString& username);
    void onLogout();

private:
    void setupUI();
    void setupNavBar();

    QWidget*             m_mainWidget   = nullptr;
    QStackedWidget*      m_stack        = nullptr;
    QWidget*             m_navBar       = nullptr;
    QLabel*              m_userLabel    = nullptr;
    QString              m_currentUser;

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
