#pragma once
#include <QWidget>

class HistoryWidget : public QWidget {
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget* parent = nullptr);

private slots:
    void onQuery();
    void onExport();

private:
    void setupUI();
    void loadData();

    class QDateEdit*   m_startDate  = nullptr;
    class QDateEdit*   m_endDate    = nullptr;
    class QComboBox*   m_typeFilter = nullptr;
    class QTableWidget* m_table     = nullptr;
    class QTabWidget*  m_tabs       = nullptr;
    class QTableWidget* m_envTable  = nullptr;
};
