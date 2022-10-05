#include <iostream>
#include <random>
#include <algorithm>
#include <string>
#include "stdio.h"
#include "raylib.h"

#define mapSize 50

constexpr float MAX_VX = 6.0f;
constexpr float ACCEL =  0.45f;
constexpr float JUMP =  15.0f;
constexpr int JUMP_FLOAT_TIME =  25;
constexpr float GRAVITY =  1.0f;
constexpr float FRICTION =  0.3f;
constexpr int CLOUD_TIME =  45;
constexpr int PLATFORM_TIME =  90;

enum AnimState {Run, Jump, Fall, Slide, Stand, Squish};

typedef struct Player {
    Rectangle col;
    float vx;
    float vy;
    bool onGround;
    bool onLeftWall;
    bool onRightWall;
    bool holdingJump;
    int holdTime;
    AnimState animState;
    int animFrame;
} Player;

enum gameState {Main, Intro, Game, Pause, Win};

Rectangle map[mapSize] = {{-40,7210,2000 ,40},{-40,-40,2000,40},{-40,-40,40,7290},{1920,-40,40,7290},
                          {0,6877,460,190},{1474,7049,450,160},{1771,6907,150,160},{1281,6720,330,100},{800,6809,170,50},{0,6731,170,150},{260,6316,140,230},{0,6116,30,630},{654,6182,140,160},{1048,6044,140,160},{1421,6044,140,160},{1760,5654,160,390},{1432,5344,130,350},{1848,5480,90,180},{1532,5343,110,40},{1194,5055,450,90},{885,5645,350,50},{1118,5034,120,410},{863,5123,260,70},{406,5568,230,50},{0,5431,230,50},{0,5234,80,80},{324,5126,230,50},{1453,4909,150,110},{1757,4727,150,110},{1446,4539,150,110},{1143,4359,150,120},{528,4591,260,150},{251,4507,50,100},{0,4377,110,200},{0,4239,40,140},{276,4093,220,40},{710,3950,220,40},{1133,3833,220,40},{1130,3660,220,40},{826,3464,210,70},{453,3404,220,40},{311,3222,220,40},{0,3047,200,190},{0,2882,90,210},{282,2717,30,40},{534,2720,30,40},{824,2836,30,40},{1116,2762,150,110},{1493,2675,30,40},{1690,2532,310,310}};
//Rectangle blocks[1000];

int clouds[5] = {27,28,29,30,47};
int cloudTime[5] = {CLOUD_TIME,CLOUD_TIME,CLOUD_TIME,CLOUD_TIME,CLOUD_TIME};
bool shouldCloudReplenish[5] = {true,true,true,true,true};

int togglePlatforms[6] = {35,36,37,38,40,41};
bool toggleStates[6] = {false,true,false,true,false,true};
int platformTime = 0;

Texture2D bushStand;
Texture2D bushWalk1;
Texture2D bushWalk2;
Texture2D bushWalk3;
Texture2D bushJump;
Texture2D bushSquish;
Texture2D bushWall;
Texture2D comic1;
Texture2D comic2;
Texture2D comic3;
Texture2D comic4;
Texture2D Ka;
Texture2D Kb;
Texture2D Kc;
Texture2D Kd;
Texture2D Ke;
Texture2D Kf;
Texture2D Kg;
Texture2D Kh;
Texture2D Ki;
Texture2D Kj;
Texture2D Kk;
Texture2D Kl;
Texture2D Km;
Texture2D Kn;
Texture2D Ko;
Texture2D Kp;
Texture2D Kq;
Texture2D Kr;
Texture2D Ks;
Texture2D Kt;
Texture2D Ku;
Texture2D Kv;
Texture2D Kw;
Texture2D Kx;
Texture2D Ky;
Texture2D Kz;
Texture2D mapBack;
Texture2D mapFront;
Texture2D cloud1;
Texture2D cloud2;
Texture2D platform1;
Texture2D platform2;
Texture2D platform1gone;
Texture2D platform2gone;

Texture2D loading;

Sound jump1;
Sound jump2;
Sound jump3;
Sound land1;
Sound land2;
Sound slide;
Sound walk1;
Sound walk2;
Sound walk3;
Sound change;
Sound forest;
Sound sky;
Font bradley;

Camera2D camera;
Player player;

int keys;
int nextKey;
double keyTime;
int lastWallJump;
int animeCount;
bool canJumpAgain;
double runTime;
int framesSinceGround;
int direction;
gameState state;
gameState lastState;
bool firstFrameOfState;
int comicState;
float songTransition;
float SFX;
float BGM;
bool inited;
Rectangle button1;
Rectangle button2;

Rectangle colliding(Rectangle col) {
    Rectangle ret = {0,0,0,0};

    for(int i=0;i<mapSize;i++) {
        if(CheckCollisionRecs(map[i],col)) {
            bool ignore = false;
            for(int j=0;j<5;j++) {
                if(clouds[j] == i) {
                    if(cloudTime[j] <= 0) {
                        ignore = true;
                    } else {
                        cloudTime[j]--;
                    }
                    shouldCloudReplenish[j] = false;
                }
            }
            for(int j=0;j<6;j++) {
                if(togglePlatforms[j] == i) {
                    ignore = !toggleStates[j];
                }
            }
            if(!ignore) {
                ret = map[i];
            }
        }
    }

//    for(int i=0;i<1000;i++) {
//        if(CheckCollisionRecs(blocks[i],col)) {
//            ret = blocks[i];
//        }
//    }

    return ret;
}

