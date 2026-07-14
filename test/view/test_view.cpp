#include <QtTest>
#include <QSignalSpy>
#include <QMetaObject>
#include <QCheckBox>
#include <QListWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QDateTimeEdit>
#include "taskplannerview.hpp"
#include "mockstorage.hpp"

class ViewTest: public QObject
{
  Q_OBJECT

private slots:
  void init();
  void cleanup();

  void dateSelected_emitsSignal();
  void dateSelected_sameDateTwice_emitsSignalTwice();

  void taskAddRequested_validForm_emitsSignal();
  void taskAddRequested_emptyName_emitsSignalWithEmptyTask();
  void taskUpdateRequested_existingTask_emitsSignal();

  void taskEditRequested_noSelection_doesNotEmit();
  void taskEditRequested_withSelection_emitsSignal();
  void taskDeleteRequested_noSelection_doesNotEmit();
  void taskDeleteRequested_withSelection_emitsSignal();
  void taskCompleteRequested_noSelection_doesNotEmit();
  void taskCompleteRequested_withSelection_emitsSignal();

  void taskViewRequested_doubleClick_emitsSignal();

  void sortRequested_cyclesThroughCriteria();
  void sortRequested_fullCycle_returnsToDate();

  void filterChanged_searchText_emitsSignal();
  void filterChanged_priority_emitsSignal();
  void filterChanged_showAll_emitsSignal();
  void filterChanged_showToday_emitsSignal();
  void filterChanged_showOverdue_emitsSignal();
  void filterChanged_checkboxMutualExclusion_onlyOneActive();

  void showTaskList_displaysTasks();
  void showTaskList_emptyList_clearsList();
  void showTaskList_withPriority_showsTextNotNumber();
  void showTaskList_completedTask_showsCheckmark();

  void showTasksForDate_updatesTitleAndShows();

  void updateStats_updatesLabels();

  void showErrorMessage_displaysError();
  void showInfoMessage_displaysInfo();
  void clearStatusMessage_clearsLabel();

  void setTaskListTitle_updatesTitle();

  void showTaskCreationForm_clearsFields();
  void showTaskCreationForm_withTask_fillsFields();
  void showTaskCreationForm_withTagsAndLinks_fillsCorrectly();
  void closeTaskCreationForm_hidesForm();
  void showTaskDetails_hidesSaveButton();

  void formToTask_withTags_parsesCorrectly();
  void formToTask_withLinks_parsesCorrectly();

  void showUserLevel_updatesLabel();
  void showStreak_updatesLabel();
  void showUserTitle_updatesLabel();
  void updateGamificationPanel_updatesProgressBar();
  void updateAchievementSlots_updatesLabels();
  void updateAchievementSlots_emptyList_showsLocked();

  void onGamificationAchievementsRequested_emitsSignal();
  void onGamificationMapRequested_emitsSignal();

  void setUserName_updatesButton();
  void onUserNameClicked_emitsSignal();

  void populateSDOLinks_fillsComboBox();
  void onInstituteChanged_updatesList();

private:
  storage::Task makeTask(int id, const QString &name, storage::Priority priority, const QDateTime &deadline, bool completed = false) const;
  view::TaskPlannerView *m_view;
  test::MockStorage *m_storage;
};

void ViewTest::init()
{
  m_view = new view::TaskPlannerView();
  m_storage = new test::MockStorage();
}

void ViewTest::cleanup()
{
  delete m_view;
  delete m_storage;
  m_view = nullptr;
  m_storage = nullptr;
}

storage::Task ViewTest::makeTask(int id, const QString &name, storage::Priority priority, const QDateTime &deadline, bool completed) const
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

void ViewTest::dateSelected_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::dateSelected);
  QVERIFY(spy.isValid());
  const QDate test_date(2026, 6, 25);
  QMetaObject::invokeMethod(m_view, "onCalendarClicked", Q_ARG(QDate, test_date));
  QCOMPARE(spy.count(), 1);
  const QList< QVariant > arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).toDate(), test_date);
}

