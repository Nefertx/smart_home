#include "devicecontrolwidget.h"
#include "databasemanager.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTcpSocket>
#include <QVBoxLayout>

DeviceControlWidget::DeviceControlWidget(const QString& username, QWidget* parent)
    : QWidget(parent), m_username(username) {
    setupUI();
    loadDevices();
}

QMap<QString, QString> DeviceControlWidget::parseParams(const QString& params) {
    QMap<QString, QString> kv;
    const QStringList pairs = params.split(';', Qt::SkipEmptyParts);
    for (const QString& p : pairs) {
        const int idx = p.indexOf('=');
        if (idx <= 0) {
            continue;
        }
        kv[p.left(idx).trimmed()] = p.mid(idx + 1).trimmed();
    }
    return kv;
}

QString DeviceControlWidget::buildParams(const QMap<QString, QString>& kv) {
    QStringList parts;
    for (auto it = kv.constBegin(); it != kv.constEnd(); ++it) {
        parts << QString("%1=%2").arg(it.key(), it.value());
    }
    return parts.join(';');
}

void DeviceControlWidget::setupUI() {
    QVBoxLayout* outerVl = new QVBoxLayout(this);
    outerVl->setContentsMargins(12, 12, 12, 12);

    QHBoxLayout* topBar = new QHBoxLayout();
    QLabel* titleLbl = new QLabel("设备控制与管理", this);
    titleLbl->setStyleSheet("font-size:16px;font-weight:bold;color:#2c3e50;");
    topBar->addWidget(titleLbl);
    topBar->addStretch();

    QLabel* filterLbl = new QLabel("类型过滤:", this);
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({"全部", "灯光", "空调", "窗帘", "摄像头"});
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeviceControlWidget::onRefresh);

    auto makeBtn = [this, topBar](const QString& text, const QString& color, auto slot) {
        QPushButton* btn = new QPushButton(text, this);
        btn->setStyleSheet(QString("QPushButton{background:%1;color:white;padding:5px 12px;border-radius:4px;}"
                                   "QPushButton:hover{opacity:0.85;}")
                               .arg(color));
        connect(btn, &QPushButton::clicked, this, slot);
        topBar->addWidget(btn);
    };

    topBar->addWidget(filterLbl);
    topBar->addWidget(m_filterCombo);
    makeBtn("刷新", "#3498db", &DeviceControlWidget::onRefresh);
    makeBtn("添加设备", "#27ae60", &DeviceControlWidget::onAddDevice);
    makeBtn("编辑设备", "#e67e22", &DeviceControlWidget::onEditDevice);
    makeBtn("删除设备", "#e74c3c", &DeviceControlWidget::onDeleteDevice);
    outerVl->addLayout(topBar);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_cardHost = new QWidget(scroll);
    m_cardGrid = new QGridLayout(m_cardHost);
    m_cardGrid->setContentsMargins(0, 4, 0, 0);
    m_cardGrid->setHorizontalSpacing(12);
    m_cardGrid->setVerticalSpacing(12);
    scroll->setWidget(m_cardHost);

    outerVl->addWidget(scroll);
}

