#include "taskplannerview.hpp"

#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <Qt>

view::TaskPlannerView::TaskPlannerView(QWidget *parent):
    QMainWindow(parent),
    IView(),
    ui(new Ui::TaskPlanner),
    m_gamificationView(new GamificationView(this)),
    m_currentTaskId(-1),
    m_currentSortCriterion(storage::Criterion::Date),
    m_statusTimer(new QTimer(this)),
    m_isFormReadOnly(false)
{
  ui->setupUi(this);

  // ИСПРАВЛЕНИЕ ТЁМНОЙ ТЕМЫ WINDOWS: применяем стиль к форме задачи
  ui->frameTaskForm->setAttribute(Qt::WA_StyledBackground, true);
  ui->frameTaskForm->setStyleSheet(
      "QFrame#frameTaskForm { background-color: #ffffff; border: 1px solid #c5cae9; border-radius: 8px; }"
      "QFrame#frameTaskForm QLabel { color: #212121; background-color: transparent; }"
      "QFrame#frameTaskForm QLineEdit,"
      "QFrame#frameTaskForm QTextEdit,"
      "QFrame#frameTaskForm QDateTimeEdit,"
      "QFrame#frameTaskForm QComboBox {"
      "  background-color: #ffffff; color: #000000;"
      "  border: 1px solid #c5cae9; border-radius: 4px; padding: 4px 8px;"
      "}"
      "QFrame#frameTaskForm QLineEdit:focus,"
      "QFrame#frameTaskForm QTextEdit:focus,"
      "QFrame#frameTaskForm QDateTimeEdit:focus {"
      "  border: 2px solid #3f51b5;"
      "}"
      "QFrame#frameTaskForm QPushButton {"
      "  background-color: #ffffff; color: #212121;"
      "  border: 1px solid #c5cae9; border-radius: 6px; padding: 6px 12px;"
      "  font-weight: bold;"
      "}"
      "QFrame#frameTaskForm QPushButton:hover {"
      "  background-color: #f5f5f5;"
      "}"
      "QFrame#frameTaskForm QPushButton:pressed {"
      "  background-color: #e8eaf6;"
      "}");

  connectSignals();
  setupFilterLogic();
  setupGamification();
  ui->frameTaskForm->setVisible(false);

  m_statusTimer->setSingleShot(true);
  QObject::connect(m_statusTimer, &QTimer::timeout, this,
                   &TaskPlannerView::clearStatusMessage);

  QTimer::singleShot(0, [this]()
                     {
                       emit viewReady();
                     });
}

void view::TaskPlannerView::setupGamification()
{
  m_gamificationView->setLevelLabel(ui->labelLevel);
  m_gamificationView->setProgressBar(ui->progressBarExp);
  m_gamificationView->setExpValueLabel(ui->labelExpValue);
  m_gamificationView->setStreakLabel(ui->labelStreak);
  m_gamificationView->setUserRankLabel(ui->labelUserRank);
  m_gamificationView->setAchievementLabels({
      ui->achievement1,
      ui->achievement2,
      ui->achievement3,
      ui->achievement4
  });

  QObject::connect(m_gamificationView, &GamificationView::achievementsRequested,
                   this, &TaskPlannerView::onGamificationAchievementsRequested);
  QObject::connect(m_gamificationView, &GamificationView::mapRequested,
                   this, &TaskPlannerView::onGamificationMapRequested);

  // ИСПРАВЛЕНО: подключаем сигнал от кнопки "Все достижения" вместо QGroupBox
  QObject::connect(ui->btnViewAchievements, &QPushButton::clicked,
                   this, &TaskPlannerView::onGamificationAchievementsRequested);
}

void view::TaskPlannerView::showTaskList(const QList< storage::Task > &tasks)
{
  ui->listWidgetTasks->clear();

  for (const storage::Task &task: tasks)
  {
    QString line = "[" + QString::number(task.id) + "] ";
    line += task.name;

    if (!task.discipline.trimmed().isEmpty())
    {
      line += " | 📚 " + task.discipline;
    }

    line += " | " + task.deadline.toString("dd.MM.yyyy HH:mm");

    if (task.priority != storage::Priority::All)
    {
      line += " | Priority: " + QString::number(static_cast< int >(task.priority));
    }

    line += task.completed ? " | ✅" : " | ⬜";

    auto *item = new QListWidgetItem(line, ui->listWidgetTasks);
    item->setData(Qt::UserRole, task.id);
  }
}

