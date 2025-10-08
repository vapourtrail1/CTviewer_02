#pragma once
#include "c_ui/PageBase.h"
#include <QPointer>

class QPushButton;
class QTableWidget;

class DocumentPage : public PageBase {
    Q_OBJECT
public:
    explicit DocumentPage(QWidget* parent = nullptr);
    QList<MenuSpec> menus() const override;

private:
    void buildUi();

    QPointer<QPushButton> btnVisCheck_;
    QPointer<QPushButton> btnPorosity_;
    QPointer<QPushButton> btnMetrology_;
    QPointer<QPushButton> btnMaterial_;
    QPointer<QTableWidget> tableRecent_;
};

