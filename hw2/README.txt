# Stop Words Filter
CHANGES:
1. Add fileparser.h into searchshell.c headers
2. Add two functions in the fileparser
    SetFilter(val) : Filter words if val is true
    Unfiltered(word) : if this function return true, then word should not be filtered.

Method:
Use SetFilter to change the global status, then using Unfiltered(word) to filter words in the both query process and file processing process.
