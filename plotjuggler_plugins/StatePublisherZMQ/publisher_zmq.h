#ifndef STATE_PUBLISHER_ZMQ_H
#define STATE_PUBLISHER_ZMQ_H

#include <chrono>

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
  void PublishData(double current_time);

  bool enabled_ = false;
  std::chrono::time_point<std::chrono::system_clock> last_send_time_;
  zmq::context_t context_;
  zmq::socket_t socket_;
};

#endif  // STATE_PUBLISHER_ZMQ_H
