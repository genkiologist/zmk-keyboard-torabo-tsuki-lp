sekigon-gonnoc/zmk-driver-paw3222 の torabo-tsuki-nemu-link ブランチ（commit f5cef51、2026-11 取得）の写し。
ZMK v0.3 のままで「USB 接続時 1ms／BLE 15ms」のポーリングを使うため、ここに取り込んだ（11/17 田上さん依頼）。
変更点: ①zmk_endpoint_get_selected()（ZMK v0.4 の名前）→ zmk_endpoints_selected()（v0.3） ②hal/nrf_gpio.h の前で `#undef P`（keys.h との衝突回避）。
ライセンスは元のまま（Apache-2.0、LICENSE 参照）。上流が ZMK v0.4 に移ったら、この写しは捨てて west.yml に戻す。
