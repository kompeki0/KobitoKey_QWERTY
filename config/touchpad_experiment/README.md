# KobitoKey Touchpad Experiment

この実験は既存ファイルを変更せず、左右トラックボール入力を2本指タッチ接点として送るための追加ファイルです。

## 追加したもの

- `config/modules/tb_touchpad/`
  - `zmk,input-processor-tb-touchpad` を実装する外部モジュール
- `config/touchpad_experiment/KobitoKey_left_touchpad_test.overlay`
  - `tb_left_listener` を contact 0、`tb_right_listener` を contact 1 に割り当て
- `config/touchpad_experiment/KobitoKey_left_touchpad_test.conf`
  - 実験用の Kconfig 設定

## ビルド例

ワークスペースの構成に応じて `-DZMK_CONFIG` や `-DZMK_EXTRA_MODULES` のパスを調整してください。

```sh
west build -s app -b xiao_ble -d build/kobitokey-left-touchpad -- \
  -DZMK_CONFIG="C:/Users/tokyo/OneDrive/デスクトップ/diy_keyboard/KobitoKey_QWERTY/config" \
  -DZMK_EXTRA_MODULES="C:/Users/tokyo/OneDrive/デスクトップ/diy_keyboard/KobitoKey_QWERTY/config/modules/tb_touchpad" \
  -DSHIELD="KobitoKey_left rgbled_adapter" \
  -DDTC_OVERLAY_FILE="C:/Users/tokyo/OneDrive/デスクトップ/diy_keyboard/KobitoKey_QWERTY/config/touchpad_experiment/KobitoKey_left_touchpad_test.overlay" \
  -DCONF_FILE="C:/Users/tokyo/OneDrive/デスクトップ/diy_keyboard/KobitoKey_QWERTY/config/touchpad_experiment/KobitoKey_left_touchpad_test.conf"
```

## 使い方メモ

- `input-processors = <&tb_left_to_touchpad 0 12>;`
  - `param1=contact_id`, `param2=gain`
- 右側も同様に `contact_id=1`

