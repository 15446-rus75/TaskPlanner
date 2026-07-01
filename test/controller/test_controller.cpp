#include <QtTest>
#include <QSignalSpy>
#include "../../src/controller/controller.hpp"
#include "mockstorage.hpp"
#include "mockview.hpp"

class ControllerTest: public QObject
{
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  void setStorage_nullptr_doesNotCrash();
  void setView_nullptr_doesNotCrash();
  void start_withoutDependencies_doesNotCrash();

  void onViewReady_showsAllTasksByDefault();
  void onViewReady_withoutDependencies_doesNotCrash();

  void onTaskAddRequested_validTask_addsToStorage();
  void onTaskAddRequested_emptyName_rejectsAndShowsError();
  void onTaskAddRequested_invalidDeadline_rejectsAndShowsError();
  void onTaskAddRequested_validTask_refreshesView();

  void onTaskEditRequested_existingTask_showsForm();
  void onTaskEditRequested_missingTask_showsError();

  void onTaskUpdateRequested_validTask_updatesStorage();
  void onTaskUpdateRequested_emptyName_rejectsAndShowsError();

  void onTaskDeleteRequested_existingTask_removesAndNotifies();
  void onTaskDeleteRequested_missingTask_stillRemovesQuietly();

  void onCompleteRequested_existingTask_togglesCompletion();
  void onCompleteRequested_missingTask_showsError();
  void onCompleteRequested_calledTwice_togglesBackAndForth();

  void onDateSelected_firstClick_setsTitleToSelectedDate();
  void onDateSelected_sameDataTwice_deselectsDate();
  void onDateSelected_ignoresScopeFilter();

  void onSortRequested_changesCriterionAndRefreshes();

  void onFilterChanged_showAll_setsCorrectTitle();
  void onFilterChanged_showToday_setsCorrectTitle();
  void onFilterChanged_showOverdue_setsCorrectTitle();
  void onFilterChanged_scopeFilter_resetsSelectedDate();

  void onFilterChanged_priorityFilter_doesNotResetScope();
  void onFilterChanged_priorityFilter_doesNotResetSelectedDate();
  void onFilterChanged_priorityAll_showsEverything();
  void onFilterChanged_priorityLow_filtersOnlyLowTasks();

  void onFilterChanged_search_resetsDateAndScope();
  void onFilterChanged_search_callsGetTasksFilteredWithText();

  void combination_scopeTodayPlusPriorityLow_bothApply();
  void combination_dateSelectedPlusPriority_bothApply();
  void combination_dateSelectedPlusScope_scopeIgnored();

  void xp_lowPriorityOnTime_correctAmount();
  void xp_mediumPriorityOnTime_correctAmount();
  void xp_hardPriorityOnTime_correctAmount();
  void xp_overdueTask_bonusDecaysWithHours();
  void xp_veryOverdueTask_bonusIsZero();
  void xp_notCompletedTask_noXP();
  void xp_invalidPriorityAll_noBaseXP();

  void xp_unmarking_doesNotGrantXP();

  void levelUp_crossesBoundary_showsAnimation();
  void levelUp_noChange_noAnimation();
  void levelUp_atMaxLevel_noAnimation();

  void achievements_level5_unlocksWhenReached();
  void achievements_level5_notUnlockedBelow();
  void achievements_alreadyUnlocked_notUnlockedAgain();
  void achievements_multipleInOneCheck_allUnlocked();

  void achievements_tasks10_unlocksAtThreshold();
  void achievements_tasks10_notUnlockedBelow();
  void achievements_tasks250_unlocksAtThreshold();

  void achievements_onTime5_unlocksAtThreshold();
  void achievements_onTime50_notUnlockedBelow();

  void achievements_hard10_unlocksAtThreshold();
  void achievements_balancedAll_requiresAllThree();
  void achievements_balancedAll_notUnlockedIfOneMissing();

  void achievements_streak7_unlocksAtThreshold();
  void achievements_streak100_notUnlockedBelow();

  void achievements_perfectDay1_unlocksAtThreshold();

  void achievements_delete5_unlocksAtThreshold();
  void achievements_delete20_notUnlockedBelow();

  void achievements_location5_unlocksAtThreshold();
  void achievements_locationAll_unlocksWhenComplete();
  void achievements_locationAll_notUnlockedIfMissing();

  void achievements_comboNightmare_unlocksAtThreeHardPerDay();
  void achievements_comboMarathon_unlocksAtFifteenPerDay();
  void achievements_comboMarathon_notUnlockedBelow();

