#include "settingswidget.h"
#include "databasemanager.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void SettingsWidget::setupUI() {
    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setContentsMargins(12, 12, 12, 12);

    QLabel* title = new QLabel("系统设置", this);
    title->setStyleSheet("font-size:16px;font-weight:bold;color:#2c3e50;");
    vl->addWidget(title);

    QWidget* panel = new QWidget(this);
    QFormLayout* sysForm = new QFormLayout(panel);
    sysForm->setSpacing(14);
    sysForm->setContentsMargins(20, 20, 20, 20);

    m_themeCombo = new QComboBox(panel);
    m_themeCombo->addItem("默认主题");
    m_themeCombo->addItem("深色主题");
    m_themeCombo->addItem("浅蓝主题");
    m_themeCombo->setCurrentText(DatabaseManager::instance()->getSetting("theme", "默认主题"));
    sysForm->addRow("界面主题:", m_themeCombo);

    m_refreshSpin = new QSpinBox(panel);
    m_refreshSpin->setRange(1, 60);
    m_refreshSpin->setSuffix(" 秒");
    m_refreshSpin->setValue(DatabaseManager::instance()->getSetting("refresh_interval", "5").toInt());
    sysForm->addRow("数据刷新间隔:", m_refreshSpin);

    m_dbPathEdit = new QLineEdit(DatabaseManager::instance()->getSetting("db_path", "smart_home.db"), panel);
    sysForm->addRow("数据库路径:", m_dbPathEdit);

    QPushButton* saveBtn = new QPushButton("保存设置", panel);
    saveBtn->setStyleSheet("background:#2c3e50;color:white;padding:8px 20px;border-radius:4px;font-size:14px;");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsWidget::onSaveSettings);
    sysForm->addRow(saveBtn);

    vl->addWidget(panel);
    vl->addStretch();
}

void SettingsWidget::onSaveSettings() {
    DatabaseManager::instance()->setSetting("theme", m_themeCombo->currentText());
    DatabaseManager::instance()->setSetting("refresh_interval", QString::number(m_refreshSpin->value()));
    DatabaseManager::instance()->setSetting("db_path", m_dbPathEdit->text());
    QMessageBox::information(this, "成功", "系统设置已保存");
}
