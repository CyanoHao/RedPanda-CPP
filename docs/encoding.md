# Encoding

## Overview

Null-terminated byte strings (NTBS) have been widely accepted in system interfaces since ~1970s. In the old days, NTBS were processed as is. However, with the advent of Unicode, the encoding of NTBS became a problem.

| OS | File system | Console | Terminal emulator | GUI system | GUI user |
| -- | ----------- | ------- | ----------------- | ---------- | -------- |
| Windows NT | ACP | N/A | OEMCP | N/A | ACP (Win32) |
| XDG (Linux, BSD, etc.) | as is | UTF-8 | UTF-8 | UTF-8 | UTF-8 (Qt, GTK) |

- Columns:
  - File system: how do file system APIs process NTBS (e.g. `open`, `CreateFileA`).
  - Console: how does console translate stdout/stderr bytes to display characters.
  - Terminal emulator: how does terminal emulator translate stdout/stderr bytes to display characters.
  - GUI system: how does GUI framework work with system interfaces (e.g. file chooser).
  - GUI user: how does GUI framework process user-provided NTBS (e.g. message box).
- Windows NT console is the user interface for NT native applications, which use `UNICODE_STRING` structure rather than NTBS.
- Mainstream GUI frameworks on Windows NT internally use “wide” APIs.
- Qt accepts `QString`, user must explicitly specify encoding.

## Windows Code Page

[The list](./windows-code-page-list.csv), based on _[MS-LCID]: Windows Language Code Identifier (LCID) Reference_, revision 16.0, ACP and OEMCP collected by experiments on LTSC 2021 and 2024.

Missing items:

| Language | Location (or type) | Language ID | Language tag | ACP | OEMCP |
| -------- | ------------------ | ----------- | ------------ | --- | ----- |
| Central Atlas Tamazight (Latin) | Algeria | 0x085F | tzm-Latn-DZ | 1252 | 850 |
| Central Atlas Tamazight (Tifinagh) | Morocco | 0x105F | tzm-Tfng-MA | 65001 | 65001 |
| Edo | Nigeria | 0x0466 | bin-NG | 1252 | 850 |
| English | Indonesia | 0x3809 | en-ID | 1252 | 850 |
| Ibibio | Nigeria | 0x0469 | ibb-NG | 1252 | 850 |
| Manipuri | India | 0x0458 | mni-IN | 65001 | 65001 |
| Papiamento | Caribbean | pap-029 | 1252 | 850 |
| Sindhi (Devanagari) | India | 0x0459 | sd-Deva-IN | 65001 | 65001 |

Possible unified ACP/OEMCP:

| Language (or script) | ACP/OEMCP |
| -------------------- | --------- |
| อกษรไทย | 874 |
| 日本語 | 932 |
| 简体中文 | 936 |
| 한국어 | 949 |0
| 繁體中文 | 950 |
| tiếng Việt | 1258 |
| Universal | 65001 |

Possible ACP/OEMCP combinations:

| Language (or script) | ACP | OEMCP |
| -------------------- | --- | ----- |
| Latin, Central Europe | 1250 | 852 |
| српски (Serbian) and босански (Bosnian) | 1251 | 855 |
| кириллица (Cyrillic), other | 1251 | 866 |
| Latin, US | 1252 | 437 |
| Latin, Western Europe | 1252 | 850 |
| ελληνικά (Greek) | 1253 | 737 |
| Latin, | 1254 | 857 |

Native speakers of languages that Windows forces UTF-8 code page (65001):

| Language | Population / 10⁶ |
| -------- | ---------------- |
| Amharic | 35 |
| Armenian | 5.3 |
| Assamese | 15 |
| Bangla | 237 |
| Basque | 0.8 |
| Burmese | 33 |
| Central Atlas Tamazight (Morocco) | 3.1 |
| Cherokee | 0.002 |
| Divehi | 0.5 |
| Dzongkha | 0.2 |
| English (UAE) | (unknown) |
| Georgian | 3.8 |
| Gujarati | 57 |
| Hindi | 345 |
| Inuktitut | 0.04 |
| Kannada | 44 |
| Kashmiri (India) | 6.8 |
| Kazakh | 16 |
| Khmer | 17 |
| Konkani | 2.3 |
| Lao | 3.7 |
| Malayalam | 37 |
| Maltese | 0.6 |
| Manipuri | 1.8 |
| Maori | 0.05 |
| Marathi | 83 |
| Mongolian | 6.2 |
| Nepali | 19 |
| Odia | 35 |
| Oromo | 46 |
| Pashto | 44 |
| Punjabi (India) | 31 |
| Romanian (Moldova) | 2.6 |
| Russian (Moldova) | 0.4 |
| Sanskrit | 0 |
| Sindhi (India) | 1.7 |
| Sinhala | 16 |
| Somali | 24 |
| Sotho | 5.6 |
| Syriac | 0 |
| Tamil | 79 |
| Telugu | 83 |
| Tibetan | 6.0 |
| Tigrinya | 9.7 |
| Tsonga | 3.7 |
| Venda | 1.3 |
| Yi | 6.1 |
| Yiddish | 0.2 |
| | |
| Total | 1368 |
