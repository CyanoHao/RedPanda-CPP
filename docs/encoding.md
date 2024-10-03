# Encoding / 编码

<!-- Please strike through <del>outdated text</del>. -->

## Time Line / 时间线

## Windows Code Page / Windows 代码页

[The list](./windows-code-page-list.csv), based on _[MS-LCID]: Windows Language Code Identifier (LCID) Reference_, revision 16.0, ACP and OEMCP collected by experiments on LTSC 2021 and 2024.

Missing items:

| Language | Location (or type) | Language ID | Language tag | ACP | OEMCP |
| -------- | ------------------ | ----------- | ------------ | --- | ----- |
| Central Atlas Tamazight (Latin) | Algeria | 0x085F | tzm-Latn-DZ | 1252 | 850 |
| Central Atlas Tamazight (Tifinagh) | Morocco | 0x105F | tzm-Tfng-MA | 65001 | 65001 |
| Edo | Nigeria | 0x0466 | bin-NG | 1252 | 850 |
| English | Indonesia | 0x3809 | en-ID | 1252 | 850 |
| Ibibio | Nigeria | 0x0469 | ibb-NG | 1252 | 850 |
| Latin | World | 0x0476 | la-001 | 1252 | 437 |
| Manipuri | India | 0x0458 | mni-IN | 65001 | 65001 |
| Papiamento | Caribbean | pap-029 | 1252 | 850 |
| Sindhi (Devanagari) | India | 0x0459 | sd-Deva-IN | 65001 | 65001 |

Quirks:

| Language | Location (or type) | Language ID | Language tag | Behaviour |
| -------- | ------------------ | ----------- | ------------ | --------- |
| Basque | Basque | 0x042D | eu-ES | Should be ACP 1252, OEMCP 850, but Windows forces ACP/OEMCP 65001 after restart. |
| Romanian | Moldova | 0x0818 | ro-MD | Should be ACP 1252, OEMCP 850, but Windows forces ACP/OEMCP 65001 after restart. |
| Russian | Moldova | 0x0819 | ru-MD | Should be ACP 1251, OEMCP 866, but Windows forces ACP/OEMCP 65001 after restart. |

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
