#pragma once
#include <cstdint>
#include <cmath>
#include <QPointF>

#include <Process/Dataflow/PortFactory.hpp>

#include <score/graphics/RectItem.hpp>
#include <score/graphics/TextItem.hpp>
namespace Process
{

static const constexpr int MaxRowsInEffect = 5;

// TODO not very efficient since it recomputes everything every time...
// Does a grid layout with maximum N rows per column.
template<typename F>
QPointF currentWigetPos(int controlIndex, F getControlSize)
  noexcept(noexcept(getControlSize(0)))
{
  int N = MaxRowsInEffect * (controlIndex / MaxRowsInEffect);
  qreal x = 0;
  for(int i = 0; i < N; )
  {
    qreal w = 0;
    for(int j = i; j < i + MaxRowsInEffect && j < N; j++)
    {
      auto sz = getControlSize(j);
      w = std::max(w, sz.width());
    }
    x += w;
    i += MaxRowsInEffect;
  }

  qreal y = 0;
  for(int j = N; j < controlIndex; j++)
  {
    auto sz = getControlSize(j);
    y += sz.height();
  }

  return {x, y};
}

template<typename CreatePort,
         typename CreateControl,
         typename GetControlSize,
         typename GetName,
         typename GetFactory>
struct ControlSetup
{
  CreatePort createPort;
  CreateControl createControl;
  GetControlSize getControlSize;
  GetName name;
  GetFactory getFactory;
};

template<typename... Args>
auto controlSetup(Args&&... args)
{
  if constexpr(sizeof...(Args) == 4)
  {
    return controlSetup(std::forward<Args>(args)...,
                        [] (const Process::PortFactoryList& portFactory, Process::Port& port) -> Process::PortFactory& {
        return *portFactory.get(port.concreteKey());
      });
  }
  else {
    return ControlSetup<Args...>{std::forward<Args>(args)...};
  }
}

template<typename T, typename... Args>
auto createControl(
    int i,
    const ControlSetup<Args...>& setup,
    T& port,
    const Process::PortFactoryList& portFactory,
    const score::DocumentContext& doc,
    QGraphicsItem* parentItem,
    QObject* parent)
{
  // TODO put the port at the correct order wrt its index ?
  auto item = new score::EmptyRectItem{parentItem};

  // Create the port
  auto& fact = setup.getFactory(portFactory, port);
  auto portItem = setup.createPort(fact, port, doc, item, parent);

  // Create the label
  auto lab = new score::SimpleTextItem{Process::portBrush(port.type).main, item};
  if(auto name = setup.name(); name.size() > 0)
    lab->setText(std::move(name));
  else
    lab->setText(QObject::tr("Control"));

  QObject::connect(
        &port, &Process::ControlInlet::customDataChanged,
        item, [=](const QString& txt) { lab->setText(txt); });

  // Create the control
  QGraphicsItem* widg = setup.createControl(fact, port, doc, item, parent);
  widg->setParentItem(item);

  // Layout the items
  const qreal labelHeight = 10;
  const qreal labelWidth = lab->boundingRect().width();
  const auto wrect = widg->boundingRect();
  const qreal widgetHeight = wrect.height();
  const qreal widgetWidth = wrect.width();

  auto h = std::max(
      20.,
      (qreal)(widgetHeight + labelHeight + 7.));
  auto w = std::max(90., std::max(25. + labelWidth, widgetWidth));

  portItem->setPos(8., 4.);
  lab->setPos(20., 2);
  widg->setPos(18., labelHeight + 5.);

  const auto itemRect = QRectF{0., 0, w, h};

  QPointF pos = Process::currentWigetPos(i, setup.getControlSize);
  item->setPos(pos);
  item->setRect(itemRect);

  struct Controls {
    score::EmptyRectItem* item{};
    Dataflow::PortItem* port{};
    QGraphicsItem* widg{};
    score::SimpleTextItem* label{};
    QRectF itemRect;
  };
  return Controls{item, portItem, widg, lab, itemRect};
}

}