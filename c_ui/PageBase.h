#pragma once
#include <QWidget>
#include <QString>
#include <QList>

struct MenuSpec {
    QString menuTitle;        
    QList<int> actionIDs;     
};

class PageBase : public QWidget {
    Q_OBJECT
public:
    explicit PageBase(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~PageBase() = default;

    // 页面切入/切出时机（可选）
    virtual void onEnter() {}
    virtual void onLeave() {}

    // 本页面需要显示的菜单
    virtual QList<MenuSpec> menus() const { return {}; }

signals:
    void requestSwitchTo(const QString& pageId); // 让主窗体切换工作台
};
