#include "controller.hpp"

#include <cassert>
#include <algorithm>
#include <QDate>
#include <QList>
#include <QVariant>
#include <QDebug>

#include "../storage/istorage.hpp"
#include "../utils/achievements.hpp"
#include "../view/iview.hpp"

#ifndef TEST_BUILD
#include "../view/taskplannerview.hpp"
#endif

controller::Controller::Controller(QObject *parent):
  IController(parent),
  m_storage(nullptr),
  m_view(nullptr),
  m_scopeFilter(storage::Filter::ShowAll),
  m_priorityFilter(storage::Priority::All),
  m_activeCriterion(storage::Criterion::Date),
  m_dateSelected(false),
  m_selectedDate(QDate())
{}

void controller::Controller::setStorage(storage::IStorage *storage)
{
  if (!storage)
  {
    qWarning() << "Controller::setStorage: storage pointer is nullptr";
    return;
  }
  m_storage = storage;
}

void controller::Controller::setView(view::IView *view)
{
  if (!view)
  {
    qWarning() << "Controller::setView: view pointer is nullptr";
    return;
  }
  m_view = view;
}

void controller::Controller::start()
{
  if (!checkReady())
  {
    return;
  }

#ifndef TEST_BUILD
  auto *view_ptr = dynamic_cast< view::TaskPlannerView* >(m_view);
  if (!view_ptr)
  {
    qCritical() << "Controller::start: view is not a TaskPlannerView instance";
    return;
  }

  QObject::connect(view_ptr, &view::TaskPlannerView::viewReady, this, &controller::Controller::onViewReady);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskAddRequested, this, &controller::Controller::onTaskAddRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskEditRequested, this, &controller::Controller::onTaskEditRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskViewRequested, this, &controller::Controller::onTaskViewRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskUpdateRequested, this, &controller::Controller::onTaskUpdateRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskDeleteRequested, this, &controller::Controller::onTaskDeleteRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::taskCompleteRequested, this, &controller::Controller::onCompleteRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::dateSelected, this, &controller::Controller::onDateSelected);
  QObject::connect(view_ptr, &view::TaskPlannerView::sortRequested, this, &controller::Controller::onSortRequested);
  QObject::connect(view_ptr, &view::TaskPlannerView::filterChanged, this, &controller::Controller::onFilterChanged);
#endif

  onApplicationStart();
}

bool controller::Controller::checkReady() const
{
  if (!m_storage || !m_view)
  {
    qCritical() << "Controller::checkReady: storage or view module is not provided";
    qCritical() << "Storage status:" << (m_storage ? "ok" : "nullptr");
    qCritical() << "View status:" << (m_view ? "ok" : "nullptr");
    return false;
  }
  return true;
}

bool controller::Controller::validateTask(const storage::Task &task) const
{
  if (!m_view)
  {
    qCritical() << "Controller::validateTask: m_view is nullptr";
    return false;
  }
  if (task.name.trimmed().isEmpty())
  {
    m_view->showErrorMessage("Task name can't be empty.");
    return false;
  }
  if (!task.deadline.isValid())
  {
    m_view->showErrorMessage("Specify the correct deadline for completing the task.");
    return false;
  }
  return true;
}

bool controller::Controller::priorityMatches(storage::Priority taskPriority, storage::Priority filterPriority)
{
  switch (filterPriority)
  {
  case storage::Priority::Low:
  {
    return taskPriority == storage::Priority::Low;
  }
  case storage::Priority::Medium:
  {
    return taskPriority == storage::Priority::Medium;
  }
  case storage::Priority::Hard:
  {
    return taskPriority == storage::Priority::Hard;
  }
  case storage::Priority::All:
  default:
  {
    return true;
  }
  }
}

