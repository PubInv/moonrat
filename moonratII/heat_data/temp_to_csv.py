#!/usr/bin/env python3

import csv
import re
import sys


def parse_file(input_path):
    records = []
    current_timestamp = None

    # Regex to extract temperature
    temp_pattern = re.compile(r"Temp\s*:\s*([-+]?\d+(?:\.\d+)?)")

    with open(input_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            # Detect timestamp (line with only digits)
            if line.isdigit():
                current_timestamp = int(line)
                continue

            # Look for temperature
            match = temp_pattern.search(line)
            if match and current_timestamp is not None:
                temperature = float(match.group(1))

                records.append({
                    "timestamp": current_timestamp,
                    "temperature_c": temperature
                })

                current_timestamp = None  # reset after pairing

    return records


def write_csv(records, output_path):
    with open(output_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=["timestamp", "temperature_c"])
        writer.writeheader()
        writer.writerows(records)


def main():
    if len(sys.argv) != 3:
        print("Usage: python temp_to_csv.py input.txt output.csv")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    records = parse_file(input_file)
    write_csv(records, output_file)

    print(f"Wrote {len(records)} rows to {output_file}")


if __name__ == "__main__":
    main()
