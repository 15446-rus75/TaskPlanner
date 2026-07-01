#ifndef MOCKSTORAGE_HPP
#define MOCKSTORAGE_HPP

#include "istorage.hpp"
#include <QList>
#include <QString>

namespace test
{
  class MockStorage : public storage::IStorage
  {
  public:
    MockStorage() = default;
    ~MockStorage() override = default;

    void addTask(const storage::Task &task) override
    {
      addTaskCallCount++;
      lastAddedTask = task;
      m_tasks.append(task);
    }

    void removeTask(int id) override
    {
      removeTaskCallCount++;
      lastRemovedId = id;
      for (int i = 0; i < m_tasks.size(); ++i)
      {
        if (m_tasks[i].id == id)
        {
          m_tasks.removeAt(i);
          return;
        }
      }
    }

    void updateTask(const storage::Task &task) override
    {
      updateTaskCallCount++;
      lastUpdatedTask = task;
      for (auto &existing : m_tasks)
      {
        if (existing.id == task.id)
        {
          existing = task;
          return;
        }
      }
    }

    QList< storage::Task > getAllTasks() const override
    {
      getAllTasksCallCount++;
      return m_tasks;
    }

    QList< storage::Task > getTasksForDate(const QDate &date) const override
    {
      getTasksForDateCallCount++;
      lastQueriedDate = date;
      QList< storage::Task > result;
      for (const auto &task : m_tasks)
      {
        if (task.deadline.date() == date)
        {
          result.append(task);
        }
      }
      return result;
    }

    QList< storage::Task > getTasksForToday() const override
    {
      getTasksForTodayCallCount++;
      return tasksForTodayToReturn;
    }

    QList< storage::Task > getOverdueTasks() const override
    {
      getOverdueTasksCallCount++;
      return overdueTasksToReturn;
    }

    QList< storage::Task > getTasksFiltered(const QString &searchText, bool today, bool overdue, storage::Priority priority) const override
    {
      getTasksFilteredCallCount++;
      lastSearchText = searchText;
      lastTodayFlag = today;
      lastOverdueFlag = overdue;
      lastPriorityArg = priority;
      return filteredTasksToReturn;
    }

    QList< storage::Task > getSortedTasks(const QList< storage::Task > &tasks, storage::Criterion criterion) const override
    {
      getSortedTasksCallCount++;
      lastSortedInput = tasks;
      lastCriterion = criterion;
      return tasks;
    }

    void saveToFile() noexcept override
    {}

    void loadFromFile() noexcept override
    {}

    void saveGamificationData() noexcept override
    {}

    void loadGamificationData() noexcept override
    {}

    storage::UserProgress getUserProgress() const override
    {
      return m_progress;
    }

    void updateUserProgress(const storage::UserProgress &progress) override
    {
      m_progress = progress;
    }

    void addXP(int amount, const QString &reason) override
    {
      Q_UNUSED(reason);
      m_progress.currentXP += amount;
    }

    void updateStreak(const QDate &currentDate) override
    {
      Q_UNUSED(currentDate);
    }

    int getCurrentLevel() const override
    {
      return m_progress.currentLevel;
    }

    int getTotalXP() const override
    {
      return m_progress.currentXP;
    }

    int getStreakDays() const override
    {
      return m_progress.streakDays;
    }

    QList< storage::Achievement > getAllAchievements() const override
    {
    return allAchievementsToReturn;
    }

    void unlockAchievement(const QString &achievementId) override
    {
      Q_UNUSED(achievementId);
    }

    bool isAchievementUnlocked(const QString &achievementId) const override
    {
      return m_progress.unlockedAchievementIds.contains(achievementId);
    }

    storage::Achievement getAchievementById(const QString &id) const override
    {
      for (const auto &a : allAchievementsToReturn)
      {
        if (a.id == id)
        {
          return a;
        }
      }
      return {};
    }

    QList< QString > getUnlockedLocations() const override
    {
      return m_progress.unlockedLocations;
    }

    void unlockLocation(const QString &locationId) override
    {
      Q_UNUSED(locationId);
    }

    bool isLocationUnlocked(const QString &locationId) const override
    {
      Q_UNUSED(locationId);
      return false;
    }

    int getCompletedTasksCount() const override
    {
      int count = 0;
      for (const auto &task : m_tasks)
      {
        if (task.completed)
        {
          ++count;
        }
      }
      return count;
    }

    int getOnTimeCompletedCount() const override
    {
      return 0;
    }

    int getCompletedCountByPriority(storage::Priority priority) const override
    {
      int count = 0;
      for (const auto &task : m_tasks)
      {
        if (task.completed && task.priority == priority) ++count;
      }
      return count;
    }

    int getDeletedTasksCount() const override
    {
      return 0;
    }

    int getPerfectDaysCount() const override
    {
      return 0;
    }

    int getTotalLocationsCount() const override
    {
      return 0;
    }

    int getMaxTasksCompletedInOneDay() const override
    {
      return 0;
    }

    int getMaxHardTasksCompletedInOneDay() const override
    {
      return 0;
    }

    void setTasks(const QList< storage::Task > &tasks)
    {
      m_tasks = tasks;
    }

    void setProgress(const storage::UserProgress &progress)
    {
      m_progress = progress;
    }

    void setAllAchievements(const QList< storage::Achievement > &achievements)
    {
      allAchievementsToReturn = achievements;
    }


    mutable int addTaskCallCount = 0;
    mutable int removeTaskCallCount = 0;
    mutable int updateTaskCallCount = 0;
    mutable int getAllTasksCallCount = 0;
    mutable int getTasksForDateCallCount = 0;
    mutable int getTasksForTodayCallCount = 0;
    mutable int getOverdueTasksCallCount = 0;
    mutable int getTasksFilteredCallCount = 0;
    mutable int getSortedTasksCallCount = 0;

    storage::Task lastAddedTask;
    storage::Task lastUpdatedTask;
    int lastRemovedId = -1;
    mutable QDate lastQueriedDate;
    mutable QString lastSearchText;
    mutable bool lastTodayFlag = false;
    mutable bool lastOverdueFlag = false;
    mutable storage::Priority lastPriorityArg = storage::Priority::All;
    mutable QList< storage::Task > lastSortedInput;
    mutable storage::Criterion lastCriterion = storage::Criterion::Date;

    QList< storage::Task > tasksForTodayToReturn;
    QList< storage::Task > overdueTasksToReturn;
    QList< storage::Task > filteredTasksToReturn;
    QList< storage::Achievement > allAchievementsToReturn;

  private:
    QList< storage::Task > m_tasks;
    storage::UserProgress m_progress{};
  };
}

#endif
