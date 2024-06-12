# hand
hand は LeapMotion を使って空中で手をかざして音楽を奏でるアプリケーションです。

[![hand を使ったパフォーマンス動画](http://img.youtube.com/vi/tfDPjbvK638/0.jpg)]( https://www.youtube.com/watch?v=tfDPjbvK638&t=1394s )

# 使っているライブラリなど

- [ LeapMotion SDK ]( https://developer.leapmotion.com/ )
- [ Gamma ]( http://w2.mat.ucsb.edu/gamma/ )
- [ openFrameworks ]( http://openframeworks.cc )

# ビルド

## boost

./boost_1_69_0/ に boost を配置。

```bat
cd boost_1_69_0
.\bootstrap
.\b2 --with-date_time threading=multi variant=debug,release
```