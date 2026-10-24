use hell_machine::start_hell_machine;

mod check_file_reading;
mod config_parser;
mod hell_machine;

fn main() {
    let config_path = check_file_reading::check_file_reading();

    let config = config_parser::Config::from_file(config_path).expect("Error parsing config");

    start_hell_machine(&config);
}
