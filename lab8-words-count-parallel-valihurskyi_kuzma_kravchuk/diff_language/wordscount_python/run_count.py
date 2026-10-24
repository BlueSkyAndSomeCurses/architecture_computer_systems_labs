from word_count import Config, cpu_count, start_hell_machine

config = Config(
    indir="./data",
    out_by_a="./data/res_a.txt",
    out_by_n="./data/res_n.txt",
    indexing_extensions={".txt", ".md"},
    archives_extensions={".zip"},
    max_file_size=10_000_000,
    indexing_processes=cpu_count() // 2,
    merging_processes=2,
    filenames_queue_size=1000,
    raw_files_queue_size=1000,
    dictionaries_queue_size=1000,
)
start_hell_machine(config)