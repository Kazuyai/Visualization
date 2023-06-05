#include "read_bitmap.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <myShape.h>
#define KEY_ESC 27
#define KEY_SPC 32

void polarview(void);
void showBackground(boolean);
void showDrone(float, float, float);
void showTitleWindow();
void DrawString(char* ,float, float, double, double, float);
void read_stl_file(FILE *);
void read_bmp_file(FILE *);
int ReadBitMapData();

// ゲームの状態
enum STATE {
    TITLE,
    IN_UP_ANIMATION,
    IN_FALL_ANIMATION,
    PLAYING,
    BEFORE_START,
    END
};
// 現在の状態
enum STATE state = TITLE;

// ライトの位置
float lightPos[] = {1, 1, 1, 0};

// 環境
float diffuse[] = {0.2, 0.2, 0.2, 1.0};
float diffuse1[] = {0.8, 0.8, 0.8, 1.0};
float diffuse2[] = {1, 0, 0, 1.0};
float specular[] = {1.0, 1.0, 1.0, 1.0};
float ambient[] = {0.1, 0.1, 0.1, 1.0};

// 難易度変更画面用変数
char *level[3] = {"EASY", "NORMAL", "HARD"};
int selectLevel = 1;

// スコア表示用変数
int score = 0;
char *scoreTextTemplate = "SCORE : ";
char scoreText[12];

// タイトル画面のカメラ用変数
float distance = 10.0, twist = 0.0, elevation = -15.0, azimuth = 0.0;

// プレイ中に使用する変数
boolean spaceKeyPressed = FALSE;                        // スペースキーが押されているか
float theta = 0;                                        // プレビューの回転
float propellerRot = 0;                                 // プロペラの回転
float rotationSpeed = 0;                                // 回転速度
float animTime = 0;                                     // アニメーション用時間変数
float animPosY = 0;                                     // ドローンのアニメ用座標変数
float animRotZ = 0;                                     // ドローンのアニメ用回転変数
int side = 8;                                           // 縦横の長さ
int depth = 1800;                                       // ステージの長さ
int goalPos = -1750;                                    // ゴールのZ座標
float posX = 0, posY = 0, posZ = 0;                     // 現在地
float speed = 0;                                        // 上下左右の移動速度
boolean isMoving = FALSE;                               // 上下左右に移動中かどうか
boolean isMovingRight = FALSE, isMovingLeft = FALSE;    // 左右に動いているか
float dist = 0;                                         // 前の移動量

// 輪の変数
int nextRing = 0;                                       // 次にくぐる輪の配列番号
int ringRadius = 3;                                     // 半径の長さ
int rings[10][3] = {                                    // 全ての輪の配置位置
    {4, 0, -200},
    {1, -2, -350},
    {-2, 3, -500},
    {0, 3, -650},
    {-4, 2, -800},
    {3, -4, -950},
    {1, 0, -1100},
    {4, -2, -1350},
    {-2, 0, -1500},
    {0, 2, -1650}
};

// 障害物の変数
int nextEnemy = 0;                                      // 次の障害物の配列番号
int enemyRadius = 4;                                    // 半径の長さ
int enemies[5][3] = {                                   // 全ての障害物の配置位置
    {-4, 2, -300},
    {-4, -4, -650},
    {2, 3, -850},
    {0, 1, -1000},
    {-4, -2, -1300}
};

// 描画設定変数
float nearw = 0.1, farw = 3100.0, fovy = 60.0;

// ドローン表示用変数
int drone_tnum;
float *drone_x, *drone_y, *drone_z;
float *drone_nx, *drone_ny, *drone_nz;

// プロペラ表示用変数
int propeller_tnum;
float *propeller_x, *propeller_y, *propeller_z;
float *propeller_nx, *propeller_ny, *propeller_nz;

// 24ビットbmp読み込み用変数
unsigned char *image;
unsigned char *r1, *g1, *b1;
int width, height;

// ファイル読み込み用変数
FILE *fp1;    // ドローン
FILE *fp2;    // プロペラ
FILE *fp3[15]; // 背景
char *file_names[15] = {
    "front.bmp", "right.bmp", "back.bmp", "left.bmp", "bottom.bmp",
    "front2.bmp", "right2.bmp", "back2.bmp", "left2.bmp", "bottom2.bmp",
    "front3.bmp", "right3.bmp", "back3.bmp", "left3.bmp", "bottom3.bmp"
};

