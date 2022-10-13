#include <stdio.h>

// groupcast의 Itt를 가져온다. 
// TODO: ITT값을 기존의 코드에서 어떤것으로 넣어야 할지 결정해야 한다. 
float mod_ITT (float ITT) {

    // 내부에서 정의되어야 하는 함수는 CBR, VD, X의 값이다.
    float CBR;
    int VD = vechicle_density;
    float X;  // X = CBR x VD
    float a;  // a는 변경된 ITT의 값(최종 출력할 숫자)

    // CBR = 메시지를 전부 보내는 시간 / 1초(설정된 관측시간)
    // TODO: 메시지 보내는 시간 어떤것을 받아올 것인지?, 관측시간은 1초로 설정하면 괜찮은지 확인 필요
    CBR = ITT / 1;

    // // 한 그룹내의 차량 대수 or 총 차량 대수(10~30)
    // // TODO: 차량 대수를 결정하는 함수를 어떤것을 받아올 것인가?
    // VD = (MAX_NODE - 1);  //  RSU node 1대 제외

    X = CBR * VD;

    int congestion_level;
    if (X <= 6.5) {
        congestion_level = 1;
        printf("congestion_level: %d", congestion_level);
        return a = 0.1;
    } else if (6.5 < X <= 14) {
        congestion_level = 2;
        printf("congestion_level: %d", congestion_level);
        return a = 0.334;
    } else if (14 < X <= 25.5) {
        congestion_level = 3;
        printf("congestion_level: %d", congestion_level);
        return a = 0.5;
    } else {
        congestion_level = 4;
        printf("congestion_level: %d", congestion_level);
        return a = 1;
    };

    return a;
}