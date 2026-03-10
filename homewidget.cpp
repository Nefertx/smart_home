#include "homewidget.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QDateTime>

static QWidget* makeCard(const QString& title, const QString& iconText,
                          const QString& color, QLabel*& valueLbl, QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setFixedSize(180, 110);
    card->setStyleSheet(QString("QFrame{background:%1;border-radius:10px;}").arg(color));

    QVBoxLayout* vl = new QVBoxLayout(card);
    QLabel* icon = new QLabel(iconText, card);
    icon->setAlignment(Qt::AlignHCenter);
    icon->setStyleSheet("font-size:28px;");

    valueLbl = new QLabel("0", card);
    valueLbl->setAlignment(Qt::AlignHCenter);
    valueLbl->setStyleSheet("color:white;font-size:28px;font-weight:bold;");

    QLabel* titleLbl = new QLabel(title, card);
    titleLbl->setAlignment(Qt::AlignHCenter);
    titleLbl->setStyleSheet("color:rgba(255,255,255,0.85);font-size:13px;");

    vl->addWidget(icon);
    vl->addWidget(valueLbl);
    vl->addWidget(titleLbl);
    return card;
}

HomeWidget::HomeWidget(const QString& username, QWidget* parent)
    : QWidget(parent), m_username(username) {
    setupUI();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &HomeWidget::refresh);
    m_timer->start(5000);
    refresh();
}

void HomeWidget::setupUI() {
    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setContentsMargins(20,20,20,20);
    vl->setSpacing(16);

    // 欢迎语
    QLabel* welcome = new QLabel(QString("欢迎回来，%1 ！").arg(m_username), this);
    welcome->setStyleSheet("font-size:20px;font-weight:bold;color:#2c3e50;");
    m_timeLbl = new QLabel(this);
    m_timeLbl->setStyleSheet("color:#7f8c8d;font-size:13px;");
    vl->addWidget(welcome);
    vl->addWidget(m_timeLbl);

    // 统计卡片
    QGroupBox* statsBox = new QGroupBox("设备概览", this);
    QHBoxLayout* cardsLay = new QHBoxLayout(statsBox);
    cardsLay->setSpacing(16);
    cardsLay->addWidget(makeCard("在线设备", "✅", "#27ae60", m_onlineLbl, statsBox));
    cardsLay->addWidget(makeCard("离线设备", "❌", "#e74c3c", m_offlineLbl, statsBox));
    cardsLay->addWidget(makeCard("当前报警", "🔔", "#e67e22", m_alarmLbl, statsBox));
    cardsLay->addStretch();
    vl->addWidget(statsBox);

    // 快捷控制
    QGroupBox* quickBox = new QGroupBox("快捷操作", this);
    QGridLayout* grid = new QGridLayout(quickBox);
    grid->setSpacing(10);

    auto makeQuick = [&](const QString& label, int page, int row, int col) {
        QPushButton* btn = new QPushButton(label, quickBox);
        btn->setFixedHeight(60);
        btn->setStyleSheet("QPushButton{background:#3498db;color:white;border-radius:8px;font-size:14px;}"
                           "QPushButton:hover{background:#2980b9;}");
        connect(btn, &QPushButton::clicked, [this, page]{ emit navigateTo(page); });
        grid->addWidget(btn, row, col);
    };
    makeQuick("📱 设备控制", 1, 0, 0);
    makeQuick("🎬 场景模式", 2, 0, 1);
    makeQuick("📊 历史记录", 3, 0, 2);
    makeQuick("🔔 报警管理", 4, 1, 0);
    makeQuick("⚙️  系统设置", 5, 1, 1);
    vl->addWidget(quickBox);

    // 场景快捷切换
    QGroupBox* sceneBox = new QGroupBox("常用场景", this);
    QHBoxLayout* sceneLay = new QHBoxLayout(sceneBox);
    auto makeScene = [&](const QString& name, const QString& color) {
        QPushButton* btn = new QPushButton(name, sceneBox);
        btn->setFixedHeight(44);
        btn->setStyleSheet(QString("QPushButton{background:%1;color:white;border-radius:6px;font-size:13px;}"
                                   "QPushButton:hover{opacity:0.8;}").arg(color));
        connect(btn, &QPushButton::clicked, [this]{ emit navigateTo(2); });
        sceneLay->addWidget(btn);
    };
    makeScene("🏠 回家模式", "#16a085");
    makeScene("😴 睡眠模式", "#8e44ad");
    makeScene("🚶 离家模式", "#e67e22");
    makeScene("🎬 观影模式", "#2980b9");
    vl->addWidget(sceneBox);
    vl->addStretch();
}

void HomeWidget::refresh() {
    m_timeLbl->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    updateStats();
}

void HomeWidget::updateStats() {
    auto devices = DatabaseManager::instance()->getDevices();
    int online = 0, offline = 0;
    for (const auto& d : devices) {
        if (d["status"].toString() == "online") online++;
        else offline++;
    }
    auto alarms = DatabaseManager::instance()->getAlarms();
    m_onlineLbl->setText(QString::number(online));
    m_offlineLbl->setText(QString::number(offline));
    m_alarmLbl->setText(QString::number(alarms.size()));
}
