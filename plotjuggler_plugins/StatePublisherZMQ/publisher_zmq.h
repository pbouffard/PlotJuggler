#ifndef STATE_PUBLISHER_ZMQ_H
#define STATE_PUBLISHER_ZMQ_H

#include <atomic>
#include <thread>

#include <QObject>
#include <QtPlugin>
#include <zmq.hpp>

#include "PlotJuggler/statepublisher_base.h"

class StatePublisherZMQ : public PJ::StatePublisher
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.StatePublisher")
  Q_INTERFACES(PJ::StatePublisher)

public:
  StatePublisherZMQ();

  virtual const char* name() const override
  {
    return "ZMQ Publisher";
  }

  virtual ~StatePublisherZMQ() override;

  virtual bool enabled() const override
  {
    return enabled_;
  }

  virtual void updateState(double current_time) override { PublishData(current_time); }
  virtual void play(double current_time) override { PublishData(current_time); }

public slots:
  virtual void setEnabled(bool enabled) override;

private:
  bool enabled_ = false;
  zmq::context_t context_;
  zmq::socket_t publisher_;

  void PublishData(double current_time);
};

#endif  // STATE_PUBLISHER_ZMQ_H