// 描画用関数
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix(); // 全体
    glEnable(GL_DEPTH_TEST);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse1);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    
    // 開始前の処理
    if(state == TITLE) {
        polarview();
        // 背景
        showBackground(TRUE);
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        glTranslatef(0, -6, 0);
        mySolidCylinder(5, 10, 32);
        glPopMatrix();
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        showDrone(0, 0, 0);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glPopMatrix();
        showTitleWindow();
        DrawString("Hold Space", 0, -150, 500, 500, 1);
        glutSwapBuffers();
        return;
    } else if(state == IN_UP_ANIMATION) {
        polarview();
        // 背景
        showBackground(TRUE);
        glDisable(GL_TEXTURE_2D);
        glPushMatrix();
        glTranslatef(0, -6, 0);
        mySolidCylinder(5, 10, 32);
        glPopMatrix();
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        // アニメーション用の計算を行う
        showDrone(0, animPosY, 0);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glPopMatrix();
        glutSwapBuffers();
        return;
    }
    
    gluLookAt(0.0, 0, 20 - dist, 0.0, 0.0, 0.0 - dist, 0.0, 1.0, 0.0);
    // 背景
    showBackground(FALSE);
    
    glPushMatrix();     // ゴールテープ
    glBindTexture(GL_TEXTURE_2D, 30);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-16, -2, goalPos);
    glTexCoord2f(0.0, 3.0);
    glVertex3f(-16, 2, goalPos);
    glTexCoord2f(18.0, 3.0);
    glVertex3f(16, 2, goalPos);
    glTexCoord2f(18.0, 0.0);
    glVertex3f(16, -2, goalPos);
    glEnd();
    glPopMatrix();      // ゴールテープ閉じ

    glPushMatrix();     // ゴールポール右
    glScalef(1, 6, 1);
    glTranslatef(16, 0, goalPos);
    glutSolidCube(1);
    glPopMatrix();      // ゴールポール右閉じ

    glPushMatrix();     // ゴールポール左
    glScalef(1, 6, 1);
    glTranslatef(-16, 0, goalPos);
    glutSolidCube(1);
    glPopMatrix();      // ゴールポール左閉じ
    
    glDisable(GL_TEXTURE_2D);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);

    // ドローンの表示
    showDrone(posX, posY, posZ);

    glDisable(GL_LIGHTING);

    // 全ての輪を表示
    glColor3f(1,0.2,0.2);
    for(int i = (nextRing - 1 < 0) ? 0 : (nextRing - 1); i < sizeof(rings) / sizeof(rings[0]); i++) {
        glPushMatrix(); // 輪
        glTranslatef(rings[i][0], rings[i][1], rings[i][2]);
        glScalef(1, 1, 0.2);
        glutSolidTorus(1, 1 + ringRadius, 32, 32);
        glPopMatrix(); // 輪閉じ
    }

    // 障害物を設置
    glColor3f(0.2,0.2,0.2);
    if(selectLevel != 0) {
        // 全ての障害物を表示
        for(int i = (nextEnemy - 1 < 0) ? 0 : (nextEnemy - 1); i < sizeof(enemies) / sizeof(enemies[0]); i++) {
            glPushMatrix(); // 障害物
            glTranslatef(enemies[i][0], enemies[i][1], enemies[i][2]);
            glRotatef(90, 1, 0, 0);
            mySolidCylinder(enemyRadius, 0.5, 32);
            glPopMatrix(); // 障害物閉じ
        }
    }

    glDisable(GL_DEPTH_TEST);
    glPopMatrix(); // 全体閉じ

    // テキスト表示
    if(state == PLAYING) {
        sprintf(scoreText, "%s%d", scoreTextTemplate, score);
        DrawString(scoreText, -130, 200, 500, 500, 1);
    } else if(state == BEFORE_START) {
        DrawString("Press Space", 0, -150, 500, 500, 1);
    } else if(state == END) {
        // 結果画面
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0., 500, 0., 500);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1,1,1, 0.8);
        glBegin(GL_POLYGON);
        glVertex3f( 400, 400, 0.0 );
        glVertex3f( 100, 400, 0.0 );
        glVertex3f( 100, 100, 0.0 );
        glVertex3f( 400, 100, 0.0 );
        glEnd();
        glPopMatrix();
        glDisable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        DrawString("SCORE", 0, 100, 500, 500, 1);
        sprintf(scoreText, "%d", score);
        DrawString(scoreText, 0, -50, 500, 500, 3);
    }

    glutSwapBuffers();

    // ゴール判定
    if(posZ < goalPos) {
        state = END;
    } 

    // 輪くぐり判定
    // 最後の輪を通過した後は何もしない
    if(nextRing != sizeof(rings) / sizeof(rings[0])) {
        // 次の輪までのZ軸の距離
        float disZ = posZ - rings[nextRing][2];

        // 輪を通り過ぎた瞬間に判定を行う
        if(disZ < 0) {
            // XY軸での輪とドローンの距離(平方根を取る前)
            float disXY = (posX - rings[nextRing][0]) * (posX - rings[nextRing][0]) + (posY - rings[nextRing][1]) * (posY - rings[nextRing][1]);
            // 輪をくぐった時の処理 (半径の2乗の値を入れる)
            if(disXY < ringRadius * ringRadius) {
                score += 10;
                printf("-------%d-------\n", nextRing);
            }
            nextRing++;
        }
    }

    if(selectLevel != 0) {
        // 障害物の当たり判定
        // 最後の障害物を通過した後は何もしない
        if(nextEnemy != sizeof(enemies) / sizeof(enemies[0])) {
            // 次の障害物までのZ軸の距離
            float disZ = posZ - enemies[nextEnemy][2];

            // 障害物を通り過ぎた瞬間に判定を行う
            if(disZ < 0) {
                // XY軸での障害物とドローンの距離(平方根を取る前)
                float disXY = (posX - enemies[nextEnemy][0]) * (posX - enemies[nextEnemy][0]) + (posY - enemies[nextEnemy][1]) * (posY - enemies[nextEnemy][1]);
                // 障害物に当たった時の処理 (半径の2乗の値を入れる)
                if(disXY < enemyRadius * enemyRadius) {
                    state = IN_FALL_ANIMATION;
                    printf("------HIT------\n");
                }
                nextEnemy++;
            }
        }
    }
}

