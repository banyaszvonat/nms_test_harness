This program is intended to be run by WinAFL, instrumenting the DLL being fuzzed with DynamoRIO.

Dependencies:
- Profanityfilter_x64_v120.dll (Up to date version bundled with No Man's Sky as of September 2018)
- Memory dump of the profanity dictionary (will be released later)

Compile with

```
cl /Zi main.c
```

Example invocation:

```
D:\path\to\afl-fuzz.exe -i ..\testcases\in -o ..\testcases\out -D D:\path\to\DynamoRIO\bin64 -t 100000 -- -target_method harness_do_fuzz -coverage_module ProfanityFilter_x64_v120.dll -fuzz_iterations 5000 -target_module main.exe -nargs 2 -- main.exe @@
```