  void achievements_xpReward_grantedOnUnlock();
  void achievements_zeroXpReward_noXpGranted();

  void complete_triggersXpAndAchievementCheck();
  void complete_streakUpdated();

  void dailyTasksCompleted_grantsPerfectDayBonus();
  void dailyTasksCompleted_checksAchievements();

  void newDay_updatesStreak();
  void newDay_showsStreak();
  void newDay_checksAchievements();

  void applicationStart_showsCurrentLevel();
  void applicationStart_triggersNewDay();

  void achievementsRequested_showsFullList();

  void mapRequested_showsUnlockedLocations();

  void statisticsRequested_updatesStats();

  void delete_checksAchievementsAfterRemoval();

private:
  storage::Task makeTask(int id, const QString &name, storage::Priority priority,
                         const QDateTime &deadline, bool completed = false) const;
  storage::Achievement makeAchievement(const QString &id, int xpReward = 0) const;

  test::MockStorage *m_storage = nullptr;
  test::MockView *m_view = nullptr;
  controller::Controller *m_controller = nullptr;
};

void ControllerTest::init()
{
  m_storage = new test::MockStorage();
  m_view = new test::MockView();
  m_controller = new controller::Controller();
  m_controller->setStorage(m_storage);
  m_controller->setView(m_view);
}

void ControllerTest::cleanup()
{
  delete m_controller;
  delete m_view;
  delete m_storage;
  m_controller = nullptr;
  m_view = nullptr;
  m_storage = nullptr;
}

storage::Task ControllerTest::makeTask(int id, const QString &name, storage::Priority priority,
                                       const QDateTime &deadline, bool completed) const
{
  storage::Task task;
  task.id = id;
  task.name = name;
  task.description = "test description";
  task.discipline = "test discipline";
  task.deadline = deadline;
  task.priority = priority;
  task.completed = completed;
  return task;
}

storage::Achievement ControllerTest::makeAchievement(const QString &id, int xpReward) const
{
  storage::Achievement achievement;
  achievement.id = id;
  achievement.name = id;
  achievement.description = id;
  achievement.iconPath = "";
  achievement.xpReward = xpReward;
  achievement.type = "test";
  return achievement;
}

void ControllerTest::setStorage_nullptr_doesNotCrash()
{
  controller::Controller controller;
  controller.setStorage(nullptr);
  controller.onViewReady();
  QCOMPARE(m_view->showTaskListCallCount, 0);
}

void ControllerTest::setView_nullptr_doesNotCrash()
{
  controller::Controller controller;
  controller.setStorage(m_storage);
  controller.setView(nullptr);
  controller.onViewReady();
  QCOMPARE(m_storage->getAllTasksCallCount, 0);
}

void ControllerTest::start_withoutDependencies_doesNotCrash()
{
  controller::Controller controller;
  controller.start();
  QVERIFY(true);
}

void ControllerTest::onViewReady_showsAllTasksByDefault()
{
  m_storage->setTasks({ makeTask(1, "Task 1", storage::Priority::Low, QDateTime::currentDateTime().addDays(1)) });

  m_controller->onViewReady();

  QCOMPARE(m_storage->getAllTasksCallCount, 2);
  QCOMPARE(m_view->showTaskListCallCount, 1);
  QCOMPARE(m_view->lastShownTasks.size(), 1);
  QCOMPARE(m_view->lastTitle, QString("Все задачи"));
}

void ControllerTest::onViewReady_withoutDependencies_doesNotCrash()
{
  controller::Controller controller;
  controller.onViewReady();
  QVERIFY(true);
}

void ControllerTest::onTaskAddRequested_validTask_addsToStorage()
{
  const storage::Task task = makeTask(1, "New task", storage::Priority::Medium, QDateTime::currentDateTime().addDays(2));

  m_controller->onTaskAddRequested(task);

  QCOMPARE(m_storage->addTaskCallCount, 1);
  QCOMPARE(m_storage->lastAddedTask.name, QString("New task"));
  QCOMPARE(m_view->showInfoMessageCallCount, 1);
}

void ControllerTest::onTaskAddRequested_emptyName_rejectsAndShowsError()
{
  const storage::Task task = makeTask(1, "", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));

  m_controller->onTaskAddRequested(task);

  QCOMPARE(m_storage->addTaskCallCount, 0);
  QCOMPARE(m_view->showErrorMessageCallCount, 1);
}