void polarview(void) {
    glTranslatef(0.0, 0.0, -distance);
    glRotatef(-twist, 0.0, 0.0, 1.0);
    glRotatef(-elevation, 1.0, 0.0, 0.0);
    glRotatef(-azimuth, 0.0, 1.0, 0.0);
}

// 設定初期化関数
void resetSetting() {
    score = 0;
    spaceKeyPressed = FALSE;
    theta = 0;
    propellerRot = 0;
    rotationSpeed = 0;
    animTime = 0;
    animPosY = 0;
    animRotZ = 0;
    posX = 0;
    posY = 0;
    posZ = 0;
    speed = 0;
    isMoving = FALSE;
    isMovingRight = FALSE;
    isMovingLeft = FALSE;
    dist = 0;
    nextRing = 0;
    nextEnemy = 0;
}

// 背景を表示する関数
void showBackground(boolean isPreview) {
    glPushMatrix(); // 背景
    if(isPreview) {
        // 後ろ
        glBindTexture(GL_TEXTURE_2D, 12 + selectLevel * 5);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(2000, -2000, 2000);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(2000, 2000, 2000);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(-2000, 2000, 2000);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(-2000, -2000, 2000);
        glEnd();
        // 下
        glBindTexture(GL_TEXTURE_2D, 14 + selectLevel * 5);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-2000, -2000, 2000);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-2000, -2000, -2000);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(2000, -2000, -2000);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(2000, -2000, 2000);
        glEnd();
    }
    // 前
    glBindTexture(GL_TEXTURE_2D, 10 + selectLevel * 5);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-2000, -2000, -2000);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-2000, 2000, -2000);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(2000, 2000, -2000);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(2000, -2000, -2000);
    glEnd();
    // 右
    glBindTexture(GL_TEXTURE_2D, 11 + selectLevel * 5);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(2000, -2000, -2000);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(2000, 2000, -2000);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(2000, 2000, 2000);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(2000, -2000, 2000);
    glEnd();
    // 左
    glBindTexture(GL_TEXTURE_2D, 13 + selectLevel * 5);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-2000, -2000, 2000);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-2000, 2000, 2000);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(-2000, 2000, -2000);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(-2000, -2000, -2000);
    glEnd();
    glPopMatrix(); // 背景閉じ
}

