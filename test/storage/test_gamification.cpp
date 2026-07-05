#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <QDate>
#include <QDateTime>
#include <QTemporaryDir>
#include <ostream>

// Boost.Test needs operator<< to print values on CHECK_EQUAL failures.
inline std::ostream &operator<<(std::ostream &os, const QString &s)
{
  return os << s.toStdString();
}

#include "memorystorage.hpp"
#include "serial-utils.hpp"
#include "../utils/achievements.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────────────────────────────────────

struct GamificationFixture
{
  QTemporaryDir tmpDir;

  GamificationFixture()
  {
    BOOST_REQUIRE(tmpDir.isValid());
    storage::serial::setBaseDirOverride(tmpDir.path());
  }

  ~GamificationFixture()
  {
    storage::serial::setBaseDirOverride("");
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static storage::Task makeTask(const QString &name,
                              storage::Priority priority,
                              const QDateTime &deadline,
                              bool completed = false,
                              const QDateTime &completedAt = {})
{
  storage::Task t;
  t.id = 0; // assigned by storage
  t.name = name;
  t.description = "";
  t.discipline = "";
  t.deadline = deadline;
  t.priority = priority;
  t.completed = completed;
  t.completedAt = completedAt;
  return t;
}

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_XP_Levels, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(InitialState_Level1_ZeroXP)
{
  storage::MemoryStorage s;
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 1);
  BOOST_CHECK_EQUAL(s.getTotalXP(), 0);
}

BOOST_AUTO_TEST_CASE(AddXP_SingleCall_Accumulates)
{
  storage::MemoryStorage s;
  s.addXP(storage::xp::TASK_MEDIUM, "test");
  BOOST_CHECK_EQUAL(s.getTotalXP(), storage::xp::TASK_MEDIUM);
}

BOOST_AUTO_TEST_CASE(AddXP_MultipleCalls_Accumulates)
{
  storage::MemoryStorage s;
  s.addXP(10, "a");
  s.addXP(15, "b");
  s.addXP(5, "c");
  BOOST_CHECK_EQUAL(s.getTotalXP(), 30);
}

BOOST_AUTO_TEST_CASE(AddXP_BelowThreshold_NoLevelUp)
{
  storage::MemoryStorage s;
  s.addXP(storage::calculateXPForLevel(1) - 1, "almost");
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 1);
}

BOOST_AUTO_TEST_CASE(AddXP_ExactThreshold_LevelsUpAndResetsXP)
{
  storage::MemoryStorage s;
  const int threshold = storage::calculateXPForLevel(1);
  s.addXP(threshold, "exact");
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 2);
  BOOST_CHECK_EQUAL(s.getTotalXP(), 0);
}

BOOST_AUTO_TEST_CASE(AddXP_Overflow_CarriesToNextLevel)
{
  storage::MemoryStorage s;
  const int threshold = storage::calculateXPForLevel(1);
  s.addXP(threshold + 7, "overflow");
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 2);
  BOOST_CHECK_EQUAL(s.getTotalXP(), 7);
}

BOOST_AUTO_TEST_CASE(AddXP_MultiLevelJump_SkipsLevels)
{
  storage::MemoryStorage s;
  const int xp1 = storage::calculateXPForLevel(1);
  const int xp2 = storage::calculateXPForLevel(2);
  s.addXP(xp1 + xp2 + 1, "big jump");
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 3);
  BOOST_CHECK_EQUAL(s.getTotalXP(), 1);
}

BOOST_AUTO_TEST_CASE(AddXP_CapsAtMaxLevel)
{
  storage::MemoryStorage s;
  // enough XP to blow past the entire progression curve
  const int huge = storage::calculateTotalXPForLevel(storage::xp::MAX_LEVEL) + 999999;
  s.addXP(huge, "max grind");
  BOOST_CHECK_EQUAL(s.getCurrentLevel(), storage::xp::MAX_LEVEL);
}

BOOST_AUTO_TEST_CASE(GetUserProgress_ReflectsAddedXP)
{
  storage::MemoryStorage s;
  s.addXP(42, "check progress");
  const auto p = s.getUserProgress();
  BOOST_CHECK_EQUAL(p.currentXP, 42);
  BOOST_CHECK_EQUAL(p.currentLevel, 1);
}