void view::TaskPlannerView::showTasksForDate(const QDate &date, const QList< storage::Task > &tasks)
{
  ui->labelTaskListTitle->setText("📋 ЗАДАЧИ НА " + date.toString("dd.MM.yyyy"));
  showTaskList(tasks);
}

void view::TaskPlannerView::setTaskListTitle(const QString &title)
{
  ui->labelTaskListTitle->setText(title);
}

void view::TaskPlannerView::showTaskCreationForm()
{
  clearFormFields();
  setFormReadOnly(false);
  m_currentTaskId = -1;

  ui->frameTaskForm->setVisible(true);
  ui->btnFormSave->setVisible(true);
  ui->btnFormCancel->setText("❌ Отмена");
}

void view::TaskPlannerView::showTaskCreationForm(const storage::Task &task)
{
  taskToForm(task);
  setFormReadOnly(false);
  m_currentTaskId = task.id;

  ui->frameTaskForm->setVisible(true);
  ui->btnFormSave->setVisible(true);
  ui->btnFormCancel->setText("❌ Отмена");
}

void view::TaskPlannerView::showTaskDetails(const storage::Task &task)
{
  taskToForm(task);
  setFormReadOnly(true);
  m_currentTaskId = task.id;

  ui->frameTaskForm->setVisible(true);
  ui->btnFormSave->setVisible(false);
  ui->btnFormCancel->setText("Закрыть");
}

void view::TaskPlannerView::closeTaskCreationForm()
{
  ui->frameTaskForm->setVisible(false);
  clearFormFields();
  setFormReadOnly(false);
}

void view::TaskPlannerView::showErrorMessage(const QString &message)
{
  m_statusTimer->stop();
  ui->labelStatus->setStyleSheet("color: #e53935; font-weight: bold;");
  ui->labelStatus->setText("⚠️ " + message);
}

void view::TaskPlannerView::showInfoMessage(const QString &message)
{
  m_statusTimer->start(3000);
  ui->labelStatus->setStyleSheet("color: #43a047; font-weight: bold;");
  ui->labelStatus->setText("ℹ️ " + message);
}

void view::TaskPlannerView::updateStats(int total, int completed, int today)
{
  ui->labelStatsTotal->setText("Всего задач: " + QString::number(total));
  ui->labelStatsCompleted->setText("✅ Выполнено: " + QString::number(completed));
  ui->labelStatsToday->setText("📌 На сегодня: " + QString::number(today));
}

void view::TaskPlannerView::showUserLevel(int level, int currentXP, int xpToNext)
{
  m_gamificationView->showUserLevel(level, currentXP, xpToNext);
}

void view::TaskPlannerView::showStreak(int days)
{
  m_gamificationView->showStreak(days);
}

void view::TaskPlannerView::showUserTitle(const QString &title)
{
  m_gamificationView->showUserTitle(title);
}

void view::TaskPlannerView::showXPNotification(int amount, const QString &reason)
{
  m_gamificationView->showXPNotification(amount, reason);
}

void view::TaskPlannerView::showAchievementUnlocked(const storage::Achievement &achievement)
{
  m_gamificationView->showAchievementUnlocked(achievement);
}

void view::TaskPlannerView::showAchievementsList(const QList< storage::Achievement > &achievements, const QList< QString > &unlockedAchievementIds)
{
  m_gamificationView->showAchievementsList(achievements, unlockedAchievementIds);
}

void view::TaskPlannerView::showCampusMap(const QList< QString > &unlockedLocations)
{
  m_gamificationView->showCampusMap(unlockedLocations);
}

void view::TaskPlannerView::showLocationUnlocked(const QString &locationName)
{
  m_gamificationView->showLocationUnlocked(locationName);
}

void view::TaskPlannerView::showLevelUpAnimation(int newLevel, const QString &newTitle)
{
  m_gamificationView->showLevelUpAnimation(newLevel, newTitle);
}

