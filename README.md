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

## openFrameworks

./ofxOsc に ofx の addons/ofxOsc を配置。

## boost

./boost_1_69_0/ に boost を配置。

```bat
cd boost_1_69_0
.\bootstrap
.\b2 --with-date_time threading=multi variant=debug,release
```