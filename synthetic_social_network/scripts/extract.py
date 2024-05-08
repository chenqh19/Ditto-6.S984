import numpy as np
import matplotlib.pyplot as plt

def find_second_smallest(numbers):
    unique_numbers = sorted(set(numbers))
    if len(unique_numbers) > 1:
        return unique_numbers[1]
    else:
        return None

def find_mid_avg(numbers):
    unique_numbers = sorted(set(numbers))
    return unique_numbers[1]
    # if len(unique_numbers) > 1:
    #     return round((unique_numbers[1]+unique_numbers[2]+unique_numbers[3])/3, 1)
    # else:
    #     return None

def find_min(numbers):
    unique_numbers = sorted(set(numbers))
    return unique_numbers[0]

def main():
    count = 0
    results = []
    with open('../output/example.txt', 'r') as file:
        lines = file.readlines()
        for i in range(len(lines)):
            if "new_config" in lines[i]:
                if i + 3 < len(lines):
                    numbers = [float(line.split()[0]) for line in lines[i+1:i+4]]
                    rep_num = find_min(numbers)
                    if rep_num is not None:
                        results.append(rep_num)
                    else:
                        print(f"Not enough numbers at {count} to find the latency.")
                count += 1
    print(results)

    print(f"Total count: {count}")

if __name__ == "__main__":
    main()