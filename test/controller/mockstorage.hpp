#ifndef MOCKSTORAGE_HPP
#define MOCKSTORAGE_HPP

#include "istorage.hpp"
#include <QList>
#include <QString>
#include <QMap>
#include <QSet>

namespace test
{
  class MockStorage: public storage::IStorage
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
      for (auto &existing: m_tasks)
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
      for (const auto &task: m_tasks)
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

    QList< storage::Task > getTasksFiltered(const QString &searchText, bool today, bool overdue,
      storage::Priority priority) const override
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

    storage::UserProgress getUserProgress() const override
    {
      storage::UserProgress progress;
      progress.currentLevel = currentLevelToReturn;
      progress.currentXP = currentXPToReturn;
      const int xpForCurrent = storage::calculateTotalXPForLevel(currentLevelToReturn);
      const int xpForNext = storage::calculateTotalXPForLevel(currentLevelToReturn + 1);
      progress.xpToNextLevel = xpForNext - xpForCurrent;
      progress.streakDays = streakDaysToReturn;
      progress.lastActivityDate = QDate::currentDate();
      progress.currentTitle = "Тестовое звание";
      progress.unlockedAchievementIds = unlockedIds.values();
      progress.unlockedLocations = unlockedLocationsToReturn;
      return progress;
    }

    void updateUserProgress(const storage::UserProgress &progress) override
    {
      updateUserProgressCallCount++;
      userProgressToReturn = progress;
    }

    void addXP(int amount, const QString &reason) override
    {
      addXPCallCount++;
      lastXPAmount = amount;
      lastXPReason = reason;
      totalXP += amount;
      currentXPToReturn += amount;
      const int newLevel = storage::calculateLevelFromXP(totalXP);
      if (newLevel > currentLevelToReturn)
      {
        currentLevelToReturn = newLevel;
        const int xpForCurrent = storage::calculateTotalXPForLevel(currentLevelToReturn);
        currentXPToReturn = totalXP - xpForCurrent;
      }
      else
      {
        const int xpForCurrent = storage::calculateTotalXPForLevel(currentLevelToReturn);
        currentXPToReturn = totalXP - xpForCurrent;
      }
    }

    void updateStreak(const QDate &currentDate) override
    {
      updateStreakCallCount++;
      lastStreakDate = currentDate;
    }

    int getCurrentLevel() const override
    {
      getCurrentLevelCallCount++;
      return currentLevelToReturn;
    }

    int getTotalXP() const override
    {
      getTotalXPCallCount++;
      return totalXP;
    }

    int getStreakDays() const override
    {
      return streakDaysToReturn;
    }

    QList< storage::Achievement > getAllAchievements() const override
    {
      getAllAchievementsCallCount++;
      return allAchievementsToReturn;
    }

    void unlockAchievement(const QString &achievementId) override
    {
      unlockAchievementCallCount++;
      lastUnlockedAchievementId = achievementId;
      unlockedIds.insert(achievementId);
    }

    bool isAchievementUnlocked(const QString &achievementId) const override
    {
      return unlockedIds.contains(achievementId);
    }

    storage::Achievement getAchievementById(const QString &id) const override
    {
      for (const auto &a: allAchievementsToReturn)
      {
        if (a.id == id)
        {
          return a;
        }
      }
      return storage::Achievement();
    }

    QList< QString > getUnlockedLocations() const override
    {
      return unlockedLocationsToReturn;
    }

    void unlockLocation(const QString &locationId) override
    {
      unlockLocationCallCount++;
      unlockedLocationsToReturn.append(locationId);
    }

    bool isLocationUnlocked(const QString &locationId) const override
    {
      return unlockedLocationsToReturn.contains(locationId);
    }

    int getCompletedTasksCount() const override
    {
      return completedTasksCountToReturn;
    }

    int getOnTimeCompletedCount() const override
    {
      return onTimeCompletedCountToReturn;
    }

    int getCompletedCountByPriority(storage::Priority priority) const override
    {
      return completedCountByPriorityToReturn.value(priority, 0);
    }

    int getDeletedTasksCount() const override
    {
      return deletedTasksCountToReturn;
    }

    int getPerfectDaysCount() const override
    {
      return perfectDaysCountToReturn;
    }

    int getTotalLocationsCount() const override
    {
      return totalLocationsCountToReturn;
    }

    int getMaxTasksCompletedInOneDay() const override
    {
      return maxTasksCompletedInOneDayToReturn;
    }

    int getMaxHardTasksCompletedInOneDay() const override
    {
      return maxHardTasksCompletedInOneDayToReturn;
    }

    void setTasks(const QList< storage::Task > &tasks)
    {
      m_tasks = tasks;
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
    int updateUserProgressCallCount = 0;
    int addXPCallCount = 0;
    int updateStreakCallCount = 0;
    mutable int getCurrentLevelCallCount = 0;
    mutable int getTotalXPCallCount = 0;
    mutable int getAllAchievementsCallCount = 0;
    int unlockAchievementCallCount = 0;
    int unlockLocationCallCount = 0;

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
    int lastXPAmount = 0;
    QString lastXPReason;
    QDate lastStreakDate;
    QString lastUnlockedAchievementId;

    QList< storage::Task > tasksForTodayToReturn;
    QList< storage::Task > overdueTasksToReturn;
    QList< storage::Task > filteredTasksToReturn;
    storage::UserProgress userProgressToReturn{};
    int currentLevelToReturn = 1;
    int currentXPToReturn = 0;
    int totalXP = 0;
    int streakDaysToReturn = 0;
    QList< storage::Achievement > allAchievementsToReturn;
    QSet< QString > unlockedIds;
    QList< QString > unlockedLocationsToReturn;
    int completedTasksCountToReturn = 0;
    int onTimeCompletedCountToReturn = 0;
    QMap< storage::Priority, int > completedCountByPriorityToReturn;
    int deletedTasksCountToReturn = 0;
    int perfectDaysCountToReturn = 0;
    int totalLocationsCountToReturn = 0;
    int maxTasksCompletedInOneDayToReturn = 0;
    int maxHardTasksCompletedInOneDayToReturn = 0;

  private:
    QList< storage::Task > m_tasks;

    void saveToFile() noexcept override {}
    void loadFromFile() noexcept override {}
    void saveGamificationData() noexcept override {}
    void loadGamificationData() noexcept override {}
  };
}

#endif
