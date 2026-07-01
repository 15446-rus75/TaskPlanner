#ifndef ACHIEVEMENTS_HPP
#define ACHIEVEMENTS_HPP

#include <QString>
#include <cmath>

namespace storage
{
  namespace xp
  {
    inline constexpr int TASK_LOW = 10;
    inline constexpr int TASK_MEDIUM = 25;
    inline constexpr int TASK_HARD = 50;
    inline constexpr int ON_TIME_BONUS = 25;
    inline constexpr int PERFECT_DAY = 75;
    inline constexpr int OVERDUE_MAX = 50;
    inline constexpr int OVERDUE_HOURS_DECAY = 2;
    inline constexpr int FORMULA_BASE = 100;
    inline constexpr double FORMULA_MULTIPLIER = 1.05;
    inline constexpr int MAX_LEVEL = 50;
  }

  inline int calculateXPForLevel(int level)
  {
    if (level <= 0 || level > xp::MAX_LEVEL)
    {
      return 0;
    }
    return static_cast< int >(xp::FORMULA_BASE * std::pow(xp::FORMULA_MULTIPLIER, level - 1));
  }

  inline int calculateTotalXPForLevel(int targetLevel)
  {
    if (targetLevel <= 1)
    {
      return 0;
    }
    if (targetLevel > xp::MAX_LEVEL)
    {
      targetLevel = xp::MAX_LEVEL;
    }

    int total_xp = 0;
    for (qsizetype level = 1; level < targetLevel; ++level)
    {
      total_xp += calculateXPForLevel(level);
    }
    return total_xp;
  }

  inline int calculateLevelFromXP(int total_xp)
  {
    if (total_xp <= 0)
    {
      return 1;
    }

    int level = 1;
    int current_xp = 0;
    while (level < xp::MAX_LEVEL && current_xp + calculateXPForLevel(level) <= total_xp)
    {
      current_xp += calculateXPForLevel(level);
      ++level;
    }
    return level;
  }

  struct Achievement
  {
    QString id;
    QString name;
    QString description;
    QString iconPath;
    int xpReward;
    QString type;
  };

  namespace achievements
  {

    inline const Achievement LEVEL_1
    {
      "level_1",
      "Уровень 1",
      "Достигнут 1 уровень",
      ":/icons/achievement_level_1.png",
      0,
      "level"
    };

    inline const Achievement LEVEL_5
    {
       "level_5",
      "Уровень 5",
      "Достигнут 5 уровень",
      ":/icons/achievement_level_5.png",
      250,
      "level"
    };

    inline const Achievement LEVEL_10
    {
      "level_10",
      "Уровень 10",
      "Достигнут 10 уровень",
      ":/icons/achievement_level_10.png",
      500,
      "level"
    };

    inline const Achievement LEVEL_20
    {
      "level_20",
      "Уровень 20",
      "Достигнут 20 уровень",
      ":/icons/achievement_level_20.png",
      1000,
      "level"
    };

    inline const Achievement LEVEL_30
    {
      "level_30",
      "Уровень 30",
      "Достигнут 30 уровень",
      ":/icons/achievement_level_30.png",
      2000,
      "level"
    };

    inline const Achievement LEVEL_50
    {
      "level_50",
      "Максимальный уровень",
      "Достигнут 50 уровень (максимум)",
      ":/icons/achievement_level_50.png",
      5000,
      "level"
    };

    inline const Achievement TASKS_10
    {
      "tasks_10",
      "Выполнено 10 задач",
      "Успешно завершено 10 учебных задач",
      ":/icons/achievement_tasks_10.png",
      100,
      "task"
    };

    inline const Achievement TASKS_50
    {
      "tasks_50",
      "Выполнено 50 задач",
      "Успешно завершено 50 учебных задач",
      ":/icons/achievement_tasks_50.png",
      300,
      "task"
    };

   inline const Achievement TASKS_100
    {
      "tasks_100",
      "Выполнено 100 задач",
      "Успешно завершено 100 учебных задач",
      ":/icons/achievement_tasks_100.png",
      750,
      "task"
    };

    inline const Achievement TASKS_250
    {
      "tasks_250",
      "Выполнено 250 задач",
      "Успешно завершено 250 учебных задач",
      ":/icons/achievement_tasks_250.png",
      2000,
      "task"
    };


    inline const Achievement ON_TIME_5
    {
      "on_time_5",
      "Пунктуальность: 5",
      "Выполнено 5 задач точно в срок",
      ":/icons/achievement_on_time_5.png",
      100,
      "task"
    };

    inline const Achievement ON_TIME_20
    {
      "on_time_20",
      "Пунктуальность: 20",
       "Выполнено 20 задач точно в срок",
      ":/icons/achievement_on_time_20.png",
      300,
      "task"
    };

