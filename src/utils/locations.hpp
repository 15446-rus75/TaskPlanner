#ifndef LOCATIONS_HPP
#define LOCATIONS_HPP
#include <QString>
#include <QList>

namespace storage
{
  struct Location
  {
    QString id;
    QString name;
    QString description;
  };

  namespace locations
  {
    inline constexpr int TOTAL_LOCATIONS = 23;

    inline const Location ADMISSION_AND_PASS_OFFICE
    {
      "admission_and_pass_office",
      "Приёмная комиссия и бюро пропусков",
      "Место, где рождаются мечты о поступлении. Здесь решаются судьбы абитуриентов"
    };

    inline const Location METRO_POLITEKHNICHESKAYA
    {
      "metro_politekhnicheskaya",
      "Метро Политехническая",
      "Главный вход в университет. Отсюда начинается путь каждого политехника"
    };

    inline const Location SPORTS_COMPLEX
    {
      "sports_complex",
      "Спорткомплекс Политехник",
      "Лучшие спортсмены университета тренируются здесь"
    };

    inline const Location BUILDING_1
    {
      "building_1",
      "Учебный корпус 1",
      "Первый корпус. Стены помнят поколения студентов"
    };

    inline const Location HYDRO_CAMPUS_1
    {
      "hydro_campus_1",
      "Гидротехнический корпус 1",
      "Первый гидротехнический корпус. Покорение водных потоков начинается здесь"
    };

    inline const Location BUILDING_2
    {
      "building_2",
      "Учебный корпус 2",
      "Второй корпус. Традиции и инновации под одной крышей"
    };

    inline const Location HYDRO_CAMPUS_2
    {
      "hydro_campus_2",
      "Гидротехнический корпус 2",
      "Второй гидротехнический корпус. Гидротехнология в действии"
    };

    inline const Location BUILDING_3
    {
      "building_3",
      "Учебный корпус 3",
      "Третий корпус. Храм алгоритмов и кода"
    };

    inline const Location MECHANICS_BUILDING
    {
      "mechanics_building",
      "Механический корпус",
      "Механика в чистом виде. Где происходит настоящая магия инженерии"
    };

    inline const Location BUILDING_4
    {
      "building_4",
      "Учебный корпус 4",
      "Четвёртый корпус. Где рождаются инженерные решения"
    };

    inline const Location LAB_AUDITORIUM
    {
      "lab_auditorium",
      "Лабораторно-аудиторный корпус",
      "Лекции, семинары, практикумы. Вся учёба - в одном корпусе"
    };

    inline const Location BUILDING_11
    {
      "building_11",
      "Учебный корпус 11",
      "Одиннадцатый корпус. Тихая гавань для серьёзных исследований"
    };

    inline const Location HYDRO_TOWER
    {
      "hydro_tower",
      "Гидробашня",
      "Уникальная башня для проявления себя. Coworking для творческих натур"
    };

    inline const Location MECHANICS_WORKSHOP
    {
      "mechanics_workshop",
      "Механические мастерские",
      "Руки. Инструменты. Создание. Магический треугольник инженера"
    };

    inline const Location PRODUCTION_BUILDING
    {
      "production_building",
      "Учебно-производственный корпус",
      "От теории к практике. Где студенты становятся мастерами"
    };

    inline const Location CHEMISTRY_BUILDING
    {
      "chemistry_building",
      "Химический корпус",
      "Мир молекул и реакций. Там, где начинается магия химии"
    };

    inline const Location LASER_CENTER
    {
      "laser_center",
      "Центр лазерных технологий",
      "Лазеры и высокие технологии. Вишенка на торте инноваций"
    };

    inline const Location PROFESSOR_BUILDING_1
    {
      "professor_building_1",
      "1-й профессорский корпус",
      "Первый профессорский корпус. Кабинеты лучших умов университета"
    };

    inline const Location PROFESSOR_BUILDING_2
    {
      "professor_building_2",
      "2-й профессорский корпус",
      "Второй профессорский корпус. Где преподаватели создают будущее"
    };

    inline const Location MAIN_BUILDING
    {
      "main_building",
      "Главное здание",
      "Сердце университета. Архитектурный символ Политеха"
    };

    inline const Location RESEARCH_CENTER
    {
      "research_center",
      "НИК",
      "Научно-исследовательский корпус. Здесь рождаются открытия"
    };

    inline const Location RAN_CENTER
    {
      "ran_center",
      "НОЦ РАН",
      "Научно-образовательный центр имени Алфёрова. Где наука становится будущим"
    };

    inline const Location SCIENTISTS_HOUSE
    {
      "scientists_house",
      "Дом учёных",
      "Культурный центр Политеха. Где наука встречается с искусством"
    };

    inline QList< Location > getAllLocations()
    {
      return {
        ADMISSION_AND_PASS_OFFICE,
        METRO_POLITEKHNICHESKAYA,
        SPORTS_COMPLEX,
        BUILDING_1,
        HYDRO_CAMPUS_1,
        BUILDING_2,
        HYDRO_CAMPUS_2,
        BUILDING_3,
        MECHANICS_BUILDING,
        BUILDING_4,
        LAB_AUDITORIUM,
        BUILDING_11,
        HYDRO_TOWER,
        MECHANICS_WORKSHOP,
        PRODUCTION_BUILDING,
        CHEMISTRY_BUILDING,
        LASER_CENTER,
        PROFESSOR_BUILDING_1,
        PROFESSOR_BUILDING_2,
        MAIN_BUILDING,
        RESEARCH_CENTER,
        RAN_CENTER,
        SCIENTISTS_HOUSE
      };
    }

    inline Location getLocationById(const QString &id)
    {
      for (const auto &loc: getAllLocations())
      {
        if (loc.id == id)
        {
          return loc;
        }
      }
      return {};
    }
  }
}

#endif