int random(int min, int max) {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(min, max); // define the range

    return distr(gen);
}

int* randomizeKey(int* arr, int key) {
    int num = random(65,90);
    if(key == 0) {
        while(arr[1] == num || arr[2] == num) {
            num = random(65,90);
        }
        arr[0] = num;
    }
    if(key == 1) {
        while(arr[0] == num || arr[2] == num) {
            num = random(65,90);
        }
        arr[1] = num;
    }
    if(key == 2) {
        while(arr[1] == num || arr[0] == num) {
            num = random(65,90);
        }
        arr[2] = num;
    }

    return arr;
}

void mainGameLoop() {
    SCREEN_WIDTH = GetScreenWidth();
        SCREEN_HEIGHT = GetScreenHeight();

        switch (state) {
            case Main:{

                button1.x = SCREEN_WIDTH/2 - button1.width/2;
                button1.y = SCREEN_HEIGHT/2 - button1.height/2;

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(),button1)) {
                    state = Intro;
                    firstFrameOfState = true;

                    if(!inited) {
                        InitAudioDevice();
                        jump1 = LoadSound("./assets/Jump1.wav");
                        jump2 = LoadSound("./assets/Jump2.wav");
                        jump3 = LoadSound("./assets/Jump3.wav");
                        land1 = LoadSound("./assets/land1.wav");
                        land2 = LoadSound("./assets/land2.wav");
                        slide = LoadSound("./assets/slide.wav");
                        walk1 = LoadSound("./assets/walk1.wav");
                        walk2 = LoadSound("./assets/walk2.wav");
                        walk3 = LoadSound("./assets/walk3.wav");
                        change = LoadSound("./assets/change.wav");

                        SetSoundVolume(jump1, 0.5f);
                        SetSoundVolume(jump2, 0.5f);
                        SetSoundVolume(jump3, 0.5f);
                        SetSoundVolume(land1, 0.5f);
                        SetSoundVolume(land2, 0.5f);
                        SetSoundVolume(slide, 0.5f);
                        SetSoundVolume(walk1, 0.5f);
                        SetSoundVolume(walk2, 0.5f);
                        SetSoundVolume(walk3, 0.5f);
                        SetSoundVolume(change, 0.5f);

                        forest = LoadSound("./assets/forest.mp3");
                        sky = LoadSound("./assets/sky.mp3");

                        SetSoundVolume(forest, 0.5f);
                        SetSoundVolume(sky, 0.5f);
                        inited = true;
                    }
                    PlaySound(walk1);
                }
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(),button2)) {
                    state = Pause;
                    lastState = Main;
                    if(!inited) {
                        InitAudioDevice();
                        jump1 = LoadSound("./assets/Jump1.wav");
                        jump2 = LoadSound("./assets/Jump2.wav");
                        jump3 = LoadSound("./assets/Jump3.wav");
                        land1 = LoadSound("./assets/land1.wav");
                        land2 = LoadSound("./assets/land2.wav");
                        slide = LoadSound("./assets/slide.wav");
                        walk1 = LoadSound("./assets/walk1.wav");
                        walk2 = LoadSound("./assets/walk2.wav");
                        walk3 = LoadSound("./assets/walk3.wav");
                        change = LoadSound("./assets/change.wav");

                        SetSoundVolume(jump1, 0.5f);
                        SetSoundVolume(jump2, 0.5f);
                        SetSoundVolume(jump3, 0.5f);
                        SetSoundVolume(land1, 0.5f);
                        SetSoundVolume(land2, 0.5f);
                        SetSoundVolume(slide, 0.5f);
                        SetSoundVolume(walk1, 0.5f);
                        SetSoundVolume(walk2, 0.5f);
                        SetSoundVolume(walk3, 0.5f);
                        SetSoundVolume(change, 0.5f);

                        forest = LoadSound("./assets/forest.mp3");
                        sky = LoadSound("./assets/sky.mp3");

                        SetSoundVolume(forest, 0.5f);
                        SetSoundVolume(sky, 0.5f);
                        inited = true;
                    }
                    PlaySound(walk1);
                }

                BeginDrawing();

                ClearBackground(RAYWHITE);

                DrawTexture(mapBack,0,-4960+SCREEN_HEIGHT,WHITE);

                DrawTextEx(bradley, "Cabush's Kooky Climb", Vector2{SCREEN_WIDTH/2-400,155}, 80, 64.0f/10.0f, Color{30,30,30,255});
                DrawTextEx(bradley, "Cabush's Kooky Climb", Vector2{SCREEN_WIDTH/2-400,150}, 80, 64.0f/10.0f, WHITE);

                DrawRectangleRounded(Rectangle{button1.x-1,button1.y+10,button1.width+2,button1.height},0.1f,10,Color{30,30,30,255});
                DrawRectangleRounded(button1,0.1f,10,CheckCollisionPointRec(GetMousePosition(), button1) ? GREEN : DARKGREEN);
                DrawTextEx(bradley, "Start", Vector2{button1.x+20.0f, button1.y+20.0f}, 128, 128.0f/10.0f, WHITE);

                DrawRectangleRounded(Rectangle{button2.x-1,button2.y+10,button2.width+2,button2.height},0.1f,10,Color{30,30,30,255});
                DrawRectangleRounded(button2,0.1f,10,CheckCollisionPointRec(GetMousePosition(), button2) ? GRAY : DARKGRAY);
                DrawTextEx(bradley, "Settings", Vector2{button2.x+20.0f, button2.y+20.0f}, 64, 64.0f/10.0f, WHITE);

                DrawTextEx(bradley, "Artwork - Salad", Vector2{100,SCREEN_HEIGHT-64}, 32, 32.0f/10.0f, WHITE);
                DrawTextEx(bradley, "Audio,Coding - rsninjaDev", Vector2{100,SCREEN_HEIGHT-32}, 32, 32.0f/10.0f, WHITE);

                EndDrawing();
                break;}
            case Intro:{
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    comicState++;
                    PlaySound(walk1);
                }
                if(comicState == 4) {
                    state = Game;

                }
                BeginDrawing();

                ClearBackground(BLACK);

                float factor = ((SCREEN_HEIGHT-10)/2)/ 614 / 1.2;

                if(comicState >= 0) {
                    DrawTextureEx(comic1,Vector2{SCREEN_WIDTH/2 - 970*factor,SCREEN_HEIGHT/2 - 614*factor},0.0f,factor, WHITE);
                }
                if (comicState >= 1) {
                    DrawTextureEx(comic2,Vector2{SCREEN_WIDTH/2 +10,SCREEN_HEIGHT/2 - 614*factor},0.0f,factor, WHITE);
                }
                if(comicState >= 2) {
                    DrawTextureEx(comic3,Vector2{SCREEN_WIDTH/2 - 970*factor,SCREEN_HEIGHT/2 +10},0.0f,factor, WHITE);
                }
                if(comicState >= 3) {
                    DrawTextureEx(comic4,Vector2{SCREEN_WIDTH/2 + 10,SCREEN_HEIGHT/2 +10},0.0f,factor, WHITE);
                }

                DrawTextEx(bradley, "click to continue...", Vector2{SCREEN_WIDTH-250, SCREEN_HEIGHT-32}, 32, 32.0f/10.0f, WHITE);

                EndDrawing();

                break;}
            case Game:{

                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), Rectangle{10,10,30,50})) {
                    state = Pause;
                    lastState = Game;
                    PlaySound(walk1);
                }

                if(player.col.y < 5110 && IsSoundPlaying(forest) && songTransition <= 0) {
                    songTransition = 1.0;
                    PlaySound(sky);
                }

                if(player.col.y > 5110 && IsSoundPlaying(sky) && songTransition <= 0) {
                    songTransition = 1.0;
                    PlaySound(forest);
                }

                if(player.col.y < 5110) {
                    if(songTransition > 0) {
                        songTransition -= 0.01;
                        if(songTransition <= 0) {
                            StopSound(forest);
                        }

                        SetSoundVolume(forest, songTransition*BGM);
                        SetSoundVolume(sky, (1.0f-songTransition)*BGM);
                    }

                    if (!IsSoundPlaying(sky)) {
                        PlaySound(sky);
                    }

                } else {
                    if(songTransition > 0) {
                        songTransition -= 0.01;
                        if(songTransition <= 0) {
                            StopSound(sky);
                        }

                        SetSoundVolume(sky, songTransition*BGM);
                        SetSoundVolume(forest, (1.0f-songTransition)*BGM);
                    }

                    if (!IsSoundPlaying(forest)) {
                        PlaySound(forest);
                    }

                }

                if(firstFrameOfState) {
                    runTime = GetTime();

                    camera.target = (Vector2){ 0, 0 };
                    camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
                    camera.rotation = 0.0f;
                    camera.zoom = 1.0f;

                    player.col.x = 26;
                    player.col.y = 7100;
                    player.vx = 0;
                    player.vy = 0;
                    player.col.width = 100;
                    player.col.height = 100;
                    player.onGround = false;
                    player.onLeftWall = false;
                    player.onRightWall = false;
                    player.holdingJump = false;
                    player.holdTime = 0;
                    player.animState = Stand;
                    player.animFrame = 0;

                    keys[0] = 65;
                    keys[1] = 68;
                    keys[2] = 87;
                    nextKey = 0;
                    keyTime = GetTime();

                    lastWallJump = 0;

//                    cur = 0;

                    animeCount = 0;

                    canJumpAgain = true;

                    firstFrameOfState = false;

                }
                if(GetTime() - keyTime >= 10) {
                    keyTime = GetTime();
                    PlaySound(change);
                    randomizeKey(keys,nextKey);
                    if(++nextKey > 2) {
                        nextKey = 0;
                    }
                }
                for(int i=0;i<5;i++) {
                    shouldCloudReplenish[i] = true;
                }

                if(++platformTime >= PLATFORM_TIME) {
                    platformTime = 0;
                    for(int i=0;i<6;i++) {
                        toggleStates[i] = !toggleStates[i];
                    }
                }

