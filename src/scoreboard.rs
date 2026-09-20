use chrono::Local;
use serde::{Deserialize, Serialize};
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::Path;

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ScoreEntry {
    pub name: String,
    pub score: i32,
    pub date: String,
}

#[derive(Debug, Clone)]
pub struct ScoreboardManager {
    filepath: String,
    scores: Vec<ScoreEntry>,
}

impl ScoreboardManager {
    pub fn new(filepath: &str) -> Self {
        let mut sm = Self {
            filepath: filepath.to_string(),
            scores: Vec::new(),
        };
        sm.load_scores();
        sm
    }

    pub fn add_score(&mut self, name: &str, score: i32) {
        if score <= 0 {
            return;
        }

        let date_str = Local::now().format("%Y-%m-%d").to_string();
        self.scores.push(ScoreEntry {
            name: name.to_string(),
            score,
            date: date_str,
        });

        self.scores.sort_by_key(|a| std::cmp::Reverse(a.score));

        if self.scores.len() > 10 {
            self.scores.truncate(10);
        }

        self.save_scores();
    }

    pub fn get_scores(&self) -> &[ScoreEntry] {
        &self.scores
    }

    pub fn load_scores(&mut self) {
        let path = Path::new(&self.filepath);
        if path.exists() {
            if let Ok(mut file) = File::open(path) {
                let mut content = String::new();
                if file.read_to_string(&mut content).is_ok() {
                    if let Ok(parsed) = serde_json::from_str::<Vec<ScoreEntry>>(&content) {
                        self.scores = parsed;
                    }
                }
            }
        }
    }

    pub fn save_scores(&self) {
        let path = Path::new(&self.filepath);
        if let Some(parent) = path.parent() {
            let _ = fs::create_dir_all(parent);
        }

        if let Ok(json) = serde_json::to_string_pretty(&self.scores) {
            if let Ok(mut file) = File::create(path) {
                let _ = file.write_all(json.as_bytes());
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::tempdir;

    #[test]
    fn test_scoreboard_unit_add_and_sort() {
        let dir = tempdir().unwrap();
        let path = dir.path().join("scores.json");
        let mut sm = ScoreboardManager::new(path.to_str().unwrap());

        sm.add_score("Alice", 100);
        sm.add_score("Bob", 250);
        sm.add_score("Charlie", 180);

        let scores = sm.get_scores();
        assert_eq!(scores.len(), 3);
        assert_eq!(scores[0].name, "Bob");
        assert_eq!(scores[0].score, 250);
        assert_eq!(scores[1].name, "Charlie");
        assert_eq!(scores[1].score, 180);
        assert_eq!(scores[2].name, "Alice");
        assert_eq!(scores[2].score, 100);
    }

    #[test]
    fn test_scoreboard_unit_ignores_non_positive_scores() {
        let dir = tempdir().unwrap();
        let path = dir.path().join("scores.json");
        let mut sm = ScoreboardManager::new(path.to_str().unwrap());

        sm.add_score("Zero", 0);
        sm.add_score("Neg", -50);
        assert!(sm.get_scores().is_empty());
    }
}
