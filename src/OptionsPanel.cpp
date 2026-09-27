#include "OptionsPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>

OptionsPanel::OptionsPanel(QWidget *parent)
    : QGroupBox(tr("Options"), parent)
    , m_connectRadio(new QRadioButton(tr("TCP connect (-sT)")))
    , m_synRadio(new QRadioButton(tr("SYN (-sS)")))
    , m_udpRadio(new QRadioButton(tr("UDP (-sU)")))
    , m_pingRadio(new QRadioButton(tr("Ping only (-sn)")))
    , m_portsEdit(new QLineEdit)
    , m_timingCombo(new QComboBox)
    , m_serviceCheck(new QCheckBox(tr("Service versions (-sV)")))
    , m_osCheck(new QCheckBox(tr("OS detection (-O)")))
    , m_skipPingCheck(new QCheckBox(tr("Skip host discovery (-Pn)")))
    , m_privilegeNote(new QLabel)
{
    m_connectRadio->setChecked(true);
    m_portsEdit->setPlaceholderText(tr("e.g. 22,80,443 or 1-1024 (empty: nmap's top 1000)"));

    // The item's index is the number after -T.
    m_timingCombo->addItems({tr("-T0 Paranoid"), tr("-T1 Sneaky"), tr("-T2 Polite"),
                             tr("-T3 Normal"), tr("-T4 Aggressive"), tr("-T5 Insane")});
    m_timingCombo->setCurrentIndex(3);

    m_privilegeNote->setWordWrap(true);
    m_privilegeNote->setEnabled(false); // drawn greyed out, like the options it explains
    m_privilegeNote->hide();

    auto *scanTypeRow = new QHBoxLayout;
    scanTypeRow->addWidget(m_connectRadio);
    scanTypeRow->addWidget(m_synRadio);
    scanTypeRow->addWidget(m_udpRadio);
    scanTypeRow->addWidget(m_pingRadio);
    scanTypeRow->addStretch();

    auto *detectionRow = new QHBoxLayout;
    detectionRow->addWidget(m_serviceCheck);
    detectionRow->addWidget(m_osCheck);
    detectionRow->addWidget(m_skipPingCheck);
    detectionRow->addStretch();

    auto *layout = new QFormLayout(this);
    layout->addRow(tr("Scan type:"), scanTypeRow);
    layout->addRow(tr("Ports:"), m_portsEdit);
    layout->addRow(tr("Timing:"), m_timingCombo);
    layout->addRow(tr("Detection:"), detectionRow);
    layout->addRow(m_privilegeNote);

    // Switching radio buttons toggles two of them; only the newly checked one
    // needs to report the change.
    for (QRadioButton *radio : {m_connectRadio, m_synRadio, m_udpRadio, m_pingRadio}) {
        connect(radio, &QRadioButton::toggled, this, [this](bool checked) {
            if (!checked)
                return;
            updateEnabledState();
            emit optionsChanged();
        });
    }
    connect(m_portsEdit, &QLineEdit::textChanged, this, &OptionsPanel::optionsChanged);
    connect(m_timingCombo, &QComboBox::currentIndexChanged, this, &OptionsPanel::optionsChanged);
    for (QCheckBox *check : {m_serviceCheck, m_osCheck, m_skipPingCheck})
        connect(check, &QCheckBox::toggled, this, &OptionsPanel::optionsChanged);

    updateEnabledState();
}

ScanOptions OptionsPanel::options() const
{
    ScanOptions options;
    if (m_synRadio->isChecked())
        options.scanType = ScanType::Syn;
    else if (m_udpRadio->isChecked())
        options.scanType = ScanType::Udp;
    else if (m_pingRadio->isChecked())
        options.scanType = ScanType::PingOnly;
    else
        options.scanType = ScanType::Connect;

    options.ports = m_portsEdit->text();
    options.timing = m_timingCombo->currentIndex();
    options.serviceVersion = m_serviceCheck->isChecked();
    options.osDetection = m_osCheck->isChecked();
    options.skipPing = m_skipPingCheck->isChecked();
    return options;
}

void OptionsPanel::setPrivileges(const Privileges &privileges)
{
    m_privilegedAllowed = privilegedOptionsAllowed(privileges);

    // Disabled widgets still show their tooltip, so the reason stays one
    // hover away. The note below repeats it for users who don't hover.
    const QString reason = missingPrivilegesText(privileges);
    m_synRadio->setToolTip(reason);
    m_udpRadio->setToolTip(reason);
    m_osCheck->setToolTip(reason);
    m_privilegeNote->setText(tr("SYN, UDP and OS detection are unavailable. %1").arg(reason));
    m_privilegeNote->setVisible(!m_privilegedAllowed);

    updateEnabledState();
}

void OptionsPanel::updateEnabledState()
{
    const bool scansPorts = !m_pingRadio->isChecked();

    m_synRadio->setEnabled(m_privilegedAllowed);
    m_udpRadio->setEnabled(m_privilegedAllowed);
    m_portsEdit->setEnabled(scansPorts);
    m_serviceCheck->setEnabled(scansPorts);
    m_osCheck->setEnabled(scansPorts && m_privilegedAllowed);
}