void view::TaskPlannerView::updateGamificationPanel()
{
  // Панель обновляется через отдельные вызовы showUserLevel, showStreak и т.д.
}

void view::TaskPlannerView::clearStatusMessage()
{
  ui->labelStatus->clear();
}

void view::TaskPlannerView::onCalendarClicked(const QDate &date)
{
  emit dateSelected(date);
}

void view::TaskPlannerView::onSearchTextChanged(const QString &text)
{
  emit filterChanged(storage::Filter::Search, QVariant(text));
}

void view::TaskPlannerView::onFilterStateChanged(Qt::CheckState state)
{
  Q_UNUSED(state)

  QCheckBox *clickedCheckBox = qobject_cast< QCheckBox * >(sender());
  if (!clickedCheckBox)
  {
    return;
  }

  if (clickedCheckBox->isChecked())
  {
    if (clickedCheckBox == ui->checkBoxAll)
    {
      ui->checkBoxToday->setChecked(false);
      ui->checkBoxOverdue->setChecked(false);
      emit filterChanged(storage::Filter::ShowAll, QVariant());
    }
    else if (clickedCheckBox == ui->checkBoxToday)
    {
      ui->checkBoxAll->setChecked(false);
      ui->checkBoxOverdue->setChecked(false);
      emit filterChanged(storage::Filter::ShowToday, QVariant());
    }
    else if (clickedCheckBox == ui->checkBoxOverdue)
    {
      ui->checkBoxAll->setChecked(false);
      ui->checkBoxToday->setChecked(false);
      emit filterChanged(storage::Filter::ShowOverdue, QVariant());
    }
  }
  else
  {
    if (!ui->checkBoxAll->isChecked() &&
        !ui->checkBoxToday->isChecked() &&
        !ui->checkBoxOverdue->isChecked())
    {
      ui->checkBoxAll->setChecked(true);
      emit filterChanged(storage::Filter::ShowAll, QVariant());
    }
  }
}

void view::TaskPlannerView::onPriorityIndexChanged(int index)
{
  const storage::Priority priority = indexToPriority(index);
  emit filterChanged(storage::Filter::Priority, QVariant::fromValue(priority));
}

void view::TaskPlannerView::onAddClicked()
{
  showTaskCreationForm();
}

void view::TaskPlannerView::onEditClicked()
{
  const int taskId = getSelectedTaskId();
  if (taskId != -1)
  {
    emit taskEditRequested(taskId);
  }
}

void view::TaskPlannerView::onDeleteClicked()
{
  const int taskId = getSelectedTaskId();
  if (taskId != -1)
  {
    emit taskDeleteRequested(taskId);
  }
}

void view::TaskPlannerView::onMarkCompleteClicked()
{
  const int taskId = getSelectedTaskId();
  if (taskId != -1)
  {
    emit taskCompleteRequested(taskId);
  }
}

void view::TaskPlannerView::onSortClicked()
{
  if (m_currentSortCriterion == storage::Criterion::Date)
  {
    m_currentSortCriterion = storage::Criterion::Priority;
  }
  else if (m_currentSortCriterion == storage::Criterion::Priority)
  {
    m_currentSortCriterion = storage::Criterion::Completed;
  }
  else
  {
    m_currentSortCriterion = storage::Criterion::Date;
  }

  emit sortRequested(m_currentSortCriterion);
}

void view::TaskPlannerView::onFormSaveClicked()
{
  storage::Task task = formToTask();

  if (m_currentTaskId == -1)
  {
    emit taskAddRequested(task);
  }
  else
  {
    task.id = m_currentTaskId;
    emit taskUpdateRequested(task);
  }

  closeTaskCreationForm();
}

void view::TaskPlannerView::onFormCancelClicked()
{
  closeTaskCreationForm();
}

void view::TaskPlannerView::onGamificationAchievementsRequested()
{
  emit achievementsRequested();
}

void view::TaskPlannerView::onGamificationMapRequested()
{
  emit mapRequested();
}