void controller::Controller::refreshView()
{
  if (!checkReady())
  {
    return;
  }

  QList< storage::Task > tasks;

  if (m_dateSelected)
  {
    tasks = m_storage->getTasksForDate(m_selectedDate);
  }
  else
  {
    switch (m_scopeFilter)
    {
    case storage::Filter::ShowAll:
    {
      tasks = m_storage->getAllTasks();
      break;
    }
    case storage::Filter::ShowToday:
    {
      tasks = m_storage->getTasksForToday();
      break;
    }
    case storage::Filter::ShowOverdue:
    {
      tasks = m_storage->getOverdueTasks();
      break;
    }
    default:
    {
      tasks = m_storage->getAllTasks();
      break;
    }
    }
  }

  if (m_priorityFilter != storage::Priority::All)
  {
    tasks.erase(
      std::remove_if(tasks.begin(), tasks.end(),
                     [this](const storage::Task &task)
                     {
                       return !priorityMatches(task.priority, m_priorityFilter);
                     }),
      tasks.end());
  }

  if (m_dateSelected)
  {
    m_view->setTaskListTitle("Задачи на " + m_selectedDate.toString("dd.MM.yyyy"));
  }
  else
  {
    switch (m_scopeFilter)
    {
    case storage::Filter::ShowAll:
    {
      m_view->setTaskListTitle("Все задачи");
      break;
    }
    case storage::Filter::ShowToday:
    {
      m_view->setTaskListTitle("Задачи на " + QDate::currentDate().toString("dd.MM.yyyy"));
      break;
    }
    case storage::Filter::ShowOverdue:
    {
      m_view->setTaskListTitle("Просроченные задачи");
      break;
    }
    default:
    {
      m_view->setTaskListTitle("Список задач");
      break;
    }
    }
  }

  tasks = m_storage->getSortedTasks(tasks, m_activeCriterion);
  m_view->showTaskList(tasks);
  updateStats();
}

void controller::Controller::updateStats()
{
  if (!checkReady())
  {
    return;
  }

  const QList< storage::Task > all_tasks = m_storage->getAllTasks();
  const int completed_count = std::count_if(all_tasks.begin(), all_tasks.end(),
                                            [](const storage::Task &task) { return task.completed; });
  const int today_count = m_storage->getTasksForToday().size();

  m_view->updateStats(all_tasks.size(), completed_count, today_count);
}

void controller::Controller::onViewReady()
{
  if (!checkReady())
  {
    return;
  }
  refreshView();
}

void controller::Controller::onTaskAddRequested(const storage::Task &task)
{
  if (!checkReady())
  {
    return;
  }
  if (!validateTask(task))
  {
    return;
  }

  m_storage->addTask(task);
  m_view->showInfoMessage("Task \"" + task.name + "\" added successfully.");
  refreshView();
}

void controller::Controller::onTaskEditRequested(int task_id)
{
  if (!checkReady())
  {
    return;
  }
  const QList< storage::Task > all_tasks = m_storage->getAllTasks();

  for (const storage::Task &task: all_tasks)
  {
    if (task.id == task_id)
    {
      m_view->showTaskCreationForm(task);
      return;
    }
  }

  m_view->showErrorMessage("Task with ID " + QString::number(task_id) + " not found.");
}

void controller::Controller::onTaskViewRequested(int task_id)
{
#ifndef TEST_BUILD
  if (!checkReady())
  {
    return;
  }

  const QList< storage::Task > all_tasks = m_storage->getAllTasks();

  for (const storage::Task &task: all_tasks)
  {
    if (task.id == task_id)
    {
      auto *view_ptr = dynamic_cast< view::TaskPlannerView* >(m_view);
      if (view_ptr)
      {
        view_ptr->showTaskDetails(task);
      }
      return;
    }
  }

  m_view->showErrorMessage("Task with ID " + QString::number(task_id) + " not found.");
#else
  Q_UNUSED(task_id);
#endif
}

void controller::Controller::onTaskUpdateRequested(const storage::Task &task)
{
  if (!checkReady())
  {
    return;
  }
  if (!validateTask(task))
  {
    return;
  }

  m_storage->updateTask(task);
  m_view->showInfoMessage("Task \"" + task.name + "\" successfully updated.");
  refreshView();
}

void controller::Controller::onTaskDeleteRequested(int task_id)
{
  if (!checkReady())
  {
    return;
  }

  const QList< storage::Task > all_tasks = m_storage->getAllTasks();
  QString task_name;

  for (const storage::Task &task: all_tasks)
  {
    if (task.id == task_id)
    {
      task_name = task.name;
      break;
    }
  }

  m_storage->removeTask(task_id);

  if (!task_name.isEmpty())
  {
    m_view->showInfoMessage("Task \"" + task_name + "\" deleted successfully.");
  }
  else
  {
    m_view->showInfoMessage("Task deleted successfully.");
  }

  onCheckAchievements();
  refreshView();
}

