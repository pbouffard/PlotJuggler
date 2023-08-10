#pragma once

#include <qvalidator.h>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QDir>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QWidget>
#include <QString>

class StuffSackOptionsWidget : public QWidget
{
  Q_OBJECT;

public:
  enum class XAxisMode
  {
    kRxTime,
    kFixed,
    kField,
  };

  explicit StuffSackOptionsWidget(QWidget* parent = nullptr) : QWidget(parent)
  {
    setMinimumWidth(400);

    QVBoxLayout* top_vbox;
    setLayout(top_vbox = new QVBoxLayout);

    // Spec File
    QHBoxLayout* spec_hbox;
    top_vbox->addLayout(spec_hbox = new QHBoxLayout);

    spec_hbox->addWidget(_spec_file_button = new QPushButton(tr("Spec File")));
    spec_hbox->addWidget(_spec_file_text = new QLineEdit, 1);
    connect(_spec_file_button, &QPushButton::clicked, this,
            &StuffSackOptionsWidget::onLoadFile);

    top_vbox->addWidget(new QLabel("X axis source:"));

    // Receive
    top_vbox->addWidget(_x_axis_rx_radio = new QRadioButton(tr("Receive Time")));
    _x_axis_rx_radio->setChecked(true);

    // Fixed Rate
    QHBoxLayout* fixed_hbox;
    top_vbox->addLayout(fixed_hbox = new QHBoxLayout);
    fixed_hbox->addWidget(_x_axis_fixed_radio = new QRadioButton(tr("Fixed Rate [Hz]:")));
    fixed_hbox->addWidget(_rate_text = new QLineEdit("100"));
    fixed_hbox->addStretch(1);
    _rate_text->setMaximumWidth(70);
    QDoubleValidator* validator = new QDoubleValidator(1e-9, 1e9, 6, this);
    validator->setNotation(QDoubleValidator::Notation::ScientificNotation);
    _rate_text->setValidator(validator);

    // Field Data
    QHBoxLayout* field_hbox;
    top_vbox->addLayout(field_hbox = new QHBoxLayout);
    field_hbox->addWidget(_x_axis_field_radio = new QRadioButton(tr("Data Field:")));
    field_hbox->addWidget(_field_text = new QLineEdit("timestamp"));
    _x_axis_field_radio->setEnabled(false);  // Non-functional placeholder for now.

    QHBoxLayout* field_mult_hbox;
    top_vbox->addLayout(field_mult_hbox = new QHBoxLayout);
    field_mult_hbox->addStretch(1);
    field_mult_hbox->addWidget(new QLabel(tr("Multiplier:")));
    field_mult_hbox->addWidget(_field_mult_text = new QLineEdit("1.0"));
    _field_mult_text->setMaximumWidth(70);
    _field_mult_text->setValidator(validator);

    // Button Group
    QButtonGroup* x_axis_button_group = new QButtonGroup(this);
    x_axis_button_group->addButton(_x_axis_rx_radio);
    x_axis_button_group->addButton(_x_axis_fixed_radio);
    x_axis_button_group->addButton(_x_axis_field_radio);

    connect(x_axis_button_group, &QButtonGroup::idClicked, this,
            &StuffSackOptionsWidget::onXAxisChange);

    loadState();
    onXAxisChange();
  }

  QString specFile() const
  {
    return _spec_file_text->text();
  }

  XAxisMode xAxisMode() const
  {
    if (_x_axis_rx_radio->isChecked())
      return XAxisMode::kRxTime;
    if (_x_axis_fixed_radio->isChecked())
      return XAxisMode::kFixed;
    return XAxisMode::kField;
  }

  double xAxisFixedRate() const
  {
    return _rate_text->text().toDouble();
  }

  QString xAxisField() const
  {
    return _field_text->text();
  }

  double xAxisFieldMult() const
  {
    return _field_mult_text->text().toDouble();
  }

  void saveState() const
  {
    QSettings settings;
    settings.setValue("ParserStuffSack.SpecFile", _spec_file_text->text());
    settings.setValue("ParserStuffSack.XAxisRxRadioIsChecked",
                      _x_axis_rx_radio->isChecked());
    settings.setValue("ParserStuffSack.XAxisFixedRadioIsChecked",
                      _x_axis_fixed_radio->isChecked());
    settings.setValue("ParserStuffSack.XAxisFieldRadioIsChecked",
                      _x_axis_field_radio->isChecked());
    settings.setValue("ParserStuffSack.RateText", _rate_text->text());
    settings.setValue("ParserStuffSack.FieldText", _field_text->text());
    settings.setValue("ParserStuffSack.FieldMultText", _field_mult_text->text());
  }

  void loadState() const
  {
    QSettings settings;
    _spec_file_text->setText(settings.value("ParserStuffSack.SpecFile").toString());
    _x_axis_rx_radio->setChecked(
        settings.value("ParserStuffSack.XAxisRxRadioIsChecked").toBool());
    _x_axis_fixed_radio->setChecked(
        settings.value("ParserStuffSack.XAxisFixedRadioIsChecked").toBool());
    _x_axis_field_radio->setChecked(
        settings.value("ParserStuffSack.XAxisFieldRadioIsChecked").toBool());
    _rate_text->setText(settings.value("ParserStuffSack.RateText").toString());
    _field_text->setText(settings.value("ParserStuffSack.FieldText").toString());
    _field_mult_text->setText(settings.value("ParserStuffSack.FieldMultText").toString());
  }

private:
  void onXAxisChange()
  {
    _rate_text->setEnabled(_x_axis_fixed_radio->isChecked());
    _field_text->setEnabled(_x_axis_field_radio->isChecked());
    _field_mult_text->setEnabled(_x_axis_field_radio->isChecked());
  }

  void onLoadFile()
  {
    QSettings settings;

    QString directory_path =
        settings.value("ParserStuffSack.loadDirectory", QDir::currentPath()).toString();

    QString filename = QFileDialog::getOpenFileName(this, tr("Load Message Definition"),
                                                    directory_path, "(*.yaml);;(*)");
    if (filename.isEmpty())
      return;

    _spec_file_text->setText(filename);

    directory_path = QFileInfo(filename).absolutePath();
    settings.setValue("ParserStuffSack.loadDirectory", directory_path);
  }

  QPushButton* _spec_file_button;
  QLineEdit* _spec_file_text;
  QRadioButton* _x_axis_rx_radio;
  QRadioButton* _x_axis_fixed_radio;
  QRadioButton* _x_axis_field_radio;
  QLineEdit* _rate_text;
  QLineEdit* _field_text;
  QLineEdit* _field_mult_text;
};
