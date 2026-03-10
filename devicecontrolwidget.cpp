#include "devicecontrolwidget.h"
#include "databasemanager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QGroupBox>
#include <QFrame>
#include <QSplitter>
#include <QMessageBox>
#include <QInputDialog>

DeviceControlWidget::DeviceControlWidget(const QString& username, QWidget* parent)
    : QWidget(parent), m_username(username) {
    setupUI();
    loadDevices();
}

void DeviceControlWidget::setupUI() {
    QVBoxLayout* outerVl = new QVBoxLayout(this);
    outerVl->setContentsMargins(12,12,12,12);

    // 顶栏
    QHBoxLayout* topBar = new QHBoxLayout();
    QLabel* titleLbl = new QLabel("设备控制与状态监控", this);
    titleLbl->setStyleSheet("font-size:16px;font-weight:bold;color:#2c3e50;");
    topBar->addWidget(titleLbl);
    topBar->addStretch();

    QLabel* filterLbl = new QLabel("类型过滤:", this);
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({"全部","灯光","空调","窗帘","摄像头","传感器"});
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeviceControlWidget::onRefresh);

    QPushButton* refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setStyleSheet("background:#3498db;color:white;padding:5px 12px;border-radius:4px;");
    connect(refreshBtn, &QPushButton::clicked, this, &DeviceControlWidget::onRefresh);

    topBar->addWidget(filterLbl);
    topBar->addWidget(m_filterCombo);
    topBar->addWidget(refreshBtn);
    outerVl->addLayout(topBar);

    // 分割布局：左边设备列表，右边控制面板
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // 左：设备列表
    QWidget* leftPanel = new QWidget(splitter);
    QVBoxLayout* leftVl = new QVBoxLayout(leftPanel);
    leftVl->setContentsMargins(0,0,0,0);
    QLabel* listTitle = new QLabel("设备列表", leftPanel);
    listTitle->setStyleSheet("font-weight:bold;color:#555;");
    leftVl->addWidget(listTitle);
    m_deviceList = new QListWidget(leftPanel);
    m_deviceList->setStyleSheet("QListWidget::item{padding:8px;border-bottom:1px solid #ecf0f1;}"
                                "QListWidget::item:selected{background:#d6eaf8;}");
    connect(m_deviceList, &QListWidget::currentRowChanged,
            this, &DeviceControlWidget::onDeviceSelected);
    leftVl->addWidget(m_deviceList);

    // 右：控制面板
    m_ctrlPanel = new QWidget(splitter);
    QVBoxLayout* ctrlVl = new QVBoxLayout(m_ctrlPanel);

    QGroupBox* infoBox = new QGroupBox("设备信息", m_ctrlPanel);
    QVBoxLayout* infoVl = new QVBoxLayout(infoBox);
    m_nameLbl   = new QLabel("—", infoBox);
    m_typeLbl   = new QLabel("—", infoBox);
    m_statusLbl = new QLabel("—", infoBox);
    m_nameLbl->setStyleSheet("font-size:15px;font-weight:bold;");
    infoVl->addWidget(m_nameLbl);
    infoVl->addWidget(m_typeLbl);
    infoVl->addWidget(m_statusLbl);
    ctrlVl->addWidget(infoBox);

    QGroupBox* ctrlBox = new QGroupBox("控制操作", m_ctrlPanel);
    QVBoxLayout* ctrlBoxVl = new QVBoxLayout(ctrlBox);

    m_toggleBtn = new QPushButton("开启设备", ctrlBox);
    m_toggleBtn->setFixedHeight(40);
    m_toggleBtn->setStyleSheet("QPushButton{background:#27ae60;color:white;border-radius:6px;font-size:14px;}"
                               "QPushButton:hover{background:#1e8449;}");
    m_toggleBtn->setEnabled(false);
    connect(m_toggleBtn, &QPushButton::clicked, this, &DeviceControlWidget::onToggleDevice);
    ctrlBoxVl->addWidget(m_toggleBtn);

    // 参数调节（亮度/温度/开合度）
    QLabel* paramTitle = new QLabel("参数调节:", ctrlBox);
    m_paramLbl = new QLabel("值: 0", ctrlBox);
    m_paramSlider = new QSlider(Qt::Horizontal, ctrlBox);
    m_paramSlider->setRange(0, 100);
    m_paramSlider->setValue(50);
    m_paramSlider->setEnabled(false);
    connect(m_paramSlider, &QSlider::valueChanged, [this](int v){
        m_paramLbl->setText(QString("值: %1").arg(v));
    });
    ctrlBoxVl->addWidget(paramTitle);
    ctrlBoxVl->addWidget(m_paramSlider);
    ctrlBoxVl->addWidget(m_paramLbl);

    QPushButton* applyBtn = new QPushButton("应用参数", ctrlBox);
    applyBtn->setStyleSheet("background:#e67e22;color:white;padding:6px;border-radius:4px;");
    connect(applyBtn, &QPushButton::clicked, this, &DeviceControlWidget::onSetParam);
    ctrlBoxVl->addWidget(applyBtn);

    ctrlVl->addWidget(ctrlBox);
    ctrlVl->addStretch();

    splitter->addWidget(leftPanel);
    splitter->addWidget(m_ctrlPanel);
    splitter->setSizes({260, 500});

    outerVl->addWidget(splitter);
}