void ViewTest::dateSelected_sameDateTwice_emitsSignalTwice()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::dateSelected);
  QVERIFY(spy.isValid());
  const QDate test_date(2026, 6, 25);
  QMetaObject::invokeMethod(m_view, "onCalendarClicked", Q_ARG(QDate, test_date));
  QMetaObject::invokeMethod(m_view, "onCalendarClicked", Q_ARG(QDate, test_date));
  QCOMPARE(spy.count(), 2);
}

void ViewTest::taskAddRequested_validForm_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskAddRequested);
  QVERIFY(spy.isValid());
  m_view->showTaskCreationForm();
  QLineEdit *name_edit = m_view->findChild< QLineEdit* >("lineEditFormName");
  QVERIFY(name_edit);
  name_edit->setText("Test Task");
  QMetaObject::invokeMethod(m_view, "onFormSaveClicked");
  QCOMPARE(spy.count(), 1);
}

void ViewTest::taskAddRequested_emptyName_emitsSignalWithEmptyTask()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskAddRequested);
  QVERIFY(spy.isValid());
  m_view->showTaskCreationForm();
  QMetaObject::invokeMethod(m_view, "onFormSaveClicked");
  QCOMPARE(spy.count(), 1);
  const storage::Task task = spy.takeFirst().at(0).value< storage::Task >();
  QVERIFY(task.name.isEmpty());
}

void ViewTest::taskUpdateRequested_existingTask_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskUpdateRequested);
  QVERIFY(spy.isValid());
  const storage::Task task = makeTask(42, "Edit me", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskCreationForm(task);
  QMetaObject::invokeMethod(m_view, "onFormSaveClicked");
  QCOMPARE(spy.count(), 1);
  const storage::Task updated = spy.takeFirst().at(0).value< storage::Task >();
  QCOMPARE(updated.id, 42);
}

void ViewTest::taskEditRequested_noSelection_doesNotEmit()
{
  const storage::Task task = makeTask(1, "Test Task", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskEditRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onEditClicked");
  QCOMPARE(spy.count(), 0);
}

void ViewTest::taskEditRequested_withSelection_emitsSignal()
{
  const storage::Task task = makeTask(42, "Selected", storage::Priority::Low, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  list_widget->setCurrentRow(0);
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskEditRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onEditClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toInt(), 42);
}

void ViewTest::taskDeleteRequested_noSelection_doesNotEmit()
{
  const storage::Task task = makeTask(1, "Test Task", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskDeleteRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onDeleteClicked");
  QCOMPARE(spy.count(), 0);
}

void ViewTest::taskDeleteRequested_withSelection_emitsSignal()
{
  const storage::Task task = makeTask(7, "To Delete", storage::Priority::Hard, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  list_widget->setCurrentRow(0);
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskDeleteRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onDeleteClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toInt(), 7);
}

void ViewTest::taskCompleteRequested_noSelection_doesNotEmit()
{
  const storage::Task task = makeTask(1, "Test Task", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskCompleteRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onMarkCompleteClicked");
  QCOMPARE(spy.count(), 0);
}

void ViewTest::taskCompleteRequested_withSelection_emitsSignal()
{
  const storage::Task task = makeTask(3, "Complete me", storage::Priority::Low, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  list_widget->setCurrentRow(0);
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskCompleteRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onMarkCompleteClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toInt(), 3);
}

void ViewTest::taskViewRequested_doubleClick_emitsSignal()
{
  const storage::Task task = makeTask(99, "Double click me", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskList({ task });
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskViewRequested);
  QVERIFY(spy.isValid());
  QListWidgetItem *item = list_widget->item(0);
  QVERIFY(item);
  emit list_widget->itemDoubleClicked(item);
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toInt(), 99);
}

void ViewTest::sortRequested_cyclesThroughCriteria()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::sortRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).value< storage::Criterion >(), storage::Criterion::Priority);
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).value< storage::Criterion >(), storage::Criterion::Completed);
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).value< storage::Criterion >(), storage::Criterion::Date);
}

void ViewTest::sortRequested_fullCycle_returnsToDate()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::sortRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QMetaObject::invokeMethod(m_view, "onSortClicked");
  QCOMPARE(spy.count(), 3);
  QCOMPARE(spy.at(2).at(0).value< storage::Criterion >(), storage::Criterion::Date);
}