void view::TaskPlannerView::connectSignals()
{
  QObject::connect(ui->calendarWidget, &QCalendarWidget::clicked,
                   this, &TaskPlannerView::onCalendarClicked);
  QObject::connect(ui->lineEditSearch, &QLineEdit::textChanged,
                   this, &TaskPlannerView::onSearchTextChanged);

  QObject::connect(ui->checkBoxAll, &QCheckBox::checkStateChanged,
                   this, &TaskPlannerView::onFilterStateChanged);
  QObject::connect(ui->checkBoxToday, &QCheckBox::checkStateChanged,
                   this, &TaskPlannerView::onFilterStateChanged);
  QObject::connect(ui->checkBoxOverdue, &QCheckBox::checkStateChanged,
                   this, &TaskPlannerView::onFilterStateChanged);

  QObject::connect(ui->comboBoxPriority, QOverload< int >::of(&QComboBox::currentIndexChanged),
                   this, &TaskPlannerView::onPriorityIndexChanged);

  QObject::connect(ui->btnAdd, &QPushButton::clicked,
                   this, &TaskPlannerView::onAddClicked);
  QObject::connect(ui->btnEdit, &QPushButton::clicked,
                   this, &TaskPlannerView::onEditClicked);
  QObject::connect(ui->btnDelete, &QPushButton::clicked,
                   this, &TaskPlannerView::onDeleteClicked);
  QObject::connect(ui->btnMarkComplete, &QPushButton::clicked,
                   this, &TaskPlannerView::onMarkCompleteClicked);
  QObject::connect(ui->btnSort, &QPushButton::clicked,
                   this, &TaskPlannerView::onSortClicked);

  QObject::connect(ui->btnFormSave, &QPushButton::clicked,
                   this, &TaskPlannerView::onFormSaveClicked);
  QObject::connect(ui->btnFormCancel, &QPushButton::clicked,
                   this, &TaskPlannerView::onFormCancelClicked);

  QObject::connect(ui->listWidgetTasks, &QListWidget::itemDoubleClicked,
                   this, [this](QListWidgetItem *item)
                   {
                     if (item)
                     {
                       const int taskId = item->data(Qt::UserRole).toInt();
                       emit taskViewRequested(taskId);
                     }
                   });
  QObject::connect(ui->btnViewAchievements, &QPushButton::clicked,
                   this, &TaskPlannerView::onGamificationAchievementsRequested);
}

void view::TaskPlannerView::setupFilterLogic()
{
  ui->comboBoxPriority->clear();
  ui->comboBoxPriority->addItem("🎯 Приоритет: Все", static_cast< int >(storage::Priority::All));
  ui->comboBoxPriority->addItem("🔴 Высокий", static_cast< int >(storage::Priority::Hard));
  ui->comboBoxPriority->addItem("🟡 Средний", static_cast< int >(storage::Priority::Medium));
  ui->comboBoxPriority->addItem("🟢 Низкий", static_cast< int >(storage::Priority::Low));

  ui->comboBoxFormPriority->clear();
  ui->comboBoxFormPriority->addItem("🟢 Низкий");
  ui->comboBoxFormPriority->addItem("🟡 Средний");
  ui->comboBoxFormPriority->addItem("🔴 Высокий");
}

storage::Task view::TaskPlannerView::formToTask() const
{
  storage::Task task;
  task.name = ui->lineEditFormName->text();
  task.description = ui->textEditFormDescription->toPlainText();
  task.discipline = ui->lineEditFormDiscipline->text();
  task.deadline = ui->dateTimeFormDeadline->dateTime();
  task.priority = indexToPriority(ui->comboBoxFormPriority->currentIndex());
  task.completed = false;

  const QStringList rawTags = ui->lineEditFormTags->text().split(",", Qt::SkipEmptyParts);
  for (const QString &tag: rawTags)
  {
    task.tags.append(tag.trimmed());
  }

  const QString rawLinks = ui->lineEditFormLinks->text();
  if (!rawLinks.isEmpty())
  {
    const QStringList links = rawLinks.split(",", Qt::SkipEmptyParts);
    for (const QString &link: links)
    {
      task.tags.append("link:" + link.trimmed());
    }
  }

  return task;
}

