#pragma once

#include <QWidget>

#include "PlotJuggler/messageparser_base.h"
#include "stuff_sack_ui.h"

class ParserStuffSack : public PJ::ParserFactoryPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.ParserFactoryPlugin")
  Q_INTERFACES(PJ::ParserFactoryPlugin)

public:
  ParserStuffSack();

  const char* name() const override
  {
    return "ParserStuffSack";
  }

  const char* encoding() const override
  {
    return "stuff_sack";
  }

  PJ::MessageParserPtr createParser(const std::string& topic_name,
                                    const std::string& type_name,
                                    const std::string& schema,
                                    PJ::PlotDataMapRef& data) override;

  QWidget* optionsWidget() override
  {
    return _widget;
  }

private:
  StuffSackOptionsWidget* _widget;
};