// ドローンを描画する関数
void showDrone(float x, float y, float z) {
    glPushMatrix(); // ドローン
    glTranslatef(x, y, z);
    if(state == IN_FALL_ANIMATION) {
        //glTranslatef(0,animPosY,0);
        glRotatef(animRotZ, 0, 0, 1);
    } 
    if(isMovingRight) {
        glRotatef(-30, 0, 0, 1);
    } else if(isMovingLeft) {
        glRotatef(30, 0, 0, 1);
    }
    for(int i = 0; i < drone_tnum; i++) {
        glBegin(GL_POLYGON);
        glNormal3f(drone_nx[i], drone_ny[i], drone_nz[i]);
        glVertex3f(drone_x[3 * i + 0], drone_y[3 * i + 0], drone_z[3 * i + 0]);
        glVertex3f(drone_x[3 * i + 1], drone_y[3 * i + 1], drone_z[3 * i + 1]);
        glVertex3f(drone_x[3 * i + 2], drone_y[3 * i + 2], drone_z[3 * i + 2]);
        glEnd();
    }

    // 4つのプロペラを表示する
    for(int i = 0; i < 4; i++) {
        glPushMatrix(); // プロペラ
        if(i == 0) {
            glTranslatef(1.9, 1, 2.2);
        } else if(i == 1) {
            glTranslatef(1.9, 1, -2.2);
        } else if(i == 2) {
            glTranslatef(-1.9, 1, 2.2);
        } else {
            glTranslatef(-1.9, 1, -2.2);
        }
        if(state == PLAYING) {
            glRotatef(dist * 20, 0, 1, 0);
        } else if(state == TITLE || state == IN_UP_ANIMATION) {
            glRotatef(propellerRot * 5, 0, 1, 0);
        }
        
        for(int j = 0; j < propeller_tnum; j++) {
            glBegin(GL_POLYGON);
            glNormal3f(propeller_nx[j], propeller_ny[j], propeller_nz[j]);
            glVertex3f(propeller_x[3 * j + 0], propeller_y[3 * j + 0], propeller_z[3 * j + 0]);
            glVertex3f(propeller_x[3 * j + 1], propeller_y[3 * j + 1], propeller_z[3 * j + 1]);
            glVertex3f(propeller_x[3 * j + 2], propeller_y[3 * j + 2], propeller_z[3 * j + 2]);
            glEnd();
        }
        glPopMatrix(); // プロペラ閉じ
    }

    glPopMatrix(); // ドローン閉じ
}

