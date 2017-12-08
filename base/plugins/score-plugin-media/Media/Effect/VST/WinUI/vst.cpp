
#include <iostream>
#include <aeffect.h>
#include <aeffectx.h>
#include <QWindow>
void show_vst2_editor(AEffect* effect, uint16_t width, uint16_t height)
{
  auto container = reinterpret_cast<QWindow*>(effect->resvd2);
  if(container)
  {
    effect->dispatcher(effect, effEditOpen, 0, 0, (void*)container->winId(), 0);
    container->show();
    container->setMinimumHeight(height);
    container->setMaximumHeight(height);
    container->setMinimumWidth(width);
    container->setMaximumWidth(width);
    return;
  }
  container = new QWindow;

  effect->dispatcher(effect, effEditOpen, 0, 0, (void*)container->winId(), 0);
  container->show();
  effect->resvd2 = reinterpret_cast<VstIntPtr>(container);
}

void hide_vst2_editor(AEffect* effect)
{
  effect->dispatcher(effect, effEditClose, 0, 0, nullptr, 0);
  if(effect->resvd2)
  {
    auto container = reinterpret_cast<QWindow*>(effect->resvd2);
    container->close();
    container->deleteLater();
    effect->resvd2 = 0;
  }
}