void ControllerTest::onTaskAddRequested_invalidDeadline_rejectsAndShowsError()
{
  const storage::Task task = makeTask(1, "Task without deadline", storage::Priority::Low, QDateTime());

  m_controller->onTaskAddRequested(task);

  QCOMPARE(m_storage->addTaskCallCount, 0);
  QCOMPARE(m_view->showErrorMessageCallCount, 1);
}

void ControllerTest::onTaskAddRequested_validTask_refreshesView()
{
  const storage::Task task = makeTask(1, "Task", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));

  m_controller->onTaskAddRequested(task);

  QCOMPARE(m_view->showTaskListCallCount, 1);
  QCOMPARE(m_view->updateStatsCallCount, 1);
}

void ControllerTest::onTaskEditRequested_existingTask_showsForm()
{
  const storage::Task task = makeTask(42, "Editable", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });

  m_controller->onTaskEditRequested(42);

  QCOMPARE(m_view->showTaskCreationFormCallCount, 1);
  QVERIFY(m_view->lastFormHadTask);
  QCOMPARE(m_view->lastFormTask.id, 42);
}

void ControllerTest::onTaskEditRequested_missingTask_showsError()
{
  m_storage->setTasks({});

  m_controller->onTaskEditRequested(999);

  QCOMPARE(m_view->showTaskCreationFormCallCount, 0);
  QCOMPARE(m_view->showErrorMessageCallCount, 1);
}

void ControllerTest::onTaskUpdateRequested_validTask_updatesStorage()
{
  const storage::Task task = makeTask(1, "Updated", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1));

  m_controller->onTaskUpdateRequested(task);

  QCOMPARE(m_storage->updateTaskCallCount, 1);
  QCOMPARE(m_storage->lastUpdatedTask.name, QString("Updated"));
}

void ControllerTest::onTaskUpdateRequested_emptyName_rejectsAndShowsError()
{
  const storage::Task task = makeTask(1, "   ", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1));

  m_controller->onTaskUpdateRequested(task);

  QCOMPARE(m_storage->updateTaskCallCount, 0);
  QCOMPARE(m_view->showErrorMessageCallCount, 1);
}

void ControllerTest::onTaskDeleteRequested_existingTask_removesAndNotifies()
{
  const storage::Task task = makeTask(7, "To delete", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });

  m_controller->onTaskDeleteRequested(7);

  QCOMPARE(m_storage->removeTaskCallCount, 1);
  QCOMPARE(m_storage->lastRemovedId, 7);
  QCOMPARE(m_view->showInfoMessageCallCount, 1);
}

void ControllerTest::onTaskDeleteRequested_missingTask_stillRemovesQuietly()
{
  m_storage->setTasks({});

  m_controller->onTaskDeleteRequested(123);

  QCOMPARE(m_storage->removeTaskCallCount, 1);
  QCOMPARE(m_view->showInfoMessageCallCount, 1);
}

void ControllerTest::onCompleteRequested_existingTask_togglesCompletion()
{
  const storage::Task task = makeTask(3, "Toggle me", storage::Priority::Low, QDateTime::currentDateTime().addDays(1), false);
  m_storage->setTasks({ task });

  m_controller->onCompleteRequested(3);

  QCOMPARE(m_storage->updateTaskCallCount, 1);
  QVERIFY(m_storage->lastUpdatedTask.completed);
}

void ControllerTest::onCompleteRequested_missingTask_showsError()
{
  m_storage->setTasks({});

  m_controller->onCompleteRequested(999);

  QCOMPARE(m_storage->updateTaskCallCount, 0);
  QCOMPARE(m_view->showErrorMessageCallCount, 1);
}

void ControllerTest::onCompleteRequested_calledTwice_togglesBackAndForth()
{
  const storage::Task task = makeTask(3, "Toggle twice", storage::Priority::Low, QDateTime::currentDateTime().addDays(1), false);
  m_storage->setTasks({ task });

  m_controller->onCompleteRequested(3);
  QVERIFY(m_storage->lastUpdatedTask.completed);

  m_controller->onCompleteRequested(3);
  QVERIFY(!m_storage->lastUpdatedTask.completed);
}

void ControllerTest::onDateSelected_firstClick_setsTitleToSelectedDate()
{
  const QDate date(2026, 6, 25);

  m_controller->onDateSelected(date);

  QCOMPARE(m_storage->getTasksForDateCallCount, 1);
  QCOMPARE(m_storage->lastQueriedDate, date);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + date.toString("dd.MM.yyyy")));
}

