﻿#include <QDebug>
#include "messagebox.h"
#ifdef Q_WS_WIN
#include "Windows.h"
#endif
void CustomMessageBox::showEvent(QShowEvent* pEvent)
{
#ifndef QT_NO_ACCESSIBILITY
	if (m_icon == QMessageBox::Critical || m_icon == QMessageBox::Warning)
	{
		QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
	}
#endif
	QDialog::showEvent(pEvent);
}

CustomMessageBox::CustomMessageBox(QWidget* /*parent*/) :
	BaseWindow<QDialog> (OnlyCloseButton, NoResize),
	ui(new Ui::CustomMessageBox)
{
	ui->setupUi(this);
}

CustomMessageBox::CustomMessageBox(QMessageBox::Icon icon, const QString& title, const QString& text,
                                   QMessageBox::StandardButtons buttons, QWidget* parent, Qt::WindowFlags /*flags*/) :
	BaseWindow<QDialog>(OnlyCloseButton, NoResize, parent),
	ui(new Ui::CustomMessageBox),
	m_icon(icon)
{
	ui->setupUi(this);
	setupCustomWindow();
	setupWindowIcons();
	ui->icon->setPixmap(standardIcon(icon));
	ui->LTitle->setText(title);
	ui->text->setText(text);

	if (parent != NULL)
	{
#ifdef Q_WS_WIN
		SetWindowPos(effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#endif
		// HACK END
		parent->showNormal();
		parent->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		parent->raise();
		parent->activateWindow();
	}

	if(buttons == QMessageBox::NoButton)
	{
		buttons = QMessageBox::Ok | QMessageBox::Cancel;
	}

	clickedButton = NULL;
	uint mask = QMessageBox::FirstButton;
	bool isFirst = true;

	while(mask <= QMessageBox::LastButton)
	{
		uint sb = buttons & mask;
		mask <<= 1;

		if(!sb)
		{
			continue;
		}

		QPushButton* button = ui->buttonBox->addButton(static_cast<QDialogButtonBox::StandardButton>(sb));

		if (isFirst)
		{
			isFirst = false;
			button->setFocus();
		}

		QMessageBox::ButtonRole  role = static_cast<QMessageBox::ButtonRole>(ui->buttonBox->buttonRole(button));

		if(role == QMessageBox::RejectRole || role == QMessageBox::NoRole)
		{
			cancelButton = static_cast<QMessageBox::StandardButton>(ui->buttonBox->standardButton(button));
		}
	}

	setWindowModality(Qt::ApplicationModal);
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}


QPixmap CustomMessageBox::standardIcon(QMessageBox::Icon icon)
{
	QStyle* style = this->style();
	int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, NULL, this);
	QIcon tmpIcon;

	switch(icon)
	{
		case QMessageBox::Information:
			tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, NULL , this);
			break;

		case QMessageBox::Warning:
			tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, NULL, this);
			break;

		case QMessageBox::Critical:
			tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, NULL, this);
			break;

		case QMessageBox::Question:
			tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, NULL, this);
			break;

		case QMessageBox::NoIcon:
			return ui->tbMenu->pixmap()->scaled(iconSize, iconSize);

		default:
			break;
	}

	if(!tmpIcon.isNull())
	{
		return tmpIcon.pixmap(iconSize);
	}

	return QPixmap();
}

QMessageBox::StandardButton CustomMessageBox::_clickedButton()
{
	return  clickedButton != NULL ? static_cast<QMessageBox::StandardButton>(ui->buttonBox->standardButton(clickedButton)) : cancelButton;
}

CustomMessageBox::~CustomMessageBox()
{
	delete ui;
}

QMessageBox::StandardButton CustomMessageBox::critical(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons)
{
	return showNewMessageBox(parent, QMessageBox::Critical, title, text, buttons);
}

QMessageBox::StandardButton CustomMessageBox::information(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons)
{
	return showNewMessageBox(parent, QMessageBox::Information, title, text, buttons);
}

QMessageBox::StandardButton CustomMessageBox::question(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons)
{
	return showNewMessageBox(parent, QMessageBox::Question, title, text, buttons);
}

QMessageBox::StandardButton CustomMessageBox::warning(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons)
{
	return showNewMessageBox(parent, QMessageBox::Warning, title, text, buttons);
}

void CustomMessageBox::about(QWidget* parent, const QString& title, const QString& text)
{
	showNewMessageBox(parent, QMessageBox::NoIcon, title, text, QMessageBox::Ok);
}

QMessageBox::StandardButton CustomMessageBox::showNewMessageBox(QWidget* parent, QMessageBox::Icon icon,
        const QString& title, const QString& text,
        QMessageBox::StandardButtons buttons)
{
	CustomMessageBox msgBox = CustomMessageBox(icon, title, text, buttons, parent);
	msgBox.exec();
	return msgBox._clickedButton();
}

void CustomMessageBox::buttonClicked(QAbstractButton* button)
{
	clickedButton = button;
}

void CustomMessageBox::accept()
{
	isClosed = true;
	done(QDialog::Accepted);
}

void CustomMessageBox::reject()
{
	isClosed = true;
	done(QDialog::Rejected);
}

QWidget* CustomMessageBox::getTitleBar()
{
	return ui->titleBar;
}

QLabel* CustomMessageBox::getTitleLabel()
{
	return ui->LTitle;
}

QLabel* CustomMessageBox::getTitleIcon()
{
	return ui->tbMenu;
}

QPushButton* CustomMessageBox::getCloseBtn()
{
	return ui->pbClose;
}

QWidget* CustomMessageBox::centralWidget()
{
	return ui->m_centralWidget;
}
