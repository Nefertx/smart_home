#include "mainwindow.h"
#include "loginwidget.h"
#include "homewidget.h"
#include "devicecontrolwidget.h"
#include "scenewidget.h"
#include "historywidget.h"
#include "alarmwidget.h"
#include "settingswidget.h"
#include "databasemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QApplication>
#include <QScrollArea>
#include <QResizeEvent>
#include <QFont>
#include <QtGlobal>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("智能家居监控平台");
    // setWindowIcon(QIcon(":/icons/smarthome.ico"));
    resize(820, 520);
    setMinimumSize(480, 320);

    // 初始化数据库
    DatabaseManager::instance()->init();

    // 先显示登录界面（覆盖整个窗口）
    m_loginWidget = new LoginWidget(this);
    QScrollArea* loginScroll = new QScrollArea(this);
    loginScroll->setWidget(m_loginWidget);
    loginScroll->setWidgetResizable(true);
    loginScroll->setFrameShape(QFrame::NoFrame);
    setCentralWidget(loginScroll);
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);

    applyAdaptiveUiScale();
}

void MainWindow::onLoginSuccess(const QString& username) {
    m_currentUser = username;
    setupUI();
    setupNavBar();

    // 用主界面替换登录界面
    QWidget* main = new QWidget(this);
    QVBoxLayout* vl = new QVBoxLayout(main);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);
    vl->addWidget(m_navBar);
    vl->addWidget(m_stack);
    setCentralWidget(main);
    m_mainWidget = main;

    applyAdaptiveUiScale();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    applyAdaptiveUiScale();
}

void MainWindow::applyAdaptiveUiScale() {
    // Keep scaling gentle so the UI remains readable while still responsive.
    const double sx = width() / 1200.0;
    const double sy = height() / 750.0;
    const double scale = qBound(0.80, qMin(sx, sy), 1.25);

    const int fontPt = qBound(9, static_cast<int>(10 * scale), 13);
    if (fontPt != m_lastFontPt) {
        QFont f = qApp->font();
        f.setPointSize(fontPt);
        qApp->setFont(f);
        m_lastFontPt = fontPt;
    }

    if (m_navBar) {
        const int navH = qBound(40, static_cast<int>(50 * scale), 64);
        m_navBar->setFixedHeight(navH);
        updateNavButtonStyles(scale);

        if (m_logoLabel) {
            QFont lf = m_logoLabel->font();
            lf.setPointSize(qBound(11, static_cast<int>(14 * scale), 18));
            lf.setBold(true);
            m_logoLabel->setFont(lf);
        }

        if (m_userLabel) {
            QFont uf = m_userLabel->font();
            uf.setPointSize(qBound(9, static_cast<int>(10 * scale), 13));
            m_userLabel->setFont(uf);
        }
    }
}

void MainWindow::updateNavButtonStyles(double scale) {
    const int padV = qBound(4, static_cast<int>(6 * scale), 10);
    const int padH = qBound(8, static_cast<int>(14 * scale), 18);
    const int radius = qBound(3, static_cast<int>(5 * scale), 8);
    const int fontPt = qBound(9, static_cast<int>(10 * scale), 13);

    for (int i = 0; i < m_navButtons.size(); ++i) {
        QPushButton* btn = m_navButtons.at(i);
        if (!btn) {
            continue;
        }
        const bool active = (i == m_currentPage);
        const QString bg = active ? "#1abc9c" : "transparent";
        const QString hover = active ? "#17a589" : "#34495e";
        btn->setStyleSheet(QString(
            "QPushButton{color:white;padding:%1px %2px;border-radius:%3px;"
            "background:%4;font-weight:%5;font-size:%6pt;}"
            "QPushButton:hover{background:%7;}"
        ).arg(padV).arg(padH).arg(radius).arg(bg).arg(active ? 700 : 500).arg(fontPt).arg(hover));
    }
}

