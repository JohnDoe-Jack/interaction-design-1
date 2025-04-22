#pragma once
#include "imu.hpp"
#include "sound.hpp" // Sound クラスを使うのでインクルード
#include <M5Unified.h>

class SlotMachine {
private:
    static constexpr int COLS = 3; // スロット列数
    static constexpr int ROWS = 3; // スロット行数

    IMU &imuRef;        // IMUクラス参照
    Sound &soundRef;     // Soundクラス参照
    bool stopped[COLS]; // 各列の停止フラグ
    bool finished;      // 全列停止後の状態フラグ
    bool didWeWin;      // 当選フラグ
    float mWinChance;   // 当選率

    // アニメーション表示用の一時絵柄
    uint16_t symbols[COLS][ROWS];

    // 実際に停止した時に反映される最終絵柄
    uint16_t finalSymbols[COLS][ROWS];

    // 定数定義 (マジックナンバー、マジックストリングの解消)
    static constexpr float ACCEL_THRESHOLD = 2.0f;
    static constexpr int STOP_DELAY_MS = 500;
    static constexpr int UPDATE_DELAY_MS = 50;
    static constexpr char const *WIN_MESSAGE = "YOU WIN!!";
    static constexpr char const *LOSE_MESSAGE = "TRY AGAIN";


public:
    // 当選率を指定可能なコンストラクタ
    explicit SlotMachine(IMU &imu, Sound &sound, float chanceOfWin = 0.0f) // Sound クラスへの参照を追加
        : imuRef(imu), soundRef(sound), finished(false), didWeWin(false), mWinChance(chanceOfWin) { // コンストラクタで Sound 参照を初期化
        for (int i = 0; i < COLS; ++i) {
            stopped[i] = false;
            for (int j = 0; j < ROWS; ++j) {
                symbols[i][j] = 0;
                finalSymbols[i][j] = 0;
            }
        }
    }

    // 初期化
    void begin() {
        soundRef.begin(); 
        soundRef.play(BEGIN_SOUND_FILE); // Sound クラスの play() を使用
        delay(10000);
        imuRef.initIMU();
        M5.Display.setTextSize(2);
        initSlots();
    }