//                if (IsKeyDown(KEY_ONE)) {
//                    player.col.x = 0;
//                    player.col.y = 0;
//                }
//
//                int change = GetMouseWheelMove() * 10;
//
//                if (IsKeyDown(KEY_LEFT_SHIFT)) {
//                    blocks[cur].width += (float)change;
//                } else {
//                    blocks[cur].height += (float)change;
//                }

//                blocks[cur].x = (float)GetMouseX() + (camera.target.x - SCREEN_WIDTH/2);
//                blocks[cur].y = (float)GetMouseY() + (camera.target.y - SCREEN_HEIGHT/2);

//                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
//                    printf("{%d,%d,%d,%d},",(int)blocks[cur].x,(int)blocks[cur].y,(int)blocks[cur].width,(int)blocks[cur].height);
//
//                    cur++;
//                }

//                if (IsKeyDown(KEY_RIGHT)) {
//                    player.col.x += 15;
//                }
//                if (IsKeyDown(KEY_LEFT)) {
//                    player.col.x -= 15;
//                }
//                if (IsKeyDown(KEY_UP)) {
//                    player.col.y -= 15;
//                    player.vy = 0;
//                }
//                if (IsKeyDown(KEY_DOWN)) {
//                    player.col.y += 15;
//                }

                // left
                if (IsKeyDown(keys[0]) && (lastWallJump != 2 || player.onRightWall)) {
                    if(player.vx > -MAX_VX) {
                        player.vx -= ACCEL;
                    }
                }

                // right
                if (IsKeyDown(keys[1]) && (lastWallJump != 1 || player.onLeftWall)) {
                    if(player.vx < MAX_VX) {
                        player.vx += ACCEL;
                    }
                }

                // jump
                if (IsKeyReleased(keys[2])) {
                    canJumpAgain = true;
                }
                if (IsKeyDown(keys[2])) {
                    if(player.holdingJump) {
                        if(--player.holdTime <= 0) {
                            player.holdingJump = false;
                        }
                    } else if (player.onGround && canJumpAgain) {
                        player.vy = -JUMP;
                        player.holdingJump = true;
                        player.holdTime = JUMP_FLOAT_TIME;
                        canJumpAgain = false;
                        framesSinceGround = 5;
                        int rand = random(0,2);
                        PlaySound(rand == 0 ? jump1 : (rand == 1 ? jump2 : jump3));
                    } else if(player.onRightWall && canJumpAgain) {
                        if (lastWallJump != 1) {
                            player.vy = -JUMP;
                            player.vx = -JUMP / 3;
                            player.holdingJump = true;
                            player.holdTime = JUMP_FLOAT_TIME;
                            lastWallJump = 1;
                            canJumpAgain = false;
                            player.animState = Jump;
                            int rand = random(0,2);
                            PlaySound(rand == 0 ? jump1 : (rand == 1 ? jump2 : jump3));
                        }
                    } else if (player.onLeftWall && canJumpAgain) {
                        if (lastWallJump != 2) {
                            player.vy = -JUMP;
                            player.vx = JUMP / 3;
                            player.holdingJump = true;
                            player.holdTime = JUMP_FLOAT_TIME;
                            lastWallJump = 2;
                            canJumpAgain = false;
                            player.animState = Jump;
                            int rand = random(0,2);
                            PlaySound(rand == 0 ? jump1 : (rand == 1 ? jump2 : jump3));
                        }
                    }
                } else {
                    player.holdingJump = false;
                }

                // gravity
                if(!player.onGround) {
                    player.vy += (GRAVITY * (player.holdingJump ? 0.5f : 1.0f));
                }

                // wall slide
                if((player.onLeftWall || player.onRightWall) && !player.onGround) {
                    player.vy = std::min(player.vy, 1.5f);
                }

                // friction
                if(!IsKeyDown(keys[0]) && !IsKeyDown(keys[1])) {
                    float fric = FRICTION / (player.onGround ? 1.0f : 2.0f);
                    if(player.vx > 0) { player.vx -= fric;}
                    if(player.vx < 0) { player.vx += fric;}
                    if(std::abs(player.vx) < fric * 2) { player.vx = 0;}
                }

                player.col.x += player.vx;

                Rectangle col = colliding(player.col);

                if(col.width != 0 && col.height != 0) {
                    player.col.x -= player.vx;

                    if(player.vx > 0) {
                        player.col.x = col.x - player.col.width - 0.1f;
                    } else {
                        player.col.x = col.x + col.width + 0.1f;
                    }

                    player.vx = 0;
                }

                player.col.y += player.vy;

                col = colliding(player.col);

                if(col.width != 0 && col.height != 0) {
                    player.col.y -= player.vy;

                    if(player.vy > 0) {
                        player.col.y = col.y - player.col.height - 0.1f;
                    } else {
                        player.col.y = col.y + col.height + 0.1f;
                    }

                    player.vy = 0;
                }

                Rectangle leftWallCheck = {player.col.x - 4.5f, player.col.y + 5.0f, 3.0f, player.col.height-10.0f};
                col = colliding(leftWallCheck);
                if(!player.onLeftWall && (col.width != 0 && col.height != 0) && !player.onGround) {
                    PlaySound(slide);
                }
                player.onLeftWall = (col.width != 0 && col.height != 0);

                Rectangle rightWallCheck = {player.col.x + player.col.width + 1.5f, player.col.y + 5.0f, 3.0f, player.col.height-10.0f};
                col = colliding(rightWallCheck);
                if(!player.onRightWall && (col.width != 0 && col.height != 0) && !player.onGround) {
                    PlaySound(slide);
                }
                player.onRightWall = (col.width != 0 && col.height != 0);

                Rectangle groundCheck = {player.col.x + 5.0f, player.col.y + player.col.height + 1.5f, player.col.width-10.0f, 3.0f};
                col = colliding(groundCheck);
                if (col.width != 0 && col.height != 0) {
                    lastWallJump = 0;
                    if(!player.onGround) {
                        framesSinceGround = 5;
                        int rand = random(0,1);
                        PlaySound(rand == 1 ? land1 : land2);
                    }
                    player.onGround = true;
                } else {
                    player.onGround = false;
                }

                for(int i=0;i<5;i++) {
                    if(shouldCloudReplenish[i] && cloudTime[i] < CLOUD_TIME) {
                        cloudTime[i]++;
                    }
                }

                if((player.vx == 0 && player.onGround) || (!player.onGround && (player.holdTime <= 0) || !IsKeyDown(keys[2]))) {
                    player.animState = Stand;
                }

                if((player.vx > 0 || player.vx < 0) && player.onGround && !(player.onRightWall || player.onLeftWall)) {
                    player.animState = Run;
                }

                if(player.onLeftWall && player.vy > 0) {
                    player.animState = Slide;
                    direction = 1;
                }
                if(player.onRightWall && player.vy > 0) {
                    player.animState = Slide;
                    direction = -1;
                }

                if(framesSinceGround > 0) {
                    framesSinceGround--;
                    player.animState = Squish;
                    if(framesSinceGround == 0 && !player.onGround) {
                        player.animState = Jump;
                    }
                }

                camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };

                if(SCREEN_WIDTH > 1920) {
                    camera.zoom = 2.0f;
                } else if(SCREEN_WIDTH > 3840) {
                    camera.zoom = 3.0f;
                } else {
                    camera.zoom = 1.0f;
                }

                camera.target.x = std::max(std::min(player.col.x + player.col.width/(2.0f * camera.zoom),1920-SCREEN_WIDTH/(2.0f * camera.zoom)), SCREEN_WIDTH/(2.0f * camera.zoom));
                camera.target.y = std::max(std::min(player.col.y + player.col.height/(2.0f * camera.zoom),7210-SCREEN_HEIGHT/(2.0f * camera.zoom)), SCREEN_HEIGHT/(2.0f * camera.zoom) + 2250);


