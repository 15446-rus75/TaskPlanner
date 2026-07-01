#include "gamification_view.hpp"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace
{
constexpr int k_achievementDialogWidth = 400;
constexpr int k_achievementDialogHeight = 500;
constexpr int k_mapDialogWidth = 600;
constexpr int k_mapDialogHeight = 400;

constexpr int k_xpAnimationDurationMs = 3000;
constexpr int k_levelAnimationDurationMs = 5000;
}

view::GamificationView::GamificationView(QWidget *parent):
    QWidget(parent),
    m_level_label(nullptr),
    m_progress_bar(nullptr),
    m_exp_value_label(nullptr),
    m_streak_label(nullptr),
    m_user_rank_label(nullptr),
    m_xp_animation(nullptr),
    m_level_animation(nullptr),
    m_opacity_effect(nullptr)
{}

void view::GamificationView::setLevelLabel(QLabel *label)
{
  m_level_label = label;
}

void view::GamificationView::setProgressBar(QProgressBar *progress_bar)
{
  m_progress_bar = progress_bar;
}

void view::GamificationView::setExpValueLabel(QLabel *label)
{
  m_exp_value_label = label;
}

void view::GamificationView::setStreakLabel(QLabel *label)
{
  m_streak_label = label;
}

void view::GamificationView::setUserRankLabel(QLabel *label)
{
  m_user_rank_label = label;
}

void view::GamificationView::setAchievementLabels(const QList< QLabel* > &labels)
{
  m_achievement_labels = labels;
}

void view::GamificationView::showUserLevel(int level, int current_xp, int xp_to_next)
{
  if (m_level_label)
  {
    m_level_label->setText("Lv." + QString::number(level));
  }
  if (m_progress_bar)
  {
    m_progress_bar->setMaximum(xp_to_next);
    m_progress_bar->setValue(current_xp);
  }
  if (m_exp_value_label)
  {
    m_exp_value_label->setText(QString("%1 / %2").arg(current_xp).arg(xp_to_next));
  }
}

void view::GamificationView::showStreak(int days)
{
  if (m_streak_label)
  {
    m_streak_label->setText("🔥 " + QString::number(days) + " дней");
  }
}

void view::GamificationView::showUserTitle(const QString &title)
{
  if (m_user_rank_label)
  {
    m_user_rank_label->setText("🏅 " + title);
  }
}

void view::GamificationView::showXPNotification(int amount, const QString &reason)
{
  Q_UNUSED(amount);
  Q_UNUSED(reason);

  if (!m_opacity_effect)
  {
    m_opacity_effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacity_effect);
  }

  m_opacity_effect->setOpacity(1.0);

  if (m_xp_animation)
  {
    m_xp_animation->stop();
  }

  m_xp_animation = new QPropertyAnimation(m_opacity_effect, "opacity", this);
  m_xp_animation->setDuration(k_xpAnimationDurationMs);
  m_xp_animation->setStartValue(1.0);
  m_xp_animation->setEndValue(0.0);
  m_xp_animation->start();
}

void view::GamificationView::showAchievementUnlocked(const storage::Achievement &achievement)
{
  Q_UNUSED(achievement);
  // Визуальное уведомление о разблокировке достижения будет реализовано в будущих итерациях
}

void view::GamificationView::showAchievementsList( const QList< storage::Achievement > &achievements, const QList< QString > &unlocked_achievement_ids)
{
  QDialog dialog(this);
  dialog.setWindowTitle("Достижения");
  dialog.setMinimumSize(500, 600);

  dialog.setStyleSheet(
      "QDialog { background-color: #ffffff; }"
      "QLabel { color: #212121; }"
      "QListWidget { background-color: #ffffff; border: 1px solid #c5cae9; border-radius: 6px; }"
      "QListWidget::item { padding: 10px; border-bottom: 1px solid #e0e0e0; }"
      "QListWidget::item:selected { background-color: #e8eaf6; }");

  QVBoxLayout *layout = new QVBoxLayout(&dialog);

  QLabel *title_label = new QLabel("🏆 Ваши достижения", &dialog);
  title_label->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #3f51b5; padding: 8px;");
  layout->addWidget(title_label);

  QListWidget *list_widget = new QListWidget(&dialog);
  list_widget->setSpacing(4);

  int unlocked_count = 0;

  for (const storage::Achievement &achievement : achievements)
  {
    QListWidgetItem *item = new QListWidgetItem();

    const bool is_unlocked = unlocked_achievement_ids.contains(achievement.id);

    QString text;
    if (is_unlocked)
    {
      text = "🏆 " + achievement.name + " ✓";
      item->setForeground(QColor("#3f51b5"));
      item->setBackground(QColor("#e8eaf6"));
      ++unlocked_count;
    }
    else
    {
      text = "🔒 " + achievement.name;
      item->setForeground(QColor("#9e9e9e"));
    }

    if (!achievement.description.isEmpty())
    {
      text += "\n" + achievement.description;
    }
    if (achievement.xpReward > 0)
    {
      text += "\n💰 Награда: +" + QString::number(achievement.xpReward) + " XP";
    }

    item->setText(text);
    item->setData(Qt::UserRole, achievement.id);
    item->setSizeHint(QSize(0, 60));

    list_widget->addItem(item);
  }

  layout->addWidget(list_widget);

  QLabel *stats_label = new QLabel(
      QString("📊 Разблокировано: %1 из %2")
          .arg(unlocked_count)
          .arg(achievements.size()),
      &dialog);
  stats_label->setStyleSheet("color: #5c6bc0; font-size: 11px; padding: 4px;");
  layout->addWidget(stats_label);

  dialog.exec();
}

void view::GamificationView::showCampusMap(const QList< QString > &unlocked_locations)
{
  QDialog dialog(this);
  dialog.setWindowTitle("Карта кампуса");
  dialog.setMinimumSize(k_mapDialogWidth, k_mapDialogHeight);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QLabel *map_label = new QLabel("Карта кампуса", &dialog);
  map_label->setAlignment(Qt::AlignCenter);

  QListWidget *location_list = new QListWidget(&dialog);

  for (const QString &location : unlocked_locations)
  {
    QListWidgetItem *item = new QListWidgetItem();
    item->setText(location + " ✓");
    location_list->addItem(item);
  }

  layout->addWidget(map_label);
  layout->addWidget(location_list);
  dialog.exec();
}

void view::GamificationView::showLocationUnlocked(const QString &location_name)
{
  Q_UNUSED(location_name);
}

void view::GamificationView::showLevelUpAnimation(int new_level, const QString &new_title)
{
  Q_UNUSED(new_title);

  if (m_level_label)
  {
    m_level_label->setText("Lv." + QString::number(new_level));
  }

  if (!m_opacity_effect)
  {
    m_opacity_effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacity_effect);
  }

  m_opacity_effect->setOpacity(1.0);

  if (m_level_animation)
  {
    m_level_animation->stop();
  }

  m_level_animation = new QPropertyAnimation(m_opacity_effect, "opacity", this);
  m_level_animation->setDuration(k_levelAnimationDurationMs);
  m_level_animation->setStartValue(1.0);
  m_level_animation->setEndValue(0.0);
  m_level_animation->start();
}