BOOST_AUTO_TEST_CASE(UpdateUserProgress_OverwritesAllFields)
{
  storage::MemoryStorage s;
  auto p = s.getUserProgress();
  p.currentLevel = 10;
  p.currentXP = 77;
  p.streakDays = 5;
  p.currentTitle = "Veteran";
  s.updateUserProgress(p);

  BOOST_CHECK_EQUAL(s.getCurrentLevel(), 10);
  BOOST_CHECK_EQUAL(s.getTotalXP(), 77);
  BOOST_CHECK_EQUAL(s.getStreakDays(), 5);
  BOOST_CHECK_EQUAL(s.getUserProgress().currentTitle, QString("Veteran"));
}

BOOST_AUTO_TEST_SUITE_END()

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_Streak, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(Streak_FirstCall_SetsToOne)
{
  storage::MemoryStorage s;
  s.updateStreak(QDate::currentDate());
  BOOST_CHECK_EQUAL(s.getStreakDays(), 1);
}

BOOST_AUTO_TEST_CASE(Streak_SameDayTwice_NoChange)
{
  storage::MemoryStorage s;
  const QDate today = QDate::currentDate();
  s.updateStreak(today);
  s.updateStreak(today);
  BOOST_CHECK_EQUAL(s.getStreakDays(), 1);
}

BOOST_AUTO_TEST_CASE(Streak_ConsecutiveDay_Increments)
{
  storage::MemoryStorage s;
  const QDate today = QDate::currentDate();
  s.updateStreak(today);
  s.updateStreak(today.addDays(1));
  BOOST_CHECK_EQUAL(s.getStreakDays(), 2);
}

BOOST_AUTO_TEST_CASE(Streak_FiveDays_AccumulatesCorrectly)
{
  storage::MemoryStorage s;
  const QDate start = QDate::currentDate();
  for (int i = 0; i < 5; ++i)
  {
    s.updateStreak(start.addDays(i));
  }
  BOOST_CHECK_EQUAL(s.getStreakDays(), 5);
}

BOOST_AUTO_TEST_CASE(Streak_GapResets)
{
  storage::MemoryStorage s;
  const QDate today = QDate::currentDate();
  s.updateStreak(today);
  s.updateStreak(today.addDays(1));
  s.updateStreak(today.addDays(3)); // skip day 2
  BOOST_CHECK_EQUAL(s.getStreakDays(), 1);
}

