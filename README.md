# Image Steganography in C

**Hide any file inside a BMP image using LSB encoding.**

**Author:** Damaraju Prudhvikrishna

## 1. Overview

This is a command-line application written in C. It hides a secret file inside a BMP image using the Least Significant Bit (LSB) technique and extracts it again later.

Changing only the last bit of each image byte alters the picture so slightly that the difference is not noticeable.

## 2. Features

- Encodes any type of file (text, image, document, etc.) into a BMP image.
- Decodes the hidden file from the encoded (stego) image.
- A magic string identifies images that were encoded by this program.
- The secret file's extension is stored inside the image and restored on decoding.
- Checks that the image has enough capacity before encoding.
- Validates command-line arguments and file extensions.
- Opens files in binary mode and closes them on both success and failure.

## 3. Project Files

| File | Purpose |
|---|---|
| `main.c` | Entry point. Reads the operation (`-e` or `-d`), checks argument counts, and calls encoding or decoding. |
| `encode.c` / `encode.h` | Argument validation, capacity check, and all encoding functions. |
| `decode.c` / `decode.h` | Argument validation and all decoding functions. |
| `common.h` | Shared definitions such as `MAGIC_STRING`. |

## 4. Build

Compile all source files together using GCC:

```bash
gcc main.c encode.c decode.c -o a.out
```

## 5. Usage

### Encoding

```bash
./a.out -e <source.bmp> <secret_file> [output.bmp]
```

- `source.bmp`: The cover image. It must have the `.bmp` extension.
- `secret_file`: The file to hide. It must have an extension.
- `output.bmp`: Optional. If omitted, the result is saved as `stego.bmp`.

### Decoding

```bash
./a.out -d <stego.bmp> [output_name]
```

- `stego.bmp`: The encoded image.
- `output_name`: Optional, without extension. The original extension is added automatically. If omitted, the program asks for a name.

### Examples

```bash
./a.out -e beautiful.bmp secret.txt
./a.out -e beautiful.bmp secret.txt hidden.bmp
./a.out -d stego.bmp output
./a.out -d hidden.bmp
```

## 6. How It Works

The first 54 bytes of the BMP (the header) are copied unchanged. After that, each bit of the secret data is stored in the least significant bit of one image byte, so one data byte needs eight image bytes.

The information is stored in this order:

| Order | Data stored | Image bytes used |
|---|---|---|
| 1 | Magic string | 8 per character |
| 2 | Size of the secret file extension | 32 |
| 3 | Secret file extension (for example `.txt`) | 8 per character |
| 4 | Size of the secret file | 32 |
| 5 | Secret file data | 8 per byte |
| 6 | Remaining image data, copied unchanged | - |

Decoding reads the same fields in the same order. If the magic string does not match, the image is rejected as not encoded by this program.

## 7. Encoding Flow

```text
check_operation
      |
validate_encode_args
      |
do_encoding
      |
open_files
      |
check_capacity
      |
copy_bmp_header
      |
encode_magic_string
      |
encode_secret_file_extn_size
      |
encode_secret_file_extn
      |
encode_secret_file_size
      |
encode_secret_file_data
      |
copy_remaining_img_data
```

## 8. Decoding Flow

```text
check_operation
      |
valid_decode_args
      |
do_decoding
      |
open_decode_files
      |
decode_magic_string
      |
decode_secret_file_extn_size
      |
decode_secret_file_extn
      |
decode_secret_file_size
      |
decode_secret_file_data
```

## 9. Capacity

The image must be large enough to hold everything listed in Section 6.

The program computes:

```text
required bytes = (magic string + 4 + extension size + 4 + secret file size) x 8

image capacity = (width x 3, rounded up to a multiple of 4) x height
```

If the image capacity is smaller than the required bytes, encoding fails. Use a larger image or a smaller secret file.

## 10. Limitations and Notes

- Only BMP images with a 54-byte header (standard 24-bit BMP) are supported.
- The `secret_extn` array is 7 bytes, so extensions of up to 6 characters including the dot are supported (for example `.txt`, `.csv`, `.json`, `.html`). Keep the array size the same in `encode.h` and `decode.h`.
- The `secret_extn` member must be a character array, not a pointer.
- The encoded image looks the same as the original, but it must not be re-saved or compressed by an editor, or the hidden data will be lost.

## 11. Error Cases

| Situation | Result |
|---|---|
| No arguments, or wrong argument count | Usage message is printed and the program exits with 1. |
| Source or output image is not a `.bmp` | Validation fails. |
| Secret file has no extension, or the only dot is at the start of the name (for example `./secret`) | Validation fails. |
| Secret file extension is longer than the `secret_extn` array allows | Validation fails. |
| Image too small for the secret file | Encoding fails. |
| Image was not encoded by this program | Magic string check fails and decoding stops. |

## 12. Testing

After decoding, compare the result with the original secret file. No output means the files are identical.

```bash
cmp secret.txt output.txt
```

### Suggested Test Cases

- A normal secret file and a large enough image.
- A secret file that is too large for the image.
- An empty secret file.
- A secret file with no extension.
- A wrong image passed to the decode option.
- Decoding with and without the optional output name.

## 13. AI Usage

This project used an AI assistant, Claude (by Anthropic), as a support tool. It was used for the following:

- **Code review:** Pointing out possible problems in `encode.c` and `decode.c`, such as a possible buffer overflow when copying the extension and the capacity calculation.
- **Suggested fixes:** Binary file modes, an extension length check, a more accurate capacity check, and removing debug prints.
- **README:** Took some assistance while preparing this README file.

The AI output was treated as suggestions. The program design and core logic are my own.
