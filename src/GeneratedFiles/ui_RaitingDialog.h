/********************************************************************************
** Form generated from reading UI file 'RaitingDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RAITINGDIALOG_H
#define UI_RAITINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RaitingDialog
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *titleBar;
    QHBoxLayout *horizontalLayout;
    QLabel *tbMenu;
    QLabel *LTitle;
    QPushButton *pbClose;
    QWidget *m_centralWidget;
    QGridLayout *mainLayout;
    QDialogButtonBox *buttonBox;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label;
    QLabel *label_5;
    QLabel *label_4;
    QCheckBox *checkBox;
    QPlainTextEdit *plainTextEdit;

    void setupUi(QWidget *RaitingDialog)
    {
        if (RaitingDialog->objectName().isEmpty())
            RaitingDialog->setObjectName(QStringLiteral("RaitingDialog"));
        RaitingDialog->resize(377, 440);
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/app.ico"), QSize(), QIcon::Normal, QIcon::Off);
        RaitingDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(RaitingDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(1, 1, 1, 0);
        titleBar = new QWidget(RaitingDialog);
        titleBar->setObjectName(QStringLiteral("titleBar"));
        titleBar->setMinimumSize(QSize(0, 23));
        titleBar->setMaximumSize(QSize(16777215, 23));
        titleBar->setStyleSheet(QStringLiteral(""));
        horizontalLayout = new QHBoxLayout(titleBar);
        horizontalLayout->setSpacing(4);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(5, 0, 1, 0);
        tbMenu = new QLabel(titleBar);
        tbMenu->setObjectName(QStringLiteral("tbMenu"));
        tbMenu->setMaximumSize(QSize(15, 15));
        tbMenu->setPixmap(QPixmap(QString::fromUtf8(":/icons/app.ico")));
        tbMenu->setScaledContents(true);

        horizontalLayout->addWidget(tbMenu);

        LTitle = new QLabel(titleBar);
        LTitle->setObjectName(QStringLiteral("LTitle"));
        LTitle->setMinimumSize(QSize(100, 22));
        LTitle->setMaximumSize(QSize(16777215, 22));
        LTitle->setStyleSheet(QStringLiteral(""));
        LTitle->setTextFormat(Qt::AutoText);

        horizontalLayout->addWidget(LTitle);

        pbClose = new QPushButton(titleBar);
        pbClose->setObjectName(QStringLiteral("pbClose"));
        pbClose->setMinimumSize(QSize(22, 22));
        pbClose->setMaximumSize(QSize(22, 22));
        pbClose->setIconSize(QSize(22, 22));
        pbClose->setFlat(true);

        horizontalLayout->addWidget(pbClose);


        verticalLayout->addWidget(titleBar);

        m_centralWidget = new QWidget(RaitingDialog);
        m_centralWidget->setObjectName(QStringLiteral("m_centralWidget"));
        mainLayout = new QGridLayout(m_centralWidget);
        mainLayout->setObjectName(QStringLiteral("mainLayout"));
        buttonBox = new QDialogButtonBox(m_centralWidget);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        mainLayout->addWidget(buttonBox, 7, 0, 1, 2);

        label_2 = new QLabel(m_centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        mainLayout->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(m_centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        mainLayout->addWidget(label_3, 2, 0, 1, 1);

        label = new QLabel(m_centralWidget);
        label->setObjectName(QStringLiteral("label"));

        mainLayout->addWidget(label, 0, 0, 1, 1);

        label_5 = new QLabel(m_centralWidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        mainLayout->addWidget(label_5, 4, 0, 1, 1);

        label_4 = new QLabel(m_centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        mainLayout->addWidget(label_4, 3, 0, 1, 1);

        checkBox = new QCheckBox(m_centralWidget);
        checkBox->setObjectName(QStringLiteral("checkBox"));

        mainLayout->addWidget(checkBox, 6, 1, 1, 1);

        plainTextEdit = new QPlainTextEdit(m_centralWidget);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

        mainLayout->addWidget(plainTextEdit, 5, 0, 1, 2);

        mainLayout->setRowStretch(0, 2);
        mainLayout->setRowStretch(1, 2);
        mainLayout->setRowStretch(2, 2);
        mainLayout->setRowStretch(3, 2);

        verticalLayout->addWidget(m_centralWidget);


        retranslateUi(RaitingDialog);

        QMetaObject::connectSlotsByName(RaitingDialog);
    } // setupUi

    void retranslateUi(QWidget *RaitingDialog)
    {
        RaitingDialog->setWindowTitle(QApplication::translate("RaitingDialog", "RAITE_CLIENT", Q_NULLPTR));
        tbMenu->setText(QString());
        LTitle->setText(QApplication::translate("RaitingDialog", "RAITE_CLIENT", Q_NULLPTR));
        label_2->setText(QApplication::translate("RaitingDialog", "DESIGN", Q_NULLPTR));
        label_3->setText(QApplication::translate("RaitingDialog", "USABLILITY", Q_NULLPTR));
        label->setText(QApplication::translate("RaitingDialog", "SPEED", Q_NULLPTR));
        label_5->setText(QApplication::translate("RaitingDialog", "YOUR_OPINION", Q_NULLPTR));
        label_4->setText(QApplication::translate("RaitingDialog", "POSIBILITIES", Q_NULLPTR));
        checkBox->setText(QApplication::translate("RaitingDialog", "DO_NOT_DISTURB_ME", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class RaitingDialog: public Ui_RaitingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RAITINGDIALOG_H