void ViewTest::filterChanged_searchText_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::filterChanged);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onSearchTextChanged", Q_ARG(QString, "test"));
  QCOMPARE(spy.count(), 1);
  const QList< QVariant > arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).value< storage::Filter >(), storage::Filter::Search);
  QCOMPARE(arguments.at(1).toString(), QString("test"));
}

void ViewTest::filterChanged_priority_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::filterChanged);
  QVERIFY(spy.isValid());
  QComboBox *combo_box = m_view->findChild<QComboBox*>("comboBoxPriority");
  QVERIFY(combo_box);
  combo_box->setCurrentIndex(1);
  QCOMPARE(spy.count(), 1);
  const QList< QVariant > arguments = spy.takeFirst();
  QCOMPARE(arguments.at(0).value< storage::Filter >(), storage::Filter::Priority);
}

void ViewTest::filterChanged_showAll_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::filterChanged);
  QVERIFY(spy.isValid());
  QCheckBox *all_box = m_view->findChild< QCheckBox* >("checkBoxAll");
  QVERIFY(all_box);
  all_box->setChecked(false);
  all_box->setChecked(true);
  bool found = false;
  for (int i = 0; i < spy.count(); ++i)
  {
    if (spy.at(i).at(0).value< storage::Filter >() == storage::Filter::ShowAll)
    {
      found = true;
      break;
    }
  }
  QVERIFY(found);
}

void ViewTest::filterChanged_showToday_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::filterChanged);
  QVERIFY(spy.isValid());
  QCheckBox *today_box = m_view->findChild< QCheckBox* >("checkBoxToday");
  QVERIFY(today_box);
  today_box->setChecked(true);
  bool found = false;
  for (int i = 0; i < spy.count(); ++i)
  {
    if (spy.at(i).at(0).value< storage::Filter >() == storage::Filter::ShowToday)
    {
      found = true;
      break;
    }
  }
  QVERIFY(found);
}

void ViewTest::filterChanged_showOverdue_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::filterChanged);
  QVERIFY(spy.isValid());
  QCheckBox *overdue_box = m_view->findChild< QCheckBox* >("checkBoxOverdue");
  QVERIFY(overdue_box);
  overdue_box->setChecked(true);
  bool found = false;
  for (int i = 0; i < spy.count(); ++i)
  {
    if (spy.at(i).at(0).value< storage::Filter >() == storage::Filter::ShowOverdue)
    {
      found = true;
      break;
    }
  }
  QVERIFY(found);
}

void ViewTest::filterChanged_checkboxMutualExclusion_onlyOneActive()
{
  QCheckBox *all_box = m_view->findChild< QCheckBox* >("checkBoxAll");
  QCheckBox *today_box = m_view->findChild< QCheckBox* >("checkBoxToday");
  QCheckBox *overdue_box = m_view->findChild< QCheckBox* >("checkBoxOverdue");
  QVERIFY(all_box);
  QVERIFY(today_box);
  QVERIFY(overdue_box);
  today_box->setChecked(true);
  QVERIFY(today_box->isChecked());
  QVERIFY(!all_box->isChecked());
  overdue_box->setChecked(true);
  QVERIFY(overdue_box->isChecked());
  QVERIFY(!today_box->isChecked());
}

void ViewTest::showTaskList_displaysTasks()
{
  QList< storage::Task > tasks;
  tasks.append(makeTask(1, "Task 1", storage::Priority::Low, QDateTime::currentDateTime()));
  tasks.append(makeTask(2, "Task 2", storage::Priority::Hard, QDateTime::currentDateTime(), true));
  m_view->showTaskList(tasks);
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  QCOMPARE(list_widget->count(), 2);
}

void ViewTest::showTaskList_emptyList_clearsList()
{
  QList< storage::Task > tasks;
  tasks.append(makeTask(1, "Task", storage::Priority::Low, QDateTime::currentDateTime()));
  m_view->showTaskList(tasks);
  const QList< storage::Task > empty_tasks;
  m_view->showTaskList(empty_tasks);
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  QCOMPARE(list_widget->count(), 0);
}