void DeviceControlWidget::addDeviceCard(const QMap<QString, QVariant>& device, int index) {
    const int id = device.value("id").toInt();
    const bool isOnline = device.value("status").toString() == "online";
    const QString title = QString("%1\n[%2] %3")
                              .arg(device.value("name").toString(),
                                   device.value("type").toString(),
                                   device.value("grp").toString());
    const QString param = device.value("params").toString().isEmpty() ? "参数: -"
                                                                        : "参数: " + device.value("params").toString();
    const QString status = isOnline ? "状态: 在线" : "状态: 离线（点击尝试配对）";

    QPushButton* card = new QPushButton(title + "\n" + status + "\n" + param, m_cardHost);
    card->setMinimumHeight(130);
    card->setCheckable(true);
    card->setChecked(id == m_selectedDeviceId);
    card->setStyleSheet(isOnline
                            ? "QPushButton{background:#ffffff;border:1px solid #bdc3c7;border-radius:10px;"
                              "padding:10px;text-align:left;color:#2c3e50;}"
                              "QPushButton:hover{border-color:#3498db;background:#f5fbff;}"
                              "QPushButton:checked{border:2px solid #1abc9c;}"
                            : "QPushButton{background:#e5e7eb;border:1px solid #9ca3af;border-radius:10px;"
                              "padding:10px;text-align:left;color:#6b7280;}"
                              "QPushButton:hover{background:#d1d5db;}"
                              "QPushButton:checked{border:2px solid #6b7280;}");

    connect(card, &QPushButton::clicked, this, [this, id] { onCardClicked(id); });

    const int colCount = 3;
    const int row = index / colCount;
    const int col = index % colCount;
    m_cardGrid->addWidget(card, row, col);
}

