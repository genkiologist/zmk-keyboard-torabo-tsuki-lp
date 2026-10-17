
[torabo-tsuki LP](https://github.com/sekigon-gonnoc/torabo-tsuki-lp)用のZMKファームウェア

* _centralがついているuf2をトラックボールがついている方に、_peripheralを反対側に書き込んでください
* キーマップはkeymap-editorおよびzmk-studioで編集できます
## ブランチ `torabo-tsuki-lp-4x5keys2`（2026-10-17）

自作基板 **torabo-tsuki-om_RS_arranged**（KiCad プロジェクト `torabo-tsuki-lp-OM-Arranged-main`）用。
左右とも同じ基板を同じ面を上にして使う分割キーボード（4行×5列×2＝40キー、BLE Micro Pro）。
`torabo-tsuki-OM` ブランチから分岐し、回路図のネットリスト（kicad-cli で出力）と設定を突き合わせた。

| 信号 | BMP ピン | GPIO | 設定上の値 | 一致 |
|---|---|---|---|---|
| COL0〜COL4 | 10 / 17 / 11 / 16 / 12 | P0.22 / P1.14 / P0.23 / P1.15 / P1.00 | `kscan0` col-gpios | 一致 |
| ROW0〜ROW3 | 21 / 20 / 19 / 18 | P0.05 / P0.00 / P0.30 / P0.03 | `kscan0` row-gpios | 一致 |
| ダイオード | — | — | ROW→アノード／カソード→スイッチ→COL ＝ 既定の `row2col` | 一致 |
| SCLK / SDIO | 6 / 7 | P0.18 / P0.16 | `spi0_default` | 一致 |
| MOTION / CS | 8 / 9 | P0.19 / P0.20 | snippet `input-trackball`（irq / cs） | 一致 |
| センサ電源 | 2 | P0.08 | snippet `input-trackball`（power-gpios） | 一致 |
| LED | 3 | P0.11 | `&bmp 2`（bmp_boost の nexus で P0.11） | 一致 |
| 右手の列順 | — | — | 右ブロックは昇順・`col-offset = <7>`（鏡像ではない） | 一致 |

* ピン配置は OM ブランチから変更なし。右手＝central（トラックボール）、左手＝peripheral（`build.yaml` も OM のまま）
* エンコーダ（J2: RE-A=P1.03 / RE-S=P0.09 / RE-B=P0.10）はこの基板には載せないため、設定は OM のまま触っていない
  （OM の `encoder_*` は A 相を P0.09 から読んでおり、回路図の RE-A（P1.03）とは一致しない。載せる場合はここを直す）
