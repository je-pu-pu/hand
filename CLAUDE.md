# CLAUDE.md（hand プロジェクト）

## git

**commit / push はユーザーが TortoiseGit で実行する。Claude は `git commit` / `git push` を直接叩かない。**

Claude がやるのは「コミットダイアログを開く準備」まで。

1. コミット対象が未追跡なら、意図したファイルだけ `git add`（一括 `git add` はしない）
2. コミットメッセージを `%TEMP%\<任意名>.txt` に BOM なし UTF-8 で書き出す
3. `TortoiseGitProc.exe /command:commit /path:"<repo>" /logmsgfile:"<txt>"` を起動
4. 成否は `git rev-parse HEAD` の起動前後比較で判定する（ExitCode は当てにならない）

`.claude/settings.json` に拒否ルールを置いてあるので、機械的にも `git commit` / `git push` は実行できない。

**コミットメッセージに `Claude-Session: https://claude.ai/...` の行（セッションへのリンク）を付けない。**
公開リポジトリなので、本文だけにする（2026-10-14 指示）。

### 踏んだ落とし穴（繰り返さないこと）

- **`/path` には対象ファイルだけを `*` 区切りで渡す。** リポジトリのルートを渡すとダイアログに全変更ファイルが並び、
  関係ない変更を巻き込んだままコミットされる。`git add` でインデックスを絞っても**ダイアログの選択は絞れない**。
- **コミット後に必ず `git show --stat HEAD` でメッセージと中身の一致を検証する。** 一度これを怠って、
  `.gitignore` だけのつもりのコミットに別の 2 機能が混入した（`git reset HEAD~1` で作り直した）。
- **コミットメッセージに `git commit` / `git push` という文字列を入れない。**
  入れる必要があるときは、メッセージをシェル経由で渡さずツールでファイルに直接書き出す。
  シェルコマンドの文字列全体が拒否ルールに引っかかって弾かれる。
- **成否はプロセスの生存ではなく HEAD で判定する。** コミット完了後はプロセスが消えるので、
  「プロセスがない＝起動しなかった」と誤読しやすい。

Claude が実行して OK: `git checkout -b` / `git add` / `git status` / `git diff` / `git log` / `git pull` / `git branch -d`。

## ビルド

ビルド手順・外部ライブラリの配置先は [README.md](README.md) を参照。

- `openFrameworksHand/openFrameworksHand.sln` の **Release|Win32** をビルドすれば、gamma と main も一緒にビルドされる
- x64 構成は未整備なので使わない
- openFrameworks の場所は `OF_ROOT_DIR`（環境変数で上書き可、既定は `E:\lib\of_v0.9.8_vs_release`）

### ビルドの検証

- **`ExitCode 0` はビルドされた証拠にならない。** MSBuild はタイムスタンプで判定するため、
  ソースが `.obj` より古いとコンパイルを黙ってスキップする。
  ファイルを退避・復元したとき（`Copy-Item` は更新日時を引き継ぐ）に実際に起きた。
  **ソース → `.obj` → `.exe` の順に新しいこと**、あるいは **exe 内に目的の文字列が入っていること**まで確認する。
- **アプリが起動中だとリンクに失敗する**（`LNK1104`）。ビルド前に `openFrameworksHand.exe` の
  プロセスを確認し、動いていたらユーザーに終了を依頼する（勝手に kill しない）。

## 文字コード

**`main/` 配下のソースは Shift-JIS ( CP932 ) + CRLF** で、日本語コメントが入っている。
UTF-8 で書き戻すと既存コメントが壊れるので、編集前に必ず文字コードを確認すること。
（`openFrameworksHand/src/` 配下と `main/Hand.h` / `Config.*` / `serialize.h` は ASCII / UTF-8）
