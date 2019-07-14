# RTC_PCF8563

NXP Semiconductors PCF8563（RTC）用の I2C通信実装

for Arduino IDE

## 概要

[PCF8563](https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf)
は比較的入手性がよく対応動作電圧範囲も広く廉価な I2C接続 RTCで、
Arduino IDE のライブラリマネージャや
Playground にも以前から対応ライブラリが数種登録されている。
だがデータシートの読み違えに起因すると思われるエラッタが見られたり、
時刻情報の取扱が目的に合わなかったため、改めて実装し直おすことにした。
それに際して本ライブラリで必要とする BCDカレンダー時刻型関数群も整備して
[Futilities](https://github.com/askn37/Futilities)
に収蔵している。

動作確認は
[aitendo HZ-8563](http://www.aitendo.com/product/13987)
で行った。

![](http://aitendo3.sakura.ne.jp/aitendo_data/product_img/microcontroller/HZ-8563/HZ-8563.png)

## Arduino IDE への導入

1. .ZIPアーカイブをダウンロードする。[Click here](https://github.com/askn37/RTC_PCF8563/archive/master.zip)

2. ライブラリマネージャで読み込む

    スケッチ -> ライブラリをインクルード -> .ZIP形式のライブラリをインストール...

3. 依存関係があるライブラリも同様に読み込む

    [Futilities](https://github.com/askn37/Futilities) -- BCDカレンダー時刻型関数ライブラリを含む

## とっかかり

結線は、PCF8563 と Arduino (UNO) の VCC(+5V)と GNDのほか、
SCL(A5) 同士、SDA(A4) 同士を繋ぐだけである。

このライブラリでは時刻情報は 32bit幅の BCD数値型で返されるため、
単に表示出力するだけなら HEX値として扱えば良い。

```c
#include <RTC_PCF8563.h>

RTC_PCF8563 RTC;

void setup (void) {
  while (!RTC.begin()) {
    Serial.println(F("RTC not ready"));
    delay(1000);
  }
}

void loop (void) {
  bcddatetime_t bcddatetime = RTC.now();

  Serial.print(bcddatetime.date, HEX);
  Serial.write(' ');
  Serial.print(bcddatetime.time, HEX);
  Serial.println();
  delay(1000);
}
```

PCF8563 の INT（割込出力）ピンは、
設定したアラーム／タイマーが ONになると LOW出力になる。
オープンドレイン出力のため、結線した場合は
Arduino側では INPUT_PULLUP する必要がある。

PCF8563 の COT（clock out : 外部クロック出力）ピンは、
設定した周波数で LOW と Hi-Z の Duty比 50% 出力を繰り返す。
これもオープンドレイン出力のため、
結線した場合はプルアップする必要がある。

# リファレンス

### RTC_PCF8563 (void)

コンストラクタ。設定引数はない。

### bool begin (long I2CSPEED = I2C\_SPEED\_STANDARD, uint8\_t I2CADDR = PCF8563\_ADDR)

I2Cバスを有効化し、RTCとの通信を準備する。
I2C通信に成功すれば真を返す。

第1引数は I2C速度で、省略時の既定値は I2C\_SPEED\_STANDARD（100Khz）である。
MCUが対応していれば I2C\_SPEED\_DOUBLE（200Khz）も使えるが、
I2C\_SPEED\_FAST（400Khz）は指定しても
（PCF8563が対応する最大速度を超えてしまうので）正しく機能しないだろう。

第2引数は RTCの I2Cアドレスを指定する。
PCF8563 の場合の省略時の既定値は 0x51 である。

### bool reset (void)

RTCにリセットコマンドを発行する。成功すれば真を返す。
リセットされるのは 起動時テスト状態終了、アラーム停止、タイマー停止である。

### bool isRunning (void)

RTCが通常モードで駆動していれば真を返す。
電源投入直後の初期化動作がまだ終わっていなければ偽を返す。

### bool isAlarm (void)

RTCがアラーム割込（INTピン＝LOW）を発生中なら真を返す。

### bool isTimer (void)

RTCがタイマー割込（INTピン＝LOW）を発生中なら真を返す。

### uint8_t getStatus (void)

RTCの現在のステータスを読み込んで返す。

返値は、以下のマクロ定数と AND することで
アラームとタイマーの発生状態をまとめて知ることができる。

|マクロ定数|意味|
|---|---|
|RTC\_CTRL\_AF|真なら発生中|
|RTC\_CTRL\_TF|真なら発生中|

### bcddatetime\_t now (bool TWICE = true)

RTCから時刻情報を取得して、bcddatetime\_t 構造体（BCDカレンダー時刻型）で返す。
bcddatetime\_t 構造体には date と time メンバーがあり、
いずれも BCD形式である。
これを epoch や MJD に変換するには
[BCD日時変換](https://github.com/askn37/Futilities#bcdtimeh)
を参照のこと。

引数は省略時の既定値では true であり、
これは RTCに対して "2度読み" を行うことを指示する。
false を指示すると時刻を1回読んだだけで返ってくるが、その場合は
"005959" の次に "005900" が返ってくるような巻戻りが稀に起こる。
RTCが時刻情報を更新しているまさにその瞬間に読み出すとこのようになるので
不具合を避けるには1秒以内に 2回以上読み込んで同じ値が返ってくることを確認しなければならない。
初期値 true はそのような動作をデフォルトで行う。

COTピンを配線し、外部クロック出力を 1Hzに設定している場合は、
COT=LOW のときに読み込みはじめればそれは必ず時刻情報が更新直後であることが保証されるので
false を指定して無駄になる I2C通信を省くことができる。

### time\_t epoch (bool TWICE = true)

RTCから時刻情報を取得して、time\_t 時刻型で返す。
引数の意味は now() に同じである。
time\_t 型の表現範囲を超える年月が設定されている場合は正しい値を返さない。

### bool adjust (bcddatetime_t BCDDATETIME)

bcddatetime_t 構造体を引数にとり、時刻情報を初期化する。
正常なら真を返す。

```c
bcddatetime_t bcddatetime = {
	0x20180530,	// bcd date
	0x00123456	// bcd time
};
RTC.adjust(bcddatetime);
```

曜日指定とセンチュリービットは設定値から判断して自動設定する。

RTCは年号を下2桁しか取り扱わない。
その桁あふれと当該年の閏日を監督するのがセンチュリービットである。

PCF8563 の場合、センチュリービットは 1bitだけ持ち、
0 == 西暦2000年台、1 == 西暦2100年代（または1900年代）と見做す。
これは年号の下2桁が "00" のときに閏日を設けるかの指定で、
センチュリービットが 0 なら 2/29が存在、1 で無視（2/28 の次は 3/1）である。
センチュリービットは "99" 年が "00" 年になるときの桁あふれで自動トグルするため、
つまるところ PCF8563 が無修正で正しい年日を扱えるのは最長 200年間である。
（RTCの品種によってはセンチュリービットを 2bit持ち、最長 400年間に対応するものもある）

なお設定する時刻を世界標準時にするか地域標準時にするかは使用者の裁量である。
時計表示用途とする場合は地域標準時のほうが便利であろう。
しかし GPSと連携して時刻校正を行う場合は（GPSから得られる時刻が世界標準時なので）
時差を計算する必要がある。

### bool activeAlarm (bool ALARMFLAG)

引数が真ならアラームを有効化、偽ならアラームを無効化する。
操作に成功すれば真を返す。

アラームが発生すると以後 INTピンが LOWであり続ける。
次のアラームサイクルに入るには activeAlarm(false) で現在のアラームを切り、
改めて activeAlarm(true) を発行しなければならない。

INTピン出力はアラームとタイマーで共用である。
どちらが（あるいは両方が）現在発生中かは getStatus() で判別できる。

### bool setAlarm (bcdtime_t ALARMDATE)

アラーム発生時刻を bcdtime_t 型の BCD形式で設定する。成功すると真を返す。

アラームに指定できるのは、日時分および曜日の4項目で、
各項目は BCD 2桁の 1バイト幅を持ち、かつ MSBが真なら 該当桁のアラームは無視される。

```
TIMERFLAG == 0xDDHHMMWW

  DD = BCD 01 - 31 day,     HEX 0x80 is Disable
  HH = BCD 00 - 23 hour,    HEX 0x80 is Disable
  MM = BCD 00 - 59 minute,  HEX 0x80 is Disable
  WW = BCD 00 - 06 weekday, HEX 0x80 is Disable
```

曜日は日曜 == 0 〜 土曜 == 6 である。

以下に設定例を挙げる。

|指定値|意味|
|---|---|
|0xFFFF00FF|毎正時00分（60分毎）|
|0xFF0123FF|毎日1時23分（24時間毎）|
|0x01FFFFFF|毎月1日0時0分（1ヶ月毎）|
|0x030105FF|毎年3月1日5時0分（1年毎）※2月29日だけは毎年発生とはならない|
|9xFFFFFF00|毎週日曜0時0分（1週間毎）|
|0x13FFFF05|13日の金曜日0時0分（1年に2回程度）|

これらの例では無視したい項目には 0xFF を指定しているが、
有効な項目が桁上りによってマッチした結果、
暗黙的に 0時あるいは 0分を指示していたかのように振る舞うことがある。

アラームの最短発生間隔は 1時間/回で、
かつ発生タイミングは（桁上りをトリガーとするので）
毎時正分あるいは毎分正秒に固定である。
これ以上の頻度でアラームを発生させるには毎回再設定するか、setTimer() を使用する。

```c
// 3分毎にアラームを発生させて毎回再設定する例
// 得られた時刻を epoch に変換して次回発生時刻を計算している

RTC.activeAlarm(false);
bcddatetime_t bcddatetime = RTC.now();
time_t epoch = bcdTimeToEpoch(bcddatetime.time);
bcdtime_t newAlarm = 0xFFFF00FF | epochToBcdTime(epoch + 180);
RTC.setAlarm(newAlarm);
RTC.activeAlarm(true);
```

なお setAlarm() で指定した設定は、activeAlarm(true) を発行しなければ有効にはならない。
また RTCの電源起動時（バックアップバッテリー再装着時やリセット発行時）は
すべての MSBがセットされて全アラームは無効になる。

### bcdtime_t getAlarm (void)

現在のアラーム設定値を bcdtime_t 型で返す。
RTCを再起動したりしていなければ、以前設定したとおりの値が返る。

### bool activeTimer (bool TIMERFLAG)

引数が真ならタイマー出力を有効化、偽ならタイマー出力を無効化する。
操作に成功すれば真を返す。

タイマーが発生すると以後 INTピンが LOWであり続ける。
次のタイマーサイクルに入るには activeTimer(false) で現在のアラームを切り、
改めて activeTimer(true) を発行しなければならない。

INTピン出力はアラームとタイマーで共用である。
どちらが（あるいは両方が）現在発生中かは getStatus() で判別できる。

### bool activePulse (bool)

引数が真なら INTピンのアラーム／タイマー出力を、パルス出力モードにする。
偽なら通常の、明示的に OFFするまで ON状態を維持する出力モードにする。

この機能は次の setTimer() と組み合わせた場合に特に意味があり、
activeTimer(true) としているあいだ、指定の周期でパルスを出力し続ける。
INTピンが Hi-Zを維持する期間は長くとも 8msec程度（64Hz）で、
それ以下のパルスになる場合はデューティ比50%程度になる。

### bool setTimer (uint16\_t TIMERSET)

引数でインターバルタイマー発生タイミングを設定する。成功すると真を返す。

タイマー割込周期は、プリスケーラ分周比×カウントダウン値で決まる。
プリスケーラ分周比は以下のマクロ定数で指定できる。

|マクロ定数|意味|
|---|---|
|RTC\_TIMER\_DISABLE|タイマー停止|
|RTC\_TIMER\_4KHZ|1/4096秒|
|RTC\_TIMER\_64HZ|1/64秒|
|RTC\_TIMER\_1S|1秒|
|RTC\_TIMER\_60S|60秒|

これに 1〜255 の範囲のカウントダウン値を加算して、引数に渡す。
0値を加算した場合は無効である（が停止しているわけではないので電力は消費する）

|指定値|意味|
|---|---|
|RTC\_TIMER\_64HZ + 8|125ms間隔|
|RTC\_TIMER\_1S + 20|20秒間隔|
|RTC\_TIMER\_1S + 60|1分間隔|
|RTC\_TIMER\_60S + 1|1分間隔|
|RTC\_TIMER\_60S + 5|5分間隔|

なお setTimer() で指定した設定は、activeTimer(true) を発行しなければ有効にはならない。
また RTCの電源起動時（バックアップバッテリー再装着時やリセット発行時）は
タイマーは無効になる。

### bool setClockOut (uint8\_t CLOCKSET)

COTピンに出力するクロックアウトの発生パルス周波数を設定する。成功すると真を返す。

引数には以下のマクロ定数が指定できる。

|マクロ定数|意味|
|---|---|
|RTC\_COT\_DISABLE|COT停止 (Hi-Z固定)|
|RTC\_COT\_32KHZ|32.768kHz|
|RTC\_COT\_1KHZ|1.024kHz|
|RTC\_COT\_32HZ|32Hz|
|RTC\_COT\_1HZ|1Hz|

COTピンはオープンドレイン出力であり、ON == LOW、OFF == Hi-Z である。
デューティ比は 50%なので、1Hz設定なら LOW 0.5秒 + Hi-Z 0.5秒 が 1周期となる。

COTピンは MCUの時間管理や高精度タイマーの校正、チャージポンプ昇圧回路の駆動などに使える。
単なるLチカに使うなら、LEDのカソードを COTピンに、
アノードを適切な電流制限抵抗か CRDを介して VCCに結線すれば良い。

### uint8\_t getClockOut (void)

現在のクロックアウト設定を取得して返す。
RTCを再起動したりしていなければ、以前設定したとおりの値が返る。

## サンプルスケッチ

### AdjustAndAlarm.ino

スケッチをコンパイルした時刻で RTCを初期化する。
さらに毎分正秒に発火するアラームをテストする。
RTCの INTピンは D9に結線する。

### AdjustGPS.ino

GPSを時刻情報源とし、
日本標準時（JST : +9:00）を得て RTCを設定する。
最大誤差 +1秒未満。

GPSには
[秋月電子 AE-GYSFDMAXB](http://akizukidenshi.com/catalog/g/gK-09991/)
が使える。
GPSは 9600bps（購入時初期値）に設定しておき
GPSの TXピンを D5に、RXピンを D6に結線する。

このスケッチは
[GPS_MTK333X](https:/github.com/askn37/GPS_MTK333X)
ライブラリを使用する。

### CenturyLeapDay.ino

センチュリービットの効果を例示する。
2000/02/28、2000/02/29、2100/02/28 の各々の正子の前後5秒間を表示する。

### IntervalTimer.ino

5秒毎に発火するタイマーをテストする。
RTCの INTピンは D9に結線する。

### FlashPluses.ino

16Hzのパルス出力で フラッシュLチカする。
RTCの INTピンは LEDのカソードに、LEDのアノードは適切な抵抗か CRDを介して VCCに配線する。

## 既知の不具合／制約／課題

- 主要な AVR 以外はテストされていない。
- 古い Arduino IDE には対応しない。1.8.5で動作確認。少なくとも C++11 は使えなければならない。
- 英文マニュアルが未整備である。

## 改版履歴

- 0.1.2
- 0.1.1

## 使用許諾

MIT

## 著作表示

朝日薫 / askn
(SenseWay Inc.)
Twitter: [@askn37](https://twitter.com/askn37)
GitHub: https://github.com/askn37
