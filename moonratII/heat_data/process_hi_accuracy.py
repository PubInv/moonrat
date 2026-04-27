#!/usr/bin/env python3

import csv
import sys


def parse_file(input_path):
    records = []

    with open(input_path, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]

    # Process in pairs
    for i in range(0, len(lines), 2):
        try:
            timestamp = int(lines[i])
            temperature = float(lines[i + 1])

            records.append({
                "timestamp": timestamp,
                "temperature_c": temperature
            })

        except IndexError:
            print(f"Warning: unmatched line at end of file: {lines[i]}")
        except ValueError:
            print(f"Warning: bad data near lines {i}-{i+1}")

    return records


def write_csv(records, output_path):
    with open(output_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=["timestamp", "temperature_c"])
        writer.writeheader()
        writer.writerows(records)


def main():
    if len(sys.argv) != 3:
        print("Usage: python temp_pairs_to_csv.py input.txt output.csv")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    records = parse_file(input_file)
    write_csv(records, output_file)

    print(f"Wrote {len(records)} rows to {output_file}")


if __name__ == "__main__":
    main()
