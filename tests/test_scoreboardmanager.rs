use blockeater_game::scoreboard::ScoreboardManager;
use tempfile::tempdir;

#[test]
fn test_adds_and_sorts_scores() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("scores.json");
    let mut sm = ScoreboardManager::new(file_path.to_str().unwrap());

    sm.add_score("PlayerC", 300);
    sm.add_score("PlayerA", 100);
    sm.add_score("PlayerB", 500);

    let scores = sm.get_scores();
    assert_eq!(scores.len(), 3);
    assert_eq!(scores[0].name, "PlayerB");
    assert_eq!(scores[0].score, 500);
    assert_eq!(scores[1].name, "PlayerC");
    assert_eq!(scores[1].score, 300);
    assert_eq!(scores[2].name, "PlayerA");
    assert_eq!(scores[2].score, 100);
}

#[test]
fn test_limits_scores_to_ten() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("scores.json");
    let mut sm = ScoreboardManager::new(file_path.to_str().unwrap());

    for i in 0..12 {
        sm.add_score(&format!("Player{}", i), i * 100);
    }

    let scores = sm.get_scores();
    assert_eq!(scores.len(), 10);
    assert_eq!(scores.last().unwrap().score, 200);
}

#[test]
fn test_saves_and_loads_scores() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("scores.json");

    // 1. Create manager, add scores, saves automatically
    {
        let mut sm1 = ScoreboardManager::new(file_path.to_str().unwrap());
        sm1.add_score("Hero", 9001);
        sm1.add_score("Zero", 10);
    }

    // 2. Verify file exists
    assert!(file_path.exists());

    // 3. New manager loads saved scores
    let sm2 = ScoreboardManager::new(file_path.to_str().unwrap());
    let scores = sm2.get_scores();
    assert_eq!(scores.len(), 2);
    assert_eq!(scores[0].name, "Hero");
    assert_eq!(scores[0].score, 9001);
    assert_eq!(scores[1].name, "Zero");
    assert_eq!(scores[1].score, 10);
}

#[test]
fn test_creates_file_on_add_if_non_existent() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("sub/dir/scores.json");
    assert!(!file_path.exists());

    let mut sm = ScoreboardManager::new(file_path.to_str().unwrap());
    assert!(sm.get_scores().is_empty());

    sm.add_score("First", 100);
    assert_eq!(sm.get_scores().len(), 1);
    assert!(file_path.exists());
}

#[test]
fn test_does_not_add_zero_or_negative_scores() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("scores.json");
    let mut sm = ScoreboardManager::new(file_path.to_str().unwrap());

    sm.add_score("Zero", 0);
    sm.add_score("Negative", -100);

    assert!(sm.get_scores().is_empty());
}
