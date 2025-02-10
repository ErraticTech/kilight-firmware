
# Enable setting VERSION in project() call
cmake_policy(SET CMP0048 NEW)

# Use new option() behavior
cmake_policy(SET CMP0077 NEW)

# Use new FetchContent_Declare() timestamp behavior
cmake_policy(SET CMP0135 NEW)

# Pico SDK calls FetchContent_Populate() the old way, need to set this to not emit warnings
cmake_policy(SET CMP0169 OLD)
