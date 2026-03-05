#ifndef ICONTROLLER_HPP
#define ICONTROLLER_HPP

#include <QObject>
#include "../view/iview.hpp"
#include "../storage/istorage.hpp"

namespace controller
{
  class IController: public QObject
  {
    Q_OBJECT
  public:
    virtual ~IController() = default;
    virtual void setStorage(storage::IStorage* storage) = 0;
    virtual void setView(view::IView* view) = 0;
    virtual void start() = 0;
  };
}

#endif
