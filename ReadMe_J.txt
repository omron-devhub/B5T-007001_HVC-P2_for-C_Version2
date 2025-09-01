----------------------------------------------------
 B5T-007001 サンプルコード
----------------------------------------------------
(1) 本提供物について
  B5T-007001(HVC-P2)のサンプルコードを提供いたします。
    1-1) B5T-007001とコマンド送受信(機能実行)を行うサンプルコード
    1-2) HVCのセンシング結果を安定化する結果安定化ライブラリ(STBLib)のソースコード

(2) サンプルコード内容
  B5T-007001の各設定値の設定/取得、および、以下の各処理を実行し、その結果を標準出力に出力しています。

   1 : Detection/Estimation                       顔認証を除く9機能の検出/推定を実行しています。
   2 : Recognition(Identify)                      顔認証(識別)を実行しています。
   3 : Recognition(Verify)                        顔認証(照合)を実行しています。
   4 : Register data                              顔認証データ登録機能を実行しています。
   5 : Delete specified data                      指定された顔認証データ１つを削除しています。
   6 : Delete specified user                      指定のユーザの顔認証データを削除しています。
   7 : Delete all data                            全てのユーザの顔認証データを削除しています。
   8 : Save Album                                 アルバムをホスト装置側に保存しています。
   9 : Load Album                                 ホスト装置側に保存されているアルバムを読み込んでいます。
  10 : Save Album on Flash ROM                    アルバムをフラッシュROMに書き込んでいます。
  11 : Reformat Flash ROM                         フラッシュROMのアルバムデータ保存領域を再フォーマットしています。
  12 : Set Number of registered people in album   アルバム登録人数の設定を行っています。
  13 : Get Number of registered people in album   アルバム登録人数を取得しています。

  * 本サンプルは、性別・年齢・顔認証(識別)に対してSTBLibを用いることで安定化処理を実施しています。
    （起動時の引数でSTBLibの使用有無を選択可能です)
  * 上記の3,12,13の処理は、B5T-007001のバージョン1.2.3以降でのみ実行できます。

(3) ディレクトリ構成
    bin/                            ビルド時の出力ディレクトリ
    import/                         STBLibを利用するためのインポートディレクトリ
    platform/                       ビルド環境
        Windows/                        Windowsでのビルド環境
        Linux/                          Linuxでのビルド環境
    src/
        HVCApi/                     B5T-007001インターフェース関数
            HVCApi.c                    API関数
            HVCApi.h                    API関数定義
            HVCDef.h                    構造体定義
            HVCExtraUartFunc.h          API関数から呼び出す外部関数定義
        STBApi/                     STBLibインターフェース関数
            STBWrap.c                   STBLibラッパー関数
            STBWrap.h                   STBLibラッパー関数定義
        bmp/                        B5T-007001から取得した画像をビットマップファイルに保存する関数
            bitmap_windows.c            Windowsで動作する関数
            bitmap_linux.c              Linuxで動作する関数
        uart/                       UARTインターフェース関数
            uart_windows.c              Windowsで動作するUART関数
            uart_linux.c                Linuxで動作するUART関数
            uart.h                      UART関数定義
        Album/                      アルバムファイル保存/読込関数
            Album.c                     B5T-007001から取得したアルバムのI/Oを行う関数
        Sample/                     各処理サンプル
            main.c                      サンプルコード
    STBLib/                         STBLib関連の一式
        doc/                            STBLibに関する資料一式
        bin/                            STBLibビルド時のSTB.dll、STB.lib出力ディレクトリ
        platform/                       STBLib ビルド環境
            Windows/                        Windowsでのビルド環境
            Linux/                          Linuxでのビルド環境
        src/                            STBLib ソースコード本体

(4) サンプルコードのビルド方法
  * Windows の場合
  1. 本サンプルコードはWindows10/11上で動作するよう作成しています。
     VC10(Visual Studio 2010 C++)以降でコンパイル可能です。
  2. コンパイル後は、bin/Windows以下にexeファイルが生成されます。
     また、exeファイルと同じディレクトリに、STBLibのDLLファイルが必要です。
    （あらかじめSTB.dllは格納されています）

  補足: STBLibをビルドする際はMFCライブラリが必要です。事前にインストールしてください。
        STBLibを変更した場合は、sample.exeファイルと同じ場所に最新のSTB.dllを配置してください。
        また、import/ディレクトリ以下のファイルも差し替えてください。

  * Linux の場合
  1. STBLib/platform/Linux以下にあるbuild.shを起動し、STBLibのビルドを実施してください。
  2. STBLib/bin/Linux配下に生成された、STB.a、libSTB.soファイルをimport/lib配下へコピーしてください。
  3. platform/Linux/Sample以下にあるbuild.shを実行することでコンパイル、リンクされます。

(5) サンプルコードの実行方法
  本サンプルコードの実行時に下記のように起動引数を指定する必要があります。

    Usage: sample.exe <com_port> <baudrate> [use_STB]
       com_port: B5T-007001が接続しているCOM番号
       baudrate: UARTのボーレート
       use_STB : STBLibの使用/不使用 (STB_ON or STB_OFF)
                 ※ この引数を省略した場合は「STB_ON」として動作します。

  実行例) 
  * Windowsの場合
   sample.exe 1 921600 STB_ON

  * Linuxの場合
   - Windowsの場合と起動引数は同様ですが、サンプルコードを実行させるためには、
     B5T-007001が/dev/ttyACM0として接続されていることが前提となります。
     サンプル実行用シェル(Sample.sh)を用意していますので、参考にしてください。

   - サンプル実行用シェルでの記載例(Sample.sh:6行目)
      ./Sample 0 921600 STB_ON
     * この場合、第1引数"0"については、Linux版では無視されます。
       "921600bps", "STBLib使用"として起動します。


[ご使用にあたって]
・本サンプルコードおよびドキュメントの著作権はオムロンに帰属します。
・本サンプルコードは動作を保証するものではありません。
・本サンプルコードは、Apache License 2.0にて提供しています。
・STBLibはB5T-007001の専用品です。
  ご使用に当たっては、該当製品の【ご承諾事項】をご承諾の上でお使いいただくものとします。

----
オムロン株式会社
Copyright(C) 2014-2025 OMRON Corporation, All Rights Reserved.