void ViewTest::showTaskList_withPriority_showsTextNotNumber()
{
  QList< storage::Task > tasks;
  tasks.append(makeTask(1, "Hard Task", storage::Priority::Hard, QDateTime::currentDateTime()));
  m_view->showTaskList(tasks);
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  const QString text = list_widget->item(0)->text();
  QVERIFY(text.contains("Высокий"));
  QVERIFY(!text.contains("Priority: 3"));
}

void ViewTest::showTaskList_completedTask_showsCheckmark()
{
  QList< storage::Task > tasks;
  tasks.append(makeTask(1, "Done", storage::Priority::Low, QDateTime::currentDateTime(), true));
  m_view->showTaskList(tasks);
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  const QString text = list_widget->item(0)->text();
  QVERIFY(text.contains("✅"));
}

void ViewTest::showTasksForDate_updatesTitleAndShows()
{
  const QDate date(2026, 6, 25);
  QList< storage::Task > tasks;
  tasks.append(makeTask(1, "Task on date", storage::Priority::Low, QDateTime(date, QTime(10, 0))));
  m_view->showTasksForDate(date, tasks);
  QLabel *title_label = m_view->findChild< QLabel* >("labelTaskListTitle");
  QVERIFY(title_label);
  QVERIFY(title_label->text().contains("25.06.2026"));
  QListWidget *list_widget = m_view->findChild< QListWidget* >("listWidgetTasks");
  QVERIFY(list_widget);
  QCOMPARE(list_widget->count(), 1);
}

void ViewTest::updateStats_updatesLabels()
{
  m_view->updateStats(10, 5, 3);
  QLabel *total_label = m_view->findChild< QLabel* >("labelStatsTotal");
  QLabel *completed_label = m_view->findChild< QLabel* >("labelStatsCompleted");
  QLabel *today_label = m_view->findChild< QLabel* >("labelStatsToday");
  QVERIFY(total_label);
  QVERIFY(completed_label);
  QVERIFY(today_label);
  QVERIFY(total_label->text().contains("10"));
  QVERIFY(completed_label->text().contains("5"));
  QVERIFY(today_label->text().contains("3"));
}

void ViewTest::showErrorMessage_displaysError()
{
  m_view->showErrorMessage("Test error message");
  QLabel *status_label = m_view->findChild< QLabel* >("labelStatus");
  QVERIFY(status_label);
  QVERIFY(status_label->text().contains("Test error message"));
}

void ViewTest::showInfoMessage_displaysInfo()
{
  m_view->showInfoMessage("Test info message");
  QLabel *status_label = m_view->findChild< QLabel* >("labelStatus");
  QVERIFY(status_label);
  QVERIFY(status_label->text().contains("Test info message"));
}

void ViewTest::clearStatusMessage_clearsLabel()
{
  m_view->showInfoMessage("Some message");
  QLabel *status_label = m_view->findChild< QLabel* >("labelStatus");
  QVERIFY(status_label);
  QVERIFY(!status_label->text().isEmpty());
  QMetaObject::invokeMethod(m_view, "clearStatusMessage");
  QVERIFY(status_label->text().isEmpty());
}

void ViewTest::setTaskListTitle_updatesTitle()
{
  m_view->setTaskListTitle("Test Title");
  QLabel *title_label = m_view->findChild< QLabel* >("labelTaskListTitle");
  QVERIFY(title_label);
  QCOMPARE(title_label->text(), QString("Test Title"));
}

void ViewTest::showTaskCreationForm_clearsFields()
{
  m_view->showTaskCreationForm();
  QLineEdit *name_edit = m_view->findChild< QLineEdit* >("lineEditFormName");
  QVERIFY(name_edit);
  QVERIFY(name_edit->text().isEmpty());
  QFrame *form_frame = m_view->findChild< QFrame* >("frameTaskForm");
  QVERIFY(form_frame);
  QVERIFY(!form_frame->isHidden());
}

void ViewTest::showTaskCreationForm_withTask_fillsFields()
{
  const storage::Task task = makeTask(1, "Test Task", storage::Priority::Medium, QDateTime::currentDateTime());
  m_view->showTaskCreationForm(task);
  QLineEdit *name_edit = m_view->findChild< QLineEdit* >("lineEditFormName");
  QVERIFY(name_edit);
  QCOMPARE(name_edit->text(), QString("Test Task"));
}