void DeviceControlWidget::loadDevices() {
    m_deviceList->clear();
    QString filter = m_filterCombo->currentIndex() == 0 ? "" : m_filterCombo->currentText();
    auto devices = DatabaseManager::instance()->getDevices();
    for (const auto& d : devices) {
        if (!filter.isEmpty() && d["type"].toString() != filter) continue;
        QString status = d["status"].toString();
        QString icon = (status == "online") ? "🟢" : "🔴";
        QListWidgetItem* item = new QListWidgetItem(
            QString("%1 %2 [%3] - %4").arg(icon, d["name"].toString(),
                                            d["type"].toString(), d["grp"].toString()));
        item->setData(Qt::UserRole, d["id"]);
        m_deviceList->addItem(item);
    }
}

void DeviceControlWidget::onDeviceSelected(int row) {
    if (row < 0) return;
    QListWidgetItem* item = m_deviceList->item(row);
    if (!item) return;
    m_selectedDeviceId = item->data(Qt::UserRole).toInt();
    auto device = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    updateControlPanel(device);
}

void DeviceControlWidget::updateControlPanel(const QMap<QString,QVariant>& device) {
    if (device.isEmpty()) return;
    m_nameLbl->setText("设备名: " + device["name"].toString());
    m_typeLbl->setText("类型: "   + device["type"].toString() +
                       "  分组: "  + device["grp"].toString());
    QString status = device["status"].toString();
    bool isOnline = (status == "online");
    m_statusLbl->setText(QString("状态: ") + (isOnline ? "🟢 在线" : "🔴 离线"));
    m_toggleBtn->setText(isOnline ? "关闭设备" : "开启设备");
    m_toggleBtn->setStyleSheet(isOnline
        ? "QPushButton{background:#e74c3c;color:white;border-radius:6px;font-size:14px;}"
          "QPushButton:hover{background:#c0392b;}"
        : "QPushButton{background:#27ae60;color:white;border-radius:6px;font-size:14px;}"
          "QPushButton:hover{background:#1e8449;}");
    m_toggleBtn->setEnabled(true);
    m_paramSlider->setEnabled(true);
}

void DeviceControlWidget::onToggleDevice() {
    if (m_selectedDeviceId < 0) return;
    auto device = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    bool isOnline = (device["status"].toString() == "online");
    QString newStatus = isOnline ? "offline" : "online";
    QString action    = isOnline ? "关闭设备" : "开启设备";
    DatabaseManager::instance()->updateDeviceStatus(m_selectedDeviceId, newStatus, "");
    DatabaseManager::instance()->addOperationLog(m_username, device["name"].toString(), action, "success");
    loadDevices();
    auto updated = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    updateControlPanel(updated);
    emit deviceChanged();
}

void DeviceControlWidget::onSetParam() {
    if (m_selectedDeviceId < 0) return;
    auto device = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    int val = m_paramSlider->value();
    QString type = device["type"].toString();
    QString paramStr;
    if (type == "空调")  paramStr = QString("温度:%1℃").arg(16 + val * 14 / 100);
    else if (type == "灯光")  paramStr = QString("亮度:%1%").arg(val);
    else if (type == "窗帘")  paramStr = QString("开合度:%1%").arg(val);
    else                      paramStr = QString("参数:%1").arg(val);
    DatabaseManager::instance()->updateDeviceStatus(m_selectedDeviceId, "online", paramStr);
    DatabaseManager::instance()->addOperationLog(m_username, device["name"].toString(),
                                                  "设置参数:" + paramStr, "success");
    QMessageBox::information(this, "成功", "参数设置成功: " + paramStr);
    emit deviceChanged();
}

void DeviceControlWidget::onRefresh() {
    loadDevices();
}

void DeviceControlWidget::sendCommand(int deviceId, const QString& cmd, const QString& params) {
    // 模拟网络通信（实际项目中通过 QTcpSocket 发送指令）
    Q_UNUSED(deviceId) Q_UNUSED(cmd) Q_UNUSED(params)
}
