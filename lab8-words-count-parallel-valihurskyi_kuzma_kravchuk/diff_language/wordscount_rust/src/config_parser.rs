use std::error::Error;
use std::fmt;
use std::fs;
use std::num::ParseIntError;
use std::path::Path;
use std::str::FromStr;

#[allow(dead_code)]
#[derive(Debug, Clone)]
pub struct Config {
    pub indir: String,
    pub out_by_a: String,
    pub out_by_n: String,
    pub indexing_extensions: Vec<String>,
    pub archives_extensions: Vec<String>,
    pub max_file_size: u64,
    pub indexing_threads: u32,
    pub merging_threads: u32,
    pub filenames_queue_size: usize,
    pub raw_files_queue_size: usize,
    pub dictionaries_queue_size: usize,
}

#[derive(Debug)]
pub enum ConfigError {
    MissingField(String),
    ParseError(String),
}

impl fmt::Display for ConfigError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ConfigError::MissingField(field) => write!(f, "Missing required field: {}", field),
            ConfigError::ParseError(err) => write!(f, "Parse error: {}", err),
        }
    }
}

impl Error for ConfigError {}

impl From<ParseIntError> for ConfigError {
    fn from(err: ParseIntError) -> Self {
        ConfigError::ParseError(err.to_string())
    }
}

impl FromStr for Config {
    type Err = ConfigError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut indir: Option<String> = None;
        let mut out_by_a: Option<String> = None;
        let mut out_by_n: Option<String> = None;
        let mut indexing_extensions: Option<Vec<String>> = None;
        let mut archives_extensions: Option<Vec<String>> = None;
        let mut max_file_size: Option<u64> = None;
        let mut indexing_threads: Option<u32> = None;
        let mut merging_threads: Option<u32> = None;
        let mut filenames_queue_size: Option<usize> = None;
        let mut raw_files_queue_size: Option<usize> = None;
        let mut dictionaries_queue_size: Option<usize> = None;

        for line in s.lines() {
            let line = line.trim();
            if line.is_empty() || line.starts_with('#') {
                continue;
            }
            let line = line.split('#').next().unwrap().trim();
            let parts: Vec<&str> = line.splitn(2, '=').collect();
            if parts.len() != 2 {
                continue;
            }
            let key = parts[0].trim();
            let value = parts[1].trim();

            match key {
                "indir" => {
                    indir = Some(value.trim_matches('"').to_string());
                }
                "out_by_a" => {
                    out_by_a = Some(value.trim_matches('"').to_string());
                }
                "out_by_n" => {
                    out_by_n = Some(value.trim_matches('"').to_string());
                }
                "indexing_extensions" => {
                    indexing_extensions = Some(
                        value
                            .split_whitespace()
                            .map(|s| s[1..].to_string())
                            .collect(),
                    );
                }
                "archives_extensions" => {
                    archives_extensions = Some(
                        value
                            .split_whitespace()
                            .map(|s| s[1..].to_string())
                            .collect(),
                    );
                }
                "max_file_size" => {
                    max_file_size = Some(value.parse()?);
                }
                "indexing_threads" => {
                    indexing_threads = Some(value.parse()?);
                }
                "merging_threads" => {
                    merging_threads = Some(value.parse()?);
                }
                "filenames_queue_size" => {
                    filenames_queue_size = Some(value.parse()?);
                }
                "raw_files_queue_size" => {
                    raw_files_queue_size = Some(value.parse()?);
                }
                "dictionaries_queue_size" => {
                    dictionaries_queue_size = Some(value.parse()?);
                }
                unknown_keys => {
                    eprintln!("Unknown keys in config file {unknown_keys}")
                }
            }
        }

        let indir = indir.ok_or_else(|| ConfigError::MissingField("indir".into()))?;
        let out_by_a = out_by_a.ok_or_else(|| ConfigError::MissingField("out_by_a".into()))?;
        let out_by_n = out_by_n.ok_or_else(|| ConfigError::MissingField("out_by_n".into()))?;
        let indexing_extensions = indexing_extensions
            .ok_or_else(|| ConfigError::MissingField("indexing_extensions".into()))?;
        let archives_extensions = archives_extensions
            .ok_or_else(|| ConfigError::MissingField("archives_extensions".into()))?;
        let max_file_size =
            max_file_size.ok_or_else(|| ConfigError::MissingField("max_file_size".into()))?;
        let indexing_threads =
            indexing_threads.ok_or_else(|| ConfigError::MissingField("indexing_threads".into()))?;
        let merging_threads =
            merging_threads.ok_or_else(|| ConfigError::MissingField("merging_threads".into()))?;
        let filenames_queue_size = filenames_queue_size
            .ok_or_else(|| ConfigError::MissingField("filenames_queue_size".into()))?;
        let raw_files_queue_size = raw_files_queue_size
            .ok_or_else(|| ConfigError::MissingField("raw_files_queue_size".into()))?;
        let dictionaries_queue_size = dictionaries_queue_size
            .ok_or_else(|| ConfigError::MissingField("dictionaries_queue_size".into()))?;

        Ok(Config {
            indir,
            out_by_a,
            out_by_n,
            indexing_extensions,
            archives_extensions,
            max_file_size,
            indexing_threads,
            merging_threads,
            filenames_queue_size,
            raw_files_queue_size,
            dictionaries_queue_size,
        })
    }
}

impl Config {
    pub fn from_file<P: AsRef<Path>>(path: P) -> Result<Self, Box<dyn Error>> {
        let content = fs::read_to_string(path)?;
        Ok(content.parse()?)
    }
}