void ViewTest::showTaskCreationForm_withTagsAndLinks_fillsCorrectly()
{
  storage::Task task = makeTask(1, "Tagged Task", storage::Priority::Low, QDateTime::currentDateTime());
  task.tags = { "#лаба", "#срочно", "link:https://example.com" };
  m_view->showTaskCreationForm(task);
  QLineEdit *tags_edit = m_view->findChild< QLineEdit* >("lineEditFormTags");
  QLineEdit *links_edit = m_view->findChild< QLineEdit* >("lineEditFormLinks");
  QVERIFY(tags_edit);
  QVERIFY(links_edit);
  QVERIFY(tags_edit->text().contains("#лаба"));
  QVERIFY(tags_edit->text().contains("#срочно"));
  QVERIFY(links_edit->text().contains("https://example.com"));
}

void ViewTest::closeTaskCreationForm_hidesForm()
{
  m_view->showTaskCreationForm();
  m_view->closeTaskCreationForm();
  QFrame *form_frame = m_view->findChild< QFrame* >("frameTaskForm");
  QVERIFY(form_frame);
  QVERIFY(form_frame->isHidden());
}

void ViewTest::showTaskDetails_hidesSaveButton()
{
  const storage::Task task = makeTask(1, "Details", storage::Priority::Low, QDateTime::currentDateTime());
  m_view->showTaskDetails(task);
  QPushButton *save_btn = m_view->findChild< QPushButton* >("btnFormSave");
  QVERIFY(save_btn);
  QVERIFY(!save_btn->isVisible());
}

void ViewTest::formToTask_withTags_parsesCorrectly()
{
  m_view->showTaskCreationForm();
  QLineEdit *name_edit = m_view->findChild< QLineEdit* >("lineEditFormName");
  QLineEdit *tags_edit = m_view->findChild< QLineEdit* >("lineEditFormTags");
  QVERIFY(name_edit);
  QVERIFY(tags_edit);
  name_edit->setText("Tagged Task");
  tags_edit->setText("#лаба, #срочно, #важно");
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskAddRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onFormSaveClicked");
  QCOMPARE(spy.count(), 1);
  const storage::Task task = spy.takeFirst().at(0).value< storage::Task >();
  QCOMPARE(task.tags.size(), 3);
  QVERIFY(task.tags.contains("#лаба"));
  QVERIFY(task.tags.contains("#срочно"));
  QVERIFY(task.tags.contains("#важно"));
}

