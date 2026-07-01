#ifndef MOCKVIEW_HPP
#define MOCKVIEW_HPP

#include "iview.hpp"

namespace test
{
  class MockView: public view::IView
  {
  public:
    MockView() = default;
    ~MockView() override = default;

    void showTaskList(const QList< storage::Task > &tasks) override
    {
      showTaskListCallCount++;
      lastShownTasks = tasks;
    }

    void showTasksForDate(const QDate &date, const QList< storage::Task > &tasks) override
    {
      showTasksForDateCallCount++;
      lastDateArg = date;
      lastShownTasks = tasks;
    }

    void setTaskListTitle(const QString &title) override
    {
      setTaskListTitleCallCount++;
      lastTitle = title;
    }

    void showTaskCreationForm() override
    {
      showTaskCreationFormCallCount++;
      lastFormHadTask = false;
    }

    void showTaskCreationForm(const storage::Task &task) override
    {
      showTaskCreationFormCallCount++;
      lastFormHadTask = true;
      lastFormTask = task;
    }

    void closeTaskCreationForm() override
    {
      closeTaskCreationFormCallCount++;
    }

    void showErrorMessage(const QString &message) override
    {
      showErrorMessageCallCount++;
      lastErrorMessage = message;
    }

    void showInfoMessage(const QString &message) override
    {
      showInfoMessageCallCount++;
      lastInfoMessage = message;
    }

    void updateStats(int total, int completed, int today) override
    {
      updateStatsCallCount++;
      lastStatsTotal = total;
      lastStatsCompleted = completed;
      lastStatsToday = today;
    }

    void showUserLevel(int level, int currentXP, int xpToNext) override
    {
      showUserLevelCallCount++;
      lastLevel = level;
      lastCurrentXP = currentXP;
      lastXpToNext = xpToNext;
    }

    void showStreak(int days) override
    {
      showStreakCallCount++;
      lastStreakDays = days;
    }

    void showUserTitle(const QString &title) override
    {
      showUserTitleCallCount++;
      lastUserTitle = title;
    }

    void showXPNotification(int amount, const QString &reason) override
    {
      showXPNotificationCallCount++;
      lastXPAmount = amount;
      lastXPReason = reason;
    }

    void showAchievementUnlocked(const storage::Achievement &achievement) override
    {
      showAchievementUnlockedCallCount++;
      lastUnlockedAchievement = achievement;
      unlockedAchievementHistory.append(achievement);
    }

    void showAchievementsList(const QList< storage::Achievement > &achievements) override
    {
      showAchievementsListCallCount++;
      lastAchievementsList = achievements;
    }

    void showCampusMap(const QList< QString > &unlockedLocations) override
    {
      showCampusMapCallCount++;
      lastUnlockedLocations = unlockedLocations;
    }

    void showLocationUnlocked(const QString &locationName) override
    {
      showLocationUnlockedCallCount++;
      lastUnlockedLocationName = locationName;
    }

    void showLevelUpAnimation(int newLevel, const QString &newTitle) override
    {
      showLevelUpAnimationCallCount++;
      lastNewLevel = newLevel;
      lastNewTitle = newTitle;
    }

    void updateGamificationPanel() override
    {
      updateGamificationPanelCallCount++;
    }

    int showTaskListCallCount = 0;
    int showTasksForDateCallCount = 0;
    int setTaskListTitleCallCount = 0;
    int showTaskCreationFormCallCount = 0;
    int closeTaskCreationFormCallCount = 0;
    int showErrorMessageCallCount = 0;
    int showInfoMessageCallCount = 0;
    int updateStatsCallCount = 0;

    int showUserLevelCallCount = 0;
    int showStreakCallCount = 0;
    int showUserTitleCallCount = 0;
    int showXPNotificationCallCount = 0;
    int showAchievementUnlockedCallCount = 0;
    int showAchievementsListCallCount = 0;
    int showCampusMapCallCount = 0;
    int showLocationUnlockedCallCount = 0;
    int showLevelUpAnimationCallCount = 0;
    int updateGamificationPanelCallCount = 0;

    QList< storage::Task > lastShownTasks;
    QDate lastDateArg;
    QString lastTitle;
    bool lastFormHadTask = false;
    storage::Task lastFormTask;
    QString lastErrorMessage;
    QString lastInfoMessage;
    int lastStatsTotal = 0;
    int lastStatsCompleted = 0;
    int lastStatsToday = 0;

    int lastLevel = 0;
    int lastCurrentXP = 0;
    int lastXpToNext = 0;
    int lastStreakDays = 0;
    QString lastUserTitle;
    int lastXPAmount = 0;
    QString lastXPReason;
    storage::Achievement lastUnlockedAchievement;
    QList< storage::Achievement > lastAchievementsList;
    QList< QString > lastUnlockedLocations;
    QString lastUnlockedLocationName;
    int lastNewLevel = 0;
    QString lastNewTitle;

    QList< storage::Achievement > unlockedAchievementHistory;
  };
}

#endif
