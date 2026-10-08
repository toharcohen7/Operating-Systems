import os
import shutil
import subprocess
import stat
import time

# --- Configuration ---
PROGRAM = "./copytree"
SRC_DIR = "test_src_final"
DST_DIR = "test_dest_final"

# Colors for output
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
RESET = "\033[0m"


def log(test_name, status, msg=""):
    if status == "PASS":
        print(f"{GREEN}[PASS] {test_name}{RESET}")
    else:
        print(f"{RED}[FAIL] {test_name}{RESET}")
        if msg:
            print(f"       {msg}")


# --- Robust cleanup helpers ---

def force_remove(func, path, exc_info):
    """
    Error handler for shutil.rmtree.
    Fixes permissions and retries removal.
    """
    try:
        os.chmod(path, stat.S_IRWXU)
        func(path)
    except Exception:
        pass


def clean_env():
    """Forcefully removes test directories without manual rm -rf."""
    for d in (SRC_DIR, DST_DIR):
        if os.path.exists(d):
            try:
                shutil.rmtree(d, onerror=force_remove)
            except Exception:
                subprocess.run(
                    ["/bin/rm", "-rf", d],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )


# --- Test environment setup ---

def create_base_structure():
    """Creates a rich directory structure for testing."""
    clean_env()
    os.makedirs(SRC_DIR)

    # 1. Regular file
    with open(f"{SRC_DIR}/regular.txt", "w") as f:
        f.write("I am regular")

    # 2. Executable file (for -p test)
    exe_path = f"{SRC_DIR}/script.sh"
    with open(exe_path, "w") as f:
        f.write("#!/bin/bash")
    os.chmod(exe_path, 0o755)

    # 3. Hidden file
    with open(f"{SRC_DIR}/.hidden", "w") as f:
        f.write("I am hidden")

    # 4. Empty directory
    os.makedirs(f"{SRC_DIR}/empty_dir")

    # 5. Symlink to regular file
    os.symlink("regular.txt", f"{SRC_DIR}/link_to_regular")


# --- Program runner ---

def run_program(args, expect_fail=False):
    result = subprocess.run([PROGRAM] + args, capture_output=True, text=True)
    if expect_fail:
        return result.returncode != 0, result.stderr
    return result.returncode == 0, result.stderr


# --- Test Cases ---

def test_p_flag_only():
    create_base_structure()
    success, _ = run_program(["-p", SRC_DIR, DST_DIR])
    if not success:
        log("Flag -p Only", "FAIL", "Program crashed or returned error")
        return

    src_mode = os.stat(f"{SRC_DIR}/script.sh").st_mode & 0o777
    dst_mode = os.stat(f"{DST_DIR}/script.sh").st_mode & 0o777
    if src_mode != dst_mode:
        log("Flag -p Only", "FAIL", "Permissions mismatch")
        return

    if os.path.islink(f"{DST_DIR}/link_to_regular"):
        log("Flag -p Only", "FAIL", "Symlink should be dereferenced")
        return

    log("Flag -p Only", "PASS")


def test_l_flag_only():
    create_base_structure()
    success, _ = run_program(["-l", SRC_DIR, DST_DIR])
    if not success:
        log("Flag -l Only", "FAIL", "Program crashed")
        return

    if not os.path.islink(f"{DST_DIR}/link_to_regular"):
        log("Flag -l Only", "FAIL", "Symlink not preserved")
        return

    target = os.readlink(f"{DST_DIR}/link_to_regular")
    if target != "regular.txt":
        log("Flag -l Only", "FAIL", "Wrong link target")
        return

    log("Flag -l Only", "PASS")


def test_both_flags():
    create_base_structure()
    success, _ = run_program(["-l", "-p", SRC_DIR, DST_DIR])
    if not success:
        log("Both Flags (-l -p)", "FAIL")
        return

    src_mode = os.stat(f"{SRC_DIR}/script.sh").st_mode & 0o777
    dst_mode = os.stat(f"{DST_DIR}/script.sh").st_mode & 0o777
    is_link = os.path.islink(f"{DST_DIR}/link_to_regular")

    if src_mode == dst_mode and is_link:
        log("Both Flags (-l -p)", "PASS")
    else:
        log("Both Flags (-l -p)", "FAIL", "Permissions or link check failed")


def test_hidden_and_empty():
    create_base_structure()
    run_program([SRC_DIR, DST_DIR])

    if not os.path.exists(f"{DST_DIR}/.hidden"):
        log("Hidden & Empty", "FAIL", "Hidden file skipped")
        return

    if not os.path.exists(f"{DST_DIR}/empty_dir"):
        log("Hidden & Empty", "FAIL", "Empty directory skipped")
        return

    log("Hidden & Empty", "PASS")


def test_dangling_symlink():
    create_base_structure()
    os.symlink("does_not_exist", f"{SRC_DIR}/broken_link")

    run_program(["-l", SRC_DIR, DST_DIR])
    if os.path.islink(f"{DST_DIR}/broken_link"):
        log("Dangling Link (-l)", "PASS")
    else:
        log("Dangling Link (-l)", "FAIL")

    shutil.rmtree(DST_DIR, onerror=force_remove)

    success, stderr = run_program([SRC_DIR, DST_DIR])
    if success or "No such file" in stderr or "failed" in stderr:
        log("Dangling Link (No -l)", "PASS")
    else:
        log("Dangling Link (No -l)", "FAIL")


def test_permission_denied():
    create_base_structure()
    unreadable = f"{SRC_DIR}/unreadable.txt"
    with open(unreadable, "w") as f:
        f.write("secret")
    os.chmod(unreadable, 0o000)

    success, stderr = run_program([SRC_DIR, DST_DIR])
    if not os.path.exists(f"{DST_DIR}/unreadable.txt") or "Permission denied" in stderr:
        log("Permission Denied", "PASS")
    else:
        log("Permission Denied", "FAIL")

    os.chmod(unreadable, 0o777)


def test_invalid_input():
    create_base_structure()
    success, stderr = run_program([f"{SRC_DIR}/regular.txt", DST_DIR], expect_fail=True)
    if success and ("usage" in stderr or "Not a directory" in stderr or "failed" in stderr):
        log("Invalid Input (File as Source)", "PASS")
    else:
        log("Invalid Input (File as Source)", "FAIL")


def test_cyclic_symlink():
    create_base_structure()
    cycle = f"{SRC_DIR}/cycle_dir"
    os.makedirs(cycle)
    os.symlink(".", f"{cycle}/loop_link")

    try:
        result = subprocess.run(
            [PROGRAM, "-l", SRC_DIR, DST_DIR],
            capture_output=True,
            text=True,
            timeout=2,
        )
        if result.returncode == 0 and os.path.islink(f"{DST_DIR}/cycle_dir/loop_link"):
            log("Cyclic Symlink", "PASS")
        else:
            log("Cyclic Symlink", "FAIL")
    except subprocess.TimeoutExpired:
        log("Cyclic Symlink", "FAIL", "Infinite loop detected")


if __name__ == "__main__":
    if not os.path.exists(PROGRAM):
        print("Error: Compile your program first!")
        exit(1)

    print(f"--- Stress Testing {PROGRAM} ---")

    test_p_flag_only()
    test_l_flag_only()
    test_both_flags()
    test_hidden_and_empty()
    test_dangling_symlink()
    test_permission_denied()
    test_invalid_input()
    test_cyclic_symlink()

    clean_env()
    print("--- Completed ---")