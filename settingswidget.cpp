#include "settingswidget.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTcpSocket>

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
    loadDevices();
}

void SettingsWidget::setupUI() {
    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setContentsMargins(12,12,12,12);

    QLabel* title = new QLabel("系统与设备管理", this);
    title->setStyleSheet("font-size:16px;font-weight:bold;color:#2c3e50;");
    vl->addWidget(title);

    QTabWidget* tabs = new QTabWidget(this);

    // ─── Tab1: 设备管理 ───
    QWidget* devTab = new QWidget(tabs);
    QVBoxLayout* devVl = new QVBoxLayout(devTab);

    m_deviceTable = new QTableWidget(0, 7, devTab);
    m_deviceTable->setHorizontalHeaderLabels({"ID","名称","类型","分组","IP","端口","状态"});
    m_deviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_deviceTable->setAlternatingRowColors(true);
    m_deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    devVl->addWidget(m_deviceTable);

    QHBoxLayout* devBtnRow = new QHBoxLayout();
    auto makeBtn = [&](const QString& text, const char* slot, const QString& color) {
        QPushButton* btn = new QPushButton(text, devTab);
        btn->setStyleSheet(QString("QPushButton{background:%1;color:white;border-radius:4px;padding:5px 12px;}"
                                   "QPushButton:hover{opacity:0.85;}").arg(color));
        connect(btn, SIGNAL(clicked()), this, slot);
        devBtnRow->addWidget(btn);
    };
    makeBtn("➕ 添加设备",    SLOT(onAddDevice()),    "#27ae60");
    makeBtn("✏️ 修改设备",    SLOT(onEditDevice()),   "#e67e22");
    makeBtn("🗑 删除设备",    SLOT(onDeleteDevice()), "#e74c3c");
    makeBtn("🔌 测试连接",    SLOT(onTestConnection()),"#3498db");
    devBtnRow->addStretch();
    devVl->addLayout(devBtnRow);
    tabs->addTab(devTab, "设备管理");

    // ─── Tab2: 系统设置 ───
    QWidget* sysTab = new QWidget(tabs);
    QFormLayout* sysForm = new QFormLayout(sysTab);
    sysForm->setSpacing(14);
    sysForm->setContentsMargins(20,20,20,20);

    m_themeCombo = new QComboBox(sysTab);
    m_themeCombo->addItems({"默认主题","深色主题","浅蓝主题"});
    m_themeCombo->setCurrentText(DatabaseManager::instance()->getSetting("theme","默认主题"));
    sysForm->addRow("界面主题:", m_themeCombo);

    m_refreshSpin = new QSpinBox(sysTab);
    m_refreshSpin->setRange(1, 60); m_refreshSpin->setSuffix(" 秒");
    m_refreshSpin->setValue(DatabaseManager::instance()->getSetting("refresh_interval","5").toInt());
    sysForm->addRow("数据刷新间隔:", m_refreshSpin);

    m_dbPathEdit = new QLineEdit(DatabaseManager::instance()->getSetting("db_path","smart_home.db"), sysTab);
    sysForm->addRow("数据库路径:", m_dbPathEdit);

    QPushButton* saveBtn = new QPushButton("💾 保存设置", sysTab);
    saveBtn->setStyleSheet("background:#2c3e50;color:white;padding:8px 20px;border-radius:4px;font-size:14px;");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsWidget::onSaveSettings);
    sysForm->addRow(saveBtn);
    tabs->addTab(sysTab, "系统设置");

    vl->addWidget(tabs);
}

void SettingsWidget::loadDevices() {
    auto devices = DatabaseManager::instance()->getDevices();
    m_deviceTable->setRowCount(0);
    for (const auto& d : devices) {
        int row = m_deviceTable->rowCount();
        m_deviceTable->insertRow(row);
        m_deviceTable->setItem(row, 0, new QTableWidgetItem(d["id"].toString()));
        m_deviceTable->setItem(row, 1, new QTableWidgetItem(d["name"].toString()));
        m_deviceTable->setItem(row, 2, new QTableWidgetItem(d["type"].toString()));
        m_deviceTable->setItem(row, 3, new QTableWidgetItem(d["grp"].toString()));
        m_deviceTable->setItem(row, 4, new QTableWidgetItem(d["ip"].toString()));
        m_deviceTable->setItem(row, 5, new QTableWidgetItem(d["port"].toString()));
        QString status = d["status"].toString();
        QTableWidgetItem* si = new QTableWidgetItem(status == "online" ? "🟢 在线" : "🔴 离线");
        si->setForeground(status == "online" ? Qt::darkGreen : Qt::red);
        m_deviceTable->setItem(row, 6, si);
    }
}

