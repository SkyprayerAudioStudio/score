// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Process/Style/ScenarioStyle.hpp>
#include <Scenario/Document/Minimap/Minimap.hpp>
#include <score/graphics/GraphicsItem.hpp>
#include <ossia/detail/math.hpp>

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QWidget>
#include <QGraphicsView>
#include <wobjectimpl.h>
#include <score/tools/Cursor.hpp>

W_OBJECT_IMPL(Scenario::Minimap)
namespace Scenario
{
Minimap::Minimap(QGraphicsView* vp) : m_viewport{vp}
{
  this->setAcceptHoverEvents(true);
}

void Minimap::setWidth(double d)
{
  prepareGeometryChange();
  m_width = d;
  update();
}

void Minimap::setMinDistance(double d)
{
  m_minDist = d;
}

void Minimap::setLeftHandle(double l)
{
  m_leftHandle = ossia::clamp(l, 0., m_rightHandle - m_minDist);
  update();
}

void Minimap::setRightHandle(double r)
{
  m_rightHandle = ossia::clamp(r, m_leftHandle + m_minDist, m_width);
  update();
}

void Minimap::setHandles(double l, double r)
{
  m_leftHandle = ossia::clamp(l, 0., m_rightHandle - m_minDist);
  m_rightHandle = ossia::clamp(r, m_leftHandle + m_minDist, m_width);
  update();
}

void Minimap::modifyHandles(double l, double r)
{
  setHandles(l, r);
  visibleRectChanged(m_leftHandle, m_rightHandle);
}

void Minimap::setLargeView()
{
  modifyHandles(0., m_width);
}

void Minimap::zoomIn()
{
  modifyHandles(
      m_leftHandle + 0.01 * (m_rightHandle - m_leftHandle),
      m_rightHandle - 0.01 * (m_rightHandle - m_leftHandle));
}

void Minimap::zoomOut()
{
  modifyHandles(
      m_leftHandle - 0.01 * (m_rightHandle - m_leftHandle),
      m_rightHandle + 0.01 * (m_rightHandle - m_leftHandle));
}

void Minimap::zoom(double z)
{
  modifyHandles(m_leftHandle + z, m_rightHandle - z);
}

QRectF Minimap::boundingRect() const
{
  return {0., 0., m_width, m_height};
}

void Minimap::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem* option,
    QWidget* widget)
{
  auto& sk = Process::Style::instance();
  painter->setRenderHint(QPainter::Antialiasing, false);
  painter->fillRect(
      QRectF{m_leftHandle, 0., m_rightHandle - m_leftHandle, m_height }, sk.MinimapBrush());

  painter->setPen(sk.MinimapPen());

  const double line_length = 5;
  const QPointF top_left{m_leftHandle + 1., 0};
  painter->drawLine(top_left, top_left + QPointF{line_length, 0.});
  painter->drawLine(top_left, top_left + QPointF{0., line_length});

  const QPointF top_right{m_rightHandle - 1., 0};
  painter->drawLine(top_right, top_right + QPointF{-line_length, 0.});
  painter->drawLine(top_right, top_right + QPointF{0., line_length});

  const QPointF bottom_left{m_leftHandle + 1., m_height - 0.5};
  painter->drawLine(bottom_left, bottom_left + QPointF{line_length, 0.});
  painter->drawLine(bottom_left, bottom_left + QPointF{0., -line_length});

  const QPointF bottom_right{m_rightHandle - 1., m_height - 0.5};
  painter->drawLine(bottom_right, bottom_right + QPointF{-line_length, 0.});
  painter->drawLine(bottom_right, bottom_right + QPointF{0., -line_length});
}

void Minimap::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
  m_gripLeft = false;
  m_gripRight = false;
  m_gripMid = false;

  const auto pos_x = ev->pos().x();

  if (std::abs(pos_x - m_leftHandle) < 3.)
    m_gripLeft = true;
  else if (std::abs(pos_x - m_rightHandle) < 3.)
    m_gripRight = true;
  else if (pos_x > m_leftHandle && pos_x < m_rightHandle)
    m_gripMid = true;
  else
  {
    ev->ignore();
    return;
  }

