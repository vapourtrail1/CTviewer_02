#include "c_ui/workbenches/StartPage.h"
#include "c_ui/action/ActionIDs.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QTableWidgetItem>

StartPage::StartPage(QWidget* parent) : PageBase(parent) {
    buildUi();
}

QList<MenuSpec> StartPage::menus() const {
    return {
        { QStringLiteral("�ļ�"),   { Act_NewProject, Act_Open, Act_Save, Act_SaveAs, Act_Exit } },
        { QStringLiteral("��ʼ"),   { Act_QuickImport, Act_CTReconstruct, Act_Import, Act_Export } },
        { QStringLiteral("�༭"),   { Act_Undo, Act_Redo } },
        { QStringLiteral("��ѡ��"), { Act_Preferences } }
    };
}

void StartPage::buildUi() {
    setObjectName(QStringLiteral("pageWelcome"));
    setStyleSheet("QWidget#pageWelcome{background-color:#404040;} QLabel{color:#f2f2f2;}");

    auto vl = new QVBoxLayout(this);
    vl->setContentsMargins(18, 18, 18, 18);
    vl->setSpacing(16);

    // �������
    auto banner = new QFrame(this);
    banner->setObjectName("heroBanner");
    banner->setStyleSheet("QFrame#heroBanner{background:#322F30; border-radius:10px;} QFrame#heroBanner QLabel{color:#f9f9f9;}");
    auto bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(20, 16, 20, 16);
    bannerLayout->setSpacing(8);
    auto title = new QLabel(QStringLiteral("��ӭʹ�� CTviewer_demo"), banner);
    title->setStyleSheet("font-size:24px;font-weight:700;");
    bannerLayout->addWidget(title);
    auto subtitle = new QLabel(QStringLiteral("���������Ŀ����ͨ���·�ģ����ٿ�ʼ���Ĺ�ҵ CT �������̡�"), banner);
    subtitle->setStyleSheet("font-size:14px;color:#bbbbbb;");
    subtitle->setWordWrap(true);
    bannerLayout->addWidget(subtitle);
    vl->addWidget(banner);

    // ������ʾ
    auto tipsFrame = new QFrame(this);
    tipsFrame->setObjectName("tipsFrame");
    tipsFrame->setStyleSheet("QFrame#tipsFrame{background:#322F30;border-radius:10px;} QFrame#tipsFrame QLabel{color:#d8d8d8;}");
    auto tipsLayout = new QVBoxLayout(tipsFrame);
    tipsLayout->setContentsMargins(20, 18, 20, 18);
    tipsLayout->setSpacing(10);
    auto tipsTitle = new QLabel(QStringLiteral("������ʾ"), tipsFrame);
    tipsTitle->setStyleSheet("font-size:16px;font-weight:600;");
    tipsLayout->addWidget(tipsTitle);
    auto tips = new QLabel(QStringLiteral("1.��ֱ�ӵ��� DICOM��TIFF��RAW �ȳ�����ҵ CT ���ݡ�\n2.������ѵ���ϣ��ɷ��ʰ��������Ի�ȡ���½̡̳�"), tipsFrame);
    tips->setWordWrap(true);
    tips->setStyleSheet("font-size:13px;line-height:20px;");
    tipsLayout->addWidget(tips);
    tipsLayout->addStretch();
    vl->addWidget(tipsFrame);

    // �������
    auto moduleFrame = new QFrame(this);
    moduleFrame->setObjectName("moduleFrame");
    moduleFrame->setStyleSheet(
        "QFrame#moduleFrame{background:#322F30;border-radius:10px;}"
        "QFrame#moduleFrame QPushButton{background:#2C2C2C;border-radius:8px;border:1px solid #333;color:#f5f5f5;font-size:16px;padding:18px 12px;}"
        "QFrame#moduleFrame QPushButton:hover{background:#2C2C2C;border-color:#4d6fff;}"
        "QFrame#moduleFrame QLabel{color:#f5f5f5;}"
    );
    auto moduleLayout = new QVBoxLayout(moduleFrame);
    moduleLayout->setContentsMargins(20, 18, 20, 18);
    moduleLayout->setSpacing(12);
    auto moduleTitle = new QLabel(QStringLiteral("ѡ�����ʺ����������̵ġ���ʼ��ѡ�"), moduleFrame);
    moduleTitle->setStyleSheet("font-size:16px;font-weight:600;");
    moduleLayout->addWidget(moduleTitle);

    auto grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    btnVisCheck_ = new QPushButton(QStringLiteral("�Ӿ����"), moduleFrame);
    btnPorosity_ = new QPushButton(QStringLiteral("��϶��"), moduleFrame);
    btnMetrology_ = new QPushButton(QStringLiteral("����"), moduleFrame);
    btnMaterial_ = new QPushButton(QStringLiteral("����"), moduleFrame);
    for (auto* b : { btnVisCheck_.data(), btnPorosity_.data(), btnMetrology_.data(), btnMaterial_.data() })
        b->setMinimumSize(160, 70);
    grid->addWidget(btnVisCheck_, 0, 0);
    grid->addWidget(btnPorosity_, 0, 1);
    grid->addWidget(btnMetrology_, 0, 2);
    grid->addWidget(btnMaterial_, 0, 3);
    moduleLayout->addLayout(grid);
    vl->addWidget(moduleFrame);

    // �����Ŀ
    auto recentFrame = new QFrame(this);
    recentFrame->setObjectName("recentFrame");
    recentFrame->setStyleSheet(
        "QFrame#recentFrame{background:#322F30;border-radius:10px;}"
        "QFrame#recentFrame QLabel{color:#f5f5f5;}"
        "QFrame#recentFrame QHeaderView::section{background:#2c2c2c;color:#f0f0f0;border:0;}"
        "QFrame#recentFrame QTableWidget{background:transparent;border:0;color:#f5f5f5;}"
        "QFrame#recentFrame QTableWidget::item:selected{background-color:#3d65f5;}"
    );
    auto recentLayout = new QVBoxLayout(recentFrame);
    recentLayout->setContentsMargins(20, 18, 20, 18);
    recentLayout->setSpacing(12);
    auto recentTitle = new QLabel(QStringLiteral("�����Ŀ"), recentFrame);
    recentTitle->setStyleSheet("font-size:16px;font-weight:600;");
    recentLayout->addWidget(recentTitle);

    tableRecent_ = new QTableWidget(0, 3, recentFrame);
    tableRecent_->setHorizontalHeaderLabels({ QStringLiteral("����"), QStringLiteral("λ��"), QStringLiteral("�ϴδ�") });
    tableRecent_->horizontalHeader()->setStretchLastSection(true);
    tableRecent_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tableRecent_->verticalHeader()->setVisible(false);
    tableRecent_->setShowGrid(false);
    tableRecent_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableRecent_->setSelectionMode(QAbstractItemView::SingleSelection);
    tableRecent_->setAlternatingRowColors(true);
    tableRecent_->setStyleSheet("QTableWidget{alternate-background-color:#2C2C2C;} QTableWidget QTableCornerButton::section{background:#2c2c2c;}");
    struct RecentItem { QString name; QString path; QString time; };
    const QList<RecentItem> recents = {
        { QStringLiteral("����������.vgl"), QStringLiteral("D:/Projects/CT/EngineBlock"), QStringLiteral("���� 09:24") },
        { QStringLiteral("������.vgl"),     QStringLiteral("D:/Projects/CT/GearBox"),     QStringLiteral("���� 17:42") },
        { QStringLiteral("ҶƬɨ��.vgi"),   QStringLiteral("E:/Scan/Blade"),              QStringLiteral("2024-05-12") },
        { QStringLiteral("��������.raw"),   QStringLiteral("E:/Lab/Materials"),           QStringLiteral("2024-04-28") }
    };
    for (const auto& item : recents) {
        const int row = tableRecent_->rowCount();
        tableRecent_->insertRow(row);
        tableRecent_->setItem(row, 0, new QTableWidgetItem(item.name));
        tableRecent_->setItem(row, 1, new QTableWidgetItem(item.path));
        tableRecent_->setItem(row, 2, new QTableWidgetItem(item.time));
    }
    tableRecent_->setMinimumHeight(220);
    recentLayout->addWidget(tableRecent_);
    vl->addWidget(recentFrame);
    vl->addStretch();

    // ��ת����CT�ؽ�������̨
    auto goReconstruct = [this]() { emit requestSwitchTo("volume"); };
    connect(btnVisCheck_, &QPushButton::clicked, this, goReconstruct);
    connect(btnPorosity_, &QPushButton::clicked, this, goReconstruct);
    connect(btnMetrology_, &QPushButton::clicked, this, goReconstruct);
    connect(btnMaterial_, &QPushButton::clicked, this, goReconstruct);
    connect(tableRecent_, &QTableWidget::itemDoubleClicked, this, [goReconstruct](auto*) { goReconstruct(); });
}
