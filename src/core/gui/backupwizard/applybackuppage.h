﻿#ifndef APPLYBACKUPPAGE_H
#define APPLYBACKUPPAGE_H


#include "quazipfile.h"
#include <QWizardPage>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QTableWidget>

class ApplyBackupPage : public QWizardPage
{
	Q_OBJECT
public:
	explicit ApplyBackupPage(QWidget* parent = 0);
	int nextId() const override;
	bool isComplete() const override;
	void initializePage() override;
private:
	QLabel* messageLabel;
	QLabel* backupPathLabel;
	QGridLayout* gridLayout;
	QLineEdit* backupPathLineEdit;
	QPushButton* browsePushButton;
	QCheckBox* drivesCheckBox;
	QComboBox* drivesComboBox;
	QGroupBox* changePathGroupBox;
	QGridLayout* gridLayout_2;
	QTableWidget* tableWidget;
	QStringList GetLongestCommonSubstr(QStringList strings);
	void ApplyBackup() const;
public slots:
	void browseButtonClicked();
};

#endif // APPLYBACKUPPAGE_H