//        camera.zoom = 0.5f;

                BeginDrawing();

                ClearBackground(RAYWHITE);

                BeginMode2D(camera);

                DrawTexture(mapBack, 0, 2250, WHITE);

                for(int i=0;i<mapSize;i++) {
                    int isCloud = 0;
                    int isPlatform = 0;
                    for(int j=0;j<5;j++) {
                        if(clouds[j] == i) {
                            isCloud = j+1;
                        }
                    }
                    for(int j=0;j<6;j++) {
                        if(togglePlatforms[j] == i) {
                            if (toggleStates[j]) {
                                isPlatform = 1;
                            } else {
                                isPlatform = 2;
                            }
                        }
                    }

                    if(isPlatform) {
//                        DrawRectangleRec(map[i], (isPlatform == 1 ? BLUE : RED));
                        DrawTexture((isPlatform == 1 ? (i%2==0 ? platform1 : platform2) : (i%2==0 ? platform1gone : platform2gone)), map[i].x-20,map[i].y-20,WHITE);
                    } else if(isCloud) {
                        DrawTexture(i%2 == 0 ? cloud1 : cloud2,map[i].x-20,map[i].y-20,Color{255,255,255,static_cast<unsigned char>(cloudTime[isCloud-1]*4)});
//                        DrawRectangleRec(map[i], Color{125,125,125,static_cast<unsigned char>(cloudTime[isCloud-1]*4)});
                    } else {
//                        DrawRectangleRec(map[i], DARKGREEN);
                    }

//                    const char* num = std::to_string(i).c_str();
//                    DrawText(num,(int)map[i].x,(int)map[i].y,50,BLACK);
                }



