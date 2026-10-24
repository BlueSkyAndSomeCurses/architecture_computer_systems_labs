import os
import time
import pathlib
import zipfile
import locale
import io
from collections import Counter
from multiprocessing import Process, Queue, cpu_count, Manager
import re


class Config:
    def __init__(self, indir, out_by_a, out_by_n, indexing_extensions, archives_extensions,
                 max_file_size, indexing_processes, merging_processes,
                 filenames_queue_size, raw_files_queue_size, dictionaries_queue_size):
        self.indir = indir
        self.out_by_a = out_by_a
        self.out_by_n = out_by_n
        self.indexing_extensions = indexing_extensions
        self.archives_extensions = archives_extensions
        self.max_file_size = max_file_size
        self.indexing_processes = indexing_processes
        self.merging_processes = merging_processes
        self.filenames_queue_size = filenames_queue_size
        self.raw_files_queue_size = raw_files_queue_size
        self.dictionaries_queue_size = dictionaries_queue_size


def process_text(text: str) -> Counter:
    words = re.findall(r'\b\w+\b', text.lower())
    return Counter(words)


def process_archive(file_content: bytes, config: Config) -> Counter:
    word_count = Counter()
    try:
        with zipfile.ZipFile(io.BytesIO(file_content)) as archive:
            for file in archive.infolist():
                ext = pathlib.Path(file.filename).suffix
                if ext not in config.indexing_extensions and ext not in config.archives_extensions:
                    continue
                if file.file_size > config.max_file_size:
                    continue
                with archive.open(file) as f:
                    content = f.read().decode('utf-8', errors='ignore')
                    if ext in config.indexing_extensions:
                        word_count += process_text(content)
                    elif ext in config.archives_extensions:
                        word_count += process_archive(content.encode(), config)
    except Exception:
        pass
    return word_count


def read_files(filenames_queue: Queue, raw_files_queue: Queue, config: Config, reading_time):
    start_time = time.time()
    while True:
        item = filenames_queue.get()
        if item is None:
            break
        path, is_file = item
        with open(path, 'rb') as f:
            content = f.read()
        raw_files_queue.put((content, is_file))
    for _ in range(config.indexing_processes):
        raw_files_queue.put((b'', -1))
    reading_time.value = time.time() - start_time


def parse_and_index(raw_files_queue: Queue, dictionaries_queue: Queue, config: Config):
    while True:
        content, is_file = raw_files_queue.get()
        if is_file == -1:
            break
        if is_file:
            text = content.decode('utf-8', errors='ignore')
            word_count = process_text(text)
        else:
            word_count = process_archive(content, config)
        if word_count:
            dictionaries_queue.put(word_count)




def merge_worker(dictionaries_queue: Queue, output_queue: Queue):
    merged = Counter()
    while True:
        item = dictionaries_queue.get()
        if item is None:
            break
        merged.update(item)
    output_queue.put(merged)



def write_results(word_count: Counter, config: Config):
    sorted_alpha = sorted(word_count.items())
    with open(config.out_by_a, 'w', encoding='utf-8') as f:
        for word, count in sorted_alpha:
            f.write(f"{word}: {count}\n")

    sorted_freq = sorted(word_count.items(), key=lambda x: (-x[1], x[0]))
    with open(config.out_by_n, 'w', encoding='utf-8') as f:
        for word, count in sorted_freq:
            f.write(f"{word}: {count}\n")


def start_hell_machine(config: Config):
    manager = Manager()
    reading_time = manager.Value('d', 0.0)

    filenames_queue = Queue(config.filenames_queue_size)
    raw_files_queue = Queue(config.raw_files_queue_size)
    dictionaries_queue = Queue(config.dictionaries_queue_size)
    merged_output_queue = Queue()

    total_start = time.time()

    # 1. Populate filenames queue
    finding_start = time.time()
    for path in pathlib.Path(config.indir).rglob("*"):
        if not path.is_file():
            continue
        ext = path.suffix
        is_file = 1 if ext in config.indexing_extensions else 0
        is_archive = 1 if ext in config.archives_extensions else 0
        if not is_file and not is_archive:
            continue
        filenames_queue.put((str(path), is_file))
    filenames_queue.put(None)
    finding_end = time.time()
    finding_duration = finding_end - finding_start

    # 2. Read processes
    reader = Process(target=read_files, args=(filenames_queue, raw_files_queue, config, reading_time))
    reader.start()

    # 3. Indexing processes
    indexers = []
    for _ in range(config.indexing_processes):
        p = Process(target=parse_and_index, args=(raw_files_queue, dictionaries_queue, config))
        p.start()
        indexers.append(p)

    reader.join()
    for p in indexers:
        p.join()

    for _ in range(config.merging_processes):
        dictionaries_queue.put(None)

    # 4. Merge dictionaries
    mergers = []
    for _ in range(config.merging_processes):
        p = Process(target=merge_worker, args=(dictionaries_queue, merged_output_queue))
        p.start()
        mergers.append(p)

    for p in mergers:
        p.join()

    final_counter = Counter()
    while not merged_output_queue.empty():
        final_counter.update(merged_output_queue.get())

    write_start = time.time()
    write_results(final_counter, config)
    write_end = time.time()

    total_end = time.time()

    # Print all timings
    print(f"Total={int((total_end - total_start) * 1000)}")
    print(f"Finding={int(finding_duration * 1000)}")
    print(f"Reading={int(reading_time.value * 1000)}")
    print(f"Writing={int((write_end - write_start) * 1000)}")
