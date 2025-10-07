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

    // ҳ������/�г�ʱ������ѡ��
    virtual void onEnter() {}
    virtual void onLeave() {}

    // ��ҳ����Ҫ��ʾ�Ĳ˵�
    virtual QList<MenuSpec> menus() const { return {}; }

signals:
    void requestSwitchTo(const QString& pageId); // ���������л�����̨
};
