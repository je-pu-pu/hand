# hand
hand は LeapMotion を使って空中で手をかざして音楽を奏でるアプリケーションです。

[![hand を使ったパフォーマンス動画](http://img.youtube.com/vi/tfDPjbvK638/0.jpg)]( https://www.youtube.com/watch?v=tfDPjbvK638&t=1394s )

# 使っているライブラリなど

- [ LeapMotion SDK ]( https://developer.leapmotion.com/ )
    - [ Orion ]( https://www.ultraleap.com/downloads/leap-controller/ )
        - **Orion Version 3.2.1 を使うこと**（後述）
        - 2.3.1 では古くて動かない
        - 4.1.0 以降ではジェスチャーが動かない
- [ Gamma ]( http://w2.mat.ucsb.edu/gamma/ )
- [ openFrameworks ]( http://openframeworks.cc )

# 注意事項

## Orion は 3.2.1 を使うこと（4.1.0 以降ではジェスチャーが動かない）

Orion 4.0 で LeapMotion は API を LeapC ( C API ) に一本化し、
旧 V2 C++ API ( Leap.h ) の **Gestures / Motions / Tools / Pointables / tipVelocity を廃止**した。

> Legacy applications that support Gestures, Motions, Tools, Pointables and Tip Velocity are no longer supported on Orion
> — Orion リリースノート

4.0 以降の旧 C++ API は [ LeapCxx ]( https://github.com/leapmotion/LeapCxx )
（ LeapC の上に被せた互換ラッパー ）として提供されるが、その README にも記載がある。

> Several fields which had been deprecated have been removed, including gestures and the interaction box

同梱の LeapSDK ( 3.2.1 ) を見ても、
gesture の記述は `include/Leap.h` にしか無く `include/LeapC.h` には一切無い。
つまりジェスチャー認識は V2 API 専用の機能であり、LeapC ベースになった 4.0 以降では取得できない。

そのため 4.1.0 以降のドライバをインストールすると、
`main/LeapSoundController.h` の `frame.gestures()` が常に空になり、以下が動かなくなる。

- **スワイプによるページ遷移**（ `on_swipe_gesture()` ）
- **キータップによる発音**（ `on_key_tap_gesture()` -> `l_tapped_` / `r_tapped_` ）
- `controller.config().setFloat( "Gesture.Swipe.MinLength", ... )` などの設定
  （ Config API 自体も 4.0 で大きく変わっているため効かない ）

なお `pinchStrength()` によるスライダー操作、`isExtended()` による手の形の判定
（ グー / チョキ / パー 等 ）は 4.0 以降でも動く。壊れるのは上記のジェスチャー関連のみ。

4.0 以降のドライバを使いたい場合は、手のひらの速度や指先の位置から
スワイプ / タップを自前で実装し直す必要がある。

# ビルド

`openFrameworksHand/openFrameworksHand.sln` の **Release|Win32**（ または Debug|Win32 ）をビルドする。
これだけで gamma.lib も一緒にビルドされ、`openFrameworksHand/bin/openFrameworksHand.exe` ができる。

x64 構成はインクルードパス・依存ライブラリが未設定なので使えない。Win32 でビルドすること。

`main/main.sln` は旧 CUI 版（ main ）と gamma のためのもので、アプリのビルドには不要。

## 開発環境

- Visual Studio 2026 ( プラットフォームツールセット **v145** )

## 外部ライブラリの配置

以下はリポジトリに含まれないので、各自で配置する（ `.gitignore` で除外している ）。

| 配置先 | 内容 |
| --- | --- |
| `./LeapSDK/` | LeapMotion SDK 3.2.1 |
| `./Gamma/` | Gamma |
| `./ofxOsc/` | openFrameworks の `addons/ofxOsc` |
| `./boost_1_69_0/` | boost |

### openFrameworks

openFrameworks 0.9.8 ( vs release ) はリポジトリの外に置き、
`openFrameworksHand.vcxproj` の `OF_ROOT_DIR` でその場所を指す。

既定値は `E:\lib\of_v0.9.8_vs_release` になっているので、別の場所に置く場合は
**環境変数 `OF_ROOT_DIR` を設定する**（ 環境変数があればそちらが優先される ）。

openFrameworks 本体はビルド済みの `openframeworksLib.lib` ( Win32 ) を直接リンクするため、
openFrameworks 側のプロジェクトをビルドし直す必要はない。

### Gamma

`Gamma/gamma.vcxproj` は openFrameworksHand からプロジェクト参照されており、一緒にビルドされる。
ただし Gamma はリポジトリの外から持ってくるものなので、以下を自分で設定する必要がある。

- プラットフォームツールセットを **v145** にする
- 出力先をソリューションに依存させないため `OutDir` を `$(ProjectDir)$(Configuration)\` に設定する

### boost

./boost_1_69_0/ に boost を配置。

```bat
cd boost_1_69_0
.\bootstrap
.\b2 --with-date_time threading=multi variant=debug,release
```

# 実行

作業ディレクトリを `openFrameworksHand/` にして起動する
（ wav ファイル・`data/`・`config.txt` がそこにあるため。Visual Studio から実行すれば自動的にそうなる ）。

# 照明 ( DMX ) との連携

このリポジトリに DMX を直接制御するコードは無い。
`ofApp::update()` から OSC で `localhost:7700` に `/bg`（ 背景色 ）を送っており、
それを [ QLC+ ]( https://www.qlcplus.org/ ) の OSC 入力プラグインで受けて DMX に変換している。
QLC+ のワークスペースは `dmx.qxw`。