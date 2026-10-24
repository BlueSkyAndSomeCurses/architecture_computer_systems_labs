Tg build this code, inside directory `extra_task/wordscount_par` write:

```
cargo build --release
```

to run the code enter

```
./target/release/wordscount_par <path_to_config>
```

takes:
Below you can see total time runs take when counting words in guttenberg library (subdirectory 7) on M4 Pro CPU with
24Gb RAM (8533 MT/s LPDDR5X) using program written in Rust.

| run №/threads | 1 ind. 1 merge | 2 ind. 1 merege | 2 ind. 2 merege | 4 ind. 2 merege | 6 ind. 4 merge |
|---------------|----------------|-----------------|-----------------|-----------------|----------------|
| 1             | 37210          | 22235           | 22059           | 13658           | 10836          |
| 2             | 37374          | 21821           | 22095           | 13687           | 10693          |
| 3             | 37742          | 21679           | 21758           | 13632           | 10602          |
| 4             | 37841          | 22046           | 21739           | 13589           | 10888          |
| 5             | 37077          | 21750           | 21666           | 13487           | 10696          |

Below you can find these same runs but for program written in c++.

| run №/threads | 1 ind. 1 merge | 2 ind. 1 merege | 2 ind. 2 merege | 4 ind. 2 merege | 6 ind. 4 merge |
|---------------|----------------|-----------------|-----------------|-----------------|----------------|
| 1             | 48562          | 27220           | 27794           | 22012           | 38308          |
| 2             | 46857          | 27297           | 28376           | 22109           | 31815          |
| 3             | 47693          | 27343           | 28497           | 22360           | 32446          |

We see that Rust version is considerably faster. There is 2 main reasons for that

1) In Rust we use `zip` library for unarchiving. It can handle only zip acrhives (while LibArchive supports other
   formats), this may influence performance.
2) Unicode handling. Rust has built-in bytes to utf-8 string method. Also, for segmentation we use high level wrappers
   for ICU. These may lead to ill-handled unicode and result in better performance.