void view::TaskPlannerView::taskToForm(const storage::Task &task)
{
  ui->lineEditFormName->setText(task.name);
  ui->textEditFormDescription->setPlainText(task.description);
  ui->lineEditFormDiscipline->setText(task.discipline);
  ui->dateTimeFormDeadline->setDateTime(task.deadline);
  ui->comboBoxFormPriority->setCurrentIndex(priorityToIndex(task.priority));

  QString tagsStr;
  for (const QString &tag: task.tags)
  {
    if (!tag.startsWith("link:"))
    {
      tagsStr += tag + ", ";
    }
  }
  ui->lineEditFormTags->setText(tagsStr);

  QString linksStr;
  for (const QString &tag: task.tags)
  {
    if (tag.startsWith("link:"))
    {
      linksStr += tag.mid(5) + ", ";
    }
  }
  ui->lineEditFormLinks->setText(linksStr);
}

void view::TaskPlannerView::clearFormFields()
{
  ui->lineEditFormName->clear();
  ui->textEditFormDescription->clear();
  ui->lineEditFormDiscipline->clear();
  ui->dateTimeFormDeadline->setDateTime(QDateTime::currentDateTime());
  ui->comboBoxFormPriority->setCurrentIndex(0);
  ui->lineEditFormTags->clear();
  ui->lineEditFormLinks->clear();
}

void view::TaskPlannerView::setFormReadOnly(bool readOnly)
{
  m_isFormReadOnly = readOnly;
  ui->lineEditFormName->setReadOnly(readOnly);
  ui->lineEditFormDiscipline->setReadOnly(readOnly);
  ui->textEditFormDescription->setReadOnly(readOnly);
  ui->lineEditFormTags->setReadOnly(readOnly);
  ui->lineEditFormLinks->setReadOnly(readOnly);
  ui->dateTimeFormDeadline->setReadOnly(readOnly);
  ui->comboBoxFormPriority->setEnabled(!readOnly);
}

storage::Priority view::TaskPlannerView::indexToPriority(int index) const
{
  switch (index)
  {
  case 0:
  {
    return storage::Priority::Low;
  }
  case 1:
  {
    return storage::Priority::Medium;
  }
  case 2:
  {
    return storage::Priority::Hard;
  }
  default:
  {
    return storage::Priority::Low;
  }
  }
}

int view::TaskPlannerView::priorityToIndex(storage::Priority priority) const
{
  switch (priority)
  {
  case storage::Priority::Low:
  {
    return 0;
  }
  case storage::Priority::Medium:
  {
    return 1;
  }
  case storage::Priority::Hard:
  {
    return 2;
  }
  default:
  {
    return 0;
  }
  }
}

int view::TaskPlannerView::getSelectedTaskId() const
{
  const QListWidgetItem *item = ui->listWidgetTasks->currentItem();
  if (!item)
  {
    return -1;
  }
  return item->data(Qt::UserRole).toInt();
}

void view::TaskPlannerView::updateAchievementSlots(const QList< storage::Achievement > &unlockedAchievements)
{
  // Обновляем 4 слота достижений
  const QList< QLabel* > achievementLabels = {
      ui->achievement1,
      ui->achievement2,
      ui->achievement3,
      ui->achievement4
  };

  // Очищаем все слоты
  for (QLabel *label : achievementLabels)
  {
    if (label)
    {
      label->setText("🔒 ???");
      label->setStyleSheet("background-color: #e0e0e0; border-radius: 10px; padding: 4px 8px; color: #9e9e9e;");
    }
  }

  // Заполняем слоты разблокированными достижениями (максимум 4)
  const int slotsCount = qMin(4, unlockedAchievements.size());
  for (int i = 0; i < slotsCount; ++i)
  {
    if (achievementLabels[i])
    {
      const storage::Achievement &achievement = unlockedAchievements[i];
      achievementLabels[i]->setText("🏆 " + achievement.name);
      achievementLabels[i]->setStyleSheet(
          "background-color: #e8eaf6; border-radius: 10px; padding: 4px 8px; color: #3f51b5; font-weight: bold;");
      achievementLabels[i]->setToolTip(achievement.description);
    }
  }
}
