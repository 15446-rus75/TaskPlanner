#ifndef ISTORAGE_H
#define ISTORAGE_H

#include "task.hpp"

namespace storage
{
  enum class Priority
  {
    All,
    Low,
    Medium,
    Hard
  };

  class IStorage
  {
    virtual ~IStorage();
    virtual void addTask(const Task &task) = 0;
    virtual void removeTask(int id) = 0;
    virtual void updateTask(const Task &task) = 0;
    virtual QList< Task > getAllTasks() const = 0;
    virtual QList< Task > getTasksForDate(const QDate &date) const = 0;
    virtual QList< Task > getTasksForToday() const = 0;
    virtual QList< Task > getOverdueTasks() const = 0;
    virtual QList< Task > getTasksFiltered(const Qstring &searchText, bool today, bool overdue, Priority priority) const = 0;
    virtual QList< Task > getSortedTasks(const QList< Task > &tasks, const QString &criterion) const = 0;
    //criterion = date / priority / completed
    virtual void saveToFile(const QString &path) = 0;
    virtual void loadFromFile(const QString &path) = 0;
  };
}

#endif