void ControllerTest::onDateSelected_sameDataTwice_deselectsDate()
{
  const QDate date(2026, 6, 25);

  m_controller->onDateSelected(date);
  QCOMPARE(m_storage->getTasksForDateCallCount, 1);

  m_controller->onDateSelected(date);

  QCOMPARE(m_view->lastTitle, QString("Все задачи"));
  QCOMPARE(m_storage->getAllTasksCallCount, 3);
}

void ControllerTest::onDateSelected_ignoresScopeFilter()
{
  m_controller->onFilterChanged(storage::Filter::ShowOverdue, QVariant());
  QCOMPARE(m_view->lastTitle, QString("Просроченные задачи"));

  const QDate date(2026, 7, 1);
  m_controller->onDateSelected(date);

  QCOMPARE(m_storage->getTasksForDateCallCount, 1);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + date.toString("dd.MM.yyyy")));
}

void ControllerTest::onSortRequested_changesCriterionAndRefreshes()
{
  m_controller->onSortRequested(storage::Criterion::Priority);

  QCOMPARE(m_storage->getSortedTasksCallCount, 1);
  QCOMPARE(m_storage->lastCriterion, storage::Criterion::Priority);
}

void ControllerTest::onFilterChanged_showAll_setsCorrectTitle()
{
  m_controller->onFilterChanged(storage::Filter::ShowAll, QVariant());
  QCOMPARE(m_view->lastTitle, QString("Все задачи"));
  QCOMPARE(m_storage->getAllTasksCallCount, 2);
}

void ControllerTest::onFilterChanged_showToday_setsCorrectTitle()
{
  m_controller->onFilterChanged(storage::Filter::ShowToday, QVariant());
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + QDate::currentDate().toString("dd.MM.yyyy")));
  QCOMPARE(m_storage->getTasksForTodayCallCount, 2);
}

void ControllerTest::onFilterChanged_showOverdue_setsCorrectTitle()
{
  m_controller->onFilterChanged(storage::Filter::ShowOverdue, QVariant());
  QCOMPARE(m_view->lastTitle, QString("Просроченные задачи"));
  QCOMPARE(m_storage->getOverdueTasksCallCount, 1);
}

void ControllerTest::onFilterChanged_scopeFilter_resetsSelectedDate()
{
  m_controller->onDateSelected(QDate(2026, 6, 25));
  QCOMPARE(m_storage->getTasksForDateCallCount, 1);
  QCOMPARE(m_storage->getTasksForTodayCallCount, 1);

  m_controller->onFilterChanged(storage::Filter::ShowToday, QVariant());

  QCOMPARE(m_storage->getTasksForTodayCallCount, 3);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + QDate::currentDate().toString("dd.MM.yyyy")));
  QCOMPARE(m_storage->getTasksForDateCallCount, 1);
}

void ControllerTest::onFilterChanged_priorityFilter_doesNotResetScope()
{
  m_controller->onFilterChanged(storage::Filter::ShowToday, QVariant());
  QCOMPARE(m_storage->getTasksForTodayCallCount, 2);
  const int allTasksAfterFirst = m_storage->getAllTasksCallCount;

  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::Hard));

  QCOMPARE(m_storage->getTasksForTodayCallCount, 4);
  QCOMPARE(m_storage->getAllTasksCallCount, allTasksAfterFirst + 1);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + QDate::currentDate().toString("dd.MM.yyyy")));
}

void ControllerTest::onFilterChanged_priorityFilter_doesNotResetSelectedDate()
{
  const QDate date(2026, 6, 25);
  m_controller->onDateSelected(date);
  QCOMPARE(m_storage->getTasksForDateCallCount, 1);

  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::Low));

  QCOMPARE(m_storage->getTasksForDateCallCount, 2);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + date.toString("dd.MM.yyyy")));
}

void ControllerTest::onFilterChanged_priorityAll_showsEverything()
{
  m_storage->setTasks({ makeTask(1, "Low", storage::Priority::Low, QDateTime::currentDateTime().addDays(1)),
    makeTask(2, "Medium", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1)),
    makeTask(3, "Hard", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1)),
  });

  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::All));

  QCOMPARE(m_view->lastShownTasks.size(), 3);
}