void DeviceControlWidget::loadDevices() {
    while (QLayoutItem* item = m_cardGrid->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    m_devices = DatabaseManager::instance()->getDevices();
    const QString filter = (m_filterCombo->currentIndex() == 0) ? QString() : m_filterCombo->currentText();

    int visibleIndex = 0;
    for (const auto& d : m_devices) {
        if (!filter.isEmpty() && d.value("type").toString() != filter) {
            continue;
        }
        addDeviceCard(d, visibleIndex++);
    }

    if (visibleIndex == 0) {
        QLabel* empty = new QLabel("暂无设备，请先添加设备。", m_cardHost);
        empty->setStyleSheet("color:#6b7280;padding:8px;");
        m_cardGrid->addWidget(empty, 0, 0);
    }
}

bool DeviceControlWidget::tryPairDevice(int deviceId, const QMap<QString, QVariant>& device) {
    const QString ip = device.value("ip").toString();
    const int port = device.value("port").toInt();

    QTcpSocket sock;
    sock.connectToHost(ip, static_cast<quint16>(port));
    if (!sock.waitForConnected(1500)) {
        QMessageBox::warning(this, "配对失败",
                             QString("设备 %1 配对失败：%2")
                                 .arg(device.value("name").toString(), sock.errorString()));
        DatabaseManager::instance()->updateDeviceStatus(deviceId, "offline", device.value("params").toString());
        return false;
    }

    sock.disconnectFromHost();
    DatabaseManager::instance()->updateDeviceStatus(deviceId, "online", device.value("params").toString());
    QMessageBox::information(this, "配对成功", "设备已上线，可进入控制界面。");
    return true;
}

void DeviceControlWidget::showDeviceControlDialog(const QMap<QString, QVariant>& device) {
    const int id = device.value("id").toInt();
    const QString name = device.value("name").toString();
    const QString type = device.value("type").toString();
    QMap<QString, QString> kv = parseParams(device.value("params").toString());
    if (!kv.contains("power")) {
        kv["power"] = "on";
    }

    QDialog dlg(this);
    dlg.setWindowTitle("设备控制 - " + name);
    dlg.setMinimumWidth(360);
    QVBoxLayout* vl = new QVBoxLayout(&dlg);

    QLabel* basic = new QLabel(QString("设备: %1\n类型: %2\n状态: 在线").arg(name, type), &dlg);
    basic->setStyleSheet("color:#374151;");
    vl->addWidget(basic);

    QGroupBox* box = new QGroupBox("控制项", &dlg);
    QFormLayout* form = new QFormLayout(box);

    QComboBox* power = new QComboBox(box);
    power->addItems({"on", "off"});
    power->setCurrentText(kv.value("power", "on"));
    form->addRow("开关", power);

    QSpinBox* tempSpin = nullptr;
    QComboBox* modeCombo = nullptr;
    QComboBox* fanCombo = nullptr;
    QSpinBox* brightSpin = nullptr;
    QComboBox* colorCombo = nullptr;
    QSpinBox* curtainOpen = nullptr;

    if (type == "空调") {
        modeCombo = new QComboBox(box);
        modeCombo->addItems({"制冷", "制热", "除湿", "送风", "自动"});
        modeCombo->setCurrentText(kv.value("mode", "制冷"));
        form->addRow("模式", modeCombo);

        tempSpin = new QSpinBox(box);
        tempSpin->setRange(16, 30);
        tempSpin->setValue(kv.value("temp", "24").toInt());
        form->addRow("温度(℃)", tempSpin);

        fanCombo = new QComboBox(box);
        fanCombo->addItems({"低", "中", "高", "自动"});
        fanCombo->setCurrentText(kv.value("fan", "中"));
        form->addRow("风力", fanCombo);

        const auto env = DatabaseManager::instance()->getEnvData();
        QString envText = "温度: --  湿度: --";
        if (!env.isEmpty()) {
            const auto latest = env.first();
            envText = QString("温度: %1℃  湿度: %2%")
                          .arg(QString::number(latest.value("temperature").toDouble(), 'f', 1),
                               QString::number(latest.value("humidity").toDouble(), 'f', 1));
        }
        QLabel* sensor = new QLabel(envText, box);
        sensor->setStyleSheet("color:#0f766e;font-weight:600;");
        form->addRow("环境传感", sensor);
    } else if (type == "灯光") {
        const bool advanced = name.contains("客厅");
        if (advanced) {
            brightSpin = new QSpinBox(box);
            brightSpin->setRange(0, 100);
            brightSpin->setValue(kv.value("brightness", "80").toInt());
            form->addRow("亮度(%)", brightSpin);

            colorCombo = new QComboBox(box);
            colorCombo->addItems({"暖白", "冷白", "自然光", "蓝色", "红色"});
            colorCombo->setCurrentText(kv.value("color", "暖白"));
            form->addRow("颜色", colorCombo);
        }
    } else if (type == "窗帘") {
        curtainOpen = new QSpinBox(box);
        curtainOpen->setRange(0, 100);
        curtainOpen->setValue(kv.value("open", "50").toInt());
        form->addRow("开合度(%)", curtainOpen);
    } else if (type == "摄像头") {
        QLabel* readonly = new QLabel("摄像头示例设备仅支持在线/离线与开关。", box);
        readonly->setWordWrap(true);
        readonly->setStyleSheet("color:#6b7280;");
        form->addRow(readonly);
    }

    vl->addWidget(box);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    vl->addWidget(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    kv["power"] = power->currentText();
    if (modeCombo) {
        kv["mode"] = modeCombo->currentText();
    }
    if (tempSpin) {
        kv["temp"] = QString::number(tempSpin->value());
    }
    if (fanCombo) {
        kv["fan"] = fanCombo->currentText();
    }
    if (brightSpin) {
        kv["brightness"] = QString::number(brightSpin->value());
    }
    if (colorCombo) {
        kv["color"] = colorCombo->currentText();
    }
    if (curtainOpen) {
        kv["open"] = QString::number(curtainOpen->value());
    }

    const QString params = buildParams(kv);
    DatabaseManager::instance()->updateDeviceStatus(id, "online", params);
    DatabaseManager::instance()->addOperationLog(m_username, name, "设备控制: " + params, "success");
    emit deviceChanged();
    loadDevices();
}

void DeviceControlWidget::onCardClicked(int deviceId) {
    m_selectedDeviceId = deviceId;
    auto device = DatabaseManager::instance()->getDeviceById(deviceId);
    if (device.isEmpty()) {
        return;
    }

    const bool online = device.value("status").toString() == "online";
    if (!online) {
        if (QMessageBox::question(this, "设备离线",
                                  "该设备当前离线，是否尝试配对上线？") == QMessageBox::Yes) {
            if (tryPairDevice(deviceId, device)) {
                emit deviceChanged();
            }
        }
        loadDevices();
        return;
    }

    showDeviceControlDialog(device);
}

void DeviceControlWidget::onAddDevice() {
    QDialog dlg(this);
    dlg.setWindowTitle("添加设备");
    dlg.setFixedWidth(360);
    QFormLayout* form = new QFormLayout(&dlg);

    QLineEdit* nameEdit = new QLineEdit(&dlg);
    QComboBox* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"灯光", "空调", "窗帘", "摄像头"});
    QComboBox* grpCombo = new QComboBox(&dlg);
    grpCombo->addItems({"客厅", "卧室", "厨房", "门口", "其他"});
    grpCombo->setEditable(true);
    QLineEdit* ipEdit = new QLineEdit("127.0.0.1", &dlg);
    QSpinBox* portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1024, 65535);
    portSpin->setValue(8080);

    form->addRow("设备名称:", nameEdit);
    form->addRow("设备类型:", typeCombo);
    form->addRow("所在分组:", grpCombo);
    form->addRow("IP地址:", ipEdit);
    form->addRow("端口号:", portSpin);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, [&] {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "错误", "设备名称不能为空");
            return;
        }
        if (!DatabaseManager::instance()->addDevice(nameEdit->text().trimmed(), typeCombo->currentText(),
                                                    grpCombo->currentText(), ipEdit->text().trimmed(),
                                                    portSpin->value())) {
            QMessageBox::warning(&dlg, "错误", "添加设备失败");
            return;
        }
        dlg.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        emit deviceChanged();
        loadDevices();
    }
}