void controller::Controller::onCompleteRequested(int task_id)
{
  if (!checkReady())
  {
    return;
  }

  QList< storage::Task > all_tasks = m_storage->getAllTasks();

  for (storage::Task &task: all_tasks)
  {
    if (task.id == task_id)
    {
      const bool wasCompleted = task.completed;
      task.completed = !task.completed;
      m_storage->updateTask(task);

      const QString message = task.completed
                                ? "Task \"" + task.name + "\" marked as completed."
                                : "Task \"" + task.name + "\" marked as not completed.";
      m_view->showInfoMessage(message);

      if (!wasCompleted && task.completed)
      {
        onTaskCompleted(task.id);
      }

      refreshView();
      return;
    }
  }

  m_view->showErrorMessage("Task with ID " + QString::number(task_id) + " not found.");
}

void controller::Controller::onDateSelected(const QDate &date)
{
  if (!checkReady())
  {
    return;
  }

  if (m_dateSelected && m_selectedDate == date)
  {
    m_dateSelected = false;
    m_selectedDate = QDate();
  }
  else
  {
    m_dateSelected = true;
    m_selectedDate = date;
  }

  refreshView();
}

void controller::Controller::onSortRequested(storage::Criterion criterion)
{
  if (!checkReady())
  {
    return;
  }

  m_activeCriterion = criterion;
  refreshView();
}

void controller::Controller::onFilterChanged(storage::Filter filter, const QVariant &value)
{
  if (!checkReady())
  {
    return;
  }

  if (filter == storage::Filter::Priority)
  {
    assert(value.canConvert< storage::Priority >());
    m_priorityFilter = value.value< storage::Priority >();
  }
  else if (filter == storage::Filter::Search)
  {
    m_dateSelected = false;
    m_selectedDate = QDate();
    m_scopeFilter = storage::Filter::ShowAll;
    assert(value.canConvert< QString >());
    const QString text = value.toString();
    QList< storage::Task > tasks = m_storage->getTasksFiltered(text, false, false, m_priorityFilter);
    tasks = m_storage->getSortedTasks(tasks, m_activeCriterion);
    m_view->showTaskList(tasks);
    updateStats();
    return;
  }
  else
  {
    m_scopeFilter = filter;
    m_dateSelected = false;
    m_selectedDate = QDate();
  }

  refreshView();
}

int controller::Controller::calculateTimelinessBonus(const storage::Task &task) const
{
  if (!task.completed || !task.deadline.isValid())
  {
    return 0;
  }

  const QDateTime now = QDateTime::currentDateTime();

  if (now <= task.deadline)
  {
    return storage::xp::ON_TIME_BONUS;
  }

  const qint64 overdueSeconds = task.deadline.secsTo(now);
  const qint64 overdueHours = overdueSeconds / 3600;

  const int decayedBonus = storage::xp::ON_TIME_BONUS -
                           static_cast< int >(overdueHours) * storage::xp::OVERDUE_HOURS_DECAY;

  return std::max(0, decayedBonus);
}

int controller::Controller::calculateTaskCompletionXP(const storage::Task &task) const
{
  if (!task.completed)
    return 0;

  int baseXP = 0;

  switch (task.priority)
  {
  case storage::Priority::Low:
  {
    baseXP = storage::xp::TASK_LOW;
    break;
  }
  case storage::Priority::Medium:
  {
    baseXP = storage::xp::TASK_MEDIUM;
    break;
  }
  case storage::Priority::Hard:
  {
    baseXP = storage::xp::TASK_HARD;
    break;
  }
  case storage::Priority::All:
  default:
  {
    qWarning() << "Controller::calculateTaskCompletionXP: task" << task.id
               << "has invalid priority (All), awarding 0 XP";
    baseXP = 0;
    break;
  }
  }

  return baseXP + calculateTimelinessBonus(task);
}

void controller::Controller::grantXP(int amount, const QString &reason)
{
  if (!checkReady() || amount <= 0)
  {
    return;
  }

  const int levelBefore = m_storage->getCurrentLevel();

  m_storage->addXP(amount, reason);
  m_view->showXPNotification(amount, reason);

  const int totalXP = m_storage->getTotalXP();
  const int levelAfter = storage::calculateLevelFromXP(totalXP);
  const int xpForCurrentLevel = storage::calculateTotalXPForLevel(levelAfter);
  const int xpForNextLevel = storage::calculateTotalXPForLevel(levelAfter + 1);

  QString newTitle;
  if (levelAfter >= storage::xp::MAX_LEVEL)
  {
    newTitle = "Максимальный уровень";
  }
  else
  {
    newTitle = "Уровень " + QString::number(levelAfter);
  }

  m_view->showUserLevel(levelAfter, totalXP - xpForCurrentLevel, xpForNextLevel - xpForCurrentLevel);
  m_view->showUserTitle(newTitle);

  if (levelAfter > levelBefore)
  {
    m_view->showLevelUpAnimation(levelAfter, newTitle);
  }

  m_view->updateGamificationPanel();
}