void ControllerTest::onFilterChanged_priorityLow_filtersOnlyLowTasks()
{
  m_storage->setTasks({
    makeTask(1, "Low", storage::Priority::Low, QDateTime::currentDateTime().addDays(1)),
    makeTask(2, "Medium", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1)),
    makeTask(3, "Hard", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1)),
  });

  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::Low));

  QCOMPARE(m_view->lastShownTasks.size(), 1);
  QCOMPARE(m_view->lastShownTasks.first().id, 1);
}

void ControllerTest::onFilterChanged_search_resetsDateAndScope()
{
  m_controller->onDateSelected(QDate(2026, 6, 25));
  m_controller->onFilterChanged(storage::Filter::ShowOverdue, QVariant());

  m_storage->filteredTasksToReturn = { makeTask(1, "Found", storage::Priority::Low, QDateTime::currentDateTime().addDays(1)) };

  m_controller->onFilterChanged(storage::Filter::Search, QVariant("найти"));

  QCOMPARE(m_storage->getTasksFilteredCallCount, 1);
  QCOMPARE(m_storage->lastSearchText, QString("найти"));

  const int todayCountBefore = m_storage->getTasksForTodayCallCount;
  const int dateCountBefore = m_storage->getTasksForDateCallCount;

  m_controller->onFilterChanged(storage::Filter::ShowToday, QVariant());

  QCOMPARE(m_storage->getTasksForTodayCallCount, todayCountBefore + 2);
  QCOMPARE(m_storage->getTasksForDateCallCount, dateCountBefore);
}

void ControllerTest::onFilterChanged_search_callsGetTasksFilteredWithText()
{
  m_storage->filteredTasksToReturn = {};

  m_controller->onFilterChanged(storage::Filter::Search, QVariant("экзамен"));

  QCOMPARE(m_storage->getTasksFilteredCallCount, 1);
  QCOMPARE(m_storage->lastSearchText, QString("экзамен"));
  QCOMPARE(m_view->showTaskListCallCount, 1);
}

void ControllerTest::combination_scopeTodayPlusPriorityLow_bothApply()
{
  m_storage->tasksForTodayToReturn = {
    makeTask(1, "Today Low", storage::Priority::Low, QDateTime::currentDateTime()),
    makeTask(2, "Today Hard", storage::Priority::Hard, QDateTime::currentDateTime()),
  };

  m_controller->onFilterChanged(storage::Filter::ShowToday, QVariant());
  const int todayCountAfterFirst = m_storage->getTasksForTodayCallCount;

  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::Low));

  QCOMPARE(m_storage->getTasksForTodayCallCount, todayCountAfterFirst + 2);
  QCOMPARE(m_view->lastShownTasks.size(), 1);
  QCOMPARE(m_view->lastShownTasks.first().id, 1);
}

void ControllerTest::combination_dateSelectedPlusPriority_bothApply()
{
  const QDate date(2026, 6, 25);
  m_storage->setTasks({
    makeTask(1, "On date, Low", storage::Priority::Low, QDateTime(date, QTime(10, 0))),
    makeTask(2, "On date, Hard", storage::Priority::Hard, QDateTime(date, QTime(11, 0))),
  });

  m_controller->onDateSelected(date);
  m_controller->onFilterChanged(storage::Filter::Priority, QVariant::fromValue(storage::Priority::Hard));

  QCOMPARE(m_view->lastShownTasks.size(), 1);
  QCOMPARE(m_view->lastShownTasks.first().id, 2);
  QCOMPARE(m_view->lastTitle, QString("Задачи на " + date.toString("dd.MM.yyyy")));
}

void ControllerTest::combination_dateSelectedPlusScope_scopeIgnored()
{
  const QDate date(2026, 6, 25);

  m_controller->onDateSelected(date);
  QCOMPARE(m_storage->getTasksForDateCallCount, 1);

  m_controller->onFilterChanged(storage::Filter::ShowOverdue, QVariant());

  QCOMPARE(m_storage->getOverdueTasksCallCount, 1);
  QCOMPARE(m_view->lastTitle, QString("Просроченные задачи"));
}

void ControllerTest::xp_lowPriorityOnTime_correctAmount()
{
  const storage::Task task = makeTask(1, "Low", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->lastXPAmount, 35);
}

void ControllerTest::xp_mediumPriorityOnTime_correctAmount()
{
  const storage::Task task = makeTask(1, "Medium", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->lastXPAmount, 50);
}

void ControllerTest::xp_hardPriorityOnTime_correctAmount()
{
  const storage::Task task = makeTask( 1, "Hard", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->lastXPAmount, 75);
}

