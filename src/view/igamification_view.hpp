#ifndef IGAMIFICATION_VIEW_HPP
#define IGAMIFICATION_VIEW_HPP

#include <QString>
#include <QList>
#include "../storage/istorage.hpp"

namespace view
{
class IGamificationView
{
public:
  virtual ~IGamificationView() = default;

  virtual void showUserLevel(int level, int current_xp, int xp_to_next) = 0;
  virtual void showStreak(int days) = 0;
  virtual void showUserTitle(const QString &title) = 0;
  virtual void showXPNotification(int amount, const QString &reason) = 0;
  virtual void showAchievementUnlocked(const storage::Achievement &achievement) = 0;
  virtual void showAchievementsList(const QList< storage::Achievement > &achievements, const QList< QString > &unlockedAchievementIds) = 0;
  virtual void showCampusMap(const QList< QString > &unlocked_locations) = 0;
  virtual void showLocationUnlocked(const QString &location_name) = 0;
  virtual void showLevelUpAnimation(int new_level, const QString &new_title) = 0;
};
}

#endif