//                for(int i=0;i<cur+1;i++) {
//                    DrawRectangleRec(blocks[i], DARKGREEN);
//                }

//                DrawRectangleRec(blocks[cur], GREEN);

                Texture2D playerFrame;
                animeCount++;

                switch (player.animState) {
                    case Run:
                        if(animeCount % 7 == 0) {
                            int rand = random(0,2);
                            PlaySound(rand == 0 ? walk1 : (rand == 1 ? walk2 : walk3));
                            if (++player.animFrame > 3) {
                                player.animFrame = 0;
                            }
                        }

                        switch (player.animFrame) {
                            case 0: playerFrame = bushWalk1; break;
                            case 1:case 3: playerFrame = bushWalk2; break;
                            case 2: playerFrame = bushWalk3; break;
                        }
                        break;
                    case Stand:
                        playerFrame = bushStand;
                        break;
                    case Slide:
                        playerFrame = bushWall;
                        break;
                    case Jump:
                        playerFrame = bushJump;
                        break;
                    case Fall:
                        playerFrame = bushJump;
                        break;
                    case Squish:
                        playerFrame = bushSquish;
                        break;
                }


//                DrawRectangleRec(player.col, Color{0,127,255,100});
                if(player.animState == Slide) {
                    DrawTextureRec(playerFrame,Rectangle{0,0,(direction == 1 ? -130.0f : 130.0f),144},Vector2 {player.col.x-15.0f, player.col.y-35.0f}, WHITE);
                } else {
                    DrawTextureRec(playerFrame,Rectangle{0,0,((player.animState != Stand && player.vx < 0)  ? -130.0f : 130.0f),144},Vector2 {player.col.x-15.0f, player.col.y-35.0f}, WHITE);
                }