void DeviceControlWidget::onEditDevice() {
    if (m_selectedDeviceId < 0) {
        QMessageBox::information(this, "提示", "请先点击一个设备卡片");
        return;
    }

    auto dev = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    if (dev.isEmpty()) {
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("编辑设备");
    dlg.setFixedWidth(360);
    QFormLayout* form = new QFormLayout(&dlg);

    QLineEdit* nameEdit = new QLineEdit(dev.value("name").toString(), &dlg);
    QComboBox* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({"灯光", "空调", "窗帘", "摄像头"});
    typeCombo->setCurrentText(dev.value("type").toString());
    QComboBox* grpCombo = new QComboBox(&dlg);
    grpCombo->addItems({"客厅", "卧室", "厨房", "门口", "其他"});
    grpCombo->setEditable(true);
    grpCombo->setCurrentText(dev.value("grp").toString());

    form->addRow("设备名称:", nameEdit);
    form->addRow("设备类型:", typeCombo);
    form->addRow("所在分组:", grpCombo);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(bb);
    connect(bb, &QDialogButtonBox::accepted, &dlg, [&] {
        if (!DatabaseManager::instance()->updateDevice(m_selectedDeviceId, nameEdit->text().trimmed(),
                                                       typeCombo->currentText(), grpCombo->currentText())) {
            QMessageBox::warning(&dlg, "错误", "修改设备失败");
            return;
        }
        dlg.accept();
    });
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        emit deviceChanged();
        loadDevices();
    }
}

void DeviceControlWidget::onDeleteDevice() {
    if (m_selectedDeviceId < 0) {
        QMessageBox::information(this, "提示", "请先点击一个设备卡片");
        return;
    }

    auto dev = DatabaseManager::instance()->getDeviceById(m_selectedDeviceId);
    const QString name = dev.value("name").toString();
    if (QMessageBox::question(this, "确认删除", QString("确定删除设备 \"%1\" 吗？").arg(name)) != QMessageBox::Yes) {
        return;
    }

    if (!DatabaseManager::instance()->deleteDevice(m_selectedDeviceId)) {
        QMessageBox::warning(this, "错误", "删除设备失败");
        return;
    }

    m_selectedDeviceId = -1;
    emit deviceChanged();
    loadDevices();
}

void DeviceControlWidget::onRefresh() {
    loadDevices();
}