void SettingsWidget::onAddDevice() {
    QDialog dlg(this); dlg.setWindowTitle("添加设备"); dlg.setFixedWidth(340);
    QFormLayout* form = new QFormLayout(&dlg);
    QLineEdit* nameEdit  = new QLineEdit(&dlg);
    QComboBox* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"灯光","空调","窗帘","摄像头","传感器","其他"});
    QComboBox* grpCombo  = new QComboBox(&dlg);
    grpCombo->addItems({"客厅","卧室","厨房","卫生间","门口","其他"});
    grpCombo->setEditable(true);
    QLineEdit* ipEdit    = new QLineEdit("127.0.0.1", &dlg);
    QSpinBox*  portSpin  = new QSpinBox(&dlg);
    portSpin->setRange(1024, 65535); portSpin->setValue(8080);
    form->addRow("设备名称:", nameEdit);
    form->addRow("设备类型:", typeCombo);
    form->addRow("所在分组:", grpCombo);
    form->addRow("IP地址:",   ipEdit);
    form->addRow("端口号:",   portSpin);
    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dlg);
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, [&]{
        if (nameEdit->text().isEmpty()) { QMessageBox::warning(&dlg,"错误","设备名称不能为空"); return; }
        DatabaseManager::instance()->addDevice(nameEdit->text(), typeCombo->currentText(),
                                                grpCombo->currentText(), ipEdit->text(), portSpin->value());
        dlg.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) loadDevices();
}

void SettingsWidget::onEditDevice() {
    int row = m_deviceTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"提示","请先选择一个设备"); return; }
    int id = m_deviceTable->item(row,0)->text().toInt();
    auto dev = DatabaseManager::instance()->getDeviceById(id);

    QDialog dlg(this); dlg.setWindowTitle("修改设备"); dlg.setFixedWidth(340);
    QFormLayout* form = new QFormLayout(&dlg);
    QLineEdit* nameEdit = new QLineEdit(dev["name"].toString(), &dlg);
    QComboBox* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"灯光","空调","窗帘","摄像头","传感器","其他"});
    typeCombo->setCurrentText(dev["type"].toString());
    QComboBox* grpCombo = new QComboBox(&dlg);
    grpCombo->addItems({"客厅","卧室","厨房","卫生间","门口","其他"});
    grpCombo->setEditable(true);
    grpCombo->setCurrentText(dev["grp"].toString());
    form->addRow("设备名称:", nameEdit);
    form->addRow("设备类型:", typeCombo);
    form->addRow("所在分组:", grpCombo);
    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dlg);
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, [&]{
        DatabaseManager::instance()->updateDevice(id, nameEdit->text(),
                                                   typeCombo->currentText(), grpCombo->currentText());
        dlg.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted) loadDevices();
}

void SettingsWidget::onDeleteDevice() {
    int row = m_deviceTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"提示","请先选择一个设备"); return; }
    int id = m_deviceTable->item(row,0)->text().toInt();
    QString name = m_deviceTable->item(row,1)->text();
    if (QMessageBox::question(this,"确认",QString("确定删除设备 \"%1\" 吗？").arg(name)) == QMessageBox::Yes) {
        DatabaseManager::instance()->deleteDevice(id);
        loadDevices();
    }
}

void SettingsWidget::onTestConnection() {
    int row = m_deviceTable->currentRow();
    if (row < 0) { QMessageBox::information(this,"提示","请先选择一个设备"); return; }
    int id = m_deviceTable->item(row,0)->text().toInt();
    auto dev = DatabaseManager::instance()->getDeviceById(id);
    QString ip = dev["ip"].toString();
    int port   = dev["port"].toInt();

    // 使用 QTcpSocket 测试连接
    QTcpSocket sock;
    sock.connectToHost(ip, static_cast<quint16>(port));
    if (sock.waitForConnected(2000)) {
        sock.disconnectFromHost();
        DatabaseManager::instance()->updateDeviceStatus(id, "online", "");
        QMessageBox::information(this, "连接成功",
            QString("设备 %1 (%2:%3) 连接成功！").arg(dev["name"].toString(), ip).arg(port));
    } else {
        QMessageBox::warning(this, "连接失败",
            QString("设备 %1 (%2:%3) 无法连接，已标记为离线。\n%4")
                .arg(dev["name"].toString(), ip).arg(port).arg(sock.errorString()));
        DatabaseManager::instance()->updateDeviceStatus(id, "offline", "");
    }
    loadDevices();
}

void SettingsWidget::onSaveSettings() {
    DatabaseManager::instance()->setSetting("theme",            m_themeCombo->currentText());
    DatabaseManager::instance()->setSetting("refresh_interval", QString::number(m_refreshSpin->value()));
    DatabaseManager::instance()->setSetting("db_path",          m_dbPathEdit->text());
    QMessageBox::information(this, "成功", "系统设置已保存");
}