// タイトル画面のテキストを表示する関数
void showTitleWindow() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 500, 0, 500);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 難易度選択画面の背景表示
    glColor4f(1,1,1,0.9);
    glBegin(GL_POLYGON);
    glVertex3f( 490, 450, 0.0 );
    glVertex3f( 380, 450, 0.0 );
    glVertex3f( 380, 100, 0.0 );
    glVertex3f( 490, 100, 0.0 );
    glEnd();
    // 難易度テキストの表示
    for(int i = 0; i < 3; i++) {
        glColor4f(i == 2, i == 0, i == 1, 0.8);
        glBegin(GL_POLYGON);
        glVertex3f( 480, 380 - i * 100, 0.0 );
        glVertex3f( 390, 380 - i * 100, 0.0 );
        glVertex3f( 390, 330 - i * 100, 0.0 );
        glVertex3f( 480, 330 - i * 100, 0.0 );
        glEnd();
        glColor4f(1, 1, 1, 1);
        for(int j = 0; j < strlen(level[i]); j++) {
            glRasterPos2f(440 + 15 * j - 15 * strlen(level[i]) / 2, 350 - 100 * i);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, level[i][j]);
        }
    }
    glDisable(GL_BLEND);
    // 選択枠線の表示
    glColor3f(1,1,0);
    glLineWidth(8);
    glBegin(GL_LINE_LOOP);
    glVertex3f( 480, 380 - 100 * selectLevel, 0.0 );
    glVertex3f( 390, 380 - 100 * selectLevel, 0.0 );
    glVertex3f( 390, 330 - 100 * selectLevel, 0.0 );
    glVertex3f( 480, 330 - 100 * selectLevel, 0.0 );
    glEnd();
    // 回転数バーの中身を表示
    glColor3f(0,1,0);
    glBegin(GL_POLYGON);
    glVertex3f( 150 + 200 * (propellerRot / 1080), 60, 0.0 );
    glVertex3f( 150, 60, 0.0 );
    glVertex3f( 150, 20, 0.0 );
    glVertex3f( 150 + 200 * (propellerRot / 1080), 20, 0.0 );
    glEnd();
    // 回転数バーの枠を表示
    glColor3f(0,0,1);
    glLineWidth(7);
    glBegin(GL_POLYGON);
    glVertex3f( 150, 55, 0.0 );
    glVertex3f( 142, 55, 0.0 );
    glVertex3f( 142, 25, 0.0 );
    glVertex3f( 150, 25, 0.0 );
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3f( 350, 60, 0.0 );
    glVertex3f( 150, 60, 0.0 );
    glVertex3f( 150, 20, 0.0 );
    glVertex3f( 350, 20, 0.0 );
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    DrawString("Level", 185, 160, 500, 500, 1);
}

// 文字を表示させる関数
void DrawString(char *str, float x, float y, double w, double h, float size) {
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_LINE_SMOOTH);
    // 文字の太さを変更
    glLineWidth(5);
    glPushMatrix();
    glColor3d(1., 0., 0.);
    glLoadIdentity();
    gluOrtho2D(0., w, 0, h);
    glDisable(GL_DEPTH_TEST);
    glTranslatef(x - strlen(str) * 10 * size + w / 2, y + w / 2, 0);
    glScalef(0.2 * size, 0.2 * size, 1);
    // 各文字を3回ずらして表示することで太字を疑似的に実装
    for(int i = 0; i < strlen(str); i++) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, str[i]);
        glPushMatrix();
        glTranslatef(-103, 0, 0);
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, str[i]);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(-97 - size * 3, 0, 0);
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, str[i]);
        glPopMatrix();
    }
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glDisable(GL_LINE_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
}
 
// 常時呼び出される関数
void idle(void) {
    if(state == PLAYING) {
        dist = fmod(dist + 0.2, depth);
        posZ = -dist;
        // ハードモードの場合のみ風で左に流される
        if(selectLevel == 2 && posX > -side) {
            posX -= 0.005;
        }
    }else if(state == TITLE) {
        if(!spaceKeyPressed) {
            azimuth = fmod(azimuth + 0.1, 360);
        }
    } else if(state == IN_UP_ANIMATION) {
        propellerRot += rotationSpeed;
        animTime += 0.005;
        // y = 高さ、　x = 時間とした時に y = 2x^3 - 6x^2 + 5x　という3次関数の式に沿って変化させる
        animPosY = (2 * animTime * animTime * animTime - 6 * animTime * animTime + 5 * animTime);
        if(animPosY >= 15) {
            state = BEFORE_START;
        }
    } else if(state == IN_FALL_ANIMATION) {
        posY -= 0.1;
        animRotZ += 2;
        if(posY < -20) {
            state = END;
        }
    }

    glutPostRedisplay();
}

// 特殊キーが押されたときの関数
void myKbd(unsigned char key, int x, int y) {
    switch(key) {
    case KEY_ESC:
        exit(0);
    case KEY_SPC:
        switch (state)
        {
        case TITLE:
            spaceKeyPressed = TRUE;
            rotationSpeed += 0.2;
            propellerRot += rotationSpeed;
            if(propellerRot > 1080) {
                state = IN_UP_ANIMATION;
            }
            break;
        case BEFORE_START:
            state = PLAYING;
            printf("-----START-----\n");
            break;
        case END:
            state = TITLE;
            resetSetting();
            break;
        }
        break;
    }
}

