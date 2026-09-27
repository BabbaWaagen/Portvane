#pragma once

#include "Privileges.h"
#include "ScanOptions.h"

#include <QGroupBox>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QRadioButton;

// The option builder: a curated set of nmap's most useful options.
class OptionsPanel : public QGroupBox
{
    Q_OBJECT

public:
    explicit OptionsPanel(QWidget *parent = nullptr);

    // The picked options. targets is left empty; the target field is not
    // part of this panel.
    ScanOptions options() const;

    // Greys out the options these privileges don't allow, and says why.
    void setPrivileges(const Privileges &privileges);

signals:
    void optionsChanged();

private:
    void updateEnabledState();

    QRadioButton *m_connectRadio;
    QRadioButton *m_synRadio;
    QRadioButton *m_udpRadio;
    QRadioButton *m_pingRadio;
    QLineEdit *m_portsEdit;
    QComboBox *m_timingCombo;
    QCheckBox *m_serviceCheck;
    QCheckBox *m_osCheck;
    QCheckBox *m_skipPingCheck;
    QLabel *m_privilegeNote;
    bool m_privilegedAllowed = false;
};