//                DrawRectangleRec(groundCheck, RED);
//                DrawRectangleRec(leftWallCheck, RED);
//                DrawRectangleRec(rightWallCheck, RED);

                DrawTexture(mapFront, 0, 2250, WHITE);


                Texture2D left;
                switch (keys[0]-65) {
                    case 0: left=Ka; break;
                    case 1: left=Kb; break;
                    case 2: left=Kc; break;
                    case 3: left=Kd; break;
                    case 4: left=Ke; break;
                    case 5: left=Kf; break;
                    case 6: left=Kg; break;
                    case 7: left=Kh; break;
                    case 8: left=Ki; break;
                    case 9: left=Kj; break;
                    case 10: left=Kk; break;
                    case 11: left=Kl; break;
                    case 12: left=Km; break;
                    case 13: left=Kn; break;
                    case 14: left=Ko; break;
                    case 15: left=Kp; break;
                    case 16: left=Kq; break;
                    case 17: left=Kr; break;
                    case 18: left=Ks; break;
                    case 19: left=Kt; break;
                    case 20: left=Ku; break;
                    case 21: left=Kv; break;
                    case 22: left=Kw; break;
                    case 23: left=Kx; break;
                    case 24: left=Ky; break;
                    case 25: left=Kz; break;
                }

                DrawTextureEx(left, Vector2 {player.col.x - 120+ (IsKeyDown(keys[0]) ? 10.0f : 0), player.col.y+ (IsKeyDown(keys[0]) ? 10.0f : 0)}, 0.0f, IsKeyDown(keys[0]) ? 0.8f :1.0f, (nextKey==0 && GetTime()-keyTime > 9.25f) ? Color{255,0,255,180} : Color{255,255,255,180});

                Texture2D right;
                switch (keys[1]-65) {
                    case 0: right=Ka; break;
                    case 1: right=Kb; break;
                    case 2: right=Kc; break;
                    case 3: right=Kd; break;
                    case 4: right=Ke; break;
                    case 5: right=Kf; break;
                    case 6: right=Kg; break;
                    case 7: right=Kh; break;
                    case 8: right=Ki; break;
                    case 9: right=Kj; break;
                    case 10: right=Kk; break;
                    case 11: right=Kl; break;
                    case 12: right=Km; break;
                    case 13: right=Kn; break;
                    case 14: right=Ko; break;
                    case 15: right=Kp; break;
                    case 16: right=Kq; break;
                    case 17: right=Kr; break;
                    case 18: right=Ks; break;
                    case 19: right=Kt; break;
                    case 20: right=Ku; break;
                    case 21: right=Kv; break;
                    case 22: right=Kw; break;
                    case 23: right=Kx; break;
                    case 24: right=Ky; break;
                    case 25: right=Kz; break;
                }

                DrawTextureEx(right, Vector2 {player.col.x + 120 + (IsKeyDown(keys[1]) ? 10.0f : 0), player.col.y+ (IsKeyDown(keys[1]) ? 10.0f : 0)}, 0.0f, IsKeyDown(keys[1]) ? 0.8f :1.0f, (nextKey==1 && GetTime()-keyTime > 9.25f) ? Color{255,0,255,180} : Color{255,255,255,180});

                Texture2D up;
                switch (keys[2]-65) {
                    case 0: up=Ka; break;
                    case 1: up=Kb; break;
                    case 2: up=Kc; break;
                    case 3: up=Kd; break;
                    case 4: up=Ke; break;
                    case 5: up=Kf; break;
                    case 6: up=Kg; break;
                    case 7: up=Kh; break;
                    case 8: up=Ki; break;
                    case 9: up=Kj; break;
                    case 10: up=Kk; break;
                    case 11: up=Kl; break;
                    case 12: up=Km; break;
                    case 13: up=Kn; break;
                    case 14: up=Ko; break;
                    case 15: up=Kp; break;
                    case 16: up=Kq; break;
                    case 17: up=Kr; break;
                    case 18: up=Ks; break;
                    case 19: up=Kt; break;
                    case 20: up=Ku; break;
                    case 21: up=Kv; break;
                    case 22: up=Kw; break;
                    case 23: up=Kx; break;
                    case 24: up=Ky; break;
                    case 25: up=Kz; break;
                }

                DrawTextureEx(up, Vector2 {player.col.x+ (IsKeyDown(keys[2]) ? 10.0f : 0), player.col.y - 120+ (IsKeyDown(keys[2]) ? 10.0f : 0)}, 0.0f, IsKeyDown(keys[2]) ? 0.8f :1.0f, (nextKey==2 && GetTime()-keyTime > 9.25f) ? Color{255,0,255,180} : Color{255,255,255,180});

                EndMode2D();

                DrawRectangle(10,10,10,50,Color{125,125,125,125});
                DrawRectangle(25,10,10,50,Color{125,125,125,125});

                DrawRectangle(0,SCREEN_HEIGHT-15,(SCREEN_WIDTH) * ((GetTime()-keyTime) - 0) / 10,15,Color{255,0,255,180});

