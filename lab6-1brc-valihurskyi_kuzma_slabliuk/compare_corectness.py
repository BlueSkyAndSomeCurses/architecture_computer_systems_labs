import sys

user_file = sys.argv[1]
correct_file = sys.argv[2]

user_set = set()
correct_set = set()

with open(user_file, "r", encoding="utf-8") as my_file:
    with open(correct_file, "r", encoding="utf-8") as corr_file:
        for my_line, corr_line in zip(my_file, corr_file):
            user_trim = my_line[: my_line.find("=")]
            corr_trim = corr_line[: my_line.find("=")]
            if user_trim != corr_trim:
                print(user_trim, corr_trim)