void MainWindow::setupUI() {
    m_homeWidget    = new HomeWidget(m_currentUser, this);
    m_deviceWidget  = new DeviceControlWidget(m_currentUser, this);
    m_sceneWidget   = new SceneWidget(m_currentUser, this);
    m_historyWidget = new HistoryWidget(this);
    m_alarmWidget   = new AlarmWidget(this);
    m_settingsWidget= new SettingsWidget(this);

    m_stack = new QStackedWidget(this);

    auto wrapScrollable = [this](QWidget* page) {
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setWidget(page);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        return scroll;
    };

    m_stack->addWidget(wrapScrollable(m_homeWidget));
    m_stack->addWidget(wrapScrollable(m_deviceWidget));
    m_stack->addWidget(wrapScrollable(m_sceneWidget));
    m_stack->addWidget(wrapScrollable(m_historyWidget));
    m_stack->addWidget(wrapScrollable(m_alarmWidget));
    m_stack->addWidget(wrapScrollable(m_settingsWidget));

    // 页面间信号连接
    connect(m_homeWidget, &HomeWidget::navigateTo, m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_deviceWidget, &DeviceControlWidget::deviceChanged, m_homeWidget, &HomeWidget::refresh);
    connect(m_alarmWidget, &AlarmWidget::alarmTriggered, [this](const QString& msg){
        m_alarmWidget->addAlarm("设备异常", msg, "系统");
    });
}

void MainWindow::setupNavBar() {
    m_navBar = new QWidget(this);
    m_navBar->setFixedHeight(50);
    m_navBar->setStyleSheet("background:#2c3e50; color:white;");

    QHBoxLayout* hl = new QHBoxLayout(m_navBar);
    hl->setContentsMargins(10,0,10,0);

    m_logoLabel = new QLabel("🏠 智能家居监控平台", m_navBar);
    m_logoLabel->setStyleSheet("color:white; font-size:16px; font-weight:bold;");
    hl->addWidget(m_logoLabel);
    hl->addStretch();

    m_navButtons.clear();

    auto makeBtn = [&](const QString& text, int page) {
        QPushButton* btn = new QPushButton(text, m_navBar);
        btn->setFlat(true);
        m_navButtons.append(btn);
        connect(btn, &QPushButton::clicked, [this, page]{
            m_currentPage = page;
            m_stack->setCurrentIndex(page);
            applyAdaptiveUiScale();
        });
        hl->addWidget(btn);
    };

    makeBtn("首页",       PAGE_HOME);
    makeBtn("设备控制",   PAGE_DEVICE);
    makeBtn("场景模式",   PAGE_SCENE);
    makeBtn("历史记录",   PAGE_HISTORY);
    makeBtn("报警管理",   PAGE_ALARM);
    makeBtn("系统设置",   PAGE_SETTINGS);

    connect(m_stack, &QStackedWidget::currentChanged, this, [this](int index) {
        m_currentPage = index;
        applyAdaptiveUiScale();
    });

    m_userLabel = new QLabel("用户: " + m_currentUser, m_navBar);
    m_userLabel->setStyleSheet("color:#bdc3c7; margin-left:10px;");
    hl->addWidget(m_userLabel);

    QPushButton* logoutBtn = new QPushButton("退出", m_navBar);
    logoutBtn->setStyleSheet("QPushButton{color:#e74c3c;padding:4px 10px;border:1px solid #e74c3c;border-radius:4px;}"
                             "QPushButton:hover{background:#e74c3c;color:white;}");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    hl->addWidget(logoutBtn);

    applyAdaptiveUiScale();
}

void MainWindow::onLogout() {
    m_loginWidget = new LoginWidget(this);
    connect(m_loginWidget, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);
    QScrollArea* loginScroll = new QScrollArea(this);
    loginScroll->setWidget(m_loginWidget);
    loginScroll->setWidgetResizable(true);
    loginScroll->setFrameShape(QFrame::NoFrame);
    setCentralWidget(loginScroll);
    m_currentUser.clear();
}
