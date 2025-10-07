#pragma once
#include <QMainWindow>
#include <QHash>
#include <QPointer>

class QListWidget;
class QStackedWidget;
class QListWidgetItem;
class QLabel;
class QToolButton;
class QDockWidget;
class PageBase;
class ActionManager;

class CTViewer : public QMainWindow {
    Q_OBJECT
public:
    explicit CTViewer(QWidget* parent = nullptr);
    ~CTViewer();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    // —— 标题栏（保留你原来的成员与函数） —— //
    QPointer<QWidget> titleBar_, titleLeftArea_, titleCenterArea_;
    QPointer<QLabel>  titleLabel_;
    QPointer<QToolButton> btnTitleUndo_, btnTitleUndo02_, btnMinimize_, btnMaximize_, btnClose_;
    bool   draggingWindow_ = false;
    QPoint dragOffset_;
    void buildTitleBar();
    void updateMaximizeButtonIcon();

    // —— 框架/导航 —— //
    QPointer<QStackedWidget> stack_;
    QPointer<QDockWidget>    dockNav_;
    QPointer<QListWidget>    listNav_;

    // —— 页面/菜单 —— //
    QHash<QString, PageBase*> pages_;   // pageId -> page
    ActionManager* actions_ = nullptr;
    void buildCentral();
    void buildNavDock();
    void registerUiPages();             // ⭐ 新增：把所有 Page 注册到 stack_ 和 nav list
    void switchTo(const QString& id);   // ⭐ 新增：切换页面 + 重建菜单
    void rebuildMenusFor(PageBase* page);

    void wireSignals(); // 仅保留壳子的信号（最小化）
    void setDefaults();

    // 你之前的 use VTK 保留（在 VolumePage 里用了占位）
    void allDocumentsControl();
};
