pub use std::env;
use std::{fs::File, path::Path};

pub fn check_file_reading() -> String {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        eprintln!(
            "Wrong number of cmd arguments! Got {}, expected 1",
            args.len() - 1
        );
    }

    let config_path = Path::new(&args[1]);

    if !config_path.exists() || !config_path.is_file() {
        eprintln!("Argument you provided is not config path {}", args[1]);
    }

    if let Err(e) = File::open(config_path) {
        eprintln!("Error opening file {}", e);
    }

    args.into_iter().nth(1).unwrap()
}