//                const char* num2 = (std::to_string(player.col.x) + " " + std::to_string(player.col.y)).c_str();
//                DrawText(num2,100,100,50,BLACK);

                EndDrawing();

                if(player.col.x > 1730 && player.col.y < 2550) {
                    state = Win;
                    runTime = GetTime() - runTime;
                }

                break;}
            case Pause:{
                Rectangle button3 = {20,350,150,100};

                if(CheckCollisionPointRec(GetMousePosition(),button3) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    state = lastState;
                    PlaySound(walk1);
                }

                if(CheckCollisionPointRec(GetMousePosition(),{200,100,100,10}) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    SFX = std::max(std::min((GetMouseX()-200.0f)/120.0f,1.0f),0.0f);

                    SetSoundVolume(jump1,SFX);
                    SetSoundVolume(jump2,SFX);
                    SetSoundVolume(jump3,SFX);
                    SetSoundVolume(land1,SFX);
                    SetSoundVolume(land2,SFX);
                    SetSoundVolume(slide,SFX);
                    SetSoundVolume(walk1,SFX);
                    SetSoundVolume(walk2,SFX);
                    SetSoundVolume(walk3,SFX);
                    SetSoundVolume(change,SFX);
                }
                if(CheckCollisionPointRec(GetMousePosition(),{200,150,100,10}) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    BGM = std::max(std::min((GetMouseX()-200.0f)/120.0f,1.0f),0.0f);

                    SetSoundVolume(forest,BGM);
                    SetSoundVolume(sky,BGM);
                }


                BeginDrawing();
                ClearBackground(DARKGRAY);
                DrawTextEx(bradley, "SFX", Vector2{10,80}, 64, 64.0f/10.0f, WHITE);
                DrawTextEx(bradley, "0", Vector2{170,80}, 64, 64.0f/10.0f, WHITE);
                DrawTextEx(bradley, "1", Vector2{310,80}, 64, 64.0f/10.0f, WHITE);
                DrawRectangle(200,100,100,10,GRAY);
                DrawRectangle(200 + (SFX*100.0f),90,10,35,BLUE);
                DrawTextEx(bradley, "BGM", Vector2{10,130}, 64, 64.0f/10.0f, WHITE);
                DrawTextEx(bradley, "0", Vector2{170,130}, 64, 64.0f/10.0f, WHITE);
                DrawTextEx(bradley, "1", Vector2{310,130}, 64, 64.0f/10.0f, WHITE);
                DrawRectangle(200,150,100,10,GRAY);
                DrawRectangle(200 + (BGM*100.0f),140,10,35,BLUE);

                DrawRectangleRounded(Rectangle{button3.x-1,button3.y+10,button3.width+2,button3.height},0.1f,10,Color{30,30,30,255});
                DrawRectangleRounded(button3,0.1f,10,CheckCollisionPointRec(GetMousePosition(), button3) ? GRAY : DARKGRAY);
                DrawTextEx(bradley, "Back", Vector2{button3.x+20.0f, button3.y+20.0f}, 64, 64.0f/10.0f, WHITE);
                EndDrawing();
                break;}
            case Win:{

                BeginDrawing();

                ClearBackground(BLACK);

                DrawTextEx(bradley, "Congratulations", Vector2{SCREEN_WIDTH/2-230, SCREEN_HEIGHT/2-250}, 64, 64.0f/10.0f, YELLOW);

                DrawTextureEx(comic3, Vector2{SCREEN_WIDTH/2 - 970/3, SCREEN_HEIGHT/2 - 614/3},0.0f,0.666f,WHITE);

                std::string msg = "You got back in " + std::to_string((int)runTime) + " seconds";

                DrawTextEx(bradley, msg.c_str(), Vector2{SCREEN_WIDTH/2-380, SCREEN_HEIGHT/2+200}, 64, 64.0f/10.0f, YELLOW);

                EndDrawing();
                break;}
        }
}