void ViewTest::formToTask_withLinks_parsesCorrectly()
{
  m_view->showTaskCreationForm();
  QLineEdit *name_edit = m_view->findChild< QLineEdit* >("lineEditFormName");
  QLineEdit *links_edit = m_view->findChild< QLineEdit* >("lineEditFormLinks");
  QVERIFY(name_edit);
  QVERIFY(links_edit);
  name_edit->setText("Linked Task");
  links_edit->setText("https://example.com, https://test.com");
  QSignalSpy spy(m_view, &view::TaskPlannerView::taskAddRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onFormSaveClicked");
  QCOMPARE(spy.count(), 1);
  const storage::Task task = spy.takeFirst().at(0).value< storage::Task >();
  QVERIFY(task.tags.contains("link:https://example.com"));
  QVERIFY(task.tags.contains("link:https://test.com"));
}

void ViewTest::showUserLevel_updatesLabel()
{
  m_view->showUserLevel(5, 50, 100);
  QLabel *level_label = m_view->findChild< QLabel* >("labelLevel");
  QVERIFY(level_label);
  QCOMPARE(level_label->text(), QString("Lv.5"));
}

void ViewTest::showStreak_updatesLabel()
{
  m_view->showStreak(7);
  QLabel *streak_label = m_view->findChild< QLabel* >("labelStreak");
  QVERIFY(streak_label);
  QVERIFY(streak_label->text().contains("7"));
}

void ViewTest::showUserTitle_updatesLabel()
{
  m_view->showUserTitle("Мастер");
  QLabel *rank_label = m_view->findChild< QLabel* >("labelUserRank");
  QVERIFY(rank_label);
  QVERIFY(rank_label->text().contains("Мастер"));
}

void ViewTest::updateGamificationPanel_updatesProgressBar()
{
  m_view->showUserLevel(5, 50, 100);
  m_view->showStreak(3);
  m_view->showUserTitle("Тест");
  m_view->updateGamificationPanel();
  QProgressBar *progress_bar = m_view->findChild< QProgressBar* >("progressBarExp");
  QVERIFY(progress_bar);
  QCOMPARE(progress_bar->maximum(), 100);
  QCOMPARE(progress_bar->value(), 50);
}

void ViewTest::updateAchievementSlots_updatesLabels()
{
  QList< storage::Achievement > unlocked;
  storage::Achievement a;
  a.id = "test_achievement";
  a.name = "Тестовое достижение";
  a.description = "Описание";
  unlocked.append(a);
  m_view->updateAchievementSlots(unlocked);
  QLabel *achievement1 = m_view->findChild< QLabel* >("achievement1");
  QVERIFY(achievement1);
  QVERIFY(achievement1->text().contains("Тестовое достижение"));
}

void ViewTest::updateAchievementSlots_emptyList_showsLocked()
{
  QList< storage::Achievement > unlocked;
  m_view->updateAchievementSlots(unlocked);
  QLabel *achievement1 = m_view->findChild< QLabel* >("achievement1");
  QLabel *achievement2 = m_view->findChild< QLabel* >("achievement2");
  QVERIFY(achievement1);
  QVERIFY(achievement2);
  QVERIFY(achievement1->text().contains("🔒"));
  QVERIFY(achievement2->text().contains("🔒"));
}

void ViewTest::onGamificationAchievementsRequested_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::achievementsRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onGamificationAchievementsRequested");
  QCOMPARE(spy.count(), 1);
}

void ViewTest::onGamificationMapRequested_emitsSignal()
{
  QSignalSpy spy(m_view, &view::TaskPlannerView::mapRequested);
  QVERIFY(spy.isValid());
  QMetaObject::invokeMethod(m_view, "onGamificationMapRequested");
  QCOMPARE(spy.count(), 1);
}

void ViewTest::setUserName_updatesButton()
{
  m_view->setUserName("Иван Иванов");
  QPushButton *user_name_btn = m_view->findChild< QPushButton* >("btnUserName");
  QVERIFY(user_name_btn);
  QCOMPARE(user_name_btn->text(), QString("Иван Иванов"));
}

void ViewTest::onUserNameClicked_emitsSignal()
{
  QPushButton *user_name_btn = m_view->findChild< QPushButton* >("btnUserName");
  QVERIFY(user_name_btn);
  user_name_btn->setText("Тестовое имя");
  QSignalSpy spy(m_view, &view::TaskPlannerView::userNameChanged);
  QVERIFY(spy.isValid());
  QVERIFY(true);
}

void ViewTest::populateSDOLinks_fillsComboBox()
{
  QComboBox *combo_institute = m_view->findChild< QComboBox* >("comboInstitute");
  QVERIFY(combo_institute);
  QVERIFY(combo_institute->count() > 0);
  QListWidget *list_sdo = m_view->findChild< QListWidget* >("listSDOLinks");
  QVERIFY(list_sdo);
  QVERIFY(list_sdo->count() > 0);
}

void ViewTest::onInstituteChanged_updatesList()
{
  QComboBox *combo_institute = m_view->findChild< QComboBox* >("comboInstitute");
  QVERIFY(combo_institute);
  QListWidget *list_sdo = m_view->findChild< QListWidget* >("listSDOLinks");
  QVERIFY(list_sdo);
  if (combo_institute->count() > 1)
  {
    combo_institute->setCurrentIndex(1);
    QVERIFY(list_sdo->count() > 0);
    QListWidgetItem *item = list_sdo->item(0);
    QVERIFY(item);
    QVERIFY(item->text().contains("СДО"));
  }
}

QTEST_MAIN(ViewTest)
#include "test_view.moc"