bool controller::Controller::isAchievementConditionMet(const storage::Achievement &achievement) const
{
  if (!m_storage)
  {
    return false;
  }

  const QString &id = achievement.id;

  if (id == storage::achievements::LEVEL_1.id)
  {
    return m_storage->getCurrentLevel() >= 1;
  }
  if (id == storage::achievements::LEVEL_5.id)
  {
    return m_storage->getCurrentLevel() >= 5;
  }
  if (id == storage::achievements::LEVEL_10.id)
  {
    return m_storage->getCurrentLevel() >= 10;
  }
  if (id == storage::achievements::LEVEL_20.id)
  {
    return m_storage->getCurrentLevel() >= 20;
  }
  if (id == storage::achievements::LEVEL_30.id)
  {
    return m_storage->getCurrentLevel() >= 30;
  }
  if (id == storage::achievements::LEVEL_50.id)
  {
    return m_storage->getCurrentLevel() >= storage::xp::MAX_LEVEL;
  }

  if (id == storage::achievements::TASKS_10.id)
  {
    return m_storage->getCompletedTasksCount() >= 10;
  }
  if (id == storage::achievements::TASKS_50.id)
  {
    return m_storage->getCompletedTasksCount() >= 50;
  }
  if (id == storage::achievements::TASKS_100.id)
  {
    return m_storage->getCompletedTasksCount() >= 100;
  }
  if (id == storage::achievements::TASKS_250.id)
  {
    return m_storage->getCompletedTasksCount() >= 250;
  }

  if (id == storage::achievements::ON_TIME_5.id)
  {
    return m_storage->getOnTimeCompletedCount() >= 5;
  }
  if (id == storage::achievements::ON_TIME_20.id)
  {
    return m_storage->getOnTimeCompletedCount() >= 20;
  }
  if (id == storage::achievements::ON_TIME_50.id)
  {
    return m_storage->getOnTimeCompletedCount() >= 50;
  }

  if (id == storage::achievements::HARD_10.id)
  {
    return m_storage->getCompletedCountByPriority(storage::Priority::Hard) >= 10;
  }
  if (id == storage::achievements::HARD_50.id)
  {
    return m_storage->getCompletedCountByPriority(storage::Priority::Hard) >= 50;
  }
  if (id == storage::achievements::MEDIUM_30.id)
  {
    return m_storage->getCompletedCountByPriority(storage::Priority::Medium) >= 30;
  }
  if (id == storage::achievements::LOW_50.id)
  {
    return m_storage->getCompletedCountByPriority(storage::Priority::Low) >= 50;
  }
  if (id == storage::achievements::BALANCED_ALL.id)
  {
    return m_storage->getCompletedCountByPriority(storage::Priority::Low) >= 10 &&
           m_storage->getCompletedCountByPriority(storage::Priority::Medium) >= 10 &&
           m_storage->getCompletedCountByPriority(storage::Priority::Hard) >= 10;
  }

  if (id == storage::achievements::PERFECT_DAY_1.id)
  {
    return m_storage->getPerfectDaysCount() >= 1;
  }
  if (id == storage::achievements::PERFECT_DAY_7.id)
  {
    return m_storage->getPerfectDaysCount() >= 7;
  }
  if (id == storage::achievements::PERFECT_DAY_30.id)
  {
    return m_storage->getPerfectDaysCount() >= 30;
  }

  if (id == storage::achievements::STREAK_7.id)
  {
    return m_storage->getStreakDays() >= 7;
  }
  if (id == storage::achievements::STREAK_30.id)
  {
    return m_storage->getStreakDays() >= 30;
  }
  if (id == storage::achievements::STREAK_100.id)
  {
    return m_storage->getStreakDays() >= 100;
  }

  if (id == storage::achievements::LOCATION_5.id)
  {
    return m_storage->getUnlockedLocations().size() >= 5;
  }
  if (id == storage::achievements::LOCATION_10.id)
  {
    return m_storage->getUnlockedLocations().size() >= 10;
  }
  if (id == storage::achievements::LOCATION_ALL.id)
  {
    return m_storage->getUnlockedLocations().size() >= m_storage->getTotalLocationsCount();
  }

  if (id == storage::achievements::COMBO_NIGHTMARE.id)
  {
    return m_storage->getMaxHardTasksCompletedInOneDay() >= 3;
  }
  if (id == storage::achievements::COMBO_MARATHON.id)
  {
    return m_storage->getMaxTasksCompletedInOneDay() >= 15;
  }

  if (id == storage::achievements::DELETE_5.id)
  {
    return m_storage->getDeletedTasksCount() >= 5;
  }
  if (id == storage::achievements::DELETE_20.id)
  {
    return m_storage->getDeletedTasksCount() >= 20;
  }
  if (id == storage::achievements::DELETE_100.id)
  {
    return m_storage->getDeletedTasksCount() >= 100;
  }

  qWarning() << "Controller::isAchievementConditionMet: no rule defined for achievement" << id;
  return false;
}

