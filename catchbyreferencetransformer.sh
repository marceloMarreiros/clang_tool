#!/bin/bash

# Hardcoded paths relative to the current path
CURRENT_DIR=$(pwd)

# Expected current path
# sonae_develop/fusion-configuration-assets/tools/refactoring-tools/catchbyreference

# Possible compile_commands.json paths
POSSIBLE_PATHS=(
    "$CURRENT_DIR/../../../../../build/shell/fusionIV-debug"
    "$CURRENT_DIR/../../../../../build/shell/fusionIV-debug/.qtc_clangd"
    "$CURRENT_DIR/../../../../../build/shell/fusionIV-debug/fusion-system-IV/.qtc_clangd"
    "$CURRENT_DIR/../../../../../build/Desktop-Debug/.qtc_clangd"
    "$CURRENT_DIR/../../../../../fusion-system-IV/build/Desktop-Debug/.qtc_clangd"
)

# Expected repository path
# repo_name/fusion-system-IV
DEFAULT_REPO_PATH="$CURRENT_DIR/../../../../../fusion-system-IV"

# Function to prompt the user for a path
prompt_for_path() {
    local prompt_message=$1
    local path
    read -p "$prompt_message" path
    echo "$path"
}

# Function to find compile_commands.json
find_compile_commands() {
    for path in "${POSSIBLE_PATHS[@]}"; do
        if [ -f "$path/compile_commands.json" ]; then
            echo "$path/compile_commands.json"
            return
        fi
    done
    echo ""
}

# Check if the compile_commands.json file exists in any of the possible paths
DEFAULT_COMPILE_COMMANDS_PATH=$(find_compile_commands)

# If not found, prompt the user for the path
if [ -z "$DEFAULT_COMPILE_COMMANDS_PATH" ]; then
    echo "Default compile commands file not found in any of the possible paths."
    DEFAULT_COMPILE_COMMANDS_PATH=$(prompt_for_path "Please provide the path to compile_commands.json: ")
fi

# Check if the hardcoded repo path is valid
if [ ! -d "$DEFAULT_REPO_PATH" ]; then
    echo "Default repository path not found at $DEFAULT_REPO_PATH"
    DEFAULT_REPO_PATH=$(prompt_for_path "Please provide the path to the repository: ")
fi

# Ensure the provided paths are valid
if [ ! -f "$DEFAULT_COMPILE_COMMANDS_PATH" ]; then
    echo "Error: Compile commands file not found at $DEFAULT_COMPILE_COMMANDS_PATH"
    exit 1
fi

if [ ! -d "$DEFAULT_REPO_PATH" ]; then
    echo "Error: Repository path not found at $DEFAULT_REPO_PATH"
    exit 1
fi

# Run the Clang tool
./clang-tool "$@" 2>/dev/null -p "$DEFAULT_COMPILE_COMMANDS_PATH" "$DEFAULT_REPO_PATH"