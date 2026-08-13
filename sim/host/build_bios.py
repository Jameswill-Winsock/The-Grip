import os
import shutil
import subprocess
import sys
import tempfile

ROM_SIZE = 0x8000       # 32 KiB
RESET_OFFSET = 0x7FF0   # FFFF0 - F8000

SOURCE = "bios.txt"
OUTPUT = "bios.bin"


def find_nasm():
    nasm = shutil.which("nasm")

    if nasm:
        return nasm

    # Optional: allow nasm.exe beside this script
    local = os.path.join(os.path.dirname(__file__), "nasm.exe")

    if os.path.exists(local):
        return local

    print("ERROR: NASM not found.")
    print("Install NASM or put nasm.exe beside build_bios.py")
    sys.exit(1)


def main():
    here = os.path.dirname(os.path.abspath(__file__))

    source_path = os.path.join(here, SOURCE)
    output_path = os.path.join(here, OUTPUT)

    if not os.path.exists(source_path):
        print(f"ERROR: {SOURCE} not found")
        return 1

    with open(source_path, "r", encoding="utf-8") as f:
        user_code = f.read()

    #
    # We generate the boring ROM boilerplate automatically.
    #
    asm = f"""
bits 16
org 0

{user_code}

; ============================================================
; Pad up to 8088 reset vector
; Physical FFFF0h = offset 7FF0h inside our 32 KiB ROM
; ============================================================

times 0x7FF0 - ($ - $$) db 0xFF

reset_vector:
    jmp 0xF800:0x0000

; Fill remainder of 32 KiB ROM
times 0x8000 - ($ - $$) db 0xFF
"""

    nasm = find_nasm()

    with tempfile.NamedTemporaryFile(
        mode="w",
        suffix=".asm",
        delete=False,
        encoding="utf-8"
    ) as tmp:
        tmp.write(asm)
        temp_name = tmp.name

    try:
        print(f"NASM: {nasm}")
        print(f"Building {SOURCE} -> {OUTPUT}")

        result = subprocess.run(
            [
                nasm,
                "-f", "bin",
                temp_name,
                "-o", output_path
            ],
            text=True
        )

        if result.returncode != 0:
            print("BIOS BUILD FAILED")
            return result.returncode

        size = os.path.getsize(output_path)

        if size != ROM_SIZE:
            print(
                f"ERROR: BIOS is {size} bytes, "
                f"expected {ROM_SIZE}"
            )
            return 1

        print("BIOS BUILD OK")
        print(f"Size: {size} bytes")
        print(f"Output: {output_path}")

        return 0

    finally:
        try:
            os.remove(temp_name)
        except OSError:
            pass


if __name__ == "__main__":
    sys.exit(main())