// 特殊キーが離されたときの関数
void myKbdUp(unsigned char key, int x, int y) {
    switch(key) {
        case KEY_SPC:
            switch (state)
            {
            case TITLE:
                spaceKeyPressed = FALSE;
                rotationSpeed = 0;
                propellerRot = 0;
                break;
            case BEFORE_START:
                state = PLAYING;
                printf("-----START-----\n");
                break;
            }
            break;
    }
}



// キーが押された時の関数
void mySkey(int key, int x, int y) {
    if(state != PLAYING && state != TITLE) return; 
    if(key == GLUT_KEY_UP) {
        if(state == PLAYING) {
            isMoving = TRUE;
            if(posY < side) {
                speed += 0.1;
                posY += speed;
            }
        } else {
            selectLevel = selectLevel == 0 ? 0 : selectLevel - 1;
        }
    }
    if(key == GLUT_KEY_DOWN) {
        if(state == PLAYING) {
            isMoving = TRUE;
            if(posY > -side) {
                speed -= 0.1;
                posY += speed;
            }
        }else {
            selectLevel = selectLevel == 2 ? 2 : selectLevel + 1;
        }
    }
    if(key == GLUT_KEY_RIGHT) {
        if(state == PLAYING) {
            isMoving = TRUE;
            if(posX < side) {
                speed += 0.1;
                posX += speed;
                isMovingRight = TRUE;
            }
        }
    }
    if(key == GLUT_KEY_LEFT) {
        if(state == PLAYING) {
            isMoving = TRUE;
            if(posX > -side) {
                speed -= 0.1;
                posX += speed;
                isMovingLeft = TRUE;
            }
        }
    }
}

// キーが離されたときの関数
void mySkeyUp(int key, int x, int y) {
    if(key == GLUT_KEY_UP) {
        isMoving = FALSE;
        speed = 0;
    }
    if(key == GLUT_KEY_DOWN) {
        isMoving = FALSE;
        speed = 0;
    }
    if(key == GLUT_KEY_RIGHT) {
        isMoving = FALSE;
        speed = 0;
        isMovingRight = FALSE;
    }
    if(key == GLUT_KEY_LEFT) {
        isMoving = FALSE;
        speed = 0;
        isMovingLeft = FALSE;
    }
}

// RGBの情報をimage変数に渡す関数
void makeImage(void) {
    int i, j;

    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            image[0 + 4 * (j + i * width)] = (unsigned char)r1[j + width * i];
            image[1 + 4 * (j + i * width)] = (unsigned char)g1[j + width * i];
            image[2 + 4 * (j + i * width)] = (unsigned char)b1[j + width * i];
            image[3 + 4 * (j + i * width)] = (unsigned char)255;
        }
    }
}

// 画像テクスチャ作成関数
void initTexture2(int id) {
    makeImage();

    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

// ゴールテープテクスチャ作成関数
void initTexture3(void) {
    unsigned char *image1 = (unsigned char *)malloc(4 * 8 * 8);
    unsigned char bitmap1[8][8] = {
    {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}, {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}, {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}, {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}, {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff}};

    int i, j, c;

    for(i = 0; i < 8; i++) {
        for(j = 0; j < 8; j++) {
            c = bitmap1[i][j];
            image1[0 + 4 * (j + i * 8)] = (unsigned char)c;
            image1[1 + 4 * (j + i * 8)] = (unsigned char)c;
            image1[2 + 4 * (j + i * 8)] = (unsigned char)c;
            image1[3 + 4 * (j + i * 8)] = (unsigned char)255;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 30);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

// 初期化関数
void myInit(char *progname) {
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1080, 720);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow(progname);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // ワールド背景の画像を取得する
    for(int i = 0; i < sizeof(fp3) / sizeof(fp3[0]); i++) {
        char *file_name = "";
        sprintf(file_name, "Images/%s", file_names[i]);
        fp3[i] = fopen(file_name, "rb");
        if(fp3[i] == 0) {
            fprintf(stderr, " File open error");
            exit(-1);
        } else {
            fprintf(stderr, "File open success");
        }
        read_bmp_file(fp3[i]);
        initTexture2(10 + i);
    }
    // ゴールのテクスチャを作成する
    initTexture3();
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHT0);
}

// 画面リサイズ関数
void myReshape(int width, int height) {
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, aspect, nearw, farw);
    glMatrixMode(GL_MODELVIEW);
}

