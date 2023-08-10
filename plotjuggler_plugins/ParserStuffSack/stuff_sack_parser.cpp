#include "stuff_sack_parser.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "stuff_sack-src/src/dynamic/type_descriptors.h"
#include "stuff_sack-src/src/dynamic/dynamic_types.h"

#include "stuff_sack_ui.h"

using namespace PJ;
using namespace ss::dynamic;
using namespace std::chrono;

class StuffSackParser : public MessageParser
{
public:
  StuffSackParser(std::unique_ptr<const DescriptorBuilder> types,
                  PJ::PlotDataMapRef& data, const StuffSackOptionsWidget* options)
    : MessageParser({}, data)
    , _types{ std::move(types) }
    , _start_time{ duration_cast<duration<double>>(
                       high_resolution_clock::now().time_since_epoch())
                       .count() }
    , _x_axis_mode{ options->xAxisMode() }
  {
    switch (_x_axis_mode)
    {
      case XAxisMode::kRxTime:
        break;
      case XAxisMode::kFixed:
        _delta_t = 1.0 / options->xAxisFixedRate();
        break;
      case XAxisMode::kField:
        _timestamp_field_path = options->xAxisField().split('/');
        _timestamp_field_mult = options->xAxisFieldMult();
        break;
    }
  }

  bool parseMessage(const MessageRef serialized_msg, double& timestamp) override
  {
    if (!_types)
      return false;

    const uint8_t* data = serialized_msg.data();
    const size_t len = serialized_msg.size();
    const auto [msg, status] = UnpackMessage(data, len, *_types);

    if (status != UnpackStatus::kSuccess || !msg)
      return false;

    SetTimestamp(timestamp, *msg);
    AddStruct(timestamp, msg->descriptor().name(), *msg);

    return true;
  }

private:
  using XAxisMode = StuffSackOptionsWidget::XAxisMode;

  void SetTimestamp(double& timestamp, const DynamicStruct& msg)
  {
    switch (_x_axis_mode)
    {
      case XAxisMode::kRxTime: {
        return;
      }
      case XAxisMode::kFixed: {
        const TypeDescriptor& descr = msg.descriptor();
        const auto& time_it = _current_times.find(&descr);
        if (time_it == _current_times.end())
        {
          _current_times.emplace(&descr, _start_time);
          timestamp = _start_time;
          return;
        }
        timestamp = time_it->second;
        time_it->second += _delta_t;
        return;
      }
      case XAxisMode::kField: {
        return;
      }
    }
  }

  void AddArray(double timestamp, const std::string& parent_name,
                const DynamicArray& array)
  {
    const TypeDescriptor& array_descr = array.descriptor();
    const TypeDescriptor& elem_descr = array_descr.array_elem_type();
    const std::string elem_base_name = parent_name + "[";

    using Type = TypeDescriptor::Type;
    for (size_t i = 0; i < array.size(); ++i)
    {
      const std::string elem_name = elem_base_name + std::to_string(i) + "]";
      switch (elem_descr.type())
      {
        case Type::kEnum:
        case Type::kPrimitive: {
          const double val = array.Convert<double>(i);
          getSeries(elem_name).pushBack({ timestamp, val });
          break;
        }
        case Type::kStruct:
        case Type::kBitfield: {
          AddStruct(timestamp, elem_name, array.Get<DynamicStruct>(i));
          break;
        }
        case Type::kArray: {
          AddArray(timestamp, elem_name, array.Get<DynamicArray>(i));
          break;
        }
      }
    }
  }

  void AddStruct(double timestamp, const std::string& parent_name,
                 const DynamicStruct& structure)
  {
    const TypeDescriptor& struct_descr = structure.descriptor();

    using Type = TypeDescriptor::Type;
    for (const auto& field_descr : struct_descr.struct_fields())
    {
      const std::string field_name = parent_name + "/" + field_descr->name();
      const TypeDescriptor& field_type = field_descr->type();

      switch (field_type.type())
      {
        case Type::kEnum:
        case Type::kPrimitive: {
          const double val = structure.Convert<double>(*field_descr);
          getSeries(field_name).pushBack({ timestamp, val });
          break;
        }
        case Type::kStruct:
        case Type::kBitfield: {
          AddStruct(timestamp, field_name, structure.Get<DynamicStruct>(*field_descr));
          break;
        }
        case Type::kArray: {
          AddArray(timestamp, field_name, structure.Get<DynamicArray>(*field_descr));
          break;
        }
      }
    }
  }

  const std::unique_ptr<const DescriptorBuilder> _types;
  const double _start_time;
  XAxisMode _x_axis_mode;
  double _delta_t;
  QStringList _timestamp_field_path;
  double _timestamp_field_mult;
  std::unordered_map<const TypeDescriptor*, double> _current_times;
};

ParserStuffSack::ParserStuffSack()
{
  _widget = new StuffSackOptionsWidget(nullptr);  // Deleted by layout in streamer.
}

MessageParserPtr ParserStuffSack::createParser(const std::string& topic_name,
                                               const std::string& type_name,
                                               const std::string& schema,
                                               PJ::PlotDataMapRef& data)
{
  _widget->saveState();

  QFile file(_widget->specFile());
  if (!file.exists())
  {
    throw std::runtime_error(
        tr("Error loading file: %1 does not exist").arg(file.fileName()).toStdString());
  }

  std::unique_ptr<const DescriptorBuilder> types;
  try
  {
    types = std::make_unique<const DescriptorBuilder>(
        std::move(DescriptorBuilder::FromFile(file.fileName().toStdString())));
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(
        tr("Error parsing file %1: %2").arg(file.fileName()).arg(e.what()).toStdString());
  }

  return std::make_shared<StuffSackParser>(std::move(types), data, _widget);
}
