import argparse
import random


def caesar(text: str, key: int, decode=False) -> str:
    result = ""
    for char in text:
        if char.isalpha():
            shift = key % 26 if not decode else -key % 26
            if char.isupper():
                result += chr((ord(char) - ord('A') + shift) % 26 + ord('A'))
            else:
                result += chr((ord(char) - ord('a') + shift) % 26 + ord('a'))
        else:
            result += char
    return result


def vigenere(text: str, key: str, decode=False) -> str:
    result = ""
    key_index = 0
    for char in text:
        if char.isalpha():
            if char.isupper():
                char_value = ord(char.upper()) - ord('A')
                key_value = ord(key[key_index % len(key)].upper()) - ord('A')
            else:
                char_value = ord(char.lower()) - ord('a')
                key_value = ord(key[key_index % len(key)].lower()) - ord('a')
            encode_value = key_value if not decode else -key_value
            encode_value += char_value
            encode_value %= 26
            result += chr(encode_value + ord('A')) if char.isupper() else chr(encode_value + ord('a'))
            key_index += 1
        else:
            result += char
    return result


def vernam(message, key):
    fixed = ""
    for i in range(len(message)):
        fixed += chr(ord(message[i]) ^ ord(key[i % len(key)]))
    return fixed


def generate_vernam_key(length):
    key = ""
    for i in range(length):
        key += chr(random.randint(0, 255))
    return key


def caesar_break(text):
    freq = {}
    for char in text:
        if char.isalpha():
            if char in freq:
                freq[char] += 1
            else:
                freq[char] = 1
    max_freq = max(freq.values())
    for char, count in freq.items():
        if count == max_freq:
            shift = ord(char) - ord('e')
            return caesar(text, shift, True)
    return None


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("type", choices=["caesar", "vigenere", "vernam"])
    parser.add_argument("key")
    parser.add_argument("input_file")
    parser.add_argument("output_file")
    args = parser.parse_args()
    return args