// メイン関数
void main(int argc, char **argv) {
    glutInit(&argc, argv);

    fp1 = fopen("Drone.stl", "r");
    if(fp1 == 0) {                
        fprintf(stderr, " File open error");
        exit(-1);
    } else {
        fprintf(stderr, "File open success");
    }
    read_stl_file(fp1);

    fp2 = fopen("propeller.stl", "r"); 
    if(fp2 == 0) {                     
        fprintf(stderr, " File open error");
        exit(-1);
    } else { 
        fprintf(stderr, "File open success");
    }
    read_stl_file(fp2);
    printf("%d : %d", drone_tnum, propeller_tnum);
    fclose(fp1);
    fclose(fp2);

    myInit(argv[0]);
    for(int i = 0; i < 6; i++) {
        fclose(fp3[i]);
    }
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(myKbd);
    glutKeyboardUpFunc(myKbdUp);
    glutSpecialFunc(mySkey);
    glutSpecialUpFunc(mySkeyUp);
    glutMainLoop();
}

// STLファイルの中身を読む関数
void read_stl_file(FILE *fp) {
    int i, j, k;
    int maxi = 100000;
    char buf[10000];
    char key[64], key2[64];

    unsigned int width1, height1;
    float xx, yy, zz;
    float nxx, nyy, nzz;

    int tnum;

    float *x, *y, *z;
    float *nx, *ny, *nz;

    nxx = nyy = nzz = 0.;
    tnum = 0;

    fgets(buf, 10000, fp); // title //
    printf(" buf = %s\n", buf);
    sscanf(buf, "%s", key);
    if(strcmp(key, "solid") == 0) {
        printf("OK 1\n");
    } else {
        printf("NG 1: %s\n", key);
    }
    for(i = 0; i < maxi; i++) {
        fgets(buf, 10000, fp); // title //
        sscanf(buf, "%s", key);
        if(strcmp(key, "facet") == 0) {
            //			printf("OK: %d\n",i);
        } else if(strcmp(key, "endsolid") == 0) {
            printf("Normal termination: %s\n", key);
            break;
        } else {
            printf("NG: %d, %s\n", i, key);
            exit(-1);
        }
        fgets(buf, 10000, fp); // outer loop
        fgets(buf, 10000, fp); // point1
        fgets(buf, 10000, fp); // point2
        fgets(buf, 10000, fp); // point3
        fgets(buf, 10000, fp); // end loop

        fgets(buf, 10000, fp); // title //
        sscanf(buf, "%s", key);
        if(strcmp(key, "endfacet") == 0) {
            //			printf("OK end \n");
        } else {
            printf("NG end: %s\n", key);
            exit(-1);
        }
    }
    tnum = i;
    printf(" tnum = %d\n", tnum);
    x = (float *)malloc(3 * tnum * sizeof(float));
    y = (float *)malloc(3 * tnum * sizeof(float));
    z = (float *)malloc(3 * tnum * sizeof(float));
    nx = (float *)malloc(tnum * sizeof(float));
    ny = (float *)malloc(tnum * sizeof(float));
    nz = (float *)malloc(tnum * sizeof(float));
    rewind(fp);

    fgets(buf, 10000, fp); // title //
    printf(" buf = %s\n", buf);
    sscanf(buf, "%s", key);
    if(strcmp(key, "solid") == 0) {
        printf("OK 1\n");
    } else {
        printf("NG 1: %s\n", key);
    }
    for(i = 0; i < tnum; i++) {
        fgets(buf, 10000, fp); // title //
        sscanf(buf, "%s %s %f %f %f", key, key2, &nxx, &nyy, &nzz);
        if(strcmp(key, "facet") == 0) {
            //			printf("OK: %d\n",i);
            nx[i] = nxx;
            ny[i] = nyy;
            nz[i] = nzz;
        } else if(strcmp(key, "endsolid") == 0) {
            printf("Normal termination: %s\n", key);
            break;
        } else {
            printf("NG: %d, %s\n", i, key);
            exit(-1);
        }
        fgets(buf, 10000, fp); // outer loop
        fgets(buf, 10000, fp); // point1
        sscanf(buf, "%s %f %f %f", key, &xx, &yy, &zz);
        x[3 * i + 0] = xx;
        y[3 * i + 0] = yy;
        z[3 * i + 0] = zz;
        fgets(buf, 10000, fp); // point2
        sscanf(buf, "%s %f %f %f", key, &xx, &yy, &zz);
        x[3 * i + 1] = xx;
        y[3 * i + 1] = yy;
        z[3 * i + 1] = zz;
        fgets(buf, 10000, fp); // point3
        sscanf(buf, "%s %f %f %f", key, &xx, &yy, &zz);
        x[3 * i + 2] = xx;
        y[3 * i + 2] = yy;
        z[3 * i + 2] = zz;
        fgets(buf, 10000, fp); // end loop

        fgets(buf, 10000, fp); // title //
        sscanf(buf, "%s", key);
        if(strcmp(key, "endfacet") == 0) {
            //			printf("OK STL data was stored \n");
        } else {
            printf("NG end: %s\n", key);
            exit(-1);
        }
    }
    printf("OK STL data was stored \n");
    // ファイルごとに異なる変数に代入
    if(fp == fp1) {
        drone_tnum = tnum;
        drone_x = x;
        drone_y = y;
        drone_z = z;
        drone_nx = nx;
        drone_ny = ny;
        drone_nz = nz;
    } else if(fp == fp2) {
        propeller_tnum = tnum;
        propeller_x = x;
        propeller_y = y;
        propeller_z = z;
        propeller_nx = nx;
        propeller_ny = ny;
        propeller_nz = nz;
    }
}