    inline const Achievement ON_TIME_50
    {
      "on_time_50",
      "Пунктуальность: 50",
      "Выполнено 50 задач точно в срок",
      ":/icons/achievement_on_time_50.png",
      750,
      "task"
    };


    inline const Achievement HARD_10
    {
      "hard_10",
      "Сложные задачи: 10",
      "Выполнено 10 задач с высоким приоритетом",
      ":/icons/achievement_hard_10.png",
      200,
      "task"
    };

    inline const Achievement HARD_50
    {
      "hard_50",
      "Сложные задачи: 50",
      "Выполнено 50 задач с высоким приоритетом",
      ":/icons/achievement_hard_50.png",
      500,
      "task"
    };

    inline const Achievement MEDIUM_30
    {
      "medium_30",
      "Средние задачи: 30",
      "Выполнено 30 задач со средним приоритетом",
      ":/icons/achievement_medium_30.png",
      300,
      "task"
    };

    inline const Achievement LOW_50
    {
      "low_50",
      "Легкие задачи: 50",
      "Выполнено 50 задач с низким приоритетом",
      ":/icons/achievement_low_50.png",
      250,
      "task"
    };

    inline const Achievement BALANCED_ALL
    {
      "balanced_all",
      "Мастер баланса",
      "Выполнено по 10 задач каждого приоритета",
      ":/icons/achievement_balanced.png",
      250,
      "task"
    };

    inline const Achievement PERFECT_DAY_1
    {
      "perfect_day_1",
      "Идеальный день",
      "Выполнены все задачи за 1 день",
      ":/icons/achievement_perfect_day_1.png",
      100,
      "special"
    };

    inline const Achievement PERFECT_DAY_7
    {
      "perfect_day_7",
      "Неделя продуктивности",
      "Выполнены все задачи за 7 дней подряд",
      ":/icons/achievement_perfect_day_7.png",
      300,
      "special"
    };

    inline const Achievement PERFECT_DAY_30
    {
      "perfect_day_30",
      "Месяц продуктивности",
      "Выполнены все задачи за 30 дней подряд",
      ":/icons/achievement_perfect_day_30.png",
      1000,
      "special"
    };

    inline const Achievement STREAK_7
    {
      "streak_7",
      "Серия: 7 дней",
      "Активность в приложении 7 дней подряд",
      ":/icons/achievement_streak_7.png",
      150,
      "special"
    };

    inline const Achievement STREAK_30
    {
      "streak_30",
      "Серия: 30 дней",
      "Активность в приложении 30 дней подряд",
      ":/icons/achievement_streak_30.png",
      500,
      "special"
    };

    inline const Achievement STREAK_100
    {
      "streak_100",
      "Серия: 100 дней",
      "Активность в приложении 100 дней подряд",
      ":/icons/achievement_streak_100.png",
      2000,
      "special"
    };

    inline const Achievement COMBO_NIGHTMARE
    {
      "combo_nightmare",
      "Кошмарное комбо",
      "Выполнено 3 сложных задачи за один день",
      ":/icons/achievement_nightmare.png",
      200,
      "special"
    };

    inline const Achievement COMBO_MARATHON
    {
      "combo_marathon",
      "Марафон",
      "Выполнено 15 и более задач за один день",
      ":/icons/achievement_marathon.png",
      300,
      "special"
    };


    inline const Achievement LOCATION_5
    {
        "location_5",
        "Локации: 5",
        "Открыто 5 локаций кампуса",
        ":/icons/achievement_location_5.png",
        200,
        "special"
    };

    inline const Achievement LOCATION_10
    {
      "location_10",
      "Локации: 10",
      "Открыто 10 локаций кампуса",
      ":/icons/achievement_location_10.png",
      500,
      "special"
    };

    inline const Achievement LOCATION_ALL
    {
      "location_all",
      "Исследователь кампуса",
      "Открыты все доступные локации",
      ":/icons/achievement_location_all.png",
      1500,
      "special"
    };

    inline const Achievement DELETE_5
    {
      "delete_5",
      "Удалено 5 задач",
      "Удалено 5 ненужных задач",
      ":/icons/achievement_delete_5.png",
      50,
      "special"
    };

    inline const Achievement DELETE_20
    {
      "delete_20",
      "Удалено 20 задач",
      "Удалено 20 ненужных задач",
      ":/icons/achievement_delete_20.png",
      150,
      "special"
    };

    inline const Achievement DELETE_100
    {
      "delete_100",
      "Удалено 100 задач",
      "Удалено 100 ненужных задач",
      ":/icons/achievement_delete_100.png",
      500,
      "special"
    };
  }
}

#endif
