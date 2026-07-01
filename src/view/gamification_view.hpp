#ifndef GAMIFICATION_VIEW_HPP
#define GAMIFICATION_VIEW_HPP

#include "igamification_view.hpp"

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace view
{
  class GamificationView: public QWidget, public IGamificationView
  {
    Q_OBJECT

  public:
    explicit GamificationView(QWidget *parent = nullptr);
    ~GamificationView() override = default;

    void showUserLevel(int level, int current_xp, int xp_to_next) override;
    void showStreak(int days) override;
    void showUserTitle(const QString &title) override;
    void showXPNotification(int amount, const QString &reason) override;
    void showAchievementUnlocked(const storage::Achievement &achievement) override;
    void showAchievementsList(const QList< storage::Achievement > &achievements, const QList< QString > &unlockedAchievementIds) override;
    void showCampusMap(const QList< QString > &unlocked_locations) override;
    void showLocationUnlocked(const QString &location_name) override;
    void showLevelUpAnimation(int new_level, const QString &new_title) override;

    void setLevelLabel(QLabel *label);
    void setProgressBar(QProgressBar *progress_bar);
    void setExpValueLabel(QLabel *label);
    void setStreakLabel(QLabel *label);
    void setUserRankLabel(QLabel *label);
    void setAchievementLabels(const QList< QLabel* > &labels);

  signals:
    void gamificationViewReady();
    void achievementsRequested();
    void mapRequested();

  private:
    QLabel *m_level_label;
    QProgressBar *m_progress_bar;
    QLabel *m_exp_value_label;
    QLabel *m_streak_label;
    QLabel *m_user_rank_label;
    QList< QLabel* > m_achievement_labels;

    QPropertyAnimation *m_xp_animation;
    QPropertyAnimation *m_level_animation;
    QGraphicsOpacityEffect *m_opacity_effect;
  };
}

#endif