QList< storage::Achievement > controller::Controller::checkAndUnlockAchievements()
{
  QList< storage::Achievement > newlyUnlocked;

  if (!checkReady())
  {
    return newlyUnlocked;
  }

  const QList< storage::Achievement > allAchievements = m_storage->getAllAchievements();

  for (const storage::Achievement &achievement: allAchievements)
  {
    if (m_storage->isAchievementUnlocked(achievement.id))
    {
      continue;
    }

    if (isAchievementConditionMet(achievement))
    {
      m_storage->unlockAchievement(achievement.id);
      newlyUnlocked.append(achievement);
    }
  }

  return newlyUnlocked;
}

void controller::Controller::announceUnlockedAchievements(const QList< storage::Achievement > &unlocked)
{
  if (!checkReady())
  {
    return;
  }

  for (const storage::Achievement &achievement: unlocked)
  {
    m_view->showAchievementUnlocked(achievement);

    if (achievement.xpReward > 0)
    {
      grantXP(achievement.xpReward, "Achievement: " + achievement.name);
    }
  }
}

void controller::Controller::onTaskCompleted(int taskId)
{
  if (!checkReady())
  {
    return;
  }

  const QList< storage::Task > all_tasks = m_storage->getAllTasks();
  const auto it = std::find_if(all_tasks.begin(), all_tasks.end(),
                               [taskId](const storage::Task &task) { return task.id == taskId; });

  if (it == all_tasks.end())
  {
    qWarning() << "Controller::onTaskCompleted: task" << taskId << "not found";
    return;
  }

  onCalculateXP(taskId);
  m_storage->updateStreak(QDate::currentDate());
  onCheckAchievements();
}

void controller::Controller::onCalculateXP(int taskId)
{
  if (!checkReady())
  {
    return;
  }

  const QList< storage::Task > all_tasks = m_storage->getAllTasks();
  const auto it = std::find_if(all_tasks.begin(), all_tasks.end(),
                               [taskId](const storage::Task &task) { return task.id == taskId; });

  if (it == all_tasks.end())
  {
    qWarning() << "Controller::onCalculateXP: task" << taskId << "not found";
    return;
  }

  const int xp = calculateTaskCompletionXP(*it);
  grantXP(xp, "Task completed: " + it->name);
}

void controller::Controller::onDailyTasksCompleted()
{
  if (!checkReady())
  {
    return;
  }

  grantXP(storage::xp::PERFECT_DAY, "Perfect day bonus");
  onCheckAchievements();
}

void controller::Controller::onCheckAchievements()
{
  const QList< storage::Achievement > unlocked = checkAndUnlockAchievements();
  announceUnlockedAchievements(unlocked);
}

void controller::Controller::onAchievementsRequested()
{
  if (!checkReady())
  {
    return;
  }

  m_view->showAchievementsList(m_storage->getAllAchievements());
}

void controller::Controller::onMapRequested()
{
  if (!checkReady())
  {
    return;
  }

  m_view->showCampusMap(m_storage->getUnlockedLocations());
}

void controller::Controller::onStatisticsRequested()
{
  if (!checkReady())
  {
    return;
  }

  updateStats();
  m_view->updateGamificationPanel();
}

void controller::Controller::onNewDay(const QDate &date)
{
  if (!checkReady())
  {
    return;
  }

  m_storage->updateStreak(date);
  m_view->showStreak(m_storage->getStreakDays());
  onCheckAchievements();
}

void controller::Controller::onApplicationStart()
{
  if (!checkReady())
  {
    return;
  }

  const int level = m_storage->getCurrentLevel();
  const int totalXP = m_storage->getTotalXP();
  const int xpForCurrentLevel = storage::calculateTotalXPForLevel(level);
  const int xpForNextLevel = storage::calculateTotalXPForLevel(level + 1);

  m_view->showUserLevel(level, totalXP - xpForCurrentLevel, xpForNextLevel - xpForCurrentLevel);
  onNewDay(QDate::currentDate());
}