    // 毎フレーム呼ばれる更新処理
    void update() {
        M5.update();

        // 停止していない列はランダム色でアニメーション
        animateSlots();
        soundRef.loop(); 
        // delay(100); 
        

        // 加速度検出でスロット停止 or リセット
        if (imuRef.updateIMU()) {
            auto data = imuRef.getIMUData();
            float ax = fabsf(data.accel.x);
            float ay = fabsf(data.accel.y);
            float az = fabsf(data.accel.z);

            // しきい値以上の加速度を検出
            if ((ax > ACCEL_THRESHOLD) || (ay > ACCEL_THRESHOLD) || (az > ACCEL_THRESHOLD)) {
                if (finished) {
                    // すでに全列停止後なら再度初期化
                    soundRef.stop();
                    soundRef.play(BEGIN_SOUND_FILE); // Sound クラスの play() を使用
                    initSlots();
                    soundRef.isStoppedExplicitly = false; 
                } else {
                    // 次の列を停止
                    soundRef.play(BOSN_SOUND_FILE); // Sound クラスの play() を使用
                    delay(50); // 停止音の再生時間を考慮
                    stopNextSlot();
                }
                M5.delay(STOP_DELAY_MS); // 同一動作の誤検出防止
            }
        }

        M5.delay(UPDATE_DELAY_MS);
    }


private:
  auto randomWinArray() -> void {
    uint16_t color = rand() % 0xFFFF;
    char array_pattern = rand() % 3;

    // まず全マスをランダム初期化
    for (int i = 0; i < COLS; ++i) {
      stopped[i] = false;
      for (int j = 0; j < ROWS; ++j) {
        finalSymbols[i][j] = rand() % 0xFFFF;
      }
    }

    // array_pattern によって揃う箇所を分岐
    switch (array_pattern) {
    case 0:
      // 中段がすべて同じ色
      for (int col = 0; col < COLS; ++col) {
        finalSymbols[col][1] = color;
      }
      break;
    case 1:
      // 左斜め (0,0 -> 1,1 -> 2,2 が同色)
      for (int i = 0; i < COLS; ++i) {
        finalSymbols[i][i] = color;
      }
      break;
    case 2:
      // 右斜め (0,2 -> 1,1 -> 2,0 が同色)
      for (int i = 0; i < COLS; ++i) {
        finalSymbols[i][COLS - 1 - i] = color;
      }
      break;
    }
    return;
  }
  auto randomLoseArray(bool doTwoMatches) -> void {
    // まず全マスをランダム初期化
    for (int i = 0; i < COLS; ++i) {
      for (int j = 0; j < ROWS; ++j) {
        finalSymbols[i][j] = rand() % 0xFFFF;
      }
    }

    if (!doTwoMatches) {
      //--------------------------------------------
      // 1) 全く揃わないパターンを作成
      //--------------------------------------------
      // 簡易的に「もしどこかで3つ揃いができてしまったら、最後を強制書き換え」
      // という方法で 3マス揃いを排除します
      fixThreeInARow();
      return;
    }

    //--------------------------------------------
    // 2) 2つ揃うパターンを作成
    //--------------------------------------------
    // 中段・左斜め・右斜めのいずれかをランダムで選択
    // (0:中段, 1:左斜め, 2:右斜め)
    char array_pattern = rand() % 3;
    // 2マス揃える色
    uint16_t color = rand() % 0xFFFF;

    switch (array_pattern) {
    case 0: {
      // 中段(row=1)が2つ揃い, 3つ目は別色
      finalSymbols[0][1] = color;
      finalSymbols[1][1] = color;
      // 3つ目の列だけ別色に強制
      fixCellDifferentColor(2, 1, color);
    } break;

    case 1: {
      // 左斜め (0,0), (1,1), (2,2) のうち2つだけ同色
      finalSymbols[0][0] = color;
      finalSymbols[1][1] = color;
      fixCellDifferentColor(2, 2, color);
    } break;

    case 2: {
      // 右斜め (0,2), (1,1), (2,0) のうち2つだけ同色
      finalSymbols[0][2] = color;
      finalSymbols[1][1] = color;
      fixCellDifferentColor(2, 0, color);
    } break;
    }

    // 上記の2マス揃いだけで“3つ揃い”が発生しないように最後にチェックし、
    // もし予期せず3マス揃いが生じていれば修正
    fixThreeInARow();
  }

  // 指定セルを「必ずcolorとは異なる色」に強制設定
  auto fixCellDifferentColor(int col, int row, uint16_t color) -> void {
    if (finalSymbols[col][row] == color) {
      // 強制的に異なる色に
      uint16_t newCol = rand() % 0xFFFF;
      // まれに同色になる可能性があるためループで回避

      while (newCol == color) {
        newCol = rand() % 0xFFFF;
      }

      finalSymbols[col][row] = newCol;
    }
  }

