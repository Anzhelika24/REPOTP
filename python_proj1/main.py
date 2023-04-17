from src.functions import caesar, vigenere, vernam, caesar_break, generate_vernam_key, parse_args
if __name__ == '__main__':
    args = parse_args()

    # Чтение входного файла
    with open(args.input_file, "r") as f:
        text = f.read()

    if args.type == "caesar":
        key = int(args.key)
        result = caesar(text, key)
        result2 = caesar(result, key, True)
    elif args.type == "vigenere":
        key = args.key.upper()
        result = vigenere(text, key)
        result2 = vigenere(result, key, True)
    elif args.type == "vernam":
        key = generate_vernam_key(len(text))
        result = vernam(text, key)
        result2 = vernam(result, key)
    # Запись результата в выходной файл
    with open(args.output_file, "w") as f:
        f.write(result)
        f.write('\n')
        f.write(result2)
        f.write('\n')
        if args.type == "caesar":
            f.write(caesar_break(result))