// BMPファイルの中身を読む関数
void read_bmp_file(FILE *fp) {
    int i, j, k;
    unsigned int width1, height1;
    unsigned short color;
    unsigned int dumlong;
    unsigned char buf10[10];
    unsigned char buf2[2];
    unsigned char buf20[20];
    unsigned int image_start_point, header_size;
    unsigned char pad[4];

    fread(buf10, sizeof(unsigned char), 10, fp);            // 10byte
    fread(&image_start_point, sizeof(unsigned int), 1, fp); // 14byte
    fprintf(stderr, " image start point = %d\n", image_start_point);

    fread(&header_size, sizeof(unsigned int), 1, fp); // 18byte
    fprintf(stderr, " header_size = %ld\n", header_size);
    if(image_start_point == (14 + header_size)) {
        fprintf(stderr, " header size OK! \n");
    } else {
        fprintf(stderr, " header size error \n");
        exit(-1);
    }

    fread(&width1, sizeof(unsigned int), 1, fp);  // 22byte
    fread(&height1, sizeof(unsigned int), 1, fp); // 26byte
    fprintf(stderr, " Width, Height = %d %d\n", width1, height1);

    r1 = (unsigned char *)malloc(width1 * height1);
    g1 = (unsigned char *)malloc(width1 * height1);
    b1 = (unsigned char *)malloc(width1 * height1);
    image = (unsigned char *)malloc(4 * width1 * height1);

    width = width1;
    height = height1;

    fread(buf2, sizeof(char), 2, fp);             // 28byte
    fread(&color, sizeof(unsigned short), 1, fp); // 30byte
    fread(&dumlong, sizeof(unsigned int), 1, fp); // 34byte
    if(dumlong == 0) {                            // 無圧縮
        fprintf(stderr, " It's non compress data. OK! \n");
    } else {
        fprintf(stderr, " Oh! No. Why did you compress the image ? %d\n", dumlong);
        exit(-1);
    }

    fread(buf20, sizeof(char), 20, fp);

    k = 0;
    for(j = 0; j < height1; j++) {
        for(i = 0; i < width1; i++) {
            fread(&b1[k], sizeof(unsigned char), 1, fp);
            fread(&g1[k], sizeof(unsigned char), 1, fp);
            fread(&r1[k], sizeof(unsigned char), 1, fp);
            k++;
        }
        if((3 * width) % 4 != 0)
            fread(pad, sizeof(unsigned char), (4 - (3 * width) % 4), fp);
    }
}