  // 3つ揃いが起きていないか判定し、見つけたら強制的に崩す
  // (ここでは単純にチェックが見つかれば最後のマスを書き換えるだけ)
  auto fixThreeInARow() -> void {
    // 横の3マス
    for (int r = 0; r < ROWS; ++r) {
      if (finalSymbols[0][r] == finalSymbols[1][r] &&
          finalSymbols[1][r] == finalSymbols[2][r]) {
        fixCellDifferentColor(2, r, finalSymbols[2][r]);
      }
    }
    // 縦の3マス
    for (int c = 0; c < COLS; ++c) {
      if (finalSymbols[c][0] == finalSymbols[c][1] &&
          finalSymbols[c][1] == finalSymbols[c][2]) {
        fixCellDifferentColor(c, 2, finalSymbols[c][2]);
      }
    }
    // 左斜め
    if (finalSymbols[0][0] == finalSymbols[1][1] &&
        finalSymbols[1][1] == finalSymbols[2][2]) {
      fixCellDifferentColor(2, 2, finalSymbols[2][2]);
    }
    // 右斜め
    if (finalSymbols[0][2] == finalSymbols[1][1] &&
        finalSymbols[1][1] == finalSymbols[2][0]) {
      fixCellDifferentColor(2, 0, finalSymbols[2][0]);
    }
  }
  // スロットを初期化（当選かどうかを決め、停止時に使う最終配置を作成）
  void initSlots() {
    finished = false;

    // ここで各列の停止フラグをリセット
    for (int i = 0; i < COLS; ++i) {
      stopped[i] = false;
    }

    // 当選 or ハズレを決定
    didWeWin = ((float)rand() / RAND_MAX) < mWinChance;

    if (didWeWin) {
      // 当選なら必ず1列（や1行など）が揃う形で finalSymbols を作成
      randomWinArray();
    } else {
      // ハズレなら完全ランダム
      randomLoseArray(true);
    }

    // 最初のアニメーション用に symbols もランダム初期化しておく
    for (int i = 0; i < COLS; ++i) {
      for (int j = 0; j < ROWS; ++j) {
        symbols[i][j] = rand() % 0xFFFF;
        drawSlot(i, j, symbols[i][j]);
      }
    }
  }

  // 停止していない列をランダム色で更新
  void animateSlots() {
    for (int i = 0; i < COLS; ++i) {
      if (!stopped[i]) {
        for (int j = 0; j < ROWS; ++j) {
          symbols[i][j] = rand() % 0xFFFF;
          drawSlot(i, j, symbols[i][j]);
        }
      }
    }
  }

  // 次の列を停止させ、最終絵柄を表示
  void stopNextSlot() {
    for (int col = 0; col < COLS; ++col) {
      if (!stopped[col]) {
        // この列を停止扱いにし、finalSymbolsを実際画面に反映
        stopped[col] = true;
        for (int row = 0; row < ROWS; ++row) {
          symbols[col][row] = finalSymbols[col][row];
          drawSlot(col, row, symbols[col][row]);
        }

        // もし最後の列なら判定
        if (col == COLS - 1) {
          // 当選設定の場合は必ず checkWin() も trueになる
          if (checkWin()) {
            soundRef.stop(); 
            soundRef.play(WIN_SOUND_FILE); // Sound クラスの play() を使用
            soundRef.isStoppedExplicitly = true;
            M5.Display.setTextSize(3);
            M5.Display.setCursor(10, 10);
            M5.Display.printf("YOU WIN!!");
          } else {
            soundRef.stop();
            soundRef.play(LOSE_SOUND_FILE); // Sound クラスの play() を使用
            M5.Display.setTextSize(3);
            M5.Display.setCursor(10, 10);
            M5.Display.printf("TRY AGAIN");
            soundRef.isStoppedExplicitly = true;
          }
          finished = true;
        }
        break;
      }
    }
  }

  // 当たり判定(一例: 横/縦/斜めが揃えばtrue)
  bool checkWin() {
    // 横
    for (int r = 0; r < ROWS; ++r) {
      if (symbols[0][r] == symbols[1][r] && symbols[1][r] == symbols[2][r]) {
        return true;
      }
    }
    // 縦
    for (int c = 0; c < COLS; ++c) {
      if (symbols[c][0] == symbols[c][1] && symbols[c][1] == symbols[c][2]) {
        return true;
      }
    }
    // 斜め
    if (symbols[0][0] == symbols[1][1] && symbols[1][1] == symbols[2][2]) {
      return true;
    }
    if (symbols[0][2] == symbols[1][1] && symbols[1][1] == symbols[2][0]) {
      return true;
    }
    return false;
  }

  // スロット1マスを塗りつぶす
  void drawSlot(int col, int row, uint16_t color) {
    int slotW = M5.Display.width() / COLS;
    int slotH = M5.Display.height() / ROWS;
    M5.Display.fillRect(col * slotW, row * slotH, slotW, slotH, color);
  }
};