#if defined(__APPLE__)
  CGEventRef event = CGEventCreate(nullptr);
  CGPoint loc = CGEventGetLocation(event);
  CFRelease(event);

  m_startPos = {loc.x, loc.y};
#else
  m_startPos = m_viewport->mapToGlobal(QPoint{0,0}) + ev->pos();
#endif

  m_relativeStartX
      = (ev->pos().x() - m_leftHandle) / (m_rightHandle - m_leftHandle);
  m_startY = ev->pos().y();

  if (m_setCursor)
  {
    score::hideCursor(true);
    //QApplication::changeOverrideCursor(QCursor(Qt::BlankCursor));
  }
  else
  {
#if defined(__APPLE__)
    score::hideCursor(true);
#else
    QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
#endif
    m_setCursor = true;
  }
  ev->accept();
}
void Minimap::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
  const auto pos = ev->screenPos();
  if (m_gripLeft || m_gripRight || m_gripMid)
  {
    auto dx = 0.7 * (pos.x() - m_startPos.x());
    if (m_gripLeft)
    {
      setLeftHandle(m_leftHandle + dx);
    }
    else if (m_gripRight)
    {
      setRightHandle(m_rightHandle + dx);
    }
    else if (m_gripMid)
    {
      auto dy = 0.7 * (pos.y() - m_startPos.y());

      auto newLeftHandle = std::max(m_leftHandle + dx - dy, 0.);
      auto newRightHandle = std::min(m_rightHandle + dx + dy, m_width);
      if(m_leftHandle <= 5. && newLeftHandle <= 5.)
        newRightHandle = m_rightHandle;
      if(m_rightHandle >= m_width - 5. && newRightHandle >= m_width - 5.)
        newLeftHandle = m_leftHandle;

      if(newLeftHandle != m_leftHandle || newRightHandle != m_rightHandle)
      {
        setHandles(newLeftHandle, newRightHandle);
      }
    }

    score::moveCursorPos(m_startPos);
    score::hideCursor(true);
    m_setCursor = true;

    visibleRectChanged(m_leftHandle, m_rightHandle);
  }
  ev->accept();
}

void Minimap::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
  score::showCursor();
  m_setCursor = false;

  if (m_gripLeft || m_gripRight || m_gripMid)
  {
    m_gripLeft = false;
    m_gripRight = false;
    m_gripMid = false;

    QPointF pos;
    pos.setX(ossia::clamp(
        m_leftHandle + m_relativeStartX * (m_rightHandle - m_leftHandle),
        m_leftHandle,
        m_rightHandle));
    pos.setY(m_startY);

    score::setCursorPos(QPointF{m_viewport->mapToGlobal(QPoint{0,0})} + pos);
  }
  ev->accept();
}

void Minimap::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev)
{
  rescale();
  ev->accept();
}

void Minimap::hoverEnterEvent(QGraphicsSceneHoverEvent* ev)
{
  auto& skin = score::Skin::instance();

  const auto pos_x = ev->pos().x();
  if (std::abs(pos_x - m_leftHandle) < 3.)
  {
    if (!m_setCursor)
    {
      QApplication::setOverrideCursor(skin.CursorScaleH);
      m_setCursor = true;
    }
  }
  else if (std::abs(pos_x - m_rightHandle) < 3.)
  {
    if (!m_setCursor)
    {
      QApplication::setOverrideCursor(skin.CursorScaleH);
      m_setCursor = true;
    }
  }
  else if (pos_x > m_leftHandle && pos_x < m_rightHandle)
  {
    if (!m_setCursor)
    {
      QApplication::setOverrideCursor(skin.CursorMove);
      m_setCursor = true;
    }
  }
  else
  {
    if (m_setCursor)
    {
      QApplication::restoreOverrideCursor();
      m_setCursor = false;
    }
  }
}
void Minimap::hoverMoveEvent(QGraphicsSceneHoverEvent* ev)
{
  this->hoverEnterEvent(ev);
}

void Minimap::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
  if (m_setCursor)
  {
    QApplication::restoreOverrideCursor();
    m_setCursor = false;
  }
}
}