int main() {
    float SCREEN_WIDTH  = 1280;
    float SCREEN_HEIGHT = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cabush's Kooky Climb");


    loading = LoadTexture("./assets/Loading.png");

    BeginDrawing();
    ClearBackground(WHITE);
    DrawTexture(loading,SCREEN_WIDTH/2-75,SCREEN_HEIGHT/2-16,WHITE);
    EndDrawing();

    bushStand = LoadTexture("./assets/bushStand.png");
    bushWalk1 = LoadTexture("./assets/bushWalk1.png");
    bushWalk2 = LoadTexture("./assets/bushWalk2.png");
    bushWalk3 = LoadTexture("./assets/bushWalk3.png");
    bushJump = LoadTexture("./assets/bushJump.png");
    bushSquish = LoadTexture("./assets/bushSquish.png");
    bushWall = LoadTexture("./assets/bushWall.png");
    comic1 = LoadTexture("./assets/comic1.png");
    comic2 = LoadTexture("./assets/comic2.png");
    comic3 = LoadTexture("./assets/comic3.png");
    comic4 = LoadTexture("./assets/comic4.png");
    Ka = LoadTexture("./assets/a.png");
    Kb = LoadTexture("./assets/b.png");
    Kc = LoadTexture("./assets/c.png");
    Kd = LoadTexture("./assets/d.png");
    Ke = LoadTexture("./assets/e.png");
    Kf = LoadTexture("./assets/f.png");
    Kg = LoadTexture("./assets/g.png");
    Kh = LoadTexture("./assets/h.png");
    Ki = LoadTexture("./assets/i.png");
    Kj = LoadTexture("./assets/j.png");
    Kk = LoadTexture("./assets/k.png");
    Kl = LoadTexture("./assets/l.png");
    Km = LoadTexture("./assets/m.png");
    Kn = LoadTexture("./assets/n.png");
    Ko = LoadTexture("./assets/o.png");
    Kp = LoadTexture("./assets/p.png");
    Kq = LoadTexture("./assets/q.png");
    Kr = LoadTexture("./assets/r.png");
    Ks = LoadTexture("./assets/s.png");
    Kt = LoadTexture("./assets/t.png");
    Ku = LoadTexture("./assets/u.png");
    Kv = LoadTexture("./assets/v.png");
    Kw = LoadTexture("./assets/w.png");
    Kx = LoadTexture("./assets/x.png");
    Ky = LoadTexture("./assets/y.png");
    Kz = LoadTexture("./assets/z.png");
    mapBack = LoadTexture("./assets/mapBack.png");
    mapFront = LoadTexture("./assets/mapFront.png");
    cloud1 = LoadTexture("./assets/cloud1.png");
    cloud2 = LoadTexture("./assets/cloud2.png");
    platform1 = LoadTexture("./assets/platform1.png");
    platform2 = LoadTexture("./assets/platform2.png");
    platform1gone = LoadTexture("./assets/platform1gone.png");
    platform2gone = LoadTexture("./assets/platform2gone.png");

    bradley = LoadFontEx("./assets/BRADHITC.TTF",128,nullptr,96);

    SetWindowMinSize(800,600);
    SetTargetFPS(60);

    
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    
    player.col.x = 26;
    player.col.y = 7100;
    player.vx = 0;
    player.vy = 0;
    player.col.width = 100;
    player.col.height = 100;
    player.onGround = false;
    player.onLeftWall = false;
    player.onRightWall = false;
    player.holdingJump = false;
    player.holdTime = 0;
    player.animState = Stand;
    player.animFrame = 0;

    keys[3] = {65, 68, 87};
    nextKey = 0;
    keyTime = 0;

    lastWallJump = 0;

    animeCount = 0;

    canJumpAgain = true;

    runTime = 0;

    framesSinceGround = 0;

    direction = 0;

    state = Main;

    lastState = Main;

    firstFrameOfState;

    comicState = 0;

    songTransition = 0;

    SFX = 0.5;
    BGM = 0.5;

    inited = false;

    button1 = {0,0,320,180};
    button2 = {20,20,275,100};

    emscripten_set_main_loop(mainGameLoop, 60, true);

    // while (!WindowShouldClose())
    // {
        


    // }

    // UnloadTexture(bushStand);
    // UnloadTexture(bushJump);
    // UnloadTexture(bushWall);
    // UnloadTexture(bushSquish);
    // UnloadTexture(bushWalk1);
    // UnloadTexture(bushWalk2);
    // UnloadTexture(bushWalk3);

    // UnloadSound(jump1);
    // UnloadSound(jump2);
    // UnloadSound(jump3);
    // UnloadSound(walk1);
    // UnloadSound(walk2);
    // UnloadSound(walk3);
    // UnloadSound(land1);
    // UnloadSound(land2);
    // UnloadSound(slide);
    // UnloadSound(change);

    // UnloadSound(forest);
    // UnloadSound(sky);

    // CloseAudioDevice();

    // CloseWindow();
    return 0;
}

