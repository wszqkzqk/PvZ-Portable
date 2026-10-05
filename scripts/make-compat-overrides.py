#!/usr/bin/env python3
import argparse
import re
from pathlib import Path


def read_xor(file, size):
    data = file.read(size)
    if len(data) != size:
        raise ValueError("unexpected end of main.pak")
    return bytes(byte ^ 0xF7 for byte in data)


def load_strings(pak_path):
    with pak_path.open("rb") as pak:
        if read_xor(pak, 4) != b"\xc0\x4a\xc0\xba":
            raise ValueError("invalid main.pak signature")
        if int.from_bytes(read_xor(pak, 4), "little") != 0:
            raise ValueError("unsupported main.pak version")

        records = {}
        data_offset = 0
        while True:
            flags = read_xor(pak, 1)[0]
            if flags & 0x80:
                break
            name_width = read_xor(pak, 1)[0]
            name = read_xor(pak, name_width).decode("utf-8")
            size = int.from_bytes(read_xor(pak, 4), "little")
            read_xor(pak, 8)
            key = name.replace("\\", "/").lower()
            records[key] = (data_offset, size)
            data_offset += size

        entry = records.get("properties/lawnstrings.txt")
        if entry is None:
            raise ValueError("properties/LawnStrings.txt was not found in main.pak")
        pak.seek(pak.tell() + entry[0])
        text_data = read_xor(pak, entry[1])

    if text_data.startswith((b"\xff\xfe", b"\xfe\xff")):
        text = text_data.decode("utf-16")
    else:
        try:
            text = text_data.decode("utf-8-sig")
        except UnicodeDecodeError:
            text = text_data.decode("cp1252")

    strings = {}
    matches = list(re.finditer(r"\[([^\]]+)\]", text))
    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else len(text)
        key = match.group(1).strip().upper()
        strings[key] = text[match.end():end].strip().replace("\r", "")
    return strings


def build_overrides(strings):
    required = (
        "RESTART_LEVEL_BUTTON",
        "NOT_ENCOUNTERED_YET_DESCRIPTION",
        "CRAZY_DAVE_1700",
    )
    missing = [key for key in required if key not in strings]
    if missing:
        raise ValueError("main.pak does not contain the expected compatibility strings: " + ", ".join(missing))

    sell_price = strings["CRAZY_DAVE_1700"]
    if "{SELL_PRICE}" not in sell_price:
        raise ValueError("CRAZY_DAVE_1700 does not contain {SELL_PRICE}")

    overrides = {
        "RESTART_LEVEL": strings["RESTART_LEVEL_BUTTON"],
        "NOT_ENCOUNTERED_YET": strings["NOT_ENCOUNTERED_YET_DESCRIPTION"],
        "CRAZY_DAVE_1700": sell_price.replace("{SELL_PRICE}", "{SELL_PRICE}0", 1),
    }

    header_keys = sorted(key for key in strings if key.endswith("_DESCRIPTION_HEADER"))
    if not header_keys:
        raise ValueError("no almanac description headers were found in main.pak")

    for header_key in header_keys:
        description_key = header_key[:-len("_HEADER")]
        if description_key not in strings:
            raise ValueError(f"missing matching value for {header_key}")
        if description_key == "ZOMBONI_DESCRIPTION":
            overrides[description_key] = strings[description_key].replace("{SHORTLINE}\n", "")
        else:
            overrides[description_key] = strings[header_key] + "\n" + strings[description_key]

    return overrides


def encode_xml(value):
    entities = {
        "<": "&lt;",
        "&": "&amp;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&apos;",
        "\n": "&cr;",
    }
    result = []
    has_space = False
    for char in value:
        if char == " ":
            if has_space:
                result.append("&nbsp;")
                continue
            has_space = True
        else:
            has_space = False
        result.append(entities.get(char, char))
    return "".join(result)


def write_overrides(overrides, output_path, force):
    if output_path.exists() and not force:
        raise ValueError(f"{output_path} already exists; use --force to overwrite it")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    lines = ["<Properties>"]
    for key, value in overrides.items():
        encoded = encode_xml(value)
        lines.append(f'    <String id="{key}">{encoded}</String>')
    lines.append("</Properties>")
    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate PvZ-Portable text overrides from a compatible PvZ GOTY main.pak."
    )
    parser.add_argument("main_pak", type=Path, help="path to a compatible GOTY main.pak")
    parser.add_argument("-o", "--output", type=Path, help="output XML path (default: properties/pvz-portable.xml beside main.pak)")
    parser.add_argument("--force", action="store_true", help="overwrite an existing output file")
    args = parser.parse_args()

    output_path = args.output or args.main_pak.parent / "properties" / "pvz-portable.xml"
    try:
        overrides = build_overrides(load_strings(args.main_pak))
        write_overrides(overrides, output_path, args.force)
    except (OSError, ValueError) as error:
        parser.exit(1, f"error: {error}\n")

    print(f"Wrote {len(overrides)} overrides to {output_path}")


if __name__ == "__main__":
    main()
