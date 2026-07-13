#ifndef SDO_LINKS_HPP
#define SDO_LINKS_HPP

#include <QString>
#include <QList>

namespace sdo
{
  struct Institute
  {
    QString name;
    QString url;
  };

  inline QList< Institute > get_institutes()
  {
    return
    {
      { "Гуманитарный институт", "https://dl-hum.spbstu.ru/" },
      { "Институт биомедицинских систем и биотехнологий", "https://dl-ibmst.spbstu.ru/" },
      { "Инженерно-строительный институт", "https://dl-ice.spbstu.ru/" },
      { "Институт компьютерных наук и кибербезопасности", "https://dl.spbstu.ru/" },
      { "Институт машиностроения, материалов и транспорта", "https://immet-dist.spbstu.ru/" },
      { "Физико-механический институт", "https://dl-physmech.spbstu.ru/" },
      { "Институт промышленного менеджмента, экономики и торговли", "https://dl-imet.spbstu.ru/" },
      { "Институт электроники и телекоммуникаций", "https://dl-et.spbstu.ru/" },
      { "Институт физической культуры, спорта и туризма", "https://dl-ifkst.spbstu.ru/" },
      { "Институт энергетики", "https://dl-iets.spbstu.ru/" },
      { "Институт физики и математики", "https://dl-phmath.spbstu.ru/" },
      { "Основы проектной деятельности", "https://opd.spbstu.ru/" }
    };
  }
}

#endif
