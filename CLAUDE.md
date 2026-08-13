# CLAUDE.md（hand プロジェクト）

## git

**commit / push はユーザーが TortoiseGit で実行する。Claude は `git commit` / `git push` を直接叩かない。**

Claude がやるのは「コミットダイアログを開く準備」まで。

1. コミット対象が未追跡なら、意図したファイルだけ `git add`（一括 `git add` はしない）
2. コミットメッセージを `%TEMP%\<任意名>.txt` に BOM なし UTF-8 で書き出す
3. `TortoiseGitProc.exe /command:commit /path:"<repo>" /logmsgfile:"<txt>"` を起動
4. 成否は `git rev-parse HEAD` の起動前後比較で判定する（ExitCode は当てにならない）

`.claude/settings.json` に拒否ルールを置いてあるので、機械的にも `git commit` / `git push` は実行できない。

Claude が実行して OK: `git checkout -b` / `git add` / `git status` / `git diff` / `git log` / `git pull` / `git branch -d`。

## ビルド

ビルド手順・外部ライブラリの配置先は [README.md](README.md) を参照。

- `openFrameworksHand/openFrameworksHand.sln` の **Release|Win32** をビルドすれば、gamma と main も一緒にビルドされる
- x64 構成は未整備なので使わない
- openFrameworks の場所は `OF_ROOT_DIR`（環境変数で上書き可、既定は `E:\lib\of_v0.9.8_vs_release`）

## 文字コード

**`main/` 配下のソースは Shift-JIS ( CP932 ) + CRLF** で、日本語コメントが入っている。
UTF-8 で書き戻すと既存コメントが壊れるので、編集前に必ず文字コードを確認すること。
（`openFrameworksHand/src/` 配下と `main/Hand.h` / `Config.*` / `serialize.h` は ASCII / UTF-8）