void ControllerTest::xp_overdueTask_bonusDecaysWithHours()
{
  const storage::Task task = makeTask(1, "Overdue 3h", storage::Priority::Low, QDateTime::currentDateTime().addSecs(-3 * 3600 - 60));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->lastXPAmount, 29);
}

void ControllerTest::xp_veryOverdueTask_bonusIsZero()
{
  const storage::Task task = makeTask(1, "Very overdue", storage::Priority::Low, QDateTime::currentDateTime().addDays(-30));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->lastXPAmount, 10);
}

void ControllerTest::xp_notCompletedTask_noXP()
{
  m_storage->setTasks({});

  m_controller->onCalculateXP(999);

  QCOMPARE(m_storage->addXPCallCount, 0);
}

void ControllerTest::xp_invalidPriorityAll_noBaseXP()
{
  const storage::Task task = makeTask(1, "Invalid", storage::Priority::All, QDateTime::currentDateTime().addDays(-365));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->addXPCallCount, 0);
}

void ControllerTest::xp_unmarking_doesNotGrantXP()
{
  const storage::Task task = makeTask(1, "Done", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1), true);
  m_storage->setTasks({ task });

  m_controller->onCompleteRequested(1);

  QVERIFY(!m_storage->lastUpdatedTask.completed);
  QCOMPARE(m_storage->addXPCallCount, 0);
}

void ControllerTest::levelUp_crossesBoundary_showsAnimation()
{
  m_storage->currentLevelToReturn = 1;
  const int xpForLevel2 = storage::calculateTotalXPForLevel(2);
  m_storage->totalXP = xpForLevel2 - 5;

  const storage::Task task = makeTask(1, "Level pusher", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_view->showLevelUpAnimationCallCount, 1);
  QCOMPARE(m_view->lastNewLevel, 2);
}

void ControllerTest::levelUp_noChange_noAnimation()
{
  m_storage->currentLevelToReturn = 1;
  m_storage->totalXP = 0;

  const storage::Task task = makeTask(1, "Small task", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_view->showLevelUpAnimationCallCount, 0);
}

void ControllerTest::levelUp_atMaxLevel_noAnimation()
{
  m_storage->currentLevelToReturn = storage::xp::MAX_LEVEL;
  m_storage->totalXP = storage::calculateTotalXPForLevel(storage::xp::MAX_LEVEL) + 100;

  const storage::Task task = makeTask(1, "Max level task", storage::Priority::Hard, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_view->showLevelUpAnimationCallCount, 0);
  QCOMPARE(m_view->lastUserTitle, QString("Максимальный уровень"));
}

void ControllerTest::achievements_level5_unlocksWhenReached()
{
  m_storage->currentLevelToReturn = 5;
  m_storage->allAchievementsToReturn = { makeAchievement("level_5", 250) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
  QCOMPARE(m_storage->lastUnlockedAchievementId, QString("level_5"));
}

void ControllerTest::achievements_level5_notUnlockedBelow()
{
  m_storage->currentLevelToReturn = 4;
  m_storage->allAchievementsToReturn = { makeAchievement("level_5", 250) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_alreadyUnlocked_notUnlockedAgain()
{
  m_storage->currentLevelToReturn = 10;
  m_storage->allAchievementsToReturn = { makeAchievement("level_5", 250) };
  m_storage->unlockedIds.insert("level_5");

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_multipleInOneCheck_allUnlocked()
{
  m_storage->currentLevelToReturn = 10;
  m_storage->completedTasksCountToReturn = 50;
  m_storage->allAchievementsToReturn = { makeAchievement("level_10", 500), makeAchievement("tasks_50", 300), };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 2);
}

void ControllerTest::achievements_tasks10_unlocksAtThreshold()
{
  m_storage->completedTasksCountToReturn = 10;
  m_storage->allAchievementsToReturn = { makeAchievement("tasks_10", 100) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_tasks10_notUnlockedBelow()
{
  m_storage->completedTasksCountToReturn = 9;
  m_storage->allAchievementsToReturn = { makeAchievement("tasks_10", 100) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_tasks250_unlocksAtThreshold()
{
  m_storage->completedTasksCountToReturn = 250;
  m_storage->allAchievementsToReturn = { makeAchievement("tasks_250", 2000) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_onTime5_unlocksAtThreshold()
{
  m_storage->onTimeCompletedCountToReturn = 5;
  m_storage->allAchievementsToReturn = { makeAchievement("on_time_5", 100) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_onTime50_notUnlockedBelow()
{
  m_storage->onTimeCompletedCountToReturn = 49;
  m_storage->allAchievementsToReturn = { makeAchievement("on_time_50", 750) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_hard10_unlocksAtThreshold()
{
  m_storage->completedCountByPriorityToReturn[storage::Priority::Hard] = 10;
  m_storage->allAchievementsToReturn = { makeAchievement("hard_10", 200) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_balancedAll_requiresAllThree()
{
  m_storage->completedCountByPriorityToReturn[storage::Priority::Low] = 10;
  m_storage->completedCountByPriorityToReturn[storage::Priority::Medium] = 10;
  m_storage->completedCountByPriorityToReturn[storage::Priority::Hard] = 10;
  m_storage->allAchievementsToReturn = { makeAchievement("balanced_all", 250) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_balancedAll_notUnlockedIfOneMissing()
{
  m_storage->completedCountByPriorityToReturn[storage::Priority::Low] = 10;
  m_storage->completedCountByPriorityToReturn[storage::Priority::Medium] = 10;
  m_storage->completedCountByPriorityToReturn[storage::Priority::Hard] = 9;
  m_storage->allAchievementsToReturn = { makeAchievement("balanced_all", 250) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_streak7_unlocksAtThreshold()
{
  m_storage->streakDaysToReturn = 7;
  m_storage->allAchievementsToReturn = { makeAchievement("streak_7", 150) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_streak100_notUnlockedBelow()
{
  m_storage->streakDaysToReturn = 99;
  m_storage->allAchievementsToReturn = { makeAchievement("streak_100", 2000) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_perfectDay1_unlocksAtThreshold()
{
  m_storage->perfectDaysCountToReturn = 1;
  m_storage->allAchievementsToReturn = { makeAchievement("perfect_day_1", 100) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_delete5_unlocksAtThreshold()
{
  m_storage->deletedTasksCountToReturn = 5;
  m_storage->allAchievementsToReturn = { makeAchievement("delete_5", 50) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_delete20_notUnlockedBelow()
{
  m_storage->deletedTasksCountToReturn = 19;
  m_storage->allAchievementsToReturn = { makeAchievement("delete_20", 150) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_location5_unlocksAtThreshold()
{
  for (int i = 0; i < 5; ++i)
  {
    m_storage->unlockedLocationsToReturn.append("loc_" + QString::number(i));
  }
  m_storage->allAchievementsToReturn = { makeAchievement("location_5", 200) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_locationAll_unlocksWhenComplete()
{
  m_storage->unlockedLocationsToReturn = { "a", "b", "c" };
  m_storage->totalLocationsCountToReturn = 3;
  m_storage->allAchievementsToReturn = { makeAchievement("location_all", 1500)} ;

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_locationAll_notUnlockedIfMissing()
{
  m_storage->unlockedLocationsToReturn = { "a", "b" };
  m_storage->totalLocationsCountToReturn = 3;
  m_storage->allAchievementsToReturn = { makeAchievement("location_all", 1500) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_comboNightmare_unlocksAtThreeHardPerDay()
{
  m_storage->maxHardTasksCompletedInOneDayToReturn = 3;
  m_storage->allAchievementsToReturn = { makeAchievement("combo_nightmare", 200) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_comboMarathon_unlocksAtFifteenPerDay()
{
  m_storage->maxTasksCompletedInOneDayToReturn = 15;
  m_storage->allAchievementsToReturn = { makeAchievement("combo_marathon", 300) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
}

void ControllerTest::achievements_comboMarathon_notUnlockedBelow()
{
  m_storage->maxTasksCompletedInOneDayToReturn = 14;
  m_storage->allAchievementsToReturn = { makeAchievement("combo_marathon", 300) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 0);
}

void ControllerTest::achievements_xpReward_grantedOnUnlock()
{
  m_storage->currentLevelToReturn = 5;
  m_storage->allAchievementsToReturn = { makeAchievement("level_5", 250) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->addXPCallCount, 1);
  QCOMPARE(m_storage->lastXPAmount, 250);
}

void ControllerTest::achievements_zeroXpReward_noXpGranted()
{
  m_storage->currentLevelToReturn = 1;
  m_storage->allAchievementsToReturn = { makeAchievement("level_1", 0) };

  m_controller->onCheckAchievements();

  QCOMPARE(m_storage->unlockAchievementCallCount, 1);
  QCOMPARE(m_storage->addXPCallCount, 0);
}

void ControllerTest::complete_triggersXpAndAchievementCheck()
{
  const storage::Task task = makeTask(1, "Do me", storage::Priority::Medium, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->addXPCallCount, 1);
  QVERIFY(m_storage->lastXPAmount > 0);
  QVERIFY(m_storage->getAllAchievementsCallCount >= 1);
}

void ControllerTest::complete_streakUpdated()
{
  const storage::Task task = makeTask(
    1, "Streak task", storage::Priority::Low,
    QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onCompleteRequested(1);

  QCOMPARE(m_storage->updateStreakCallCount, 1);
  QCOMPARE(m_storage->lastStreakDate, QDate::currentDate());
}

void ControllerTest::dailyTasksCompleted_grantsPerfectDayBonus()
{
  m_storage->allAchievementsToReturn = {};

  m_controller->onDailyTasksCompleted();

  QCOMPARE(m_storage->addXPCallCount, 1);
  QCOMPARE(m_storage->lastXPAmount, storage::xp::PERFECT_DAY);
}

void ControllerTest::dailyTasksCompleted_checksAchievements()
{
  m_storage->allAchievementsToReturn = {};

  m_controller->onDailyTasksCompleted();

  QVERIFY(m_storage->getAllAchievementsCallCount >= 1);
}

void ControllerTest::newDay_updatesStreak()
{
  m_storage->allAchievementsToReturn = {};
  const QDate date(2026, 6, 26);

  m_controller->onNewDay(date);

  QCOMPARE(m_storage->updateStreakCallCount, 1);
  QCOMPARE(m_storage->lastStreakDate, date);
}

void ControllerTest::newDay_showsStreak()
{
  m_storage->streakDaysToReturn = 5;
  m_storage->allAchievementsToReturn = {};

  m_controller->onNewDay(QDate::currentDate());

  QCOMPARE(m_view->showStreakCallCount, 1);
  QCOMPARE(m_view->lastStreakDays, 5);
}

void ControllerTest::newDay_checksAchievements()
{
  m_storage->allAchievementsToReturn = {};

  m_controller->onNewDay(QDate::currentDate());

  QVERIFY(m_storage->getAllAchievementsCallCount >= 1);
}

void ControllerTest::applicationStart_showsCurrentLevel()
{
  m_storage->currentLevelToReturn = 7;
  m_storage->totalXP = storage::calculateTotalXPForLevel(7) + 50;
  m_storage->allAchievementsToReturn = {};

  m_controller->onApplicationStart();

  QCOMPARE(m_view->showUserLevelCallCount, 1);
  QCOMPARE(m_view->lastLevel, 7);
}

void ControllerTest::applicationStart_triggersNewDay()
{
  m_storage->allAchievementsToReturn = {};

  m_controller->onApplicationStart();

  QCOMPARE(m_storage->updateStreakCallCount, 1);
}

void ControllerTest::achievementsRequested_showsFullList()
{
  m_storage->allAchievementsToReturn = {
    makeAchievement("level_1", 0),
    makeAchievement("tasks_10", 100),
    makeAchievement("streak_7", 150),
  };

  m_controller->onAchievementsRequested();

  QCOMPARE(m_view->showAchievementsListCallCount, 1);
  QCOMPARE(m_view->lastAchievementsList.size(), 3);
}

void ControllerTest::mapRequested_showsUnlockedLocations()
{
  m_storage->unlockedLocationsToReturn = {"main_building", "library", "gym"};

  m_controller->onMapRequested();

  QCOMPARE(m_view->showCampusMapCallCount, 1);
  QCOMPARE(m_view->lastUnlockedLocations.size(), 3);
}

void ControllerTest::statisticsRequested_updatesStats()
{
  m_controller->onStatisticsRequested();

  QCOMPARE(m_view->updateStatsCallCount, 1);
  QCOMPARE(m_view->updateGamificationPanelCallCount, 1);
}

void ControllerTest::delete_checksAchievementsAfterRemoval()
{
  const storage::Task task = makeTask(5, "Delete me", storage::Priority::Low, QDateTime::currentDateTime().addDays(1));
  m_storage->setTasks({ task });
  m_storage->allAchievementsToReturn = {};

  m_controller->onTaskDeleteRequested(5);

  QVERIFY(m_storage->getAllAchievementsCallCount >= 1);
}

QTEST_APPLESS_MAIN(ControllerTest)
#include "test_controller.moc"
