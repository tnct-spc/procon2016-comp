すてきなゆりかもめ
===
## 東京高専競技部門
第27回 全国高等専門学校プログラミングコンテスト(鳥羽大会)の競技部門で実際に使用されたソースコード．
## 開発環境
Linux Ubuntu

- Gcc 5.3.x
- Boost C++ Libraries 1.61.0
- Qt5
- OpenCV 3.1.0

## 実行方法

Hazama ... 本体  
Slave ... 複数台運用向け問題解答専用クライアント

## ディレクトリ構造  

| パス              | 説明 |
|:------------------|:-----|
| ./DisplayAnswer | 解答表示プログラム |
| ./Hazama | 本体 |
| ./ImageRecognition | 画像処理プログラム |
| ./Polygon | パズルクラス |
| ./ProbMaker | 問題自動生成機(ぶっ壊れてる) |
| ./Slave | 複数台運用向け問題解答専用クライアント |
| ./Solver | パズル組み立てアルゴリズム本体 |
| ./Test | テスト(機能していない) |
| ./Utilities | 最低限の汎用機能入れ |
| ./sample | サンプル問題等の収納場所 |
| ./picture | サンプル問題等の収納場所(gitignore) |