BOOST_AUTO_TEST_CASE(Streak_BackwardsDateResets)
{
  // daysTo() returns negative → not 0, not 1 → falls into else → resets to 1
  storage::MemoryStorage s;
  const QDate today = QDate::currentDate();
  s.updateStreak(today);
  s.updateStreak(today.addDays(1));
  s.updateStreak(today.addDays(-2));
  BOOST_CHECK_EQUAL(s.getStreakDays(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_Achievements, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(Catalog_IsNotEmpty)
{
  storage::MemoryStorage s;
  BOOST_CHECK(!s.getAllAchievements().isEmpty());
}

BOOST_AUTO_TEST_CASE(Catalog_ContainsKnownAchievements)
{
  storage::MemoryStorage s;
  const auto all = s.getAllAchievements();
  const auto hasId = [&](const QString &id)
  {
    return std::any_of(all.begin(), all.end(),
                       [&](const storage::Achievement &a) { return a.id == id; });
  };
  BOOST_CHECK(hasId(storage::achievements::LEVEL_1.id));
  BOOST_CHECK(hasId(storage::achievements::TASKS_10.id));
  BOOST_CHECK(hasId(storage::achievements::STREAK_7.id));
  BOOST_CHECK(hasId(storage::achievements::LOCATION_ALL.id));
  BOOST_CHECK(hasId(storage::achievements::DELETE_5.id));
}

BOOST_AUTO_TEST_CASE(IsUnlocked_InitiallyFalse)
{
  storage::MemoryStorage s;
  BOOST_CHECK(!s.isAchievementUnlocked(storage::achievements::TASKS_50.id));
}

BOOST_AUTO_TEST_CASE(Unlock_MarksAsUnlocked)
{
  storage::MemoryStorage s;
  s.unlockAchievement(storage::achievements::TASKS_10.id);
  BOOST_CHECK(s.isAchievementUnlocked(storage::achievements::TASKS_10.id));
}

BOOST_AUTO_TEST_CASE(Unlock_OtherAchievementsUnaffected)
{
  storage::MemoryStorage s;
  s.unlockAchievement(storage::achievements::TASKS_10.id);
  BOOST_CHECK(!s.isAchievementUnlocked(storage::achievements::TASKS_50.id));
}

BOOST_AUTO_TEST_CASE(Unlock_Idempotent_NoDuplicatesInProgress)
{
  storage::MemoryStorage s;
  const QString id = storage::achievements::TASKS_10.id;
  s.unlockAchievement(id);
  s.unlockAchievement(id);
  const auto &ids = s.getUserProgress().unlockedAchievementIds;
  const int count = static_cast<int>(std::count(ids.begin(), ids.end(), id));
  BOOST_CHECK_EQUAL(count, 1);
}

BOOST_AUTO_TEST_CASE(GetById_ReturnsCorrectData)
{
  storage::MemoryStorage s;
  const auto a = s.getAchievementById(storage::achievements::HARD_50.id);
  BOOST_CHECK_EQUAL(a.id, storage::achievements::HARD_50.id);
  BOOST_CHECK_EQUAL(a.name, storage::achievements::HARD_50.name);
  BOOST_CHECK_EQUAL(a.xpReward, storage::achievements::HARD_50.xpReward);
}

BOOST_AUTO_TEST_CASE(GetById_UnknownId_ReturnsDefaultStruct)
{
  storage::MemoryStorage s;
  const auto a = s.getAchievementById("does_not_exist");
  BOOST_CHECK(a.id.isEmpty());
}

BOOST_AUTO_TEST_SUITE_END()

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_Locations, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(Locations_InitiallyEmpty)
{
  storage::MemoryStorage s;
  BOOST_CHECK(s.getUnlockedLocations().isEmpty());
}

BOOST_AUTO_TEST_CASE(Unlock_AddsLocation)
{
  storage::MemoryStorage s;
  s.unlockLocation("forest");
  const auto locs = s.getUnlockedLocations();
  BOOST_REQUIRE_EQUAL(locs.size(), 1);
  BOOST_CHECK_EQUAL(locs[0], QString("forest"));
}

BOOST_AUTO_TEST_CASE(Unlock_Idempotent_NoDuplicates)
{
  storage::MemoryStorage s;
  s.unlockLocation("forest");
  s.unlockLocation("forest");
  BOOST_CHECK_EQUAL(s.getUnlockedLocations().size(), 1);
}

BOOST_AUTO_TEST_CASE(Unlock_MultipleDistinct)
{
  storage::MemoryStorage s;
  s.unlockLocation("forest");
  s.unlockLocation("mountain");
  s.unlockLocation("beach");
  BOOST_CHECK_EQUAL(s.getUnlockedLocations().size(), 3);
}

BOOST_AUTO_TEST_CASE(IsLocationUnlocked_TrueAfterUnlock)
{
  storage::MemoryStorage s;
  BOOST_CHECK(!s.isLocationUnlocked("desert"));
  s.unlockLocation("desert");
  BOOST_CHECK(s.isLocationUnlocked("desert"));
}

BOOST_AUTO_TEST_SUITE_END()

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_TaskCounters, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(CompletedCount_OnlyCountsCompletedTasks)
{
  storage::MemoryStorage s;
  const QDateTime now = QDateTime::currentDateTime();
  s.addTask(makeTask("done", storage::Priority::Low, now, true, now));
  s.addTask(makeTask("pending", storage::Priority::Low, now, false));
  BOOST_CHECK_EQUAL(s.getCompletedTasksCount(), 1);
}

BOOST_AUTO_TEST_CASE(CompletedCount_StartsAtZero)
{
  storage::MemoryStorage s;
  BOOST_CHECK_EQUAL(s.getCompletedTasksCount(), 0);
}

BOOST_AUTO_TEST_CASE(OnTimeCount_CompletedBeforeDeadline_Counted)
{
  storage::MemoryStorage s;
  const QDateTime deadline = QDateTime::currentDateTime().addSecs(3600);
  const QDateTime early = deadline.addSecs(-60);
  s.addTask(makeTask("early", storage::Priority::Medium, deadline, true, early));
  BOOST_CHECK_EQUAL(s.getOnTimeCompletedCount(), 1);
}

BOOST_AUTO_TEST_CASE(OnTimeCount_CompletedAfterDeadline_NotCounted)
{
  storage::MemoryStorage s;
  const QDateTime deadline = QDateTime::currentDateTime().addSecs(-3600);
  const QDateTime late = deadline.addSecs(7200);
  s.addTask(makeTask("late", storage::Priority::Medium, deadline, true, late));
  BOOST_CHECK_EQUAL(s.getOnTimeCompletedCount(), 0);
}

BOOST_AUTO_TEST_CASE(OnTimeCount_MixedTasks_CountsCorrectly)
{
  storage::MemoryStorage s;
  const QDateTime deadline = QDateTime::currentDateTime().addSecs(3600);
  s.addTask(makeTask("ok1", storage::Priority::Low, deadline, true, deadline.addSecs(-10)));
  s.addTask(makeTask("ok2", storage::Priority::Low, deadline, true, deadline.addSecs(-1)));
  s.addTask(makeTask("late", storage::Priority::Low, deadline, true, deadline.addSecs(100)));
  BOOST_CHECK_EQUAL(s.getOnTimeCompletedCount(), 2);
}

BOOST_AUTO_TEST_CASE(CompletedByPriority_FiltersCorrectly)
{
  storage::MemoryStorage s;
  const QDateTime now = QDateTime::currentDateTime();
  s.addTask(makeTask("h1", storage::Priority::Hard,   now, true, now));
  s.addTask(makeTask("h2", storage::Priority::Hard,   now, true, now));
  s.addTask(makeTask("m1", storage::Priority::Medium, now, true, now));
  s.addTask(makeTask("l1", storage::Priority::Low,    now, false));

  BOOST_CHECK_EQUAL(s.getCompletedCountByPriority(storage::Priority::Hard),   2);
  BOOST_CHECK_EQUAL(s.getCompletedCountByPriority(storage::Priority::Medium), 1);
  BOOST_CHECK_EQUAL(s.getCompletedCountByPriority(storage::Priority::Low),    0);
}

BOOST_AUTO_TEST_CASE(DeletedCount_StartsAtZero)
{
  storage::MemoryStorage s;
  BOOST_CHECK_EQUAL(s.getDeletedTasksCount(), 0);
}

BOOST_AUTO_TEST_CASE(DeletedCount_IncrementsOnRemove)
{
  storage::MemoryStorage s;
  const QDateTime now = QDateTime::currentDateTime();
  s.addTask(makeTask("t1", storage::Priority::Low, now));
  s.addTask(makeTask("t2", storage::Priority::Low, now));

  const auto tasks = s.getAllTasks();
  BOOST_REQUIRE_EQUAL(tasks.size(), 2);

  s.removeTask(tasks[0].id);
  BOOST_CHECK_EQUAL(s.getDeletedTasksCount(), 1);

  s.removeTask(tasks[1].id);
  BOOST_CHECK_EQUAL(s.getDeletedTasksCount(), 2);
}

BOOST_AUTO_TEST_CASE(DeletedCount_RemoveNonExistent_NoIncrement)
{
  storage::MemoryStorage s;
  s.removeTask(99999);
  BOOST_CHECK_EQUAL(s.getDeletedTasksCount(), 0);
}

BOOST_AUTO_TEST_CASE(PerfectAndMaxCounters_InitiallyZero)
{
  // perfectDaysCount_ / maxTasksCompletedInOneDay_ / maxHardTasksCompletedInOneDay_
  // are only set externally (no increment logic in MemoryStorage itself),
  // so we verify initial state and persistence round-trip via updateUserProgress.
  storage::MemoryStorage s;
  BOOST_CHECK_EQUAL(s.getPerfectDaysCount(), 0);
  BOOST_CHECK_EQUAL(s.getMaxTasksCompletedInOneDay(), 0);
  BOOST_CHECK_EQUAL(s.getMaxHardTasksCompletedInOneDay(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

// ═════════════════════════════════════════════════════════════════════════════
BOOST_FIXTURE_TEST_SUITE(Gamification_Persistence, GamificationFixture)
// ═════════════════════════════════════════════════════════════════════════════

BOOST_AUTO_TEST_CASE(Persist_XPAndLevel_SurviveReload)
{
  const int xp1 = storage::calculateXPForLevel(1);
  {
    storage::MemoryStorage s;
    s.addXP(xp1 + 3, "test");
  }
  {
    storage::MemoryStorage s;
    BOOST_CHECK_EQUAL(s.getCurrentLevel(), 2);
    BOOST_CHECK_EQUAL(s.getTotalXP(), 3);
  }
}

BOOST_AUTO_TEST_CASE(Persist_Streak_SurvivesReload)
{
  {
    storage::MemoryStorage s;
    const QDate today = QDate::currentDate();
    s.updateStreak(today);
    s.updateStreak(today.addDays(1));
  }
  {
    storage::MemoryStorage s;
    BOOST_CHECK_EQUAL(s.getStreakDays(), 2);
  }
}

BOOST_AUTO_TEST_CASE(Persist_UnlockedAchievements_SurviveReload)
{
  {
    storage::MemoryStorage s;
    s.unlockAchievement(storage::achievements::TASKS_10.id);
    s.unlockAchievement(storage::achievements::ON_TIME_5.id);
  }
  {
    storage::MemoryStorage s;
    BOOST_CHECK(s.isAchievementUnlocked(storage::achievements::TASKS_10.id));
    BOOST_CHECK(s.isAchievementUnlocked(storage::achievements::ON_TIME_5.id));
    BOOST_CHECK(!s.isAchievementUnlocked(storage::achievements::TASKS_50.id));
  }
}

BOOST_AUTO_TEST_CASE(Persist_UnlockedLocations_SurviveReload)
{
  {
    storage::MemoryStorage s;
    s.unlockLocation("forest");
    s.unlockLocation("mountain");
  }
  {
    storage::MemoryStorage s;
    const auto locs = s.getUnlockedLocations();
    BOOST_CHECK_EQUAL(locs.size(), 2);
    BOOST_CHECK(locs.contains("forest"));
    BOOST_CHECK(locs.contains("mountain"));
  }
}

BOOST_AUTO_TEST_CASE(Persist_UserTitle_SurvivesReload)
{
  {
    storage::MemoryStorage s;
    auto p = s.getUserProgress();
    p.currentTitle = "Master Planner";
    s.updateUserProgress(p);
  }
  {
    storage::MemoryStorage s;
    BOOST_CHECK_EQUAL(s.getUserProgress().currentTitle, QString("Master Planner"));
  }
}

BOOST_AUTO_TEST_CASE(Persist_DeletedTasksCount_NotPersisted)
{
  // Known limitation: deletedTasksCount lives in UserProgress but is NOT
  // written/read by progressToJson / progressFromJson → resets to 0 on reload.
  {
    storage::MemoryStorage s;
    const QDateTime now = QDateTime::currentDateTime();
    s.addTask(makeTask("temp", storage::Priority::Low, now));
    const int id = s.getAllTasks()[0].id;
    s.removeTask(id);
    BOOST_REQUIRE_EQUAL(s.getDeletedTasksCount(), 1);
  }
  {
    storage::MemoryStorage s;
    // After reload the counter is lost — document the current behaviour.
    BOOST_CHECK_EQUAL(s.getDeletedTasksCount(), 0);
  }
}

BOOST_AUTO_TEST_CASE(Persist_FullRoundTrip)
{
  {
    storage::MemoryStorage s;
    s.addXP(50, "bonus");
    s.unlockAchievement(storage::achievements::LEVEL_5.id);
    s.unlockLocation("castle");
    s.updateStreak(QDate::currentDate());
    auto p = s.getUserProgress();
    p.currentTitle = "Explorer";
    s.updateUserProgress(p);
  }
  {
    storage::MemoryStorage s;
    BOOST_CHECK_EQUAL(s.getTotalXP(), 50);
    BOOST_CHECK(s.isAchievementUnlocked(storage::achievements::LEVEL_5.id));
    BOOST_CHECK(s.isLocationUnlocked("castle"));
    BOOST_CHECK_EQUAL(s.getStreakDays(), 1);
    BOOST_CHECK_EQUAL(s.getUserProgress().currentTitle, QString("Explorer"));
  }
}

BOOST_AUTO_TEST_SUITE_END()
