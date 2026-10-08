import os
import sys
import shutil
import subprocess
import stat
import time

# --- Configuration ---
PROGRAM = "./copytree"
SRC_DIR = "test_src_comprehensive"
DST_DIR = "test_dest_comprehensive"

# --- ANSI Colors for Output ---
GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"
YELLOW = "\033[93m"

def log(test_name, status, message=""):
    """Helper to print colored status messages."""
    if status == "PASS":
        print(f"{GREEN}[PASS] {test_name}{RESET}")
    else:
        print(f"{RED}[FAIL] {test_name}{RESET}")
        if message:
            print(f"       Reason: {message}")

def force_remove_readonly(func, path, exc_info):
    """
    Error handler for shutil.rmtree.
    Handles read-only files and os.open errors safely.
    """
    # 1. Clear the read-only bit to allow modification
    try:
        os.chmod(path, stat.S_IWRITE | stat.S_IREAD | stat.S_IEXEC)
    except Exception:
        pass
    
    # 2. Retry the operation
    try:
        # If the function was os.open, it needs flags, so we catch that error
        func(path)
    except (TypeError, OSError):
        # If func failed (e.g. os.open missing flags), we try brute-force removal
        try:
            if os.path.isdir(path):
                os.rmdir(path)
            else:
                os.unlink(path)
        except Exception:
            pass # Best effort

def clean_env():
    """Removes test directories ensuring no permission errors."""
    if os.path.exists(SRC_DIR):
        shutil.rmtree(SRC_DIR, onerror=force_remove_readonly)
    if os.path.exists(DST_DIR):
        shutil.rmtree(DST_DIR, onerror=force_remove_readonly)
    
    # Final fallback if shutil failed
    if os.path.exists(DST_DIR) or os.path.exists(SRC_DIR):
        subprocess.run(["/bin/rm", "-rf", SRC_DIR, DST_DIR], stderr=subprocess.DEVNULL)

def create_basic_source():
    """Creates a simple source directory structure."""
    clean_env()
    os.makedirs(f"{SRC_DIR}/subdir")
    with open(f"{SRC_DIR}/file1.txt", "w") as f:
        f.write("content 1")
    with open(f"{SRC_DIR}/subdir/file2.txt", "w") as f:
        f.write("content 2")

# --- Test Cases ---

def test_existing_empty_directory():
    """
    Scenario: The destination directory already exists but is empty.
    Expected: The program should proceed without error.
    """
    create_basic_source()
    os.makedirs(DST_DIR) # Create empty destination

    result = subprocess.run([PROGRAM, SRC_DIR, DST_DIR], capture_output=True, text=True)

    if result.returncode != 0:
        log("Existing Empty Directory", "FAIL", f"Program returned error code: {result.returncode}")
        print(f"Stderr: {result.stderr}")
        return

    if os.path.exists(f"{DST_DIR}/file1.txt"):
        log("Existing Empty Directory", "PASS")
    else:
        log("Existing Empty Directory", "FAIL", "Files were not copied.")

def test_existing_non_empty_directory():
    """
    Scenario: The destination directory exists and contains files.
    Expected: The program MUST FAIL to avoid conflicts.
    """
    create_basic_source()
    os.makedirs(DST_DIR)
    
    # Create a pre-existing file
    with open(f"{DST_DIR}/pre_existing.txt", "w") as f:
        f.write("I was here before")

    # Run program
    result = subprocess.run([PROGRAM, SRC_DIR, DST_DIR], capture_output=True, text=True)

    # We EXPECT failure (Exit code 1) because the folder is not empty
    if result.returncode != 0:
        log("Existing Non-Empty Directory (Strict Check)", "PASS")
    else:
        log("Existing Non-Empty Directory (Strict Check)", "FAIL", "Program should have failed but returned success!")

def test_source_is_file():
    """
    Scenario: The user provides a file path as the source instead of a directory.
    Expected: Fail gracefully.
    """
    create_basic_source()
    file_src = f"{SRC_DIR}/file1.txt"
    
    result = subprocess.run([PROGRAM, file_src, DST_DIR], capture_output=True, text=True)

    # Check for failure code AND error message
    is_failure = result.returncode != 0
    # Looking for 'usage' because that's what your code prints
    has_err_msg = "usage" in result.stderr.lower() or "directory" in result.stderr.lower()

    if is_failure and has_err_msg:
        log("Source is a File (Invalid Input)", "PASS")
    else:
        log("Source is a File (Invalid Input)", "FAIL", f"Stderr: {result.stderr}")

def test_destination_is_file():
    """
    Scenario: The destination path is a FILE.
    Expected: Fail.
    """
    create_basic_source()
    with open(DST_DIR, "w") as f:
        f.write("I am a file blocking the directory")

    result = subprocess.run([PROGRAM, SRC_DIR, DST_DIR], capture_output=True, text=True)

    if result.returncode != 0:
        log("Destination is a File (Conflict)", "PASS")
    else:
        log("Destination is a File (Conflict)", "FAIL", "Program should have failed.")

def test_permissions_readonly_source():
    """
    Scenario: Source file has read-only permissions.
    With -p flag, destination should also be read-only.
    """
    create_basic_source()
    src_file = f"{SRC_DIR}/file1.txt"
    os.chmod(src_file, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH) # 0444

    result = subprocess.run([PROGRAM, "-p", SRC_DIR, DST_DIR], capture_output=True, text=True)

    if result.returncode != 0:
        log("Permissions Flag (-p)", "FAIL", "Program crashed")
        return

    dest_file = f"{DST_DIR}/file1.txt"
    if not os.path.exists(dest_file):
        log("Permissions Flag (-p)", "FAIL", "File not created")
        return

    mode = stat.S_IMODE(os.stat(dest_file).st_mode)
    if not (mode & stat.S_IWUSR):
        log("Permissions Flag (-p)", "PASS")
    else:
        log("Permissions Flag (-p)", "FAIL", f"Permissions not copied. Mode: {oct(mode)}")

# --- Main Execution ---

if __name__ == "__main__":
    if not os.path.exists(PROGRAM):
        print(f"{RED}Error: Executable '{PROGRAM}' not found.{RESET}")
        sys.exit(1)

    print(f"{YELLOW}--- Starting Comprehensive Tests ---{RESET}")
    
    try:
        test_existing_empty_directory()
        test_existing_non_empty_directory()
        test_source_is_file()
        test_destination_is_file()
        test_permissions_readonly_source()
    except Exception as e:
        print(f"{RED}Test Script Error: {e}{RESET}")
    finally:
        clean_env()
        print(f"{YELLOW}--- Tests Finished ---{